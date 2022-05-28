   int cpu_pid(void);
   int cpu_cores(void);
double cpu_usage(void);


#ifdef CPU_DEMO
#pragma once
#ifdef _WIN32
#include <windows.h>
static ULONGLONG cpu_diff_time_(const FILETIME one, const FILETIME two) {
    LARGE_INTEGER a, b;
    a.LowPart = one.dwLowDateTime;
    a.HighPart = one.dwHighDateTime;

    b.LowPart = two.dwLowDateTime;
    b.HighPart = two.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}
#elif defined __linux__
#include <sched.h>
#else
#include <unistd.h>
#endif

// CPU currently used by current process:
// src: https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
// src: https://stackoverflow.com/questions/23143693/retrieving-cpu-load-percent-total-in-windows-with-c

double cpu_usage(void) {
#ifndef _WIN32
    return 0;
#else
    static double last_ok = 0;
    static FILETIME prevSysIdle, prevSysKernel, prevSysUser;

    FILETIME sysIdle, sysKernel, sysUser;
    // sysKernel include IdleTime
    if (GetSystemTimes(&sysIdle, &sysKernel, &sysUser) == 0) { // GetSystemTimes func FAILED return value is zero;
        return last_ok;
    }

    if (prevSysIdle.dwLowDateTime != 0 && prevSysIdle.dwHighDateTime != 0) {
        ULONGLONG sysIdleDiff, sysKernelDiff, sysUserDiff;
        sysIdleDiff = cpu_diff_time_(sysIdle, prevSysIdle);
        sysKernelDiff = cpu_diff_time_(sysKernel, prevSysKernel);
        sysUserDiff = cpu_diff_time_(sysUser, prevSysUser);

        ULONGLONG sysTotal = sysKernelDiff + sysUserDiff;
        ULONGLONG kernelTotal = sysKernelDiff - sysIdleDiff; // kernelTime - IdleTime = kernelTime, because sysKernel include IdleTime

        if (sysTotal > 0) { // sometimes kernelTime > idleTime
            last_ok = (double)(((kernelTotal + sysUserDiff) * 100.0) / sysTotal);
        }
    }

    prevSysIdle = sysIdle;
    prevSysKernel = sysKernel;
    prevSysUser = sysUser;

    return last_ok;
#endif
}

int cpu_cores(void) {
#if defined __cplusplus
    return (int)std::thread::hardware_concurrency();
#elif defined _WIN32
    DWORD_PTR pm, sm;
    if( GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) ) if( pm ) {
        int count = 0;
        while( pm ) {
            ++count;
            pm &= pm - 1;
        }
        return count;
    }
    { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
#elif defined __linux__
    cpu_set_t prevmask, testmask;
    CPU_ZERO(&prevmask);
    CPU_ZERO(&testmask);
    sched_getaffinity(0, sizeof(prevmask), &prevmask);     //Get current mask
    sched_setaffinity(0, sizeof(testmask), &testmask);     //Set zero mask
    sched_getaffinity(0, sizeof(testmask), &testmask);     //Get mask for all CPUs
    sched_setaffinity(0, sizeof(prevmask), &prevmask);     //Reset current mask
    int num = CPU_COUNT(&testmask);
    return (num > 1 ? num : 1);
#elif defined __unix__
    // unix
    int count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? count : 1;
#else
    // omp
    int cores = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        ++cores;
    }
    return cores;
#endif
}

int cpu_pid(void) {
#ifdef _WIN32
    return _getpid();
#else
    return getpid();
#endif
}

#ifdef CPU_DEMO
#include <stdio.h>
int main() {
    printf("process-id: %d\n", cpu_pid());
    printf("%d cpus\n", cpu_cores());

    for( int i = 0; i < 100000000; ++i) { float f = sqrt(1); }
    printf("%5.2f%% usage\n", cpu_usage());
}
#define main main__
#endif // CPU_DEMO
#endif // CPU_C
