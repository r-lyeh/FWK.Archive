/*
  libco v18 (2016-09-14)
  author: byuu
  license: public domain
  libco is released to the public domain by John MacFarlane (2013-2016)
  http://byuu.org/library/libco/

## libco.x86
- Overhead: ~5x
- Supported processor(s): 32-bit x86
- Supported compiler(s): any
- Supported operating system(s):
  - Windows
  - Mac OS X
  - Linux
  - BSD

## libco.amd64
- Overhead: ~10x (Windows), ~6x (all other platforms)
- Supported processor(s): 64-bit amd64
- Supported compiler(s): any
- Supported operating system(s):
  - Windows
  - Mac OS X
  - Linux
  - BSD

## libco.ppc
- Overhead: ~20x
- Supported processor(s): 32-bit PowerPC, 64-bit PowerPC
- Supported compiler(s): GNU GCC
- Supported operating system(s):
  - Mac OS X
  - Linux
  - BSD
  - Playstation 3
- Note: this module contains compiler flags to enable/disable FPU and Altivec support.

## libco.fiber
- Overhead: ~15x
- Supported processor(s): Processor independent
- Supported compiler(s): any
- Supported operating system(s):
  - Windows

## libco.sjlj
- Overhead: ~30x
- Supported processor(s): Processor independent
- Supported compiler(s): any
- Supported operating system(s):
  - Mac OS X
  - Linux
  - BSD
  - Solaris

## libco.ucontext
- Overhead: ~300x (beware!!)
- Supported processor(s): Processor independent
- Supported compiler(s): any
- Supported operating system(s):
  - Linux
  - BSD
*/

#ifndef FIBER_H
#define FIBER_H

#if defined _MSC_VER || defined __TINYC__ // @r-lyeh
#define LIBCO_MP
#define LIBCO_MPROTECT
#ifndef __thread
#define __thread __declspec(thread)
#endif
#endif

#define fiber         cothread_t
#define fiber_active  co_active
#define fiber_create  co_create
#define fiber_delete  co_delete
#define fiber_switch  co_switch

// ---

typedef void* cothread_t;

cothread_t co_active();
cothread_t co_create(unsigned int, void (*)(void));
void co_delete(cothread_t);
void co_switch(cothread_t);

#endif // FIBER_H

// ---

#ifdef FIBER_C
#pragma once

/*[amd64, arm, ppc, x86]:
   by default, co_swap_function is marked as a text (code) section
   if not supported, uncomment the below line to use mprotect instead */
/* #define LIBCO_MPROTECT */

/*[amd64]:
   Win64 only: provides a substantial speed-up, but will thrash XMM regs
   do not use this unless you are certain your application won't use SSE */
/* #define LIBCO_NO_SSE */

#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wparentheses"
#endif

#if defined(__clang__) || defined(__GNUC__)
  #if defined(__i386__)
    #include "syn_fiber_x86.h"
  #elif defined(__amd64__)
    #include "syn_fiber_amd64.h"
  #elif defined(__arm__)
    #include "syn_fiber_arm.h"
  /* FIXME: co_create crashes on ppc64el */
  /*#elif defined(_ARCH_PPC)
    #include "syn_fiber_ppc.h" */
  #elif defined(_WIN32)
    #include "syn_fiber_win32.h"
  #else
    #include "syn_fiber_sjlj.h"
  #endif
#elif defined(_MSC_VER)
  #if defined(_M_IX86)
    #include "syn_fiber_x86.h"
  #elif defined(_M_AMD64)
    #include "syn_fiber_amd64.h"
  #else
    #include "syn_fiber_win32.h"
  #endif
#elif defined(__TINYC__)
    #include "syn_fiber_win32.h"
#else
  #error "fiber: libco: unsupported processor, compiler or operating system"
#endif

#ifdef FIBER_DEMO
#include <stdio.h>
#include <stdlib.h>
fiber self; // ~main_thread
void my_entry(void) {
    for(;;) {
        puts("1");
        fiber_switch(self);
        puts("2");
        fiber_switch(self);
    }
    exit(0); // unreachable
}
int main() {
    self = fiber_active();

    fiber coro = fiber_create(1*1024*1024, my_entry); // 1MiB; 64K instead?
    fiber_switch(coro);
    fiber_switch(coro);
    fiber_switch(coro);

    fiber_delete(coro);
}
#define main main__
#endif // FIBER_DEMO
#endif // FIBER_C
