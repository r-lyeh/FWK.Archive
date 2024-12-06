// generic set<K> container.
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/
// - rlyeh, public domain.

#ifndef SET_H
#define SET_H

#include <stdint.h>
#include <stdbool.h>

// config
#ifndef SET_REALLOC
#define SET_REALLOC REALLOC
#endif
#ifndef SET_HASHSIZE
#define SET_HASHSIZE (4096 << 4)
#endif
#ifndef SET_DONT_ERASE
#define SET_DONT_ERASE 1
#endif

// public api
#define set(K) \
    struct { set base; struct { set_item p; K key; } tmp, *ptr; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define set_init(m, cmpfn, hashfn) ( \
    (m) = REALLOC(0, sizeof(*m)), \
    set_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = hashfn ) \
    )

#define set_free(m) ( \
    set_clear(m), \
    set_free(&(m)->base), \
    (m) = REALLOC((m), 0), \
    (m) = 0 \
    )

#define set_insert(m, k) ( \
    (m)->ptr = set_cast((m)->ptr) SET_REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->key \
    )

#define set_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = set_cast((m)->ptr) set_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->key : 0 \
    )

#define set_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define set_foreach(m,key_t,k) \
    for( int i_ = 0; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; on_ = 0 )

#define set_clear(m) ( \
    set_clear(&(m)->base) \
    )

#define set_count(m)        set_count(&(m)->base)
#define set_gc(m)           set_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define set_cast(t) (decltype(t))
#else
#define set_cast(t) (void *)
#endif

typedef struct set_item {
    struct set_item *next;

    uint64_t keyhash;
    void *key;
    void *super;
} set_item;

typedef struct set {
    array(set_item*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
} set;

void  (set_init)(set *m);
void  (set_free)(set *m);

void  (set_insert)(set *m, set_item *p, void *key, uint64_t keyhash, void *super);
void  (set_erase)(set *m, void *key, uint64_t keyhash);
void* (set_find)(const set *m, void *key, uint64_t keyhash);
int   (set_count)(const set *m);
void  (set_gc)(set *m); // only if using SET_DONT_ERASE

#endif

// -------------------------------

#ifdef SET_C
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum { set_GC_SLOT = SET_HASHSIZE };
typedef int set_is_pow2_assert[ !(SET_HASHSIZE & (SET_HASHSIZE - 1)) ];

static int set_get_index(uint64_t hkey1) {
    return hkey1 & (SET_HASHSIZE-1);
}

void (set_init)(set* m) {
    set zero = {0};
    *m = zero;

    array_resize(m->array, (SET_HASHSIZE+1));
memset(m->array, 0, (SET_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (set_insert)(set* m, set_item *p, void *key, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = set_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;
}

void* (set_find)(const set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( const set_item *cur = m->array[index]; cur; cur = cur->next ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                return cur->super;
            }
        }
    }
    return 0;
}

void (set_erase)(set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( set_item *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if (prev) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if SET_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[set_GC_SLOT];
                m->array[set_GC_SLOT] = cur;
#else
                SET_REALLOC(cur,0);
#endif
                return;
            }
        }
    }
}

int (set_count)(const set* m) { // does not include GC_SLOT
    int counter = 0;
    for( int i = 0; i < SET_HASHSIZE; ++i) {
        for( const set_item *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

void (set_gc)(set* m) { // clean deferred GC_SLOT only
#if SET_DONT_ERASE
    for( set_item *next, *cur = m->array[set_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        SET_REALLOC(cur,0);
    }
    m->array[set_GC_SLOT] = 0;
#endif
}

void (set_clear)(set* m) { // include GC_SLOT
    for( int i = 0; i <= SET_HASHSIZE; ++i) {
        for( set_item *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            SET_REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
}

void (set_free)(set* m) {
    (set_clear)(m);

    array_free(m->array);
    m->array = 0;

    set zero = {0};
    *m = zero;
}

// -------------------------------

#ifdef SET_DEMO
#include <stdio.h>
#include <time.h>

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

void set_tests() {
    {
        set(int) m = {0};
        set_init(&m, cmp_int, hash_int);
            assert( 0 == set_count(&m) );
        set_insert(&m, 123);
        set_insert(&m, 456);
            assert( 2 == set_count(&m) );
            assert( set_find(&m, 123) );
            assert( set_find(&m, 456) );
            assert(!set_find(&m, 789) );
            assert( 123 == *set_find(&m, 123) );
            assert( 456 == *set_find(&m, 456) );

        set_foreach(&m,const int,k) {
            printf("%d\n", k);
        }

        set_erase(&m, 123);
            assert(!set_find(&m, 123) );
            assert( 1 == set_count(&m) );
        set_erase(&m, 456);
            assert(!set_find(&m, 456) );
            assert( 0 == set_count(&m) );
        set_free(&m);
    }

    {
        set(char*) m = {0};
        set_init(&m, cmp_str, hash_str);
            assert( set_count(&m) == 0 );
        set_insert(&m, "123");
        set_insert(&m, "456");
            assert( set_count(&m) == 2 );
            assert( set_find(&m,"123") );
            assert( set_find(&m,"456") );
            assert(!set_find(&m,"789") );

        set_foreach(&m,const char *,k) {
            printf("%s\n", k);
        }

        set_erase(&m, "123");
            assert( 0 == strcmp("456", *set_find(&m,"456")) );
            assert( set_count(&m) == 1 );
        set_erase(&m, "456");
            assert( set_count(&m) == 0 );
        set_free(&m);

        assert(!puts("Ok"));
    }
}

void set_tests2() {
    set(char*) m;

    set_init(&m, cmp_str, hash_str);

    set_insert(&m, "hello");
    set_insert(&m, "world");
    set_insert(&m, "nice");
    set_insert(&m, "hash");

    assert(!set_find(&m, "random"));

    assert(set_find(&m, "hello"));
    assert(set_find(&m, "world"));
    assert(set_find(&m, "nice"));
    assert(set_find(&m, "hash"));

    assert( 0 == strcmp("hello", *set_find(&m, "hello")));
    assert( 0 == strcmp("world", *set_find(&m, "world")));
    assert( 0 == strcmp("nice", *set_find(&m, "nice")));
    assert( 0 == strcmp("hash", *set_find(&m, "hash")));

    // reinsertion
    assert(set_insert(&m, "hello"));
    assert(0 == strcmp("hello", *set_find(&m, "hello")));

    set_free(&m);

    assert( !puts("Ok") );
}

void set_benchmark() {
    #ifndef NUM
    #define NUM 2000000
    #endif

    set(int) m;
    set_init(&m, cmp_int, hash_int);

    clock_t t0 = clock();

    for( int i = 0; i < NUM; ++i ) {
        set_insert(&m, i);
    }
    for( int i = 0; i < NUM; ++i ) {
        int *v = set_find(&m, i);
        assert( v && *v == i );
    }

    double t = (clock() - t0) / (double)CLOCKS_PER_SEC;

    printf("[0]=%d\n", *set_find(&m, 0));
    printf("[N-1]=%d\n", *set_find(&m, NUM-1));
    printf("%d ops in %5.3fs = %fM ops/s\n", (NUM*2), t, (NUM*2) / (1e6 * t) );

    set_free(&m);
}

int main() {
    set_tests();
    puts("---");
    set_tests2();
    puts("---");
    set_benchmark();
    assert(~puts("Ok"));
}

#define main main__
#endif // SET_DEMO
#endif // SET_C
