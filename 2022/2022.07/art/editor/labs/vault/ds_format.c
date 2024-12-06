// format library
// - rlyeh, public domain

#ifndef FORMAT_H
#define FORMAT_H
#include <stdarg.h>

char*           vl(const char *fmt, va_list);
char*           va(const char *fmt, ...);
int             is_va(const char *s);

#define va(...) \
    (printf || printf(__VA_ARGS__), va(__VA_ARGS__))  // vs2015 check va trick
#define vl(...) \
    (printf || vprintf(__VA_ARGS__), vl(__VA_ARGS__)) // this doesnt work afaik

#endif

// -----------------------------------------------------------------------------

#ifdef FORMAT_C
#pragma once

#if defined _MSC_VER && !defined __thread
#define __thread __declspec(thread)
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int is_va(const char *s) {
    return (1&(uintptr_t)s) && s[-1] == 0;
}
char* (vl)(const char *fmt, va_list vl) {
    va_list copy;
    va_copy(copy, vl);
    int sz = vsnprintf( 0, 0, fmt, copy ) + 1;
    va_end(copy);

    int reqlen = sz + 1; // 1 for even padding

    char* ptr;
    enum { STACK_ALLOC = 16384 };
    if( reqlen < STACK_ALLOC ) { // fit stack?
        static __thread char buf[STACK_ALLOC+1];
        static __thread int cur = 1, len = STACK_ALLOC;
        ptr = buf + ((cur+reqlen) > len ? cur = 1 : (cur += reqlen) - reqlen);
    } else { // else use heap (@fixme: single memleak per invoking thread)
        static __thread char *buf = 0;
        static __thread int cur = 1, len = -STACK_ALLOC;
        if( reqlen >= len ) buf = realloc(buf, len = abs(len) * 1.75 + reqlen);
        ptr = buf + ((cur+reqlen) > len ? cur = 1 : (cur += reqlen) - reqlen);
    }

    ptr += !(1&(uintptr_t)ptr); // align to even address only when already odd
    ptr[-1] = 0; // add header
    assert(is_va(ptr));

    vsnprintf( ptr, sz, fmt, vl );
    return (char *)ptr;
}
char* (va)(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    char *s = vl(fmt, list);
    va_end(list);
    return s;
}

// -----------------------------------------------------------------------------

#ifdef FORMAT_DEMO
#pragma once
int main() {
    char *x = va("hello %d", 123); 
    puts(x);}
#define main main__
#endif // FORMAT_DEMO
#endif // FORMAT_C
