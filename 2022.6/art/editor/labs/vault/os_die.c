#ifndef DIE_H
#define DIE_H

#define os_die(...) os_die(-__LINE__, __VA_ARGS__)
void (os_die)(int rc, const char *fmt, ...);

#endif

#ifdef DIE_C
#pragma once
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

void (os_die)(int rc, const char *fmt, ...) {
    fflush(stdout);
    // msg
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    // os
#ifdef _WIN32
    if( GetLastError() ) {
        LPSTR messageBuffer = 0;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        char buf[1024+1];
        size = size < 1024 ? size : 1024;
        memcpy( buf, messageBuffer, size );
        buf[size] = 0;

        LocalFree(messageBuffer);
        fprintf(stderr, " (%s)", buf);
    }
#endif
    // crt
    fprintf(stderr, " (%s)\r\n", strerror(errno));
    // bye
    exit(rc);
}

#ifdef DIE_DEMO
int main() {
    errno = ENOMEM;
    os_die("omg cant believe you");
}
#define main main__
#endif // DIE_DEMO
#endif // DIE_C
