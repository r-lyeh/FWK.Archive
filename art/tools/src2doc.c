/// # src2doc
//  - Tiny documentation generator for 60 programming languages (based on stddoc v1.0.1)
/// - rlyeh, public domain.
///
/// - Markdeep code comments are extracted from input and printed as a HTML file.
/// - Comments can be either `///comment`, `---comment` or `###comment`.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stddoc( FILE *in, FILE *out, FILE *css ) {
    for( int ch; (ch = fgetc(css)) != EOF; ) fputc(ch, out); fputc('\n', out);
    struct { int on, prev; } stack[256] = {0}, *quote = stack;
    for( char *buffer = (char *)malloc(16384); buffer; buffer = (free(buffer), 0))
    for( int line = 1; fgets( buffer, 16383, in ) && !feof(in) ; ++line ) {
        const char *tag = strstr(buffer, "//""/");
        tag = tag ? tag : strstr(buffer, "##""#");
        tag = tag ? tag : strstr(buffer, "--""-");
        char next = tag ? *(tag+=3) : 0;
        int markdeep = next == ' ' || next == '\r' || next == '\n', forgot_quote_end = markdeep && ( quote > stack );
        if( next == '!' ) fprintf( stderr, "Warning: (Line %d) %s", line, tag );         // warning comment
        if( next == '>' || forgot_quote_end ) if( quote > stack     ) { --quote, ++tag; if( quote  == stack ) fprintf( out, "%s\n", "~""~~\n</script>"); }
        if( next == '<'                     ) if( quote < stack+255 ) { ++quote, ++tag; fprintf( out, "%s", quote-1 == stack ? "<script type='preformatted'>\n~""~~" : "(...)"); }
        if( quote == stack+1 || markdeep ) {
            tag = tag ? tag : buffer;
            fprintf( out, "%s", tag + (tag[0] == ' ') );
        }
    }
    while( quote > stack ) { --quote; fprintf( out, "%s\n", "~""~~\n</script>" ); }
}

int main( int c, char **v ) {
    /**/ if( c == 2 ) return stddoc( fopen(v[1], "rb"), stdout, fopen("src2doc.css", "rb") ), 0;
    else if( c == 3 ) return stddoc( fopen(v[1], "rb"), fopen(v[2], "wb"), fopen("src2doc.css", "rb") ), 0;
    else if( c == 4 ) return stddoc( fopen(v[1], "rb"), fopen(v[2], "wb"), fopen(v[3], "rb") ), 0;
    else              return printf("Usage:\t%s input.src [output.html [file.css]]\n", v[0]);
}
