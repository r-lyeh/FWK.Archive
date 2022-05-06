// generic map<K,V> container.
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/
// - rlyeh, public domain.

#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>

// config
#ifndef MAP_REALLOC
#define MAP_REALLOC REALLOC
#endif
#ifndef MAP_HASHSIZE
#define MAP_HASHSIZE (4096 << 4)
#endif
#ifndef MAP_DONT_ERASE
#define MAP_DONT_ERASE 1
#endif

// public api
#define map(K,V) \
    struct { map base; struct { pair p; K key; V val; } tmp, *ptr; V* tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define map_init(m, cmpfn, hashfn) ( \
    (m) = map_cast((m)) MAP_REALLOC(0, sizeof(*(m))), \
    map_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = map_cast((m)->typed_cmp) cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = map_cast((m)->typed_hash) hashfn ) \
    )

#define map_free(m) ( \
    map_free(&(m)->base), \
    MAP_REALLOC((m), sizeof(*(m))), (m) = 0 \
    )

#define map_insert(m, k, v) ( \
    (m)->ptr = map_cast((m)->ptr) MAP_REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->val = (v), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, &(m)->ptr->val, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->val \
    )

#define map_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = map_cast((m)->ptr) map_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->val : 0 \
    )

#define map_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define map_foreach(m,key_t,k,val_t,v) \
    for( int i_ = 0; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; ) \
                for( val_t v = *(val_t *)cur_->value; on_; on_ = 0 )

#define map_clear(m) ( \
    map_clear(&(m)->base) \
    )

#define map_count(m)        map_count(&(m)->base)
#define map_gc(m)           map_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define map_cast(t) (decltype(t))
#else
#define map_cast(t) (void *)
#endif

typedef struct pair {
    struct pair *next;

    uint64_t keyhash;
    void *key;
    void *value;
    void *super;
} pair;

typedef struct map {
    array(pair*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
} map;

void  (map_init)(map *m);
void  (map_free)(map *m);

void  (map_insert)(map *m, pair *p, void *key, void *value, uint64_t keyhash, void *super);
void  (map_erase)(map *m, void *key, uint64_t keyhash);
void* (map_find)(map *m, void *key, uint64_t keyhash);
int   (map_count)(map *m);
void  (map_gc)(map *m); // only if using MAP_DONT_ERASE

#endif

// -------------------------------

#if defined MAP_C || defined MAP_DEMO
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum { MAP_GC_SLOT = MAP_HASHSIZE };
typedef int map_is_pow2_assert[ !(MAP_HASHSIZE & (MAP_HASHSIZE - 1)) ];

static int map_get_index(uint64_t hkey1) {
    return hkey1 & (MAP_HASHSIZE-1);
}

void (map_init)(map* m) {
    map c = {0};
    *m = c;

    array_resize(m->array, (MAP_HASHSIZE+1));
memset(m->array, 0, (MAP_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (map_insert)(map* m, pair *p, void *key, void *value, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->value = value;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = map_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;
}

void* (map_find)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *cur = m->array[index]; cur; cur = cur->next ) {
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

void (map_erase)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if( prev ) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if MAP_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[MAP_GC_SLOT];
                m->array[MAP_GC_SLOT] = cur;
#else
                MAP_REALLOC(cur,0);
#endif
                return;
            }
        }
    }
}

int (map_count)(map* m) {
    int counter = 0;
    for( int i = 0; i < MAP_HASHSIZE; ++i) {
        for( pair *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

void (map_gc)(map* m) {
#if MAP_DONT_ERASE
    for( pair *next, *cur = m->array[MAP_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        MAP_REALLOC(cur,0);
    }
    m->array[MAP_GC_SLOT] = 0;
#endif
}

void (map_clear)(map* m) {
    for( int i = 0; i <= MAP_HASHSIZE; ++i) {
        for( pair *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            MAP_REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
}

void (map_free)(map* m) {
    (map_clear)(m);

    array_free(m->array);
    m->array = 0;

    map c = {0};
    *m = c;
}

// -------------------------------

#ifdef MAP_DEMO
#include <stdio.h>
#include <time.h>
#ifdef __cplusplus
#include <unordered_map>
#include <map>
#endif

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

void map_benchmark() {
    #ifndef M
    #define M 100
    #endif
    #ifndef N
    #define N 50000
    #endif
    #define BENCH(CREATE,COUNT,INSERT,FIND,ITERATE,ERASE,DESTROY) do { \
        static char **bufs = 0; \
        if(!bufs) { \
            bufs = (char **)MAP_REALLOC(0, sizeof(char*) * N ); \
            for( int i = 0; i < N; ++i ) { \
                bufs[i] = (char*)MAP_REALLOC(0, 16); \
                sprintf(bufs[i], "%d", i); \
            } \
        } \
        clock_t t0 = clock(); \
        for( int i = 0; i < M; ++i ) { \
            CREATE; \
            if(i==0) printf("CREATE:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                INSERT; \
            } \
            if(i==0) printf("INSERT:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                FIND; \
            } \
            if(i==0) printf("FIND:%d ", (int)COUNT), fflush(stdout); \
            ITERATE; \
            if(i==0) printf("ITERATE:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                ERASE; \
            } \
            if(i==0) printf("REMOVE:%d ", (int)COUNT), fflush(stdout); \
            DESTROY; \
            if(i==0) printf("DESTROY%s", " "); \
        } \
        clock_t t1 = clock(); \
        double t = (t1 - t0) / (double)CLOCKS_PER_SEC; \
        int ops = (M*N)*6; \
        printf("%d ops in %fs = %.2fM ops/s (" #CREATE ")\n", ops, t, ops / (t * 1e6)); \
    } while(0)

    map(char*,int) m = 0;
    BENCH(
        map_init(m, sort_str, hash_str),
        map_count(m),
        map_insert(m, buf, i),
        map_find(m, buf),
        map_foreach(m,char*,k,int,v) {},
        map_erase(m, buf),
        map_free(m)
    );

#ifdef __cplusplus
    using std_map = std::map<const char *,int>;
    BENCH(
        std_map v,
        v.size(),
        v.insert(std::make_pair(buf, i)),
        v.find(buf),
        for( auto &kv : v ) {},
        v.erase(buf),
        {}
    );

    using std_unordered_map = std::unordered_map<const char *,int>;
    BENCH(
        std_unordered_map v,
        v.size(),
        v.insert(std::make_pair(buf, i)),
        v.find(buf),
        for( auto &kv : v ) {},
        v.erase(buf),
        {}
    );
#endif
#undef N
#undef M
}

void map_tests() {
    {
        map(int,char*) m = 0;
        map_init(m, sort_int, hash_int);
            assert( 0 == map_count(m) );
        map_insert(m, 123, "123");
        map_insert(m, 456, "456");
            assert( 2 == map_count(m) );
            assert( map_find(m, 123) );
            assert( map_find(m, 456) );
            assert(!map_find(m, 789) );
            assert( 0 == strcmp("123", *map_find(m, 123)) );
            assert( 0 == strcmp("456", *map_find(m, 456)) );

        map_foreach(m,const int,k,char*,v) {
            printf("%d->%s\n", k, v);
        }

        map_erase(m, 123);
            assert(!map_find(m, 123) );
            assert( 1 == map_count(m) );
        map_erase(m, 456);
            assert(!map_find(m, 456) );
            assert( 0 == map_count(m) );
        map_free(m);
    }

    {
        map(char*,int) m = 0;
        map_init(m, sort_str, hash_str);
            assert( map_count(m) == 0 );
        map_insert(m, "123", 123);
        map_insert(m, "456", 456);
            assert( map_count(m) == 2 );
            assert( map_find(m,"123") );
            assert( map_find(m,"456") );
            assert(!map_find(m,"789") );

        map_foreach(m,const char *,k,int,v) {
            printf("%s->%d\n", k, v);
        }

        map_erase(m, "123");
            assert( 456 == *map_find(m,"456") );
            assert( map_count(m) == 1 );
        map_erase(m, "456");
            assert( map_count(m) == 0 );
        map_free(m);

        assert(!puts("Ok"));
    }
}

void map_tests2() {
    map(char*,double) m = 0;

    map_init(m, sort_str, hash_str);

    map_insert(m, "hello", 101.1);
    map_insert(m, "world", 102.2);
    map_insert(m, "nice", 103.3);
    map_insert(m, "hash", 104.4);

    assert(!map_find(m, "random"));

    assert(map_find(m, "hello"));
    assert(map_find(m, "world"));
    assert(map_find(m, "nice") );
    assert(map_find(m, "hash") );

    assert( 101.1 == *map_find(m, "hello"));
    assert( 102.2 == *map_find(m, "world"));
    assert( 103.3 == *map_find(m, "nice"));
    assert( 104.4 == *map_find(m, "hash"));

    // reinsertion
    assert(map_insert(m, "hello", -101.1));
    assert(-101.1 == *map_find(m, "hello"));

    map_foreach(m,char*,k,double,v) {
        printf("%s -> %f\n", k, v);
    }

    map_free(m);

    assert( !puts("Ok") );
}

void map_benchmark2() {
    #ifndef NUM
    #define NUM 2000000
    #endif

    map(int,int) m = 0;
    map_init(m, sort_int, hash_int);

    clock_t t0 = clock();

    for( int i = 0; i < NUM; ++i ) {
        map_insert(m, i, i+1);
    }
    for( int i = 0; i < NUM; ++i ) {
        uint32_t *v = map_find(m, i);
        assert( v && *v == i + 1 );
    }

    double t = (clock() - t0) / (double)CLOCKS_PER_SEC;

    printf("[0]=%d\n", *map_find(m, 0));
    printf("[N-1]=%d\n", *map_find(m, NUM-1));
    printf("%d ops in %5.3fs = %fM ops/s\n", (NUM*2), t, (NUM*2) / (1e6 * t) );

    map_free(m);
}

int main() {
    map_tests();
    puts("---");
    map_tests2();
    puts("---");
    map_benchmark();
    puts("---");
    map_benchmark2();
    assert(~puts("Ok"));
}

#define main main__
#endif // MAP_DEMO
#endif // MAP_C
