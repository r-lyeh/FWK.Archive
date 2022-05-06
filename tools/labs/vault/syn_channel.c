// # channels #################################################################
// - rlyeh, public domain
//
// [src] based on cchan library by Máté Nagy (Public Domain)
//
// Go-style construct for inter-thread communication: each channel is a FIFO of
// fixed-length messages, which grows to make space for unread messages.
// - Multiple threads can write into one channel at once. Writes never block.
// - Multiple threads can read from one channel at once - but each message is
//   received only once. If there are multiple parallel readers, a random reader
//   will get each message.

#ifndef CHANNEL_H
#define CHANNEL_H

// - create a new channel
// - destroy channel (not thread-safe..)
// - put value in channel (never blocks)
// - get value from channel (return nonzero on success, doesn't block)
// - get value from channel (blocks until a value is available, or ms milliseconds have elapsed)
//   returns nonzero on success, zero on timeout

typedef struct channel channel;

channel* channel_init(int valuesize);
void     channel_quit(channel *chan);
void     channel_send(channel *chan, void *value);
int      channel_recv(channel *chan, void *output);
int      channel_wait(channel *chan, void *output, int ms);

#endif

#ifdef CHANNEL_C
#pragma once
#include <stdlib.h>
#include <string.h>
//#include "async_mutex.c"
//#include "async_condv.c"

struct channel {
    int alloc;   // allocated entries in *data
    int next;    // index of next written entry
    int first;   // index of first contained entry
    int used;    // number of used entries
    int size;    // size of one value
    char *data;  // queue data

    mutex mutex; // used to lock the channel
    condv condv; // used to signal availability of data
};

channel* channel_init(int valuesize) {
    channel* c = (channel*)REALLOC(0, sizeof (channel));
    if(!c) abort();

    c->size = valuesize;
    c->alloc = 1;
    c->next = 0;
    c->first = 0;
    c->used = 0;
    c->data = (char*)REALLOC(0, c->alloc * valuesize);
    if(!c->data) abort();

    mutex_new(&c->mutex);
    condv_init(&c->condv);

    return c;
}

void channel_quit(channel *chan) {
    mutex_del(&chan->mutex);
    condv_quit(&chan->condv);

    REALLOC(chan->data, 0);
    REALLOC(chan, 0);
}

void channel_send(channel *chan, void *value) {
    int i;

    mutex_acquire(&chan->mutex);

    if(chan->used == chan->alloc) {
        // need to expand channel buffer 

        i = chan->alloc; // save old buffer size 

        chan->alloc *= 2;

        chan->data = (char*)REALLOC(chan->data, chan->alloc * chan->size);
        if(!chan->data) {
            puts("error: no mem!");
            abort();
        }

        // move part of buffer from beginning until the first contained entry
        // after the previous end of the buffer
        // [34512] ->
        // [   12345   ]

        memcpy(chan->data + i * chan->size, chan->data, chan->first * chan->size);
        chan->next = i + chan->first;
    }

    memcpy(chan->data + chan->next * chan->size, value, chan->size);
    chan->next ++;
    if(chan->next == chan->alloc) chan->next = 0;
    chan->used ++;

    condv_emit(&chan->condv);

    mutex_release(&chan->mutex);
}

// get value from channel (return nonzero on success, doesn't block)
int channel_recv(channel *chan, void *output) {
    mutex_acquire(&chan->mutex);
    if(!chan->used) {
        mutex_release(&chan->mutex);
        return 0;
    }

    memcpy(output, chan->data + chan->first * chan->size, chan->size);

    chan->first ++;
    if(chan->first == chan->alloc) chan->first = 0;

    chan->used --;

    mutex_release(&chan->mutex);

    return 1;
}

// get value from channel (blocks until a value is available, or ms milliseconds have elapsed)
// returns nonzero on success, zero on timeout
int channel_wait(channel *chan, void *output, int ms) {
    mutex_acquire(&chan->mutex);

    if(chan->used) {
        memcpy(output, chan->data + chan->first * chan->size, chan->size);
        chan->first ++;
        if(chan->first == chan->alloc) chan->first = 0;
        chan->used --;
        mutex_release(&chan->mutex);
        return 1;
    }

    condv_wait(&chan->condv, &chan->mutex, ms);

    if(chan->used) {
        memcpy(output, chan->data + chan->first * chan->size, chan->size);
        chan->first ++;
        if(chan->first == chan->alloc) chan->first = 0;
        chan->used --;
        mutex_release(&chan->mutex);
        return 1;
    } else {
        mutex_release(&chan->mutex);
        return 0;
    }
}

#endif

#if CHANNEL_BENCH
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include "async_thread.c"
enum { N = 10, M = 1000 * 1000 };
void async_send( void *args ) {
    channel *c = (channel*)args;
    for( int i = 0; i < N*M; ++i ) {
        channel_send(c, &i);
    }
}
int main() {
    double t = omp_get_wtime();
    channel *c = channel_init( sizeof(int) );
    detach( async_send, c );
    for( int i = 0; i < N*M; ++i ) {
        int answer;
        channel_wait(c, &answer, 0);
        assert( answer == i );
    }
    channel_quit(c);
    t = omp_get_wtime() - t;
    int ops = (2*N*M)+2;
    printf("%5.2fs %5.2fM ops/s\n", t, (ops / t) / M);
}
#endif


#if CHANNEL_DEMO
#include <stdio.h>
void pstate(channel *chan) {
    int i, v;

    printf("----------------------- chan %p\n", (void *) chan);
    printf("alloc=%d next=%d first=%d used=%d\n", chan->alloc, chan->next, chan->first, chan->used);

    printf("[");
    for(i=0; i<chan->alloc; i++) {
        v = * (int *) (chan->data + i * chan->size);
        if(v >= 'a' && v <= 'z')
            printf("%c", v);
        else
            printf(".");
    }
    printf("]\n");

    printf("\n");
}
void precv(channel *chan) {
    int out;
    int res = channel_wait(chan, &out, 1000);

    if(!res) {
        printf("recv fail\n");
    } else {
        printf("recv = %c\n", out);
    }
}
int main() {
    channel *c1 = channel_init(sizeof(int));
    int v;

    pstate(c1);

    v = 'a'; channel_send(c1, &v);

    pstate(c1);

    v = 'b'; channel_send(c1, &v);
    v = 'c'; channel_send(c1, &v);

    pstate(c1);

    precv(c1);
    precv(c1);

    pstate(c1);

    v = 'd'; channel_send(c1, &v);
    v = 'e'; channel_send(c1, &v);
    v = 'f'; channel_send(c1, &v);

    pstate(c1);

    v = 'g';
    channel_send(c1, &v);

    pstate(c1);

    precv(c1);
    precv(c1);
    precv(c1);
    precv(c1);
    precv(c1);
    precv(c1); // should this fail

    channel_quit(c1);
}
#endif
