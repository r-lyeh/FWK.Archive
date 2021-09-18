// C documentation generator using markdeep.
// - rlyeh, public domain
//
// it works this way:
// 1. source files are firstly parsed:
//   - for every parsed function, all of its //markdeep comments are appended to a cache.
// 2. then, header files are parsed:
//   - ///markdeep comments, enums, structs, variables and typedefs are extracted and printed as-is.
//   - macros are extracted and printed only if requested with /// comment.
//   - functions are extracted and printed as-is. optionally, extra infos are printed if there are stored comments present in the cache.
// 3. explicit `symbols` in comments are converted into links to ease browsing.

// build:
// cl docs.c && docs ..\..\fwk.h -x=3rd_glad.h,fwk.h,fwk_main.h,fwk_compat.h, > docs.html

// @todo: learn, then by group: 1st pure typedefs, 2nd enums, 3rd structs, 4th funcs
// @todo: pretty enum as table of name/comment instead of name/value
// @todo: pretty struct as table of type/name/comment

#include "fwk.h"

int DO_CSS = 1;
int DO_DEBUG = 0;

// C preprocessor:
// 1. remove all /* C comments */
// 2. remove all // C++ comments
// 3. remove all \macro continuation lines
// 4. preserve /// comments
void preprocess_c(char *raw) {
    for(int i = 0, in_string = 0; raw[i]; ++i) {
        if( !in_string ) {
            if( raw[i] == '\"' || raw[i] == '\'' ) if( raw[i-1] != '\\' ) {
                in_string = raw[i];
                continue;
            }

            // preserve ///comments
            if( memcmp(&raw[i], "///", 3) == 0 ) {
                while( raw[i] && raw[i] >= 32 ) ++i;
                --i; continue;
            }

            // remove C++ comments
            if( raw[i] == '/' && raw[i+1] == '/' && raw[i+2] != '/' ) {
                do raw[i++] = ' '; while( raw[i] && (raw[i] >= 32 || raw[i] == '\t'));
                --i; continue;
            }

            // remove \macro continuation lines
            if(raw[i] == '\\' && (raw[i+1] && raw[i+1] < 32) && (raw[i+2] && raw[i+2] < 32)) raw[i]=raw[i+1]=raw[i+2] = ' ';
            if(raw[i] == '\\' && (raw[i+1] && raw[i+1] < 32)) raw[i]=raw[i+1] = ' ';

            // remove /* C comments */
            if(raw[i] == '/' && raw[i+1] == '*') {
                raw[i]=raw[i+1]=' ';
                while(raw[i]) if(raw[i] == '*' && raw[i+1] == '/') break; else raw[i++] = ' ';
                if (raw[i]) raw[i] = ' ';
                if (raw[i + 1]) raw[i + 1] = ' ';
                --i; continue;
            }

        } else {
            while(raw[i] == '\\' && raw[i+1]) i += 2;
            if(raw[i] == in_string ) in_string = 0;
            //if(DO_DEBUG) raw[i] = toupper(raw[i]);
        }
    }
    // remove all linefeeds within {} scopes (flatten scopes into single-lines)
    for(int i = 0, open = 0, in_string = 0; raw[i]; ++i) {
        if( !in_string ) {
            if( raw[i] == '\"' || raw[i] == '\'' ) if( raw[i-1] != '\\' ) {
                in_string = raw[i];
                continue;
            }
            // inline brackets. do not process extern "C" { ... } brackets
            if(raw[i] == '{') if(i >= 12 && !memcmp("extern \"C\" {", &raw[i-11], 12) ? 0 : 1) ++open;
            if(raw[i] == '}') if(open > 0) --open;
            if(open > 0) if(raw[i] && raw[i] < 32) raw[i] = ' ';
        } else {
            while(raw[i] == '\\' && raw[i+1]) i += 2;
            if(raw[i] == in_string ) in_string = 0;
        }
    }
}

// Markdown preprocessor:
// 1. replace all {{TEMPLATES}} if defined in OS environment && small enough for inlining
char* preprocess_md(char *raw) {
    for(int i = 0, in_string = 0; raw[i]; ++i) {
        if( !in_string ) {
            if( raw[i] == '\"' /* || raw[i] == '\'' */ ) if( raw[i-1] != '\\' ) {
                in_string = raw[i];
                continue;
            }

            // replace {{TEMPLATE}} symbols if they exist and fit in-place
            if( raw[i] == '{' && raw[i+1] == '{' && raw[i+2] >= 'A') {
                char buf[128];
                if( sscanf(raw+i+2, "%127[^}]", buf) == 1 ) {
                    int buflen = strlen(buf);
                    char *eos = &raw[i+2] + buflen;
                    if( buflen && eos[0] == '}' && eos[1] == '}' ) {
                        char *repl = getenv(buf);
                        if( repl && strlen(repl) <= (buflen + 4) ) {
                            memset(raw+i, ' ', buflen + 4);
                            memcpy(raw+i, repl, strlen(repl));
                            i += buflen+4;
                            --i; continue;
                        }
                    }
                }
            }

        } else {
            while(raw[i] == '\\' && raw[i+1]) i += 2;
            if(raw[i] == in_string ) in_string = 0;
            //if(DO_DEBUG) raw[i] = toupper(raw[i]);
        }
    }
    return raw;
}

// remove whitespaces within a line, unless they are present in a \"string\"
char *minify( const char *line ) {
    // skip initial whitespaces
    line += strspn(line, " \t\r\n");

    // reserve space
    char *bak = strdup/*STRDUP*/(line), *s = bak;

    for(int in_string = 0, seen_alphanum = 0; *line; ++line ) {
        if( !in_string ) {
            if( *line == '\"' || *line == '\'' ) if( line[-1] != '\\' ) {
                in_string = *line;
                *s++ = *line;
                continue;
            }
            // remove spaces
            if( *line > 0 && *line <= 32 ) {
                // keep spacing only if previous char was alphanum && next char is not operator
                const char *next_nbsp = line+1 + strspn(line+1, " \t\r\n");
                #define PUNCTUATORS "#" "!&|~" "+-/*%()" "<=>" "[]{}" "?:;" ",'\"\\"
                if(seen_alphanum && !strchr(PUNCTUATORS, *next_nbsp)) *s++ = ' ';

                while( *line > 0 && *line <= 32 ) ++line;

                --line;
                continue;
            }
            // save for later
            seen_alphanum = isalnum(*line);
            *s++ = *line;
        } else {
            while(*line == '\\' && line[1]) { memcpy(s, line, 2); s += 2; line += 2; }
            if(*line == in_string ) in_string = 0;

            seen_alphanum = isalnum(*line);
            *s++ = *line;
        }
    }

    *s = 0;
    return bak;
}

bool parse_c_function(const char *raw, int *out_name_pos, int *out_name_len, int *out_args_pos, bool clean_parens) {
    // try to clean up name from worst pathological case with parentheses. ie:
    // API __declspec(dllexport) int(*)(int,...)(MACRO_CALLBACK)(int(*my_callback)(int,...));
    // split declaration into 3 parts: return value, function name, and arguments.
    // 1. string is parsed backwards, from tail to head.
    // 2. arguments are parsed till all matching ( ) chars are balanced.
    // 3. function name is parsed then till '(' or ' ' are found. we remove any parens if present.
    // 4. everything else is the return type.
    bool is_function = false;
    int name = 0, args = 0;
    const char *eos = raw + strlen(raw) - 1;
    for( int i = eos - raw, open = 0; i >= 0; --i ) {
        if( raw[i] == ')' ) ++open;
        if( raw[i] == '(' ) if(open > 0) --open;

        if( raw[i] == '(' && open == 0 ) {
            /**/ if( args == 0 ) args = i;
            else if( name == 0 ) name = i;
        }
        if( (raw[i] == ' ' || raw[i] == '*') && open == 0 && args != 0 && name == 0 ) {
            name = i;
        }

        if( i == 0 ) is_function = (open == 0); // if parens are not balanced, this is not a function
    }
    if( is_function ) {
        // printf("%d %d\n", name, args);
        // printf(">> %.*s\n", args-name, raw+name);
        // printf(">> %s\n", raw + args);
        *out_args_pos = args;
        *out_name_pos = name;
        *out_name_len = args-name;
        if( clean_parens && raw[name] == '(' && raw[args-1] == ')' ) {
            *out_name_pos += 1;
            *out_name_len -= 2;
        }
        return true;
    }
    return false;
}
bool parse_c_function_NEW(const char *raw, char **out_name, char **out_args, bool clean_parens) {
    // try to clean up name from worst pathological case with parentheses. ie:
    // API __declspec(dllexport) int(*)(int,...)(MACRO_CALLBACK)(int(*my_callback)(int,...));
    // split declaration into 3 parts: return value, function name, and arguments.
    // 1. string is parsed backwards, from tail to head.
    // 2. arguments are parsed till all matching ( ) chars are balanced.
    // 3. function name is parsed then till '(' or ' ' are found. we remove any parens if present.
    // 4. everything else is the return type.
    bool is_function = false;
    int name = 0, args = 0;
    const char *eos = raw + strlen(raw) - 1;
    for( int i = eos - raw, open = 0; i >= 0; --i ) {
        if( raw[i] == ')' ) ++open;
        if( raw[i] == '(' ) if(open > 0) --open;

        if( raw[i] == '(' && open == 0 ) {
            /**/ if( args == 0 ) args = i;
            else if( name == 0 ) name = i;
        }
        if( (raw[i] == ' ' || raw[i] == '*') && open == 0 && args != 0 && name == 0 ) {
            name = i;
        }

        if( i == 0 ) is_function = (open == 0); // if parens are not balanced, this is not a function
    }
    if( is_function ) {
        // printf("%d %d\n", name, args);
        // printf(">> %.*s\n", args-name, raw+name);
        // printf(">> %s\n", raw + args);
        int namelen = args-name;
        clean_parens = clean_parens && raw[name] == '(' && raw[args-1] == ')';
        *out_args = stringf("%s", raw + args);
        *out_name = stringf("%.*s", namelen -clean_parens*2, raw+name +clean_parens );
        return true;
    }
    return false;
}

// reintroduce spacing and tabs where due, so code is pretty much readable again.
// some difficult notes when indenting7cleaning up:
// 1. structs, unions, enums use '{' to increase indenation, and '}' to decrease it.
// 2. structs, unions do line carriage after every member declaration, and keep indentation.
// 3. enums use ',' to linefeed
// 4. we do conversion from static [inline] type func() { ... } -> to -> type func();
// 5. rename functions that are declared between parens to disambiguate macros: int (print)(...);
// 6. do ellipsis on macro contents: #define print(args) ...
char *prettify(char *raw, char **opt_name) {
    enum { TABS = 4 };

    if(DO_DEBUG) puts(raw);

    static char *buf = 0; buf = REALLOC(buf, strlen(raw) * 16 + 256);
    char *s = buf;

    // hide implementation from static functions
    int is_static = strbegi(raw, "static ");
    int is_static_inline = strbegi(raw, "static inline ");
    if( is_static || is_static_inline ) {
        raw += strlen(is_static_inline ? "static inline " : "static ");
        char *eos = strchr(raw, '{');
        if(eos) eos[0] = ';', eos[1] = 0;
    }

    // hide macro details. expose only name and args
    int is_macro = strbegi(raw, "#define ");
    if( is_macro ) {
        #define KEYWORD_CHARSET "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        #define MACRO_CHARSET KEYWORD_CHARSET "$"
        int wordlen = strspn(raw+8, MACRO_CHARSET);
        for( int i = 8 + wordlen, open = 0; raw[i]; ++i ) {
            if( raw[i] == '(' ) { ++open; continue; }
            if( raw[i] == ')' ) { if(open > 0) --open; continue; }
            if( open == 0 ) {
                // hide implementation, until either any of eos or ///comment are found
                while( raw[i] && strcmp(raw+i, "/""//") ) {
                    raw[i++] = ' ';
                }
                break;
            }
        }
        memcpy(raw+=2, "macro", 5); // #define -> macro
    }

    int is_function = raw[0] != '#' && !strchr(raw, '{') && !strchr(raw, '}') && strchr(raw, '(') && strchr(raw, ')') && !strbegi(raw, "typedef");
    if( is_function ) {
        int name = 0, args = 0, namelen = 0;
        is_function = parse_c_function( raw, &name, &namelen, &args, false );
        // clean parens `int (function)(...);` cases
        if( is_function ) { 
            if( raw[name+namelen-1] == ')' ) {
            sprintf(buf, "%.*s%s%.*s%s", name, raw, raw[name-1] == '*' ? "":" ", namelen-2, raw+name+1, raw+args);
            raw = strdup(buf);
            *buf = 0;
            }
        }
    }

    int is_typedef        = strbegi(raw, "typedef "); 
    int is_typedef_enum   = strbegi(raw, "typedef enum");
    int is_typedef_union  = strbegi(raw, "typedef union");
    int is_typedef_struct = strbegi(raw, "typedef struct");
    int is_typedef_block  = is_typedef_enum || is_typedef_union || is_typedef_struct;
    if( is_typedef_block ) {
        raw += strlen("typedef "); // shorten decls on large structs/unions/enums
        char *eos = strrchr(raw, '}');
        if(eos) eos[1] = ';', eos[2] = 0;
    }

    int is_enum = strbegi(raw, "enum");

    const char *remaps[] = { // sort by strlen, then by priority
        "**", "** ",  // char**k
        "(*", " (*",  // int(*fn)();
        "*,", "*, ",  // int printf(char*,...);
        "*)", "*)",   // (char*)k here to fix ')' case below
        "*", "* ",    // char*k
        ",}", "}",    // enum{A,B,}
        ");", ");",   // int func(); here to fix ')' case below
        "};", "}; ",  // struct k{union{int k;};}; here to fix '}' case below
        "}",  "} ",   // typedef struct k{int k;}type;
        "{",  " { ",  // struct k{}
//      ",",  ", ",   // print(int a,int b)
        ";",  "; ",   // struct{int a;int b;};
        "=",  " = ",  // enum{N=1,}
    };
    // 1st pass, spacing
    for( int i = 0; raw[i]; ++i ) {
        *s++ = raw[i];
        for( int j = 0; j < countof(remaps) / 2; ++j ) {
            if(!strncmp(&raw[i], remaps[j*2+0], strlen(remaps[j*2+0]))) {
                --s;
                s += sprintf(s, "%s", remaps[j*2+1]);
                i += strlen(remaps[j*2+0]) - 1;
                break;
            }
        }
        // fix array(char*)strplit(...) or array(int)list(...) case
        // but dont in int(*callback_t)(...) case
        if( i > 0 && s[-1] == ')' && strchr(KEYWORD_CHARSET, raw[i+1]) ) {
            *s++ = ' ';
        }
    }
    if(DO_DEBUG) puts(buf);

    // prepare for 2nd pass
    *s++ = 0;
    raw =  buf + MSIZE(buf)-1 - strlen(buf)-1;
    strcpy(buf + MSIZE(buf)-1 - strlen(buf)-1, buf);
    s = buf;

    // 2nd pass, multi-lines
    for(int i = 0, level = 0; raw[i]; ++i) {
        int indent_inc = (raw[i] == '{');
        int indent_dec = (raw[i] == '}');
        int linefeed = indent_inc || indent_dec || (raw[i] == ';') || (is_enum && raw[i] == ',');

        if( !linefeed ) {
            *s++ = raw[i];
        } else {

            if( indent_dec ) {
                if( is_enum ) s += sprintf(s, "\n%*s", (level-1)*TABS, "");
                else          if( level > 0 ) s -= TABS;
            }

            /**/ if( indent_inc ) s += sprintf(s, "%c%s\n%*s", raw[i], raw[i+1] == ';' ? ";" : "", ++level * TABS, "");
            else if( indent_dec ) s += sprintf(s, "%c%s\n%*s", raw[i], raw[i+1] == ';' ? ";" : "", --level * TABS, "");
            else                  s += sprintf(s, "%c%s\n%*s", raw[i], raw[i+1] == ';' ? ";" : "",   level * TABS, "");

            // consume semicolon
            while(raw[i+1] == ';') ++i;
            // consume whitespaces
            while(raw[i+1] == ' ') ++i;
        }
    }
    // if(DO_DEBUG) puts(buf); // no need to debug this one, gonna be printed anyways

    // prepare for last pass
    *s = 0;
    s = buf;
    // last pass: remove extra linefeeds at eos
    while( s[strlen(s)-1] == '\n' ) s[strlen(s)-1] = 0;

    // extract type+name if needed
    if( opt_name ) {
        const char *line = buf;
        bool is_function = strchr(line, '(') && strchr(line, '(') < strrchr(line, ')');

        char s[1024]; int n,nl,a;
        /**/ if( line[0] == '#' )          { sscanf(line, "%*s %s", s);   *opt_name = strdup(stringf("#%s",s)); }
        else if( strbegi(line, "macro"))   { sscanf(line, "%*s %[^(]",s); *opt_name = strdup(stringf("m%s",s)); }
        else if( strbegi(line, "union"))   { sscanf(line, "%*s %s",s);    *opt_name = strdup(stringf("u%s",s)); }
        else if( strbegi(line, "struct"))  { sscanf(line, "%*s %s",s);    *opt_name = strdup(stringf("s%s",s)); }
        else if( strbegi(line, "enum"))    { sscanf(line, "%*s %s",s);    *opt_name = strdup(stringf("e%s",s)); }
        else if( strbegi(line, "typedef")) { 
            char *last_word = strrchr(line, ' ') + 1;
            sscanf(last_word, "%[^[;]", s);
            if( strchr(s, ')') ) {
                line += strlen("typedef ");
                parse_c_function(line,&n,&nl,&a,1);
                line += n;
                if(*line == '*') line++, --nl;
                sprintf(s, "%.*s", nl, line);
            }
            *opt_name = strdup(stringf("t%s",s));
        }
        else if( line[0] != '#' && is_function && parse_c_function(line,&n,&nl,&a,1)) { 
            while(line[n] == ' ' || line[n] == '*') ++n, --nl; 
            sprintf(s,"%.*s",nl,line+n); 
            *opt_name = strdup(stringf("f%s",s));
        }
        // else isvar {}

        // trim whitespaces
        if(*opt_name && strchr(*opt_name, ' ')) {
        *strchr(*opt_name, ' ') = 0;
        }

        // disambiguate macros()<->defines
        if(*opt_name && *opt_name[0] == 'm') {
            #if 1
            char upper[128] = {0}, *u = upper, lower[128] = {0}, *l = lower;
            for( char *p = *opt_name+1; *p; ++p) {
                *u++ = toupper(*p);
                *l++ = tolower(*l);
            }
            if( !strcmp(lower, *opt_name+1) ) *opt_name[0] = 'm';
            if( !strcmp(upper, *opt_name+1) ) *opt_name[0] = 'd';
            #else
            char *found = strstr(line, stringf("%s(", (*opt_name)+1));
            if(!found)
            *opt_name[0] = 'd';
            #endif
        }
    }

    return buf;
}

int sort_strcmp( const void *str1, const void *str2 ) {
    const char *const *pp1 = str1;
    const char *const *pp2 = str2;
    return strcmpi(*pp1, *pp2);
}

// db cache
map(char*,array(char*)) db;
array(char*) doc;
array(char*) macros;
array(char*) defines;
array(char*) structs;
array(char*) unions;
array(char*) enums;
array(char*) typedefs;
array(char*) functions;

char *learn(char *line) {
    // @todo: learn markdeep from [line] based on array(context)
    // @todo: update caches & context
    // [...]

    // continue with pretty line
    char *typename;
    line = prettify(strdup(line), &typename);

    if( typename[0] == 'd' ) array_push(defines, typename+1);
    if( typename[0] == 't' ) array_push(typedefs, typename+1);
    if( typename[0] == 'e' ) array_push(enums, typename+1);
    if( typename[0] == 's' ) array_push(structs, typename+1);
    if( typename[0] == 'u' ) array_push(structs/*unions*/, typename+1);
    if( typename[0] == 'f' ) array_push(functions, typename+1);
    if( typename[0] == 'm' ) array_push(macros, typename+1);

    return line;
}

bool group_together() {
    // find relevant syntax blocks that belong together:
    // - they are a group if lines are sticked together, with no newlines.
    // - they are part of a group if both syntax blocks are sharing keywords on them.
    return true;
}

void print_markdeep(const char *line) { // "/// #hello markdeep"
    const char *text = line + 3 + (line[3] == ' ');
    const char *word = text + strspn(text, " \t");
    if( !word[0] ) return;

    int needs_blank =
        strbegi(word, "---") || strbegi(word, "~~~") || strbegi(word, "```");

    int needs_heading =
        strbegi(word, "# ") || // section
        strbegi(word, "**") || strbegi(word, "* ") || // heading and diagrams
        strbegi(word, "- ") || strbegi(word, "1. ") || // un/ordered lists
        strbegi(word, "---") || strbegi(word, "~~~") || strbegi(word, "```") // linebreaks and snippets
            ? 0 : 1;

    printf("%s%s%s\n", needs_blank ? "\n":"", needs_heading ? "- ":"", text);
}

void print_decl(char *line) { // non-const, since prettify can modify input
    char *typename;
    char *pretty = prettify(line, &typename);

    char type = typename[0], *name = typename+1;

    char *header = "";
    /**/ if( type == 'd' ) header = "🄳"; //
    else if( type == 't' ) header = "🅃"; // 🅣
    else if( type == 'e' ) header = "🄴"; // 🅔
    else if( type == 'u' ) header = "🅂"; // 🅢
    else if( type == 's' ) header = "🅂"; // 🅢
    else if( type == 'f' ) header = "🄵"; // 🅕Ⓕ
    else if( type == 'm' ) header = "🄼"; // 🅜

    // make pretty->oneliner
    bool truncated = 0;
    char *oneliner = strdup(pretty);
    if( strchr(oneliner, '\n') ) {
        char *patch = strstr(oneliner, name);
        if( patch ) patch += strlen(name), *patch++ = ';', *patch++ = 0, truncated = 1;
    }
    if( strbegi(oneliner, "typedef") && strchr(oneliner, '(') ) {
        sprintf(oneliner, "typedef %s;", name); // truncate pointer-to-functions to max
        truncated = 1;
    }

    // anchor
    printf("<a name=\"%s\"></a>\n", name);

    // functions
    printf("<details><summary><code lang=C>%s %s</code></summary>\n", header, oneliner);

    if( truncated ) {
        printf("~~~~~~C\n"); // linenumbers\n");
        printf("%s %s\n", header, pretty);
        printf("~~~~~~\n");
    }

    printf("\nReposition the `video_t` stream. See also: [video_seek](#video_seek)\n", name);

    printf("\nReturn|{{return}}\n");
    printf("-----|------\n");
    printf("`arg1`|{{description}}\n\n");

    printf("\n~~~~~~C linenumbers\n");
    printf("{{example.c}}\n");
    printf("~~~~~~\n");

    printf("\n!!! Tip\n");
    printf("    Use `WITH_VIDEO_YCBCR=1` directive to optimize video decoding.\n");

    printf("\n!!! WARNING\n");
    printf("    Fix me: *Audio decoder might exhaust buffers on Linux!*\n");

    printf("\n</details>\n\n");
}

int main(int argc, char **argv) {
    // write header+intro
    DO_CSS = flag("-nocss") ? 0 : optioni("--css", DO_CSS);
    DO_DEBUG = optioni("--debug", DO_DEBUG) | flag("-dbg");
    const char *excludes = option("--excluded-sections,--excluded,-x", "");

    if( DO_CSS ) {
        char *css = file_read("art/docs/docs.css");
        char *md = file_read("art/docs/docs.md");
        strrepl(&css, "\r\n", "\n");
        strrepl(&md, "\r\n", "\n");
        puts( css );
        puts( preprocess_md(md) );
    }

    // headers are enabled by default. sources do not.
    const char *fname = argc > 1 ? argv[1] : __FILE__;
    int is_header_file = strstr(".h.H", file_ext(fname)) ? 1 : 0;
    char *section = STRDUP(file_name(fname));

    // read & preprocess C file
    char *raw = file_read(fname);
    preprocess_c(raw);
    // if(DO_DEBUG) fprintf(stderr, "%s\n", raw);

    // split into lines
    array(char*) split = strsplit(raw, "\r\n");
    if(DO_DEBUG) fprintf(stderr, "split %d lines\n", (int)array_count(split));

    // minify every line, then learn & cache
    array(char*) file = 0;
    for(int i = 0; i < array_count(split); ++i) {
        char *line = minify(split[i]);
        if( !line[0] ) continue;

        is_header_file |= (strbegi(line, "#pragma once") || strbegi(line, "#line")); // likely a header file
        if(!is_header_file) continue;

        // handle new heading sections
        if( strbegi(line, "#line") ) {
            char newfile[128];
            if(sscanf(line, "%*s %*d %[^\r\n]", newfile) == 1 ) {
                // remove quotes if present
                char *p = &newfile[ newfile[0] == '\"' ];
                if(strrchr(p, '\"')) *strrchr(p, '\"') = 0;
                // store
                section = STRDUP(p);
            }
        }
        // do not process if section is excluded from processing...
        if( section && excludes[0] ) {
            bool is_in_list = !!strstr(excludes, section);
            bool is_whole_list = !strcmp(excludes, section);
            if( is_whole_list || is_in_list ) {
                continue;
            }
        }
        // ...else, process if needed
        if( section ) {
            // remove extension if present
            char *found = strrchr(section, '.'); if(found) *found = 0;
            // print section in Uppercase form
            char *name = section + (strbegi(section, "fwk_") ? 4 : 0);
            //name[0] = toupper(name[0]);
            printf("## %s\n\n", name);
            // reset
            section = NULL;
        }

        if(strendi(line, "/""//-")) continue; // discard explicitly excluded lines

        learn(line);

        bool is_macro = line[0] == '#';
        bool is_enum = strbegi(line, "enum ") || strbegi(line, "enum{");
        bool is_struct = strbegi(line, "struct ") || strbegi(line, "struct{");
        bool is_union = strbegi(line, "union ") || strbegi(line, "union{");
        bool is_block = is_enum || is_struct || is_union;
        bool is_variable = strstr(line, "extern ");
        bool is_function = !is_macro && strchr(line, '(') && strchr(line, '(') < strrchr(line, ')');
        bool is_typedef = !is_block && strbegi(line, "typedef ");

        // @todo: sort out display priorities

        // if(DO_DEBUG) printf("L%d [%s]\n", i, line);

        char *request = strstr(line, "/""//+");
        if( request ) {
            // explicit request
            request[3] = ' '; // hide plus sign
            print_markdeep(request);
            request[0] = 0;
            // ...then print decl
            print_decl(line);
            continue;
        }

        if( is_variable || is_function || is_typedef ) {
            print_decl(line);
            continue;
        }
        if( is_block ) {
            print_decl(line);
            continue;
        }

        if( 0 == strcmp(line, "/""//") ) {
            // markdeep blank line
            puts("");
            continue;
        }
        else
        if( strbegi(line, "/""//" ) ) {
            // markdeep literal
            print_markdeep(line);
            continue;
        }
        else
        if( strstr(line, "/""//") ) {
            // inlined markdeep: print markdeep comment, then code
            puts("");
            char *md = strstr(line, "/""//");
            if( md[3] ) { print_markdeep(md); md[0] = 0; }

            print_decl(line);
            continue;
        }
    }

    puts("\n## 🄳 defines");
    array_sort(defines, sort_strcmp);
    array_unique(defines, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(defines); i < end; ++i)    printf("%s[%s](#%s)", sep, defines[i], defines[i]), sep = ", "; }

    puts("\n## 🅃 typedefs");
    array_sort(typedefs, sort_strcmp);
    array_unique(typedefs, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(typedefs); i < end; ++i)   printf("%s[%s](#%s)", sep, typedefs[i], typedefs[i]), sep = ", "; }

    puts("\n## 🄴 enums");
    array_sort(enums, sort_strcmp);
    array_unique(enums, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(enums); i < end; ++i)      printf("%s[%s](#%s)", sep, enums[i], enums[i]), sep = ", "; }

    puts("\n## 🅂 structs");
    array_sort(structs, sort_strcmp);
    array_unique(structs, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(structs); i < end; ++i)    printf("%s[%s](#%s)", sep, structs[i], structs[i]), sep = ", "; }

    //puts("\n## 🅄 unions");
    //array_sort(unions, sort_strcmp);
    //array_sort(unions, sort_strcmp);
    //{ char *sep = ""; for(int i = 0, end = array_count(unions); i < end; ++i)     printf("%s[%s](#%s)", sep, unions[i], unions[i]), sep = ", "; }

    puts("\n## 🄵 functions");
    array_sort(functions, sort_strcmp);
    array_unique(functions, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(functions); i < end; ++i)  printf("%s[%s](#%s)", sep, functions[i], functions[i]), sep = ", "; }

    puts("\n## 🄼 macros");
    array_sort(macros, sort_strcmp);
    array_unique(macros, sort_strcmp);
    { char *sep = ""; for(int i = 0, end = array_count(macros); i < end; ++i)     printf("%s[%s](#%s)", sep, macros[i], macros[i]), sep = ", "; }

    fprintf(stderr, "%s\n", "Ok");
}

// full example below:

#line 1000 "docs.h"

/// # This is a demo
/// Below you can find a diagram that illustrates the whole process.
/// *************************************
/// *                                   *
/// * +----------> [] ----+             *
/// * |                   |             *
/// * +-------------------+             *
/// *                                   *
/// *************************************

/* this is a C comment
*/

#   include\
<stdio.h>
  #  define my_test(...) \
    do_nothing

/// this is my image class
struct my_image_t {
    unsigned int width, height, bpp : 10;
    void *pixels;
    union {
        int dummy;
    };
};

typedef struct my_image_t my_image;

/// this is an enumeration
enum my_enumeration {
    ONE = 1,
    TWO = 2,
    THREE,
    FOUR
};

#define my_macro_exposed1() ///
#define my_macro_exposed2() ///exposed with a comment
#define my_macro_not_exposed() ///-

/// this function acts like printf. `fmt` is a C formatted string.
void function_exposed1(char *fmt, ...);
map(char*,array(int)) (function_exposed2)(
    int line1,
    int line2,
    ...
);

array(char*) function_exposed_3(int line);

extern int version;

///------------------
