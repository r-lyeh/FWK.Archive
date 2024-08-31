// simple logger. likely slow, though.
// - rlyeh, public domain.
//
// - [x] colors based on content.
// - [x] no logging categories. throughput flood configurable by percentage (see LOG_LEVEL var).
// - [x] print fatal errors always. with optional callstack (see LOG_PRINT_STACK macro).

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#  if !defined LOG_LEVEL && (defined NDEBUG || defined SHIPPING)
#define LOG_LEVEL 0   //   0% - no messages logged (only fatal errors will)
#elif !defined LOG_LEVEL
#define LOG_LEVEL 100 // 100% - all messages logged
#endif

#ifndef LOG_TIMER
#define LOG_TIMER() 0.0 /* user-defined apptime clock here */
#endif

#ifndef LOG_PRINT_STACK
#define LOG_PRINT_STACK(f) ftrace(f,+16) /* user-defined callstack printer here */
#endif

#  if !defined LOG_ENABLE_ANSI && defined _WIN32
// #include <winsock2.h>
#define LOG_ENABLE_ANSI() os_ansi() // for(unsigned mode=0;!mode;mode=1) SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4))
#elif !defined LOG_ENABLE_ANSI
#define LOG_ENABLE_ANSI()
#endif

#ifdef _MSC_VER
#define LOG_DIRCHR "\\"
#else
#define LOG_DIRCHR "/"
#endif

#define LOGCOL(...) do { \
    static char must_log = -1; \
    static enum { undefined=-1, restore=0, r=31,g,y,b,p,t,w,inv=0x80 } color = undefined; \
    char buf[2048]; snprintf(buf, 2048, "" __VA_ARGS__); \
    if( color == undefined ) { \
        LOG_ENABLE_ANSI(); \
        char low[2048]; int i; for(i=0;buf[i];++i) low[i] = 32|buf[i]; low[i]='\0'; \
        /**/ if( strstr(low,"fatal")|| strstr(low,"panic") || strstr(low,"assert") ) color=r|inv,must_log=1; \
        else if( strstr(low,"fail") || strstr(low,"error") ) color=r; \
        else if( strstr(low,"warn") || strstr(low,"alert") ) color=y; /*beware,caution*/ \
        else if( strstr(low,"info") || strstr(low,"succe") ) color=g; /*ok, no error*/ \
        else if( strstr(low,"debug") ) color=t; \
        else if( strstr(low,"trace") ) color=p; else color=restore; \
        if( must_log < 0 ) { /* original splitmix64 by Sebastiano Vigna (CC0)*/ \
            uint64_t z = (__LINE__ + __COUNTER__ + UINT64_C(0x9E3779B97F4A7C15)); \
            z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9); \
            must_log = (unsigned)((z ^ (z >> 31)) % 100) < LOG_LEVEL; } } \
    if( must_log ) { \
        double timer = LOG_TIMER(); \
        const char *short_file = strrchr(LOG_DIRCHR __FILE__, 0[LOG_DIRCHR]) + 1; \
        if(color&0x80) fprintf(stderr, "\033[7m"); \
        if(timer>0)fprintf(stderr, "\033[%dm%07.3fs %s (%s:%d)", color&0x7f, timer, &buf[buf[0]=='!'], short_file, __LINE__); \
        else       fprintf(stderr, "\033[%dm%s (%s:%d)", color&0x7f, &buf[buf[0]=='!'], short_file, __LINE__); \
        fprintf(stderr, "\033[%dm\n", restore); \
        if(color&inv || buf[0] == '!') LOG_PRINT_STACK(stderr); \
        log_puts(buf); /* <-- optional, only if logger2 is included */ } \
} while(0)

#ifdef LOGGER3_C
#pragma once
#ifdef LOGGER3_DEMO
int main() {
    LOGCOL("Test 1 - normal message. lines starting with '!' will print %s", "callstack");
    LOGCOL("Test 2 - trace message");
    LOGCOL("Test 3 - debug message");
    LOGCOL("Test 4 - info message");
    LOGCOL("Test 5 - warning message");
    LOGCOL("Test 6 - error message");
    LOGCOL("Test 7 - fatal error message (fatal errors are always printed, despite LOG_LEVEL var)");
}
#define main main__
#endif // LOGGER3_DEMO
#endif // LOGGER3_C
