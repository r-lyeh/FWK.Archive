#ifndef ROPE_H
#define ROPE_H

typedef struct rope rope;

void rope_init( rope *s, int reserve, float grow );
void rope_append( rope *s, const char *buf, int len );
void rope_dump( rope *s );
int rope_len( rope *s );

#endif

#ifdef ROPE_C
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct rope {
    struct rope *root;
    struct rope *next;
    int len, cap;
    float grow;
    unsigned char *data;
    // unsigned char data[rope_BLOCKSIZE - sizeof(rope*)*2 - sizeof(int)*2 )];
};

void rope_init( rope *s, int reserve, float grow ) {
    memset(s, 0, sizeof(rope));
    s->grow = grow;
    s->data = realloc( s->data, (s->len = 0, s->cap = reserve) );
}

void rope_append( rope *s, const char *buf, int len ) {
    while( s->next ) s = s->next;
    if( len > 0 && s->cap > 0 ) {
        int max = len > s->cap ? s->cap : len;
        memcpy(s->data + s->len, buf, max);
        s->len += max;
        s->cap -= max;
        buf += max;
        len -= max;
    }
    if( len > 0 ) {
        s->next = malloc( sizeof(rope) );
        // linear (no decimals) or incremental (decimals)
        rope_init( s->next, (s->grow - (int)s->grow) > 0 ? (len+1) * s->grow : s->grow, s->grow );
        rope_append( s->next, buf, len );
    }
}

void rope_dump( rope *s ) {
    puts("--");
    for( ; s ; s = s->next ) {
        printf("%03d/%03d bytes: \"%.*s\"\n", s->len, s->cap + s->len, s->len, s->data);
    }
}

int rope_len( rope *s ) {
    int len = 0;
    for( ; s ; s = s->next ) {
        len += s->len;
    }
    return len;
}

#ifdef ROPE_DEMO
int main() {
    rope s;
    rope_init(&s, 5, 5); // 1.75); // 1.75f);
    rope_dump(&s);

    rope_append(&s, "hello world.", 12);
    rope_dump(&s);

    rope_append(&s, "there's a lady who's sure all that glitter's gold.", 50);
    rope_dump(&s);

    printf("%d bytes\n", rope_len(&s));
}
#define main main__
#endif // ROPE_DEMO
#endif // ROPE_C
