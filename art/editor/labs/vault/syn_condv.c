// # condition variables ######################################################
// - rlyeh, public domain

#ifndef CONDV_H
#define CONDV_H

typedef union condv {
    char opaque_cpp_x64_vs2013[16];
    char opaque_cpp_x64_vs2015[88];
    char opaque_cpp_x64_vs2017[88];
    char opaque_cpp_x86_vs2013[8];
    char opaque_cpp_x86_vs2015[48];
    char opaque_cpp_x86_vs2017[48];
    char opaque_win_x64_vs2013[8];
    char opaque_win_x64_vs2015[8];
    char opaque_win_x64_vs2017[8];
    char opaque_win_x86_vs2013[4];
    char opaque_win_x86_vs2015[4];
    char opaque_win_x86_vs2017[4];
} condv;

void condv_init( condv *c );
void condv_quit( condv *c );
void condv_emit( condv *c );
void condv_wait( condv *c, void *mutex, int ms );

#endif

#ifdef CONDV_C
#pragma once
//#include "async_mutex.c"

#if defined(__cplusplus)
#   include <chrono>
#   include <condition_variable>
#   include <mutex>
#   define CONDV_API                       "cpp"
#   define condv_t                          std::condition_variable_any
#   define condv_initial(condv)             (new (condv) condv_t)
#   define condv_destroy(condv)             ((condv)->~condition_variable_any())
#   define condv_signal(condv)              ((condv)->notify_one())
#   define condv_timedwait(condv,mt,ms)     ((condv)->wait_for(std::unique_lock<mutex_t>(*(mt)), std::chrono::milliseconds(ms)))
#elif defined(__TINYC__) // not supported
//#   include <winsock2.h>
#   define CONDV_API                       "tcc"
#   define condv_t                          int
#   define condv_initial(condv)             ((void)0)
#   define condv_destroy(condv)             ((void)0)
#   define condv_signal(condv)              ((void)0)
#   define condv_timedwait(condv,mt,ms)     ((void)0)
#elif defined(_WIN32)
//#   include <winsock2.h>
#   define CONDV_API                       "win"
#   define condv_t                          CONDITION_VARIABLE
#   define condv_initial(condv)             (InitializeConditionVariable((condv)))
#   define condv_destroy(condv)             ((void)0)
#   define condv_signal(condv)              (WakeConditionVariable((condv)))
#   define condv_timedwait(condv,mt,ms)     (SleepConditionVariableCS((condv), (PCRITICAL_SECTION)(mt), (ms) ? (ms) : INFINITE))
#else
#   include <pthread.h>
#   include <sys/time.h>
#   define CONDV_API                       "pth"
#   define condv_t                          pthread_cond_t
#   define condv_initial(condv)             (pthread_cond_init(condv, NULL))
#   define condv_destroy(condv)             (pthread_cond_destroy(condv))
#   define condv_signal(condv)              (pthread_cond_signal(condv))
//# define condv_timedwait(condv,mt,ms)     (pthread_cond_timedwait(condv,mt,ms))
static int condv_timedwait( condv_t *c, mutex *m, int ms ) {
    if( !ms ) {
        pthread_cond_wait(c, m);
        return 1;
    } else {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, NULL);

        timeout.tv_sec = now.tv_sec + ms / 1000;
        timeout.tv_nsec = (now.tv_usec + (ms % 1000) * 1000) * 1000;
        if(timeout.tv_nsec > 1000000000) {
            timeout.tv_nsec -= 1000000000;
            timeout.tv_sec ++;
        }
        return pthread_cond_timedwait(c, m, &timeout);
    }
}
#endif

typedef int condv__is_large_enough[ sizeof(condv) >= sizeof(condv_t) ];

const char *condv_api = CONDV_API;

void condv_init( condv *c ) {
    condv_initial((condv_t*)c);
}
void condv_quit( condv *c ) {
    condv_destroy((condv_t*)c);
}
void condv_emit( condv *c ) {
    condv_signal((condv_t*)c);
}
void condv_wait( condv *c, void *m, int ms ) {
    condv_timedwait( (condv_t*)c, (mutex*)m, ms );
}

#endif


#if CONDV_DEMO
#include <assert.h>
#include <stdio.h>
#include "async_thread.c"

int main() {

#ifdef _MSC_VER
    const char *msc = "vs-deprecated";
    /**/ if(_MSC_VER >= 1910) msc = "vs2017";
    else if(_MSC_VER >= 1900) msc = "vs2015";
    else if(_MSC_VER >= 1800) msc = "vs2013";
    else if(_MSC_VER >= 1700) msc = "vs2012";
    else if(_MSC_VER >= 1600) msc = "vs2010";
    else if(_MSC_VER >= 1500) msc = "vs2008";
    else if(_MSC_VER >= 1400) msc = "vs2005";
    else if(_MSC_VER >= 1310) msc = "vs2003";

#ifdef _WIN64
    printf("char opaque_%s_%s_%s[%d];\n", condv_api, "x64", msc, (int)sizeof(condv_t));
#else
    printf("char opaque_%s_%s_%s[%d];\n", condv_api, "x32", msc, (int)sizeof(condv_t));
#endif

#endif

    mutex m = {0};
    mutex_new( &m );

    condv c = {0};
    condv_init( &c );

    detach( condv_emit, &c );
    condv_wait( &c, &m, 1000 );

    condv_quit( &c );
    assert( puts("ok") >= 0 );
}
#endif
