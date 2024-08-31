// # semaphores ################################################################
// combination of mutex+condvar. used to perform operations on shared data.
// - rlyeh, public domain.

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef struct semaphore semaphore;

void semaphore_init( semaphore *s );
void semaphore_wait( semaphore *s );
void semaphore_emit( semaphore *s );
void semaphore_quit( semaphore *s );

#endif

#ifdef SEMAPHORE_C
#pragma once
//#include "async_condv.c"
//#include "async_mutex.c"

struct semaphore {
    mutex mtx;
    condv cnd;
    int alarm;
};

void semaphore_init( semaphore *s ) {
    semaphore v = {0}; *s = v;
    mutex_new( &s->mtx );
    condv_init( &s->cnd );
}
void semaphore_wait( semaphore *s ) {
    mutex_acquire( &s->mtx );
    while( !s->alarm ) {
        condv_wait(&s->cnd, &s->mtx, 0);
    }
    s->alarm = 0;
    mutex_release(&s->mtx);
}
void semaphore_emit( semaphore *s ) {
    mutex_acquire(&s->mtx);
    s->alarm = 1;
    mutex_release(&s->mtx);
    condv_emit(&s->cnd);
}
void semaphore_quit( semaphore *s ) {
    condv_quit( &s->cnd );
    mutex_del( &s->mtx );
    semaphore v = {0}; *s = v;
}

#endif

#if SEMAPHORE_DEMO
#include <assert.h>
#include <stdio.h>
int main() {
    semaphore s = {0};
    semaphore_init(&s);
    semaphore_emit(&s);
    semaphore_quit(&s);
    assert( puts("ok") >= 0 );
}
#endif
