// # thread-local storage ######################################################
// - rlyeh, public domain.

#ifndef TLS_H
#define TLS_H

void tls_add_init( void (*func)(void *arg), void *arg );
void tls_add_quit( void (*func)(void *arg), void *arg );
void tls_init();
void tls_quit();

#endif

#ifdef TLS_C
#pragma once

static struct tls {
    void (*funs[256])( void *arg );
    void *args[256];
    int count;
} inits = {0}, quits = {0};

void tls_add_init( void (*func)(void *arg), void *arg ) {
    int with = inits.count++;
    inits.funs[ with ] = func;
    inits.args[ with ] = arg;
}
void tls_add_quit( void (*func)(void *arg), void *arg ) {
    int with = quits.count++;
    quits.funs[ with ] = func;
    quits.args[ with ] = arg;
}
void tls_init() {
    for( int i = 0; i < inits.count; ++i ) {
        inits.funs[i]( inits.args[i] );
    }
}
void tls_quit() {
    for( int i = inits.count; i-- > 0; ) {
        quits.funs[i]( quits.args[i] );
    }
}

/*
void tlspush(void*);
void*tlspop();

__thread any argstack[32] = {0};
__thread void **argptr = argstack;
void tlspush(void *arg) {
    *argptr++ = arg;
}
void*tlspop() {
    return *(--argptr);
}
*/

#endif

#if TLS_DEMO
#include <stdio.h>
#include "async_thread.c"
#include "async_sleep.c"

void myinit1(void *arg) { puts("hi1!"); }
void myinit2(void *arg) { puts("hi2!"); }
void myquit2(void *arg) { puts("bye2!"); }
void myquit1(void *arg) { puts("bye1!"); }

void hello(void *arg) { puts("hello"); }

int main() {
    tls_add_init( myinit1, 0 );
    tls_add_quit( myquit1, 0 );
    tls_add_init( myinit2, 0 );
    tls_add_quit( myquit2, 0 );

    detach( hello, 0 );
    sleep_ss(1);

    thread( 10, hello, 0 );
    join(10);
}

#endif
