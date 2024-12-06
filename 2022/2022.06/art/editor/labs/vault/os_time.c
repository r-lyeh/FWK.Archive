// [x] time: clock. @todo: sleep/yield, timer(tmr,fn,time)/cancel(tmr)/every(tmr,fn,time)
// - rlyeh, public domain

uint64_t time_ss();
uint64_t time_ms();
uint64_t time_us();
uint64_t time_ns();

// time -----------------------------------------------------------------------

#ifdef TIME_C
#pragma once
#define TIME_E3 1000ULL
#define TIME_E6 1000000ULL
#define TIME_E9 1000000000ULL

#ifdef                 CLOCK_MONOTONIC_RAW
#define TIME_MONOTONIC CLOCK_MONOTONIC_RAW
#elif defined          CLOCK_MONOTONIC
#define TIME_MONOTONIC CLOCK_MONOTONIC
#else
#define TIME_MONOTONIC CLOCK_REALTIME // untested
#endif

uint64_t time_ns() {
    uint64_t nanotimer;
    #if defined _WIN
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        nanotimer = (uint64_t)li.QuadPart;
    #elif defined _AND
        nanotimer = (uint64_t)clock();
    #elif defined TIME_MONOTONIC
        struct timespec ts;
        clock_gettime(TIME_MONOTONIC, &ts);
        nanotimer = (TIME_E9 * (uint64_t)ts.tv_sec) + ts.tv_nsec;
    #else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        nanotimer = (TIME_E6 * (uint64_t)tv.tv_sec) + tv.tv_usec;
    #endif

    static uint64_t epoch = 0, nanofreq = 0;
    if( !nanofreq ) {
        #if defined _WIN
            LARGE_INTEGER li;
            QueryPerformanceFrequency(&li);
            nanofreq = li.QuadPart;
        #elif defined _AND
            nanofreq = CLOCKS_PER_SEC;
        #elif defined TIME_MONOTONIC
            nanofreq = TIME_E9;
        #else
            nanofreq = TIME_E6;
        #endif
        epoch = nanotimer;
    }

    // [ref] https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
    // Computes (a*b)/c without overflow, as long as both (a*b) and the overall result fit into 64-bits.
    uint64_t a = nanotimer - epoch;
    uint64_t b = TIME_E9;
    uint64_t c = nanofreq;
    uint64_t q = a / c;
    uint64_t r = a % c;
    return q * b + r * b / c;
}
uint64_t time_us() {
    return time_ns() / TIME_E3;
}
uint64_t time_ms() {
    return time_ns() / TIME_E6;
}
uint64_t time_ss() {
    return time_ns() / TIME_E9;
}

#endif