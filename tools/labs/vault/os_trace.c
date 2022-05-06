// [x] hexdump
// [x] backtraces
// - rlyeh, public domain

// ----------------------------------------------------------------------------

#ifndef HEXDUMP_H
#define HEXDUMP_H

static
void hexdump_compact( const void *ptr, int len ) {
    char hexbuf[256] = {0}, strbuf[256] = {0}, *data = (char*)ptr, width = 16;
    for( int jt = 0; jt < len; jt += width ) {
        char *hex = hexbuf, *str = strbuf;
#if 0
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", /*"0x%02x,",*/ (unsigned char)*data);
            str += sprintf( str, "%c", *data >= 32 && *data != '\\' ? *data : '.' );
        }
        /*LOGINFO(,*/printf("%06x %s%*.s ; %-16s\n", jt, hexbuf, (int)(width * 3 /*5*/ - (hex - hexbuf)), hexbuf, strbuf);
#endif
#if 0
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", /*"0x%02x,",*/ (unsigned char)*data);
            str += sprintf( str, " %c ", /*"0x%02x,",*/ *data >= 32 && *data != '\\' ? *data : '.');
        }
        /*LOGINFO(,*/printf("%06x %s\n       %s\n", jt, hexbuf, strbuf);
#endif
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", /*"0x%02x,",*/ (unsigned char)*data);
            str += sprintf( str, "%c", /*"0x%02x,",*/ *data >= 32 && *data != '\\' ? *data : '.');
        }
        /*LOGINFO(,*/printf("%06x %-*s%s\n", jt, width*3, hexbuf, strbuf);
    }
}

#define hexdump(ptr, len) hexdump(#ptr, ptr, len)
static void (hexdump)( const char *title, const void *ptr, int len ) {
    /*LOGINFO(,*/printf("--- [%p] %d bytes -- %s\n", ptr, len, title );
    hexdump_compact(ptr, len);
}

#endif // HEXDUMP_H

// ----------------------------------------------------------------------------

// ## debug: backtraces
// Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// If you are linking your binary using GNU ld you need to add --export-dynamic
// - rlyeh, public domain.

int trace( char **buffer, int traces );
int ftrace( FILE *fp, int traces );

#ifdef TRACE_C
#pragma once

#if defined(__APPLE__)
#include <execinfo.h> // backtrace, backtrace_symbols
#include <dlfcn.h>    // dladdr, Dl_info
#elif defined(__GNUC__)
#include <execinfo.h>  // backtrace, backtrace_symbols
#elif defined(_WIN32) // && !defined __TINYC__
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) { //$
    static HANDLE process = 0;
    if( !process ) process = GetCurrentProcess();
    if( !process ) exit( puts( "error: no current process" ) );
    static int init = 0;
    if( !init ) init = SymSetOptions(SYMOPT_UNDNAME), SymInitialize( process, NULL, TRUE );
    if( !init ) exit( puts( "error: cannot initialize DbgHelp.lib" ) );

    typedef USHORT (WINAPI *pFN)(); // TINYC
    //typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
    static pFN rtlCaptureStackBackTrace = 0;
    if( !rtlCaptureStackBackTrace ) {
        rtlCaptureStackBackTrace = (pFN)GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace");
    }
    if( !rtlCaptureStackBackTrace ) {
        return 0;
    }
    return rtlCaptureStackBackTrace(1, maxtraces, (PVOID *)addr, (DWORD *) 0);
}
static char **backtrace_symbols(void *const *array,int size) { //$
    HANDLE process = GetCurrentProcess();
    enum { MAXSYMBOLNAME = 512 - sizeof(IMAGEHLP_SYMBOL64) };
    char symbol64_buf     [ 512 ];
    char symbol64_bufblank[ 512 ] = {0};
    IMAGEHLP_SYMBOL64 *symbol64       = (IMAGEHLP_SYMBOL64*)symbol64_buf;
    IMAGEHLP_SYMBOL64 *symbol64_blank = (IMAGEHLP_SYMBOL64*)symbol64_bufblank;
    symbol64_blank->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    symbol64_blank->MaxNameLength = (MAXSYMBOLNAME-1) / 2; //wchar?

    int symlen = size * sizeof(char *);
    char **symbols = (char **)MALLOC(symlen);

    if( symbols ) {
        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = NULL;
        }

        char begin[1024];
        for( int i = 0; i < size; ++i ) {
            char **symbuf, *buffer = begin;

            DWORD64 dw1 = 0, dw2 = 0;
            *symbol64 = *symbol64_blank;
            if( SymGetSymFromAddr64( process, (DWORD64)array[i], &dw1, symbol64 ) ) {
                IMAGEHLP_LINE64 l64 = {0};
                l64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                if( SymGetLineFromAddr64( process, (DWORD64)array[i], (DWORD*)&dw2, &l64 ) ) {
                    // int lenbase( const char *str );
                    int base = 0; // lenbase( l64.FileName );
                    buffer += sprintf(buffer,"%s (%s:%d)%c", symbol64->Name, &l64.FileName[ base ], l64.LineNumber, 0);
                } else {
                    buffer += sprintf(buffer,"%s (??)%c", symbol64->Name, 0);
                }
            } else  buffer += sprintf(buffer,"(??)%c", 0);

            size_t buflen = buffer - begin + 1;

            symbuf = (char **)REALLOC( symbols, symlen + buflen );
            if( symbuf ) {
                memcpy( (char *)symbuf + symlen, begin, buflen );
                symbuf[ i ] = (char *)(size_t)symlen;

                symbols = symbuf;
                symlen += buflen;
            } else break;
        }

        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = (char *)symbols + (size_t)symbols[ i ];
        }
    }

    return symbols;
}
#else
static int backtrace(void **heap, int num) { return 0; }
static char **backtrace_symbols(void *const *sym,int num) { return 0; }
#endif

void trace_cb( int traces, int (*yield)(const char *)) { //$
    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 128 };

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return;
    if( traces > maxtraces ) traces = maxtraces;

    void *stack[ maxtraces ];
    traces = backtrace( stack, traces );
    char **symbols = backtrace_symbols( stack, traces );

    char demangled[1024], buf[1024];
    int L = 0, B = inc>0 ? skip - 1 : traces, E = inc>0 ? traces : skip - 1;
    for( int i = B; ( i += inc ) != E; ) {
#ifdef __linux__
        char *address = strstr( symbols[i], "[" ) + 1; address[strlen(address) - 1] = '\0';
        char *binary = symbols[i]; strstr( symbols[i], "(" )[0] = '\0';
        char command[1024]; sprintf(command, "addr2line -e %s %s", binary, address);
        FILE *fp = popen( command, "rb" );
        if( !fp ) {
            exit( puts( "cannot invoke 'addr2line'" ) );
        }
        char *line_p = fgets(demangled, sizeof(demangled), fp);
        symbols[i] = demangled;
        pclose(fp);
#elif __APPLE__
        struct Dl_info info;
        if( dladdr(stack[i], &info) && info.dli_sname ) {
            char *dmgbuf = info.dli_sname[0] != '_' ? NULL :
                 __cxa_demangle(info.dli_sname, NULL, 0, NULL);
            strcpy( demangled, dmgbuf ? dmgbuf : info.dli_sname );
            symbols[i] = demangled;
            FREE( dmgbuf );
        }
#endif
        //sprintf(buf, "%3d %#016p %s", ++L, stack[i], symbols[i]);
        sprintf(buf, "%3d %s", ++L, symbols[i]);
        if( yield(buf) < 0 ) break;
    }

    FREE( symbols );
}

static __thread char *trace_strbuf[128] = {0};
static __thread int trace_counter = 0, trace_len = 0;
int trace_(const char *line) {
    int len = strlen(line);
    trace_len += len + 1;
    trace_strbuf[trace_counter] = (char*)REALLOC(trace_strbuf[trace_counter], (len * 1.5));
    strcpy(trace_strbuf[trace_counter], line );
    trace_counter = (trace_counter +1) % 128;
    return 1;
}
int trace( char **buf, int traces ) { //$
    //if( trace_ ) trace_str_ = REALLOC(trace_str_, trace_lenbuf_ = 0);
    trace_counter = trace_len = 0;
    trace_cb( traces, trace_ );
    *buf = (char*)REALLOC( *buf, trace_len + 1 ); (*buf)[0] = 0; 
    for( int i = 0; i < trace_counter; ++i ) {
        strcat(*buf, trace_strbuf[i] ); // <-- optimize
        strcat(*buf, "\n");
    }
    return 0;
}
int ftrace( FILE *fp, int traces ) { //$
    char *buf = 0;
    trace(&buf, traces);
    fputs(buf, fp);
    FREE(buf);
    return 0;
}

#ifdef TRACE_DEMO
#pragma once
#include <stdio.h>
void B_func() { 
    trace_cb(-64, puts);
    puts("--");
    trace_cb(+64, puts);

    char *cs = 0;
    trace(&cs, +16);
    puts( cs ? cs : "" );
    free( cs );

    ftrace(stdout, +16);
}
void A_func() {
    B_func();
}
int main() {
    A_func();
}
#define main main__
#endif // TRACE_DEMO
#endif // TRACE_C
