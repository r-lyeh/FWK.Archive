// - rlyeh, public domain

#ifndef EXIT_H
#define EXIT_H

void os_exit(int rc, const char *msg);
#define os_exit(rc, ...) os_exit(rc, va("Error: " __VA_ARGS__))

#endif

#ifdef EXIT_C
#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

void (os_exit)(int rc, const char *msg) {
    fprintf(stderr, "%s\n", msg);

#ifdef _WIN32
    MessageBoxA(0,msg,0,0);
#endif

    exit(rc);
}

#endif
