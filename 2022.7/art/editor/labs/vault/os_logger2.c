#ifndef LOGGER2_H
#define LOGGER2_H
#include <stdio.h>

// create logger with given buffer size
void log_create( FILE *fp, int bufsz );

// append line to all registered logs
int log_puts( const char *msg, ... );

// utils
char *log_stamp(void); // -> "2020-03-17 20.15.41 Tue"

#endif

#ifdef LOGGER2_C
#pragma once
#include <string.h>
#include <time.h>
#include <stdarg.h>

static FILE *loggers[32] = {0};

char *log_stamp(void) {
    static __thread char date[64];
    time_t rawtime; time( &rawtime );
    struct tm *ti = localtime( &rawtime );
    sprintf(date, "%04d-%02d-%02d %02d.%02d.%02d %.3s",
        1900 + ti->tm_year,
        ti->tm_mon + 1, 
        ti->tm_mday,
        ti->tm_hour, ti->tm_min, ti->tm_sec,
        "SunMonTueWedThuFriSat"+3*ti->tm_wday
    );
    return date;
}

void log_create( FILE *fp, int bufsz ) {
    // append logger for broadcasting
    for( int i = 0; i < 32; ++i) {
        if( !loggers[i] ) { loggers[i] = fp; break; }
    }
    // write header
    fprintf(fp, "--""- New session [built %s %s]\n", __DATE__, __TIME__);
    fflush(fp);
    // release builds might flush logs every 16KiB; any other build could flush every line
    setvbuf(fp, NULL, _IOFBF, bufsz > 2 ? bufsz : 2);
    fprintf(fp, "%s\n", log_stamp());
}

int log_puts( const char *msg, ... ) {
    va_list vl;
    va_start(vl, msg);
    for( int i = 0; i < 32; ++i ) {
        FILE *fp = loggers[i];
        if( !fp ) break;
        vfprintf(fp, msg, vl);
        fprintf(fp, "\n");
    }
    va_end(vl);
    return !feof(stdout);
}

#ifdef LOGGER2_DEMO
int main() {
    char logname[128]; sprintf(logname, "log %s.txt", log_stamp());

    log_create( stdout, 16384 );
    log_create( fopen(logname,"a+b"), 16384 ); // append mode
    log_create( fopen("log-latest.txt","w+b"), 16384 ); // create

    log_puts("hello %d", 123);
    log_puts("hello %s", log_stamp());
}
#define main __main
#endif // LOGGER_DEMO
#endif // LOGGER_C
