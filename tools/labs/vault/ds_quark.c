// quarks ----------------------------------------------------------------------
// - rlyeh, public domain

#ifndef QUARK_H
#define QUARK_H

typedef unsigned quark;

quark quark_intern(char *s);
char* quark_str(quark q);
int   quark_len(quark q);
int   quark_hash(quark q);
int   quark_cmp(quark a, quark b);

#define quark_intern(__VA_ARGS__) quark_intern(va(__VA_ARGS__))

#endif

// -----------------------------------------------------------------------------

#ifdef QUARK_C
#pragma once

typedef struct quark_dictionary {
    char *block;
    uint64_t block_used;
    array(char*) strings;
    unsigned index;
} quark_dictionary;

static __thread quark_dictionary qd = {0};

quark (quark_intern)(char *s) {
    if( !qd.block ) qd.block = REALLOC(0, 4 * 1024 * 1024 ); // 
    uint64_t n = strlen(s) + 1;
    array_push(qd.strings, qd.block + qd.block_used + 4);
    memcpy(qd.block + qd.block_used, s - 4, n + 4);
    qd.block_used += n + 4;
    return ++qd.index;
}
char *quark_str(quark q) {
    return q > 0 && q <= qd.index ? qd.strings[ q - 1 ] : "";
}
int quark_len(quark q) {
    return strlen(quark_str(q));
}
int quark_hash(quark q) {
    return strhash(quark_str(q));
}
int quark_cmp(quark q1, quark q2) {
    return strcmp(quark_str(q1), quark_str(q2));
}

#endif
