// - rlyeh, public domain

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdio.h>
#ifdef _MSC_VER
#include <omp.h>
#define benchmark(t,...) \
    for(double b_=omp_get_wtime(), e_=b_; e_==b_; t=(((e_=omp_get_wtime())-b_)*1.0), \
        (void)(!0["" #__VA_ARGS__] ? 0 : (printf("" __VA_ARGS__), printf(" (%.4fs)\n",t))))
#else
#include <time.h>
#define benchmark(t,...) \
    for(clock_t b_=clock(), e_=b_; e_==b_; t=(((e_=clock())-b_)*1.0/CLOCKS_PER_SEC), \
        (void)(!0["" #__VA_ARGS__] ? 0 : (printf("" __VA_ARGS__), printf(" (%.3fs)\n",t))))
#endif

#endif // BENCHMARK_H

#ifdef BENCHMARK_DEMO
#pragma once
int main() {
    double t;
    benchmark(t, "benchmark i/o, %d times", 1000) {
        for(int i = 0; i < 1000; ++i) {
            fclose(fopen(__FILE__,"rb"));
        }
    }
    benchmark(t) { // benchmark title (as seen above) is optional
        for(int i = 0; i < 1000; ++i) {
            fclose(fopen(__FILE__,"rb"));
        }
    }
}
#define main __main
#endif // BENCHMARK_DEMO
