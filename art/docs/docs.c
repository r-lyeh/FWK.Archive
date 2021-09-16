/// **docs**
/// # about
/// a C documentation generator using markdeep. it works this way:
/// 1. source files are firstly parsed:
///   - for every parsed function, all of its //markdeep comments are appended to a cache.
/// 1. then, header files are parsed:
///   - ///markdeep comments, enums, structs, variables and typedefs are extracted and printed as-is.
///   - macros are extracted and printed only if requested with /// comment.
///   - functions are extracted and printed as-is. optionally, extra infos are printed if there are stored comments present in the cache.
/// 1. explicit `symbols` in comments are converted into links to browse more easily.

// build:
// cl docs.c && docs ..\..\fwk.h -x=3rd_glad.h,fwk.h,fwk_main.h,fwk_compat.h, > docs.html && copy /y docs.html ..\..\fwk.html

// @todo: ///+ ///- 
// @todo: minify functions (till ;)
// @todo: maxify
// @todo: pretty align fn:   type:5   symbol:16 (arg1, arg2, arg3)
// @todo: pretty (funcname)->funcname
// @todo: pretty (spacing)
// @todo: pretty (} lining)
// @todo: pretty enum as table of name/comment instead of name/value
// @todo: pretty struct as table of type/name/comment
// @todo: pretty sort, then by group: 1st pure typedefs, 2nd enums, 3rd structs, 4th funcs
// parse_decl: eos: ; or } (pure decl, or decl+impl), remove block if }, 1st: args, matching parens; 2nd: symbol and (symbol); 3rd: rettype
// tags: 🄵 🄴 🅂 🅃 🄼    Ⓕ  🅕 🅔 🅢 🅣 🅜 

#include "fwk.h"

// preprocessor:
// 1. remove all /* C comments */
// 2. remove all // C++ comments
// 3. remove all \macro continuation lines
// 4. preserve /// comments
void preprocess(char *raw) {
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

            // remove \macro continuation liness
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
            // raw[i] = toupper(raw[i]); // DEBUG
        }
    }
    // remove all linefeeds within {} scopes (flatten scopes into single-lines)
    for(int i = 0, open = 0, in_string = 0; raw[i]; ++i) {
        if( !in_string ) {
            if( raw[i] == '\"' || raw[i] == '\'' ) if( raw[i-1] != '\\' ) {
                in_string = raw[i];
                continue;
            }
            // inline brackets. @fixme do not process extern "C" { ... } brackets
            if(raw[i] == '{') ++open;
            if(raw[i] == '}') --open;
            if(open > 0) if(raw[i] && raw[i] < 32) raw[i] = ' ';
            // if(open > 0) raw[i] = toupper(raw[i]); // DEBUG
        } else {
            while(raw[i] == '\\' && raw[i+1]) i += 2;
            if(raw[i] == in_string ) in_string = 0;
        }
    }
}

// remove whitespaces within a line, unless they are present in a \"string\"
char *minify( const char *line ) {
    // skip initial whitespaces
    line += strspn(line, " \t\r\n");

    // reserve space
    char *bak = strdup/*STRDUP*/(line)/*stringf("%s", line)*/, *s = bak; *s = 0;

#if 1
    for(int in_string = 0, seen_alphanum = 0; *line; ++line ) {
        if( !in_string ) {
            if( *line == '\"' || *line == '\'' ) if( line[-1] != '\\' ) {
                in_string = *line;
                *s++ = *line;  // feature when commented out!!
                continue;
            }
            // remove spaces
            if( *line > 0 && *line <= 32 ) {
                // keep spacing only if previous char was alphanum && next char is not operator
                const char *next_nbsp = line+1 + strspn(line+1, " \t\r\n");
                if(seen_alphanum && !strchr("#" "!&|~" "+-/*%()" "<=>" "[]{}" "?:;" ",'\"\\", *next_nbsp)) *s++ = ' ';

                while( *line > 0 && *line <= 32 ) ++line;

                --line;
                continue;
            }
            // save for later
            seen_alphanum = isalnum(*line);
            *s++ = *line;
        } else {
            while(*line == '\\' && line[1]) { memcpy(s, line, 2); s += 2; line += 2; }  // feature when commented out!!
            if(*line == in_string ) in_string = 0;

            seen_alphanum = isalnum(*line);
            *s++ = *line; // feature when commented out!!
        }
    }
#else
    for( ; *line; ++line ) {
        if( *line > 0 && *line <= 32 ) {
            // keep spacing only if previous char was alphanum && next char is not operator
            //const char *next_nbsp = line+1 + strspn(line+1, " \t\r\n");
            //if(seen_alphanum && !strchr("#" "!&|~" "+-/*%()" "<=>" "[]{}" "?:;" ",'\"\\", *next_nbsp)) *s++ = ' ';

            while( *line > 0 && *line <= 32 ) ++line;

            --line;
            continue;
        }
        *s++ = *line;
    }
#endif

*s = 0;
    return bak;
}

// reintroduce spacing and tabs where due, so code is pretty much readable again.
char *maxify() {
    // spaces after ';' . Ie, for(int i=0; i<10; ++i)
    // remove markdeep comments if present
    // if multiline, nested tabs {} scopes as much as needed.
    return 0;
}

// consume all chars within a syntax block.
// - within typedefs/structs/unions/variables, till enclosing `;` is found.
// - within functions(), till enclosing `{}` match is found.
bool syntax_block(array(char*) lines, int *row, int *col) {
    return true;
}

bool group_together() {
    // find relevant syntax blocks that belong together:
    // - they are a group if lines are sticked together, with no newlines.
    // - they are part of a group if both syntax blocks are sharing keywords on them.
    return true;
}
void print_markdeep(const char *line) {
    const char *text = line + 3 + (line[3] == ' ');
    const char *word = text + strspn(text, " \t");

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
void print_snippet(const char **lines) {

}

int main(int argc, char **argv) {
    // write header+intro
    bool do_css = atoi(option("--css", "1")); if(flag("-nocss")) do_css = 0;
    bool do_debug = atoi(option("--debug", "0")) | flag("-dbg");
    const char *excludes = option("--excluded-sections,--excluded,-x", "");

    if(do_css) system("type art\\docs\\docs.css"); // @fixme: non windows
    if(do_css) system("type art\\docs\\docs.md");  // @fixme: non windows

    // parse file. @fixme: remove all /*C comments*/ before doing any parsing
    const char *fname = argc > 1 ? argv[1] : __FILE__;
    char *raw = file_read(fname);

    preprocess(raw);
//    puts(raw); // hexdump(raw, strlen(raw)+1); //

    array(char*) split = strsplit(raw, "\r\n");
//    printf("split %d lines\n", (int)array_count(split));

//printf("%d\n",__LINE__);
    array(char*) file = 0;
    for(int i = 0; i < array_count(split); ++i) {
        char *line = split[i];
        line = minify(line);
        if(line[0]) array_push(file, line);
    }
    assert( array_count(file) > 0 );
//printf("%d\n",__LINE__);

    // DEBUG
    if(do_debug)
    for( int i = 0, end = array_count(file); i < end; ++i) fprintf(stdout, "L%d: %s\n", i, file[i]);

    int is_header_file = strstr(".h.H", file_ext(fname)) ? 1 : 0; // headers enabled by default. sources do not.
    char *section = STRDUP(file_name(fname));

    for( int i = 0, end = array_count(file); i < end; ++i) {
        char *line = file[i];

        bool is_macro = line[0] == '#';
        bool is_enum = strbegi(line, "enum ") || strbegi(line, "enum{");
        bool is_struct = strbegi(line, "struct ") || strbegi(line, "struct{");
        bool is_union = strbegi(line, "union ") || strbegi(line, "union{");
        bool is_block = is_enum || is_struct || is_union;
        bool is_variable = strstr(line, "extern ");
        bool is_function = !is_macro && strchr(line, '(') && strchr(line, '(') < strrchr(line, ')');
        bool is_typedef = !is_block && strbegi(line, "typedef ");

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
        // do not process if section is excluded from processing
        // else, process if needed
        if( section && excludes[0] ) {
            bool is_in_list = !!strstr(excludes, section);
            bool is_whole_list = !strcmp(excludes, section);
            if( is_whole_list || is_in_list ) {
                continue;
            }
        }
        if( section ) {
            // remove extension if present
            char *found = strrchr(section, '.'); if(found) *found = 0;
            // print section
            printf("## %s\n\n", section + (strbegi(section, "fwk_") ? 4 : 0));
            section = NULL;
        }

        // @todo: sort out display priorities

        // DEBUG
        // printf("L%d [%s]\n", i, line);

        if( is_variable || is_function || is_typedef ) {
            puts("\n~~~~~~C linenumbers");
            puts(line); // till ';'
            puts("~~~~~~\n");
            continue;
        }
        if( is_block ) {
            puts("\n~~~~~~C linenumbers");
            puts(line); // maxify
            puts("~~~~~~\n");
            continue;
        }

        if( 0 == strcmp(line, "/""//") ) {
            // separator
            puts("");
            continue;
        }
        else
        if( strbegi(line, "/""//" ) ) {
            // markdeep line
            //puts("");
            print_markdeep(line);
            continue;
        }
        else
        if( strstr(line, "/""//") ) {
            // inlined markdeep. @todo: print markdeep first, then code
            puts("");
            char *md = strstr(line, "/""//");
            if( md[3] ) print_markdeep(md); md[0] = 0;

            puts("\n~~~~~~C linenumbers");
            puts(line);
            puts("~~~~~~\n");
            continue;
        }
    }

    fprintf(stderr, "ok");
}


#line 1000 "docs.h"

/// **docs**
/// # about
/// a C documentation generator using markdeep. it works this way:
/// 1. source files are parsed
///   - ///markdeep comments are extracted and linked internally to the closest function. Ie, stored in a hashmap where key is the function name that is closer to the markdeep comment.
/// 1. then header files are parsed.
///   - ///markdeep comments, enums, structs, variables and typedefs are extracted and printed as-is.
///   - macros are extracted and printed only if requested with /// comment.
///   - functions are extracted and printed as-is. optionally, extra infos are printed only if there is an entry in the hashmap for them.
/// 1. explicit `symbols` in comments are converted into links to browse more easily.

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
#define my_macro_not_exposed()

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

