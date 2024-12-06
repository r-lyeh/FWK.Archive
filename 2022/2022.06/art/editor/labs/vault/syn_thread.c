// # threads ###################################################################
// - rlyeh, public domain
//
// [ref] http://bisqwit.iki.fi/story/howto/openmp/
// [ref] https://github.com/tinycthread/tinycthread/
// [ref] https://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c#answer-12746081
// todo: affinities

#ifndef THREAD_H
#define THREAD_H
#include <stdbool.h>
#include <stdint.h>

uint64_t thread_self();
void thread_yield();
bool detach( void (*func)(void *), void *arg );
bool thread(int thread_id, void (*func)(void *), void *arg);
bool join(int thread_id);

#endif

#ifdef THREAD_C
#pragma once
//#include "c_ifdef.c"
//#include "async_tls.c"

#include <stdint.h>
#include <stdbool.h>
#ifdef _WIN32
//#   include <winsock2.h>
#   include <windows.h>  // SwitchToThread()
#   include <process.h>  // _getpid()
#   define THREAD_API "win"
#else
#   include <pthread.h>
#   include <sched.h>    // sched_yield()
#   include <sys/time.h> // timespec
#   include <time.h>     // nanosleep()
#   include <unistd.h>   // getpid()
#   if __linux__
#   include <sys/syscall.h>
#   endif
#   define THREAD_API "pth"
#endif
#if defined(__cplusplus)
#   include <thread>
#   undef  THREAD_API
#   define THREAD_API "cpp"
#endif

#ifndef THREAD_MAX
#define THREAD_MAX 1024
#endif 

const char *thdapi = THREAD_API;

uint64_t thread_self() {
    IFDEF(IOS,     return (mach_port_t)pthread_mach_thread_np( pthread_self() ) );
    IFDEF(OSX,     return (mach_port_t)pthread_mach_thread_np( pthread_self() ) );
    IFDEF(LINUX,   return (uint64_t)syscall(SYS_gettid) );
    IFDEF(WINDOWS, return GetCurrentThreadId() );
    IFDEF(ANDROID, return pthread_self() );
    // fallback
    static __thread int id;
    return (intptr_t)&id;
}

void thread_yield() {
    IFDEF(CPP, std::this_thread::yield(),
        IFDEF(WINDOWS, SwitchToThread(), // also, YieldProcessor(); Sleep(0); SleepEx(0, FALSE); _mm_pause();
            pthread_yield()       // also, sched_yield(); __asm__ __volatile__("pause" ::: "memory"); nanosleep();
        );
    );
}

#if defined(__cplusplus)
    static std::thread threads[THREAD_MAX];
    bool detach( void (*func)(void *), void *arg ) {
        return std::thread( [=]{ tls_init(); func(arg); tls_quit(); }).detach(), true;
    }
    bool thread(int thread_id, void (*func)(void *), void *arg) {
        return threads[thread_id] = std::thread( [&]{ tls_init(); func(arg); tls_quit(); } ), true;
    }
    bool join(int thread_id) {
        return threads[thread_id].join(), true;
    }
#else
    static void *threads[THREAD_MAX];
    typedef struct thread_args {
        void *func;
        void *args;
    } thread_args;
    #ifdef _WIN32
    static DWORD WINAPI thread_wrapper(LPVOID opaque) {
    #else
    static void *thread_wrapper( void *opaque ) {
    #endif
        void (*func)(void *) = ((thread_args *)opaque)->func;
        void *arg0 = ((thread_args *)opaque)->args;
        REALLOC( opaque, 0 );
        tls_init();
        func( arg0 );
        tls_quit();
        IFNDEF(WINDOWS, pthread_exit((void**)0) );
        return 0;
    }
    static void* launch( void (*func)(void *), void *args ) {
        thread_args *ta = (struct thread_args*)REALLOC( 0, sizeof(struct thread_args));
        ta->func = func;
        ta->args = args;
        IFDEF(WINDOWS, {
            return CreateThread(NULL, 0, thread_wrapper, (LPVOID)ta, 0, NULL);
        }, {
            void *ret;
            if( pthread_create(&ret, NULL, thread_wrapper, (void *)ta) != 0 ) {
                *ret = 0;
            }
            return ret;
        });
    }
    bool detach( void (*func)(void *), void *arg ) {
        void *thr = launch(func, arg);
        IFDEF(WINDOWS, return CloseHandle(thr) != 0 ? true : false,
            return pthread_detach(thr) == 0 ? true : false );
    }
    bool thread(int thread_id, void (*func)(void *), void *arg) {
        return !!(threads[thread_id] = launch(func, arg));
    }
    bool join(int thread_id) {
        IFDEF(WINDOWS, {
            if( WaitForSingleObject(threads[thread_id], INFINITE) != WAIT_FAILED ) {
                return (CloseHandle(threads[thread_id]), true);
            }
            return false;
        }, {
            void *nil;
            int err;
            do {
                err = pthread_join(threads[thread_id], &nil );
            } while( err == EINTR );
            return err == 0;
        } );
    }
#endif

#endif

#if THREAD_DEMO
#include <omp.h>
#include <stdio.h>
int writes = 0;
int N = 1024, M = 1024 * 1024;
void write( void *arg ) {
    for( int j = 0; j < M; ++j ) ++writes;
}
int main() {
    double t = omp_get_wtime();
    for( int id = 0; id < N; ++id ) {
        thread( id, write, &writes );
    }
    for( int id = 0; id < N; ++id ) {
        join( id );
    }
    t = omp_get_wtime() - t;
    printf("%5.2fs %6.2fM ops/s\n", t, ((N*M)/t) / M );
    printf("thread-id %lld, %5.2f%% concurrent-writes found\n", thread_self(), (writes*100.) / (N*M));
}
#endif
