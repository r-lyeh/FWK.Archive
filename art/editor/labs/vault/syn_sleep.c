// # sleep functions ###########################################################
// - rlyeh, public domain

#ifndef SLEEP_H
#define SLEEP_H

void sleep_ns( double ns );
void sleep_us( double us );
void sleep_ms( double ms );
void sleep_ss( double ss );

#endif

#ifdef SLEEP_C
#pragma once
#ifdef _WIN32
//#include <winsock2.h>
#else
#include <time.h>
#endif

void sleep_ns( double ns ) {
    if( ns > 0 ) {
#ifdef _WIN32
        LARGE_INTEGER li;      // Windows sleep in 100ns units
        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        li.QuadPart = (LONGLONG)(__int64)(-ns/100); // Negative for relative time
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#else
        struct timespec wait = {0};
        wait.tv_sec = ns / 1e9;
        wait.tv_nsec = ns - wait.tv_sec * 1e9;
        nanosleep(&wait, NULL);
#endif
    }
}
void sleep_us( double us ) {
    sleep_ns(us * 1e3);
}
void sleep_ms( double ms ) {
    sleep_ns(ms * 1e6);
}
void sleep_ss( double ss ) {
    sleep_ns(ss * 1e9);
}

#endif
