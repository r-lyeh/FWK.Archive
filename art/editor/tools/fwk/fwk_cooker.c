// data pipeline
// - rlyeh, public domain.
// ----------------------------------------------------------------------------
// @todo: threads should steal workloads from job queue
// @todo: restore errno/errorlevel checks
// @todo: +=, -=, that_asset.ini
// @todo: @dae FLAGS+=-U
// @todo: SF2_SOUNDBANK=TOOLS/soundbank.sf2
// @fixme: leaks (worth?)
// -----------------------------------------------------------------------------

const char *ART = "art/";
const char *TOOLS = ifdef(win32, "art/editor/tools/bin/", "art/editor/tools//bin/");
const char *EDITOR = ifdef(win32, "art/editor/", "art/editor//");
const char *FWK_INI = "fwk.ini";

typedef struct cook_script_t {
    char *infile;     // free after use
    char *finalfile;  // free after use. can be either infile or a totally different file
    char *script;
    int compress_level;
} cook_script_t;

static
cook_script_t cook_script(const char *rules, const char *infile, const char *outfile) {
    // by default, assume:
    // - no script is going to be generated (empty script)
    // - if no script is going to be generated, output is in fact input file.
    // - no compression is going to be required.
    cook_script_t cs = { 0 };

    // reuse script heap from last call if possible (optimization)
    static __thread char *script = 0;
    if(script) script[0] = 0;

    // reuse parsing maps if possible (optimization)
    static __thread map(char*, char*) symbols = 0;
    static __thread map(char*, char*) groups = 0;

    if(!symbols) map_init(symbols, less_str, hash_str);
    if(!groups)  map_init(groups, less_str, hash_str);

    // pretty (truncated) input (C:/prj/FWK/art/file.wav -> file.wav)
    int artlen = strlen(ART);
    const char *pretty = infile;
    if( !strncmp(pretty, ART, artlen) ) pretty += artlen;
    while(pretty[0] == '/') ++pretty;

    map_find_or_add(symbols, "INFILE", STRDUP(infile));
    map_find_or_add(symbols, "INPUT", STRDUP(infile));
    map_find_or_add(symbols, "PRETTY", STRDUP(pretty));
    map_find_or_add(symbols, "OUTPUT", STRDUP(outfile));
    map_find_or_add(symbols, "TOOLS", STRDUP(TOOLS));
    map_find_or_add(symbols, "EDITOR", STRDUP(EDITOR));
    map_find_or_add(symbols, "PROGRESS", STRDUP(va("%03d", cooker_progress())));

    // start parsing. parsing is enabled by default
    int enabled = 1;
    array(char*)lines = strsplit(rules, "\r\n");
    for( int i = 0, end = array_count(lines); i < end; ++i ) {
        // skip blanks
        int blanks = strspn(lines[i], " \t");
        char *line = lines[i] + blanks;

        // discard full comments
        if( line[0] == ';' ) continue;
        // truncate inline comments
        if( strstr(line, ";") ) *strstr(line, ";") = 0;
        // trim ending spaces
        char *eos = line + strlen(line); while(eos > line && eos[-1] == ' ' ) *--eos = 0;
        // discard non-specific lines
        if( line[0] == '@' ) {
            int with_wine = flag("--with-wine") && !!strstr(line, "@win");
            int parse = 0
                | ifdef(win32, (!!strstr(line, "@win")), 0)
                | ifdef(linux, (!!strstr(line, "@lin") ? 1 : with_wine), 0)
                | ifdef(osx,   (!!strstr(line, "@osx") ? 1 : with_wine), 0);

            if( !parse ) continue;

            line = strchr(line+1, ' ');
            if(!line) continue;
            line += strspn(line, " \t");
        }
        // execute `shell` commands
        if( line[0] == '`' ) {
            char *eos = strrchr(++line, '`');
            if( eos ) *eos = 0;

                // replace all symbols
                char* nl = STRDUP(line);
                for each_map(symbols, char*, key, char*, val) {
                    strrepl(&nl, key, val);
                }
                lines[i] = line = nl; // @fixme:leak

            static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
            thread_mutex_lock( &lock );
                system(line); // strcatf(&script, "%s\n", line);
            thread_mutex_unlock( &lock );

            continue;
        }
        // process [sections]
        if( line[0] == '[' ) {
            enabled = 1;
            int is_cook = !!strstr(line, "[cook]");
            int is_compress = !!strstr(line, "[compress]");
            if( !is_cook && !is_compress ) {
                // remove hint cook tag if present. that's informative only.
                if(strbegi(line, "[cook ") ) memcpy(line+1, "    ", 4);

                // start parsing expressions like `[media && !avi && mp3]`
                array(char*) tags = strsplit(line, " []&");

                // let's check whether INPUT belongs to tags above
                char **INPUT = map_find(symbols, "INPUT");
                bool found_in_set = true;

                for( int i = 0, end = array_count(tags); i < end; ++i) { char *tag = tags[i];
                    bool negate = false;
                    while(*tag == '!') negate ^= 1, ++tag;

                    // find tag in groups map
                    // either a group or an extension
                    char **is_group = map_find(groups, tag);
                    if( is_group ) {
                        char *list = *is_group;
                        char *INPUT_EXT = file_ext(infile); INPUT_EXT = strrchr(INPUT_EXT, '.'); // .ext1.ext -> .ext
                        char *ext = INPUT_EXT; ext += ext[0] == '.'; // dotless
                        bool in_list = strbegi(list, ext) || strstri(list, va(",%s,",ext)) || strendi(list, va(",%s",ext));
                        if( !in_list ^ negate ) { found_in_set = false; break; }
                    } else {
                        char *ext = va(".%s", tag);
                        bool found = !!strendi(*INPUT, ext);
                        if( !found ^ negate ) { found_in_set = false; break; }
                    }
                }
                //
                enabled = found_in_set ? 1 : 0;
            }
        }
        // either SYMBOL=, group=, or regular script line
        if( enabled && line[0] != '[' ) {
            enum { group, symbol, regular } type = regular;
            int tokenlen = strspn(line, "-+_.|0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
            char *token = va("%.*s", tokenlen, line);
            char *equal = strchr(line, '=');
            if( equal ) {
                if( equal == &line[tokenlen] ) { // if key=value expression found
                    // discriminate: symbols are uppercase and never begin with digits. groups are [0-9]+[|][a-z].
                    type = strcmp(strupper(token), token) || isdigit(token[0]) ? group : symbol;
                }
            }
            if( type == group  ) map_find_or_add(groups, token, STRDUP(equal+1));
            if( type == symbol ) {
                // @todo: perform the replacement/union/intersection on set here
                bool is_add = strendi(token, "+");
                bool is_del = strendi(token, "-");

                // if present, remove last sign from token -> (FLAGS1+)=, (FLAGS1-)=
                if(is_add || is_del) token[strlen(token) - 1] = 0;

                map_find_or_add(symbols, token, STRDUP(equal+1));
            }
            // for each_map(symbols, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
            // for each_map(groups, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
            // if( type != regular ) printf("%s found >> %s\n", type == group ? "group" : "symbol", line);

            if( type == regular ) {
                char** INPUT = map_find(symbols, "INPUT");
                char** OUTPUT = map_find(symbols, "OUTPUT");

                // parse return code
                char *has_errorlevel = strstr(line, "=="); //==N form
                int errorlevel = has_errorlevel ? atoi(has_errorlevel + 2) : 0;
                if( has_errorlevel ) memcpy(has_errorlevel, "   ", 3);

                // detect if newer extension is present, and thus update OUTPUT if needed
                char *newer_extension = strstr(line, "->"); if(newer_extension) {
                    *newer_extension = 0;
                    newer_extension += 2 + strspn(newer_extension + 2, " ");

                    strcatf(&*OUTPUT, ".%s", newer_extension);
                }

                // replace all symbols
                char* nl = STRDUP(line);
                for each_map(symbols, char*, key, char*, val) {
                    strrepl(&nl, key, val);
                }
                lines[i] = line = nl; // @fixme:leak

                // convert slashes
                ifdef(win32,
                    strswap(line, "/", "\\")
                , // else
                    strswap(line, "\\", "/")
                );

                // append line
                strcatf(&script, "%s\n", line);

                // handle return code here
                // if(has_errorlevel)
                // strcatf(&script, "IF NOT '%%ERRORLEVEL%%'=='%d' echo ERROR!\n", errorlevel);

                // rename output->input for further chaining, in case it is needed
                if( newer_extension ) {
                    *INPUT[0] = 0;
                    strcatf(&*INPUT, "%s", *OUTPUT);
                }
            }
        }
    }

    // compression
    char* ext = file_ext(infile); ext = strrchr(ext, '.'); ext += ext[0] == '.'; // dotless INPUT_EXT

    char** OUTPUT = map_find(symbols, "OUTPUT");
    char* belongs_to = 0;
    for each_map(groups, char*, key, char*, val) {
        if( !isdigit(key[0]) ) {
            char *comma = va(",%s,", ext);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                belongs_to = key;
                //goto break1; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
        }
    }
    break1:;
    char *compression = 0;
    for each_map(groups, char*, key, char*, val) {
        if( isdigit(key[0]) ) {
            char *comma = va(",%s,", ext);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                compression = key;
                //goto break2; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
            comma = va(",%s,", belongs_to);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                compression = key;
                //goto break2; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
        }
    }
    break2:;

    cs.compress_level = 0;
    if( compression ) {
        // last chance to optionally override the compressor at command-line level
        static const char *user_compression, **init = 0;
        if( !init ) *(init = &user_compression) = option("--with-compressor", "");
        if( user_compression[0] ) compression = (char*)user_compression;

        /**/ if(strstr(compression, "PPP"))  cs.compress_level = atoi(compression) | PPP;
        else if(strstr(compression, "ULZ"))  cs.compress_level = atoi(compression) | ULZ;
        else if(strstr(compression, "LZ4"))  cs.compress_level = atoi(compression) | LZ4X;
        else if(strstr(compression, "CRSH")) cs.compress_level = atoi(compression) | CRSH;
        else if(strstr(compression, "DEFL")) cs.compress_level = atoi(compression) /*| DEFL*/;
        //else if(strstr(compression, "LZP"))  cs.compress_level = atoi(compression) | LZP1; // not supported
        else if(strstr(compression, "LZMA")) cs.compress_level = atoi(compression) | LZMA;
        else if(strstr(compression, "BALZ")) cs.compress_level = atoi(compression) | BALZ;
        else if(strstr(compression, "LZW"))  cs.compress_level = atoi(compression) | LZW3;
        else if(strstr(compression, "LZSS")) cs.compress_level = atoi(compression) | LZSS;
        else if(strstr(compression, "BCM"))  cs.compress_level = atoi(compression) | BCM;
        else                                 cs.compress_level = atoi(compression) /*| DEFL*/;
    }

    // if script was generated...
    if( script && script[0]) {
        // update outfile
        cs.finalfile = *OUTPUT;

        // amalgamate script
        array(char*) lines = strsplit(script, "\r\n");

        #if is(win32)
            char *joint = strjoin(lines, " && ");
            cs.script = joint;
        #else
            if( flag("--with-wine") ) {
                // dear linux/osx/bsd users:
                // tools going wrong for any reason? cant compile them maybe?
                // small hack to use win32 pipeline tools instead
                char *joint = strjoin(lines, " && wine " );
                cs.script = va("wine %s", /*TOOLS,*/ joint);
            } else {
                char *joint = strjoin(lines, " && " );
                cs.script = va("export LD_LIBRARY_PATH=%s && %s", TOOLS, joint);
            }
        #endif
    } else {
        // ... else bypass infile->outfile
        char** INFILE = map_find(symbols, "INFILE");
        cs.finalfile = *INFILE;

        // and return an empty script
        cs.script = "";
    }

    map_clear(symbols);
    map_clear(groups);
    return cs;
}

// ----------------------------------------------------------------------------

struct fs {
    char *fname, status;
    uint64_t stamp;
    uint64_t bytes;
};

static array(struct fs) fs_now;
static threadlocal array(char*) added;
static threadlocal array(char*) changed;
static threadlocal array(char*) deleted;
static threadlocal array(char*) uncooked;

static
array(struct fs) zipscan_filter(int threadid, int numthreads) {
    // iterate all previously scanned files
    array(struct fs) fs = 0;
    for( int i = 0, end = array_count(fs_now); i < end; ++i ) {
        // during workload distribution, we assign random files to specific thread buckets.
        // we achieve this by hashing the basename of the file. we used to hash also the path 
        // long time ago but that is less resilient to file relocations across the repository. 
        // excluding the file extension from the hash also helps from external file conversions.
        char *fname = file_name(fs_now[i].fname);
        char *dot = strrchr(fname, '.'); if(dot) *dot = '\0';

        // skip if list item does not belong to this thread bucket
        uint64_t hash = hash_str(fname);
        unsigned bucket = (hash % numthreads);
        if(bucket != threadid) continue;

        array_push(fs, fs_now[i]);
    }
    return fs;
}

static
int zipscan_diff( zip* old, array(struct fs) now ) {
    array_free(added);
    array_free(changed);
    array_free(deleted);
    array_free(uncooked);

    // if not zipfile is present, all files are new and must be added
    if( !old ) {
        for( int i = 0; i < array_count(now); ++i ) {
            array_push(uncooked, STRDUP(now[i].fname));
        }
        return 1;
    }

    // compare for new & changed files
    for( int i = 0; i < array_count(now); ++i ) {
        int found = zip_find(old, now[i].fname);
        if( found < 0 ) {
            array_push(added, STRDUP(now[i].fname));
            array_push(uncooked, STRDUP(now[i].fname));
        } else {
            uint64_t oldsize = atoi64(zip_comment(old,found)); // zip_size(old, found); returns sizeof processed asset. return original size of unprocessed asset, which we store in comment section
            uint64_t oldstamp = atoi64(zip_modt(old, found)+20);
            if( oldsize != now[i].bytes || oldstamp > (now[i].stamp + 1) ) { // @fixme: should use hash instead. hashof(tool) ^ hashof(args used) ^ hashof(rawsize) ^ hashof(rawdate)
                printf("%s:\t%u vs %u, %u vs %u\n", now[i].fname, (unsigned)oldsize,(unsigned)now[i].bytes, (unsigned)oldstamp,(unsigned)now[i].stamp);
                array_push(changed, STRDUP(now[i].fname));
                array_push(uncooked, STRDUP(now[i].fname));
            }
        }
    }
    // compare for deleted files
    for( int i = 0; i < zip_count(old); ++i ) {
        char *oldname = zip_name(old, i);
        int idx = zip_find(old, oldname); // find latest versioned file in zip
        unsigned oldsize = zip_size(old, idx);
        if (!oldsize) continue;

        struct fs *found = 0; // zipscan_locate(now, oldname);
        for(int j = 0; j < array_count(now); ++j) {
            if( !strcmp(now[j].fname,oldname)) {
                found = &now[j];
                break;
            }
        }

        if( !found ) {
            array_push(deleted, STRDUP(oldname));
        }
    }
    return 1;
}

// ----------------------------------------------------------------------------

typedef struct cook_worker {
    const char **files;
    const char *rules;
    int threadid, numthreads;
    thread_ptr_t self;
    volatile int progress;
} cook_worker;

static cook_worker jobs[100] = {0};
static volatile bool cooker_cancelable = false, cooker_cancelling = false;

static
int cook(void *userdata) {
    cook_worker *job = (cook_worker*)userdata;

    // start progress
    volatile int *progress = &job->progress;
    *progress = 0;

    // scan disk from fs_now snapshot
    array(struct fs) filtered = zipscan_filter(job->threadid, job->numthreads);
    //printf("Scanned: %d items found\n", array_count(now));

    // prepare out tempname
    char COOKER_TMPFILE[64]; snprintf(COOKER_TMPFILE, 64, "temp_%02d", job->threadid);

    // prepare zip
    char zipfile[64]; snprintf(zipfile, 64, ".art[%02x].zip", job->threadid);
    if( file_size(zipfile) == 0 ) unlink(zipfile);

    // populate added/deleted/changed arrays by examining current disk vs last cache
    zip *z;
    {
        z = zip_open(zipfile, "r+b");
        zipscan_diff(z, filtered);
        if( z ) zip_close(z);

        fflush(0);

        z = zip_open(zipfile, "a+b");
        if( !z ) {
            unlink(zipfile);
            z = zip_open(zipfile, "a+b"); // try again
            if(!z) PANIC("cannot open file for updating: %s", zipfile);
        }
    }

    // deleted files
    //time_t time_now = time(NULL);
    //struct tm *tm_now = localtime(&time_now);
    for( int i = 0, end = array_count(deleted); i < end; ++i ) {
        printf("Deleting %03d%% %s\n", (i+1) == end ? 100 : (i * 100) / end, deleted[i]);
        FILE* out = fopen(COOKER_TMPFILE, "wb"); fclose(out);
        FILE* in = fopen(COOKER_TMPFILE, "rb");
        char *comment = "0";
        zip_append_file/*_timeinfo*/(z, deleted[i], comment, in, 0/*, tm_now*/);
        fclose(in);
    }
    // added or changed files
    for( int i = 0, end = array_count(uncooked); i < end && !cooker_cancelling; ++i ) {
        *progress = ((i+1) == end ? 90 : (i * 90) / end); // (i+i>0) * 100.f / end;

        // start cook
        const char *fname = uncooked[i]; //job->files[j];
        int inlen = file_size(fname);

        // generate a cooking script for this asset
        cook_script_t cs = cook_script(job->rules, fname, COOKER_TMPFILE);
        // puts(cs.script);

        // log to batch file for forensic purposes, if explicitly requested
        static __thread bool logging = 0, *init = 0; if(!init) *(init = &logging) = !!flag("--cook-debug");
        if( logging ) {
            FILE *logfile = fopen(va("cook%d.cmd",job->threadid), "a+t");
            if( logfile ) { fprintf(logfile, "@rem %s\n%s\n", fname, cs.script); fclose(logfile); }
            // maybe log fprintf(logfile, "@rem %*.s\n", 4096, os_exec_output()); ?
        }

        // invoke cooking script and recap status
        int rc = cs.script[0] ? (os_exec)(cs.script) : 0;
        int outlen = file_size(cs.finalfile);
        int failed = cs.script[0] ? rc || !outlen : 0;

        // print errors, or...
        if( failed ) {
            PRINTF("Import failed: %s while executing:\n%s\n%s\nReturned: %d (%#x)\n", fname, cs.script, os_exec_output(), rc, rc);
        }
        // ...process only if included. may include optional compression.
        else if( cs.compress_level >= 0 ) {
            FILE *in = fopen(cs.finalfile, "rb");

#if 0
                struct stat st; stat(fname, &st);
                struct tm *timeinfo = localtime(&st.st_mtime);
                ASSERT(timeinfo);

                // pretty (truncated) input (C:/prj/FWK/art/file.wav -> file.wav)
                static threadlocal int artlen = 0; if(!artlen) artlen = strlen(ART);
                const char *pretty = fname;
                if( !strncmp(pretty, ART, artlen) ) pretty += artlen;
                while(pretty[0] == '/') ++pretty;
                fname = pretty;
                //puts(fname);
#endif

                char *comment = va("%d", inlen);
                if( !zip_append_file/*_timeinfo*/(z, fname, comment, in, cs.compress_level/*, timeinfo*/) ) {
                    PANIC("failed to add processed file into %s: %s", zipfile, fname);
                }

            fclose(in);
        }
    }

    zip_close(z);

    // end progress
    *progress = 100;
    return 1;
}

static
int cook_async( void *userptr ) {
#ifndef FWK_COOKER_STANDALONE
    while(!window_handle()) sleep_ms(100); // wait for window handle to be created
#endif

    int ret = cook(userptr);
    thread_exit( ret );
    return ret;
}

bool cooker_start( const char *masks, int flags ) {
    const char *rules = file_read(FWK_INI);
    if(rules[0] == 0) return false;

    cooker_cancelable = !!( flags & COOKER_CANCELABLE );

    if( strstr(rules, "ART=") ) {
        ART = va( "%s", strstr(rules, "ART=") + 4 );
        char *r = strchr( ART, '\r' ); if(r) *r = 0;
        char *n = strchr( ART, '\n' ); if(n) *n = 0;
        char *s = strchr( ART, ';' );  if(s) *s = 0;
        char *w = strchr( ART, ' ' );  if(w) *w = 0;
        char *t = file_pathabs(ART); for(int i = 0; t[i]; ++i) if(t[i]=='\\') t[i] = '/';
        if( !strendi(t, "/") ) strcat(t, "/");
        ART = STRDUP(t); // @leak
    }

    if( strstr(rules, "TOOLS=") ) {
        TOOLS = va( "%s", strstr(rules, "TOOLS=") + 6 );
        char *r = strchr( TOOLS, '\r' ); if(r) *r = 0;
        char *n = strchr( TOOLS, '\n' ); if(n) *n = 0;
        char *s = strchr( TOOLS, ';' );  if(s) *s = 0;
        char *w = strchr( TOOLS, ' ' );  if(w) *w = 0;
        char *t = file_pathabs(TOOLS); for(int i = 0; t[i]; ++i) if(t[i]=='\\') t[i] = '/';
        if( !strendi(t, "/") ) strcat(t, "/");
        TOOLS = STRDUP(t); // @leak
    }

    if( strstr(rules, "EDITOR=") ) {
        EDITOR = va( "%s", strstr(rules, "EDITOR=") + 7 );
        char *r = strchr( EDITOR, '\r' ); if(r) *r = 0;
        char *n = strchr( EDITOR, '\n' ); if(n) *n = 0;
        char *s = strchr( EDITOR, ';' );  if(s) *s = 0;
        char *w = strchr( EDITOR, ' ' );  if(w) *w = 0;
        char *t = file_pathabs(EDITOR); for(int i = 0; t[i]; ++i) if(t[i]=='\\') t[i] = '/';
        if( !strendi(t, "/") ) strcat(t, "/");
        EDITOR = STRDUP(t); // @leak
    }

    // scan disk
    const char **list = file_list(ART, "**");
    // inspect disk
    for( int i = 0; list[i]; ++i ) {
        const char *fname = list[i];

        // skip special files, folders and internal files like .art.zip
        const char *dir = file_path(fname);
        if( dir[0] == '.' ) continue; // discard system dirs and hidden files
        if( strbegi(dir, TOOLS) ) continue; // discard tools folder
        if( !file_ext(fname)[0] ) continue; // discard extensionless entries
        if( !file_size(fname)) continue; // skip dirs and empty files

        // exclude vc c/c++ .obj files. they're not 3d wavefront .obj files
        if( strend(fname, ".obj") ) {
            char header[4] = {0};
            for( FILE *in = fopen(fname, "rb"); in; fclose(in), in = NULL) {
                fread(header, 1, 2, in);
            }
            if( !memcmp(header, "\x64\x86", 2) ) continue;
            if( !memcmp(header, "\x00\x00", 2) ) continue;
        }
        // exclude vc/gcc files
        if( strend(fname, ".a") || strend(fname, ".pdb") || strend(fname, ".lib") || strend(fname, ".ilk") || strend(fname, ".exp") ) {
            continue;
        }

        // @todo: normalize path & rebase here (absolute to local)
        // [...]
        // fi.normalized = ; tolower->to_underscore([]();:+ )->remove_extra_underscores

        if (file_name(fname)[0] == '.') continue; // skip system files
        if (file_name(fname)[0] == ';') continue; // skip comment files

        struct fs fi = {0};
        fi.fname = STRDUP(fname);
        fi.bytes = file_size(fname);
        fi.stamp = file_stamp(fname); // human-readable base10 timestamp

        array_push(fs_now, fi);
    }

    // spawn all the threads
    int num_jobs = cooker_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        jobs[i].self = 0;
        jobs[i].threadid = i;
        jobs[i].numthreads = flags & COOKER_ASYNC ? num_jobs : 1;
        jobs[i].files = list;
        jobs[i].rules = rules;
        jobs[i].progress = -1;
    }
    for( int i = 0; i < num_jobs; ++i ) {
        if( jobs[i].numthreads > 1 ) {
            jobs[i].self = thread_init(cook_async, &jobs[i], "cook_async()", 0/*STACK_SIZE*/);
        } else {
            return !!cook(&jobs[0]);
        }
    }

    (void) flags;
    return true;
}

void cooker_stop() {
    // join all threads
    int num_jobs = cooker_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        if(jobs[i].self) thread_join(jobs[i].self);
    }
    // remove all temporary outfiles
    const char **temps = file_list("./", "temp_*");
    for( int i = 0; temps[i]; ++i ) unlink(temps[i]);
}

int cooker_progress() {
    int count = 0, sum = 0;
    for( int i = 0, end = cooker_jobs(); i < end; ++i ) {
        if( jobs[i].progress >= 0 ) {
            sum += jobs[i].progress;
            ++count;
        }
    }
    return cooker_jobs() ? sum / (count+!count) : 100;
}

void cooker_cancel() {
    if( cooker_cancelable ) cooker_cancelling = true;
}

int cooker_jobs() {
    int num_jobs = optioni("--with-cook-jobs", ifdef(tcc, 1, cpu_cores()*2)), max_jobs = countof(jobs);
    ifdef(ems, num_jobs = 0);
    return clampi(num_jobs, 0, max_jobs);
}

void cooker_config( const char *art_path, const char *tools_path, const char *fwk_ini ) { // @todo: test run-from-"bin/" case on Linux.
    const char *rules = file_read(fwk_ini ? fwk_ini : FWK_INI);
    if( rules && rules[0] != 0 ) {
        if( strstr(rules, "ART=") ) {
            ART = STRDUP( strstr(rules, "ART=") + 4 ); // @leak
            char *r = strchr( ART, '\r' ); if(r) *r = 0;
            char *n = strchr( ART, '\n' ); if(n) *n = 0;
            char *s = strchr( ART, ';' );  if(s) *s = 0;
            char *w = strchr( ART, ' ' );  if(w) *w = 0;
            if( ART[strlen(ART) - 1] != '/' ) strcat((char*)ART, "/");
        }
        if( strstr(rules, "TOOLS=") ) {
            TOOLS = STRDUP( strstr(rules, "TOOLS=") + 6 ); // @leak
            char *r = strchr( TOOLS, '\r' ); if(r) *r = 0;
            char *n = strchr( TOOLS, '\n' ); if(n) *n = 0;
            char *s = strchr( TOOLS, ';' );  if(s) *s = 0;
            char *w = strchr( TOOLS, ' ' );  if(w) *w = 0;
            if( TOOLS[strlen(TOOLS) - 1] != '/' ) strcat((char*)TOOLS, "/");
        }
    }

    ART = STRDUP( art_path ? art_path : ART );
    TOOLS = STRDUP( tools_path ? tools_path : TOOLS );
    FWK_INI = STRDUP( fwk_ini ? fwk_ini : FWK_INI );

    assert( ART[strlen(ART) - 1] == '/' );
    assert( TOOLS[strlen(TOOLS) - 1] == '/' );
    assert( FWK_INI[strlen(FWK_INI) - 1] != '/' );
}

// ----------------------------------------------------------------------------

#ifdef FWK_COOKER_STANDALONE
int main(int argc, char **argv) {
    clock_t timer = clock();

    cooker_config(NULL, NULL, NULL);

    if( argc > 1 && argv[1][0] != '-' ) {
        char *rules = file_read("fwk.ini");
        cook_script_t cs = cook_script(rules, argv[1], "a.out");
        puts(cs.script);
        exit(1);
    }

    if( file_directory(TOOLS) && cooker_jobs() ) {
        cooker_start( "**", 0|COOKER_ASYNC/*|COOKER_CANCELABLE*/ );
        cooker_stop();
    }

    printf("Ok, %5.2fs.\n", (clock() - timer) / (double)CLOCKS_PER_SEC);
}
#endif
