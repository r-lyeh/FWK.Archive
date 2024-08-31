// # lockfree queues (multiple consumer-multiple producer) #####################
// License: WTFPL. https://github.com/darkautism/lfqueue
// Use -O0 flag to compile (needed?).

#ifndef MCMP_H
#define MCMP_H

struct mcmp;

int   mcmp_new(struct mcmp *ctx);
int   mcmp_del(struct mcmp *ctx);
int   mcmp_add(struct mcmp *ctx, void *data);
void *mcmp_pop(struct mcmp *ctx );

#endif

#ifdef MCMP_C
#pragma once
//#include "../memory/memory_realloc.c"
//#include "async_atomic.c"
#include <stdlib.h> 
#include <string.h>
#include <errno.h>

struct mcmp_node {
    void * data;
    struct mcmp_node *next;
};

struct mcmp {
    struct mcmp_node *head;
    struct mcmp_node *tail;
    size_t count; // int
};

int mcmp_new(struct mcmp *ctx) {
    struct mcmp_node * tmpnode = memset( (char*)REALLOC(0,sizeof(struct mcmp_node)), 0, sizeof(struct mcmp_node));
    if (!tmpnode) 
        return -errno;
    
    memset(ctx,0,sizeof(struct mcmp));
    ctx->head=ctx->tail=tmpnode;
    return 0;
}

int mcmp_del(struct mcmp *ctx){
    if ( ctx->tail && ctx->head ) { // if have data in queue
        struct mcmp_node * walker = ctx->head, *tmp;
        while ( walker != ctx->tail ) { // while still have node
            tmp = walker->next;
            REALLOC(walker, 0);
            walker=tmp;
        }
        REALLOC(ctx->head, 0); // free the empty node
        memset(ctx,0,sizeof(struct mcmp));
    }
    return 0;
}

int mcmp_add(struct mcmp *ctx, void * data) {
    struct mcmp_node * p;
    struct mcmp_node * tmpnode = memset( (char*)REALLOC(0,sizeof(struct mcmp_node)), 0, sizeof(struct mcmp_node));
    if (!tmpnode)
        return -errno;
    
    tmpnode->data=data;
    do {
        p = ctx->tail;
        if ( __sync_bool_compare_and_swap(&ctx->tail,p,tmpnode)) {
            p->next=tmpnode;
            break;  
        }
    } while(1);
    __sync_add_and_fetch( &ctx->count, 1);
    return 0;
}

void * mcmp_pop(struct mcmp *ctx ) {
    void * ret=0;
    struct mcmp_node * p;
    do {
        p = ctx->head;
    } while(p==0 || !__sync_bool_compare_and_swap(&ctx->head,p,0));
    
    if( p->next==0) {
        ctx->head=p;
        return 0;
    }
    ret=p->next->data;
    ctx->head=p->next;
    __sync_add_and_fetch( &ctx->count, -1);
    REALLOC(p, 0);
    return ret;
}

#ifdef MCMP_DEMO
#include <stdio.h>
#include <stdlib.h>
int main() {
    long ret;
    struct mcmp ctx;
    mcmp_new(&ctx);
    mcmp_add(&ctx,(void *)1);
    mcmp_add(&ctx,(void *)3);
    mcmp_add(&ctx,(void *)5);
    mcmp_add(&ctx,(void *)8);
    mcmp_add(&ctx,(void *)4);
    mcmp_add(&ctx,(void *)6);

    while ( (ret = *(int*)mcmp_pop(&ctx)) != 0 )
        printf("lfq_dequeue %ld\n", ret);

    mcmp_del(&ctx);
}
#define main main__
#endif // MCMP_DEMO
#endif // MCMP_C
