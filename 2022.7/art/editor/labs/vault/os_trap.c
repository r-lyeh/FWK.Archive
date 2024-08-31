void os_trap( void(*handler)(int) );
void os_crash(void);


#ifdef TRAP_C
#pragma once
#include <float.h> // _control87
#include <signal.h>
#include <stdio.h>
void (*os_signal)(int);
void os_signal_(int handler) {
    // reload signal
    // signal(handler, os_signal_);
    // report
    if( os_signal ) {
        os_signal(handler);
    } else {
        const char *signame = "?";
        /**/ if( handler == SIGABRT ) signame = "SIGABRT";
        else if( handler == SIGFPE )  signame = "SIGFPE";
        else if( handler == SIGILL )  signame = "SIGILL";
        else if( handler == SIGINT )  signame = "SIGINT";
        else if( handler == SIGSEGV ) signame = "SIGSEGV";
        else if( handler == SIGTERM ) signame = "SIGTERM";
        // panic("!exception caught, signal: %s (%d)\n", signame, handler);
        fprintf(stderr, "exception caught, signal: %s (%d)\n", signame, handler);
        exit(-1);
    }
}
void os_trap( void(*handler)(int) ) {
#ifdef _MSC_VER
    _control87(0, _MCW_EM);      // Unmask all FPE
#endif
    signal(SIGABRT, os_signal_); // Abnormal termination
    signal(SIGFPE, os_signal_);  // Floating-point error
    signal(SIGILL, os_signal_);  // Illegal instruction
    signal(SIGINT, os_signal_);  // CTRL+C signal
    signal(SIGSEGV, os_signal_); // Illegal storage access
    signal(SIGTERM, os_signal_); // Termination request
    os_signal = handler;
}

void os_crash(void) {
    int *p = 0;
    *p = 42;
}

#ifdef TRAP_DEMO
int main() {
    os_trap(NULL);

    printf("signal? [1..6]: ");
    int test;
    if( scanf("%d", &test) ) {
        if( test == 1 ) { abort(); } // test SIGABRT
        if( test == 2 ) { float a = 0; printf("%f\n", 10/a); } // test SIGFPE
        if( test == 3 ) { raise(SIGILL); } // test SIGILL
        if( test == 4 ) { puts("ctrl-c to signal"); for(;;) {} } // test SIGINT
        if( test == 5 ) { char *p = 0; *p = 42; } // test SIGSEGV
        if( test == 6 ) { raise(SIGTERM); } // test SIGTERM
    }
}
#define main main__
#endif // TRAP_DEMO
#endif // TRAP_C


#if 0

void os_crash(void);
void os_trap(void);

#ifdef TRAP_C
#pragma once

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifndef _WIN32
#include <sys/resource.h> // setrlimit()
#else
#include <winsock2.h>
#endif

static void catch_signal( int sig ) {
    signal(sig, catch_signal); // reset for next signal

    /**/ if(sig == SIGTERM) panic("!catch_signal(SIGTERM): A termination request was sent to the program");
    else if(sig == SIGABRT) panic("!catch_signal(SIGABRT): Usually caused by an abort() or assert()");
    else if(sig == SIGSEGV) panic("!catch_signal(SIGSEGV): Segmentation Fault");
    else if(sig == SIGINT ) panic("!catch_signal(SIGINT): Interactive attention signal (usually ctrl+c)");
    else if(sig == SIGFPE ) panic("!catch_signal(SIGFPE): Arithmetic Exception");
    else if(sig == SIGILL ) panic("!catch_signal(SIGILL): Illegal Instruction");
    else                    panic("!catch_signal(): Unknown exception caught.");
}

#if defined _MSC_VER && defined __cplusplus
#   include <winsock2.h> // windows.h alternative
#   include <eh.h>       // _set_se_translator
static void _cdecl catch_se( unsigned int ex, EXCEPTION_POINTERS *p ) {
    /**/ if(ex == EXCEPTION_ACCESS_VIOLATION)    panic("!catch_se(EXCEPTION_ACCESS_VIOLATION): Memory access violation");
    else if(ex == EXCEPTION_ILLEGAL_INSTRUCTION) panic("!catch_se(EXCEPTION_ILLEGAL_INSTRUCTION): Illegal instruction");
    else if(ex == EXCEPTION_INT_DIVIDE_BY_ZERO)  panic("!catch_se(EXCEPTION_INT_DIVIDE_BY_ZERO): Integer divide by zero");
    else if(ex == EXCEPTION_STACK_OVERFLOW)      panic("!catch_se(EXCEPTION_STACK_OVERFLOW): Stack overflow");
    else                                         panic("!catch_se(): Unknown exception");
}
#endif

#if 0 // ndef __TINYC__
#include <fenv.h>
#ifdef _MSC_VER
#pragma fenv_access(on)
#else
#pragma STDC FENV_ACCESS ON
#endif
void os_trap_fpe() {
    // ignore fp exceptions
    static fenv_t fenv = {0};
    feholdexcept( &fenv );
    // config fp rounding mode to chop (round towards zero); easier to emulate in software than round-to-nearest.
    fesetround(FE_TOWARDZERO);
}
#endif

void os_trap(void) { // @todo: TRAP_C, TRAP_CPP, TRAP_FPE flags?
    // install signal handlers
    const int signals[] = {
    // The C standard defines following 6 signals:
        SIGABRT,      // abort; abnormal termination.
        SIGFPE,       // floating point exception.
        SIGILL,       // illegal; invalid instruction.
        SIGSEGV,      // segmentation violation; invalid memory access.
        SIGTERM,      // terminate; termination request sent to the program.
//      SIGINT,       // interrupt; interactive attention request sent to the program. SHIPPING ONLY?
    // OS specifics:
    #ifdef __APPLE__ // _OSX || _IOS
        SIGPIPE,
    #endif
    #ifdef _WIN32
        SIGBREAK,
    #else
        SIGBUS,       // Bus error
        SIGHUP,       // sent to a process when its controlling terminal is closed
        //SIGQUIT,    //
        //SIGKILL,    // kernel will let go of the process without informing the process of it
        //SIGTRAP,    // debugger
        //SIGSYS,
    #endif
    };
    for( int i = 0; i < sizeof(signals)/sizeof(signals[0]); ++i ) {
        //signal(signals[i], SIG_IGN); //catch_signal);
        if (signal(signals[i], catch_signal) == SIG_ERR) {
            panic("!An error occurred while setting a signal handler.");
        }
    }
#ifdef __cplusplus
    set_terminate(catch_signal); // C++11
    #ifdef _MSC_VER // _WIN32
    _set_se_translator(catch_se);
    #endif
#endif
#if 0 // ndef _WIN32
    // enable coredumps
    rlimit core_limit = { RLIM_INFINITY, RLIM_INFINITY };
    setrlimit( RLIMIT_CORE, &core_limit );
#endif
#if 0 // ndef _WIN32
    // daemon + unstopable + no zombies children (= no wait())
    #ifndef SIGCLD
    #define SIGCLD SIGCHLD
    #endif
    signal(SIGCLD, SIG_IGN); // ignore child death
    signal(SIGHUP, SIG_IGN); // ignore terminal hangups
    setpgrp();               // break away from process group
#endif
}

void os_crash(void) {
    int *p = 0;
    *p = 42;
}


#ifdef TRAP_DEMO
int main() {
    os_trap();
    os_crash();
}
#define main main__
#endif // TRAP_DEMO
#endif // TRAP_C

#endif
