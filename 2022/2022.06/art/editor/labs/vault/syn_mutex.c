// # mutexes ###################################################################
// - rlyeh, public domain

#ifndef MUTEX_H
#define MUTEX_H
#include <stdbool.h>

typedef union mutex mutex;

void mutex_new(mutex *m);
void mutex_del(mutex *m);
bool mutex_trylock(mutex *m);
void mutex_acquire(mutex *m);
void mutex_release(mutex *m);

#endif

#ifdef MUTEX_C
#pragma once

union mutex {
    char opaque_win_x64[40];
    char opaque_cpp_x64_vs2015[8];
    char opaque_cpp_x64_vs2017[80];
    char opaque_cpp_x32_vs2017[48];
};

#if defined(__cplusplus)
#   include <mutex>
#   define MUTEX_API                "cpp"
#   define mutex_t                  std::recursive_mutex
#   define mutex_initial_(mutex)    (new (mutex) mutex_t)
#   define mutex_trylock_(mutex)    ((mutex)->try_lock())
#   define mutex_acquire_(mutex)    ((mutex)->lock())
#   define mutex_release_(mutex)    ((mutex)->unlock())
#   define mutex_destroy_(mutex)    ((mutex)->~recursive_mutex())
#elif defined(_WIN32)
//#   include <winsock2.h>
#   define MUTEX_API                "win"
#   define mutex_t                  CRITICAL_SECTION
#   define mutex_initial_(mutex)    (InitializeCriticalSection(mutex))
#   define mutex_trylock_(mutex)    (TryEnterCriticalSection(mutex))
#   define mutex_acquire_(mutex)    (EnterCriticalSection(mutex))
#   define mutex_release_(mutex)    (LeaveCriticalSection(mutex))
#   define mutex_destroy_(mutex)    (DeleteCriticalSection(mutex))
#else
#   define _GNU_SOURCE
#   include <pthread.h>
#   define MUTEX_API                "pth"
#   define mutex_t                  pthread_mutex_t
//# define mutex_initial_(mutex)    (pthread_mutex_init(mutex, NULL)
#   define mutex_trylock_(mutex)    (pthread_mutex_trylock(mutex))
#   define mutex_acquire_(mutex)    (pthread_mutex_lock(mutex))
#   define mutex_release_(mutex)    (pthread_mutex_unlock(mutex))
#   define mutex_destroy_(mutex)    (pthread_mutex_destroy(mutex))
static void mutex_initial_(pthread_mutex_t *mutex) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
#endif

typedef int mtx_is_large_enough[ sizeof(mutex) >= sizeof(mutex_t) ];

const char *mutex_api = MUTEX_API;

void mutex_new(mutex *m) {
    mutex v = {0}; *m = v;
    mutex_initial_((mutex_t*)m);
}
void mutex_del(mutex *m) {
    mutex_destroy_((mutex_t*)m);
}
bool mutex_trylock(mutex *m) {
    return !!mutex_trylock_((mutex_t*)m);
}
void mutex_acquire(mutex *m) {
    mutex_acquire_((mutex_t*)m);
}
void mutex_release(mutex *m) {
    mutex_release_((mutex_t*)m);
}

#endif

#if MUTEX_BENCH
#include <omp.h>
#include <stdio.h>
int main() {
    enum { N = 100, M = 1000 * 1000 };
    double t = omp_get_wtime();
    mutex m = {0};
    mutex_new(&m);
    for( int i = 0; i < N*M; ++i ) {
        mutex_acquire(&m);
        mutex_release(&m);
    }
    mutex_del(&m);
    t = omp_get_wtime() - t;
    printf("%5.2fs %6.2fM ops/s (api: %s)\n", t, ((2*N*M+2)/t) / M, mutex_api );
}
// 2.87s  69.63M ops/s (api: cpp)
// 2.03s  98.59M ops/s (api: win)
#endif

#if MUTEX_DEMO
#include <stdio.h>
#include <assert.h>
int main() {
    mutex m = {0};
    mutex_new(&m);

    mutex_acquire(&m);
    mutex_release(&m);
    assert( mutex_trylock(&m) );
    assert( mutex_trylock(&m) );
    mutex_release(&m);
    assert( puts("ok") >= 0 );
}
#endif
