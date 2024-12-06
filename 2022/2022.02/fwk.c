/* game framework.
*  - rlyeh, public domain
*
* ## FWK License
*
* This software is available under 3 licenses. Choose whichever you prefer.
* ------------------------------------------------------------------------------
* ALTERNATIVE A - Public Domain (https://unlicense.org/)
* ------------------------------------------------------------------------------
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
* software, either in source code form or as a compiled binary, for any purpose,
* commercial or non-commercial, and by any means.
*
* In jurisdictions that recognize copyright laws, the author or authors of this
* software dedicate any and all copyright interest in the software to the public
* domain. We make this dedication for the benefit of the public at large and to
* the detriment of our heirs and successors. We intend this dedication to be an
* overt act of relinquishment in perpetuity of all present and future rights to
* this software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE B - MIT-0 (No Attribution clause)
* ------------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the "Software"), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE C - Zero BSD License (https://opensource.org/licenses/FPL-1.0.0)
* ------------------------------------------------------------------------------
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* ## License: Contributed Code ------------------------------------------------
*
* Dear Contributor,
*
* In order to ensure this project remains completely free and unencumbered by
* anyone's copyright monopoly, it is advisable that you dedicate your code-base
* contributions to the three licensing terms above. This removes any possible
* ambiguity as to what terms somebody might have thought they were contributing
* under, in case of a future dispute. These concerns are not unique to public
* domain software. Most large, established open-source projects have a
* Contributor License Agreement (CLA) process, of varying degrees of formality.
*
* Please add yourself to the list below before contributing.
* Thanks.
*
* --
*
* "I dedicate any and all copyright interest in this software to the three
* license terms listed above. I make this dedication for the benefit of the
* public at large and to the detriment of my heirs and successors. I intend
* this dedication to be an overt act of relinquishment in perpetuity of all
* present and future rights to this software under copyright law."
*
* Author (name)              I agree (YES/NO)    Files/Features (optional)
* ------------------------------------------------------------------------------
* @r-lyeh                    YES                 Initial codebase
* ------------------------------------------------------------------------------
*/

#include "fwk.h"
#define FWK_3RD
#include "fwk"

//-----------------------------------------------------------------------------
// C files

#line 1 "fwk_ds.c"
// -----------------------------------------------------------------------------
// less/sort

int less_int(int a, int b) {
    return a - b;
}
int less_u64(uint64_t a, uint64_t b) {
    return a > b ? +1 : -!!(a - b);
}
int less_ptr(void *a, void *b) {
    return (uintptr_t)a > (uintptr_t)b ? +1 : -!!((uintptr_t)a - (uintptr_t)b);
}
int less_str(char *a, char *b) {
    return strcmp((const char *)a, (const char *)b);
}

int sort_u64(const void *a, const void *b) {
    return 0[(uint64_t*)a] - 0[(uint64_t*)b];
}

// -----------------------------------------------------------------------------
// un/hash

uint32_t unhash_32(uint32_t x) {
    // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}
uint32_t hash_32(uint32_t x) {
    // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
uint64_t hash_64(uint64_t x) {
#if 1
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
#else
    // should we just use x2 hash_32?
    uint32_t hi = (x >> 32ull), lo = (x & ~0u);
    return (hash_32(hi) << 32ull) | hash_32(lo);
#endif
}
uint64_t hash_flt(double x) {
    union { double d; uint64_t i; } c;
    return c.d = x, hash_64(c.i);
}
uint64_t hash_str(const char* str) {
    uint64_t hash = 14695981039346656037ULL; // hash(0),mul(131) faster than fnv1a, a few more collisions though
    while( *str ) hash = ( (unsigned char)*str++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}
uint64_t hash_int(int key) {
    return hash_32((uint32_t)key);
}
uint64_t hash_ptr(const void *ptr) {
    uint64_t key = (uint64_t)(uintptr_t)ptr;
    return hash_64(key); // >> 3? needed?
}

// -----------------------------------------------------------------------------
// vector based allocator (x1.75 enlarge factor)

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            REALLOC( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret = (size_t*)p - 2;
        if( !p ) {
            ret = (size_t*)REALLOC( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)REALLOC( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

// -----------------------------------------------------------------------------

enum { MAP_GC_SLOT = MAP_HASHSIZE };
typedef int map_is_pow2_assert[ !(MAP_HASHSIZE & (MAP_HASHSIZE - 1)) ? 1:-1];

static int map_get_index(uint64_t hkey1) {
    return hkey1 & (MAP_HASHSIZE-1);
}

void (map_init)(map* m) {
    map c = {0};
    *m = c;

    array_resize(m->array, (MAP_HASHSIZE+1));
    memset(m->array, 0, (MAP_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()

    // @fixme
    // tcc crashes both lines above with: tcc demo_video.c fwk.c -b -g && gdb -ex=r --args demo_video.exe
    // maybe try this instead:
    // m->array = vrealloc(0, (MAP_HASHSIZE+1) * sizeof(m->array[0]));
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

    ++m->count;
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
                REALLOC(cur,0);
#endif
                --m->count;
                return;
            }
        }
    }
}

int (map_count)(map* m) {
    return m->count;
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
        REALLOC(cur,0);
    }
    m->array[MAP_GC_SLOT] = 0;
#endif
}

void (map_clear)(map* m) {
    for( int i = 0; i <= MAP_HASHSIZE; ++i) {
        for( pair *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
}

void (map_free)(map* m) {
    (map_clear)(m);

    array_free(m->array);
    m->array = 0;

    map c = {0};
    *m = c;
}

// -----------------------------------------------------------------------------

enum { set_GC_SLOT = SET_HASHSIZE };
typedef int set_is_pow2_assert[ !(SET_HASHSIZE & (SET_HASHSIZE - 1)) ? 1:-1];

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

    ++m->count;
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
                REALLOC(cur,0);
#endif
                --m->count;
                return;
            }
        }
    }
}

int (set_count)(const set* m) { // does not include GC_SLOT
    return m->count;
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
        REALLOC(cur,0);
    }
    m->array[set_GC_SLOT] = 0;
#endif
}

void (set_clear)(set* m) { // include GC_SLOT
    for( int i = 0; i <= SET_HASHSIZE; ++i) {
        for( set_item *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
}

void (set_free)(set* m) {
    (set_clear)(m);

    array_free(m->array);
    m->array = 0;

    set zero = {0};
    *m = zero;
}
#line 0
#line 1 "fwk_string.c"
#include <stdarg.h>

char* stringfv(const char *fmt, va_list vl) {
    va_list copy;
    va_copy(copy, vl);
    int sz = vsnprintf( 0, 0, fmt, copy ) + 1;
    va_end(copy);

    int reqlen = sz;
#if 0
    enum { STACK_ALLOC = 16384 };
    static __thread char buf[STACK_ALLOC];
#else
    enum { STACK_ALLOC = 128*1024 };
    static __thread char *buf = 0; if(!buf) buf = REALLOC(0, STACK_ALLOC); // leak
#endif
    static __thread int cur = 0, len = STACK_ALLOC; //printf("string stack %d/%d\n", cur, STACK_ALLOC);

    assert(reqlen < STACK_ALLOC && "no stack enough, increase STACK_ALLOC variable above");
    char* ptr = buf + (cur *= (cur+reqlen) < len, (cur += reqlen) - reqlen);

    vsnprintf( ptr, sz, fmt, vl );
    return (char *)ptr;
}
char* (stringf)(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char *s = stringfv(fmt, va);
    va_end(va);
    return s;
}

#if 1
char* (strcatf)(char **src_, const char *buf) {
    char *src = *src_;
        int srclen = (src ? strlen(src) : 0), buflen = strlen(buf);
        src = (char*)REALLOC(src, srclen + buflen + 1 );
        memcpy(src + srclen, buf, buflen + 1 );
    *src_ = src;
    return src;
}
#endif

// -----------------------------------------------------------------------------
// string utils

int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard=='\0' ) return !*s;
    if( *wildcard=='*' )  return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard=='?' )  return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}

int strcmp_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}
int strcmpi_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmpi(*ia, *ib);
}

bool strbegi(const char *a, const char *b) { // returns true if both strings match at beginning. case insensitive
    int la = strlen(a), lb = strlen(b);
    if( la < lb ) return 0;
    if( lb == 0 ) return 1;
    int len = la < lb ? la : lb;
    for( int i = 0; i < len; ++i ) {
        if( tolower(a[i]) != tolower(b[i]) ) {
            return false;
        }
    }
    return true;
}
bool strendi(const char *src, const char *sub) { // returns true if both strings match at end. case insensitive
    int srclen = strlen(src);
    int sublen = strlen(sub);
    if( sublen > srclen ) return 0;
    return !strcmpi(src + srclen - sublen, sub);
}


// Find substring in string, case insensitive. Alias for strcasestr()
// Returns first char of coincidence, or NULL.
const char *strstri( const char *src, const char *sub ){
    while( *src++ ) {
        for( const char *s = src-1, *f = sub, *c = s; ; ++f, ++c) {
            if(!*f) return s;
            if(!*c) return NULL;
            if(tolower(*c) != tolower(*f)) break;
        }
    }
    return NULL;
}

char *strupper(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = toupper(s[-1]);
    return bak;
}
char *strlower(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = tolower(s[-1]);
    return bak;
}

#ifndef __APPLE__ // BSD provides these
// Safely concatenate two strings. Always NUL terminates (unless dstcap == 0).
// Returns length of operation; if retval >= dstcap, truncation occurred.
size_t strlcat(char *dst, const char *src, size_t dstcap) {
    int dl = strlen(dst), sl = strlen(src);
    if( dstcap ) snprintf(dst + dl, dstcap - dl, "%s", src);
    return dl + sl;
}
// Safely copy two strings. Always NUL terminates (unless dstcap == 0).
// Copy src to string dst of size dstcap. Copies at most dstcap-1 characters.
// Returns length of input; if retval >= dstcap, truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t dstcap) {
    int sl = strlen(src);
    if( dstcap ) snprintf(dst, dstcap, "%*s", sl, src);
    return sl;// count does not include NUL
}
#endif

char *str16to8(const wchar_t *str) { // from wchar16(win) to utf8/ascii
    int i = 0, n = wcslen(str) * 6 - 1;
    static __thread char buffer[2048]; assert( n < 2048 );
    while( *str ) {
       if (*str < 0x80) {
          if (i+1 > n) return va("");
          buffer[i++] = (char) *str++;
       } else if (*str < 0x800) {
          if (i+2 > n) return va("");
          buffer[i++] = 0xc0 + (*str >> 6);
          buffer[i++] = 0x80 + (*str & 0x3f);
          str += 1;
       } else if (*str >= 0xd800 && *str < 0xdc00) {
          uint32_t c;
          if (i+4 > n) return va("");
          c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
          buffer[i++] = 0xf0 + (c >> 18);
          buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
          buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
          buffer[i++] = 0x80 + ((c      ) & 0x3f);
          str += 2;
       } else if (*str >= 0xdc00 && *str < 0xe000) {
          return va("");
       } else {
          if (i+3 > n) return va("");
          buffer[i++] = 0xe0 + (*str >> 12);
          buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
          buffer[i++] = 0x80 + ((*str     ) & 0x3f);
          str += 1;
       }
    }
    buffer[i] = 0;
    return va("%s", buffer);
}

char *strrepl(char **string, const char *target, const char *replace) { // may reallocate input string if needed
    //if new text is shorter than old one,then no need to heap, replace inline
    //int rlen = strlen(replace), diff = strlen(target) - rlen;
    //if( diff >= 0 ) return strswap(*string, target, replace);

    char *buf = 0, *aux = *string;
    for( int tgtlen = strlen(target); tgtlen && aux[0]; ) {
        char *found = strstr(aux, target);
        if( found ) {
            strcatf(&buf, "%.*s%s", (int)(found - aux), aux, replace);
            aux += (found - aux) + tgtlen;
        } else {
            strcatf(&buf, "%s", aux);
            break;
        }
    }
    if( buf ) {
        // strcpy(*string, buf);
        char *s = *string;
        if(s) s[0] = 0;
        strcatf(&s, "%s", buf);
        *string = s;

        FREE( buf );
    }
    return *string;
}
char *strswap(char *copy, const char *target, const char *replace) { // replaced only if new text is shorter than old one
    int rlen = strlen(replace), diff = strlen(target) - rlen;
    if( diff >= 0 ) {
        for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
            if( rlen ) s = (char*)memcpy( s, replace, rlen ) + rlen;
            if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
        }
    }
    return copy;
}
char *strcut(char *copy, const char *target) {
    return strswap(copy, target, "");
}

const char *strlerp(unsigned numpairs, const char **pairs, const char *str) { // using key-value pairs, null-terminated
    if( !pairs[0] ) {
        return str;
    }
    // find & replace all tokens; @fixme: optimize me
    char *buf = REALLOC(0, 128*1024); strcpy(buf, str);
    for( unsigned i = 0; i < numpairs; ++i ) {
        const char *token = pairs[i*2+0];
        const char *repl = pairs[i*2+1];
        while(strstr(buf, token)) {
            strrepl(&buf, token, repl);
        }
    }
    char *ret = va("%s", buf);
    FREE(buf);
    return ret;
}

array(char*) strsplit(const char *str, const char *separators) {
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    array_resize(list[slot], 0);
    *(buf[slot] = REALLOC(buf[slot], strlen(str)+1)) = '\0';

    for(char *dst = buf[slot]; str && *str; ) {
        // count literal run && terminators
        int run = strcspn(str, separators);
        int end = strspn(str + run, separators);

        // append literal run
        if( run ) {
            array_push(list[slot], dst);
            memmove(dst,str,run); dst[run] = '\0'; //strncpy(dst, str, run)
            dst += run + 1;
        }

        // skip both
        str += run + end;
    }

    return list[slot];
}
char* strjoin(array(char*) list, const char *separator) {
    static __thread int slot = 0;
    static __thread char* mems[16] = {0};

    slot = (slot+1) % 16;

    int num_list = array_count(list);
    int len = 0, inc = 0, seplen = strlen(separator);
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        len += strlen(list[i]) + inc;
        inc = seplen;
    }

    mems[slot] = REALLOC(mems[slot], len+1);
    char *p = mems[slot]; *p = 0;
    const char *sep = "";
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        p += sprintf(p, "%s%s", sep, list[i]);
        sep = separator;
    }

    return mems[slot];
}

static
uint32_t extract_utf32(const char **p) {
    if( (**p & 0x80) == 0x00 ) {
        int a = *((*p)++);
        return a;
    }
    if( (**p & 0xe0) == 0xc0 ) {
        int a = *((*p)++) & 0x1f;
        int b = *((*p)++) & 0x3f;
        return (a << 6) | b;
    }
    if( (**p & 0xf0) == 0xe0 ) {
        int a = *((*p)++) & 0x0f;
        int b = *((*p)++) & 0x3f;
        int c = *((*p)++) & 0x3f;
        return (a << 12) | (b << 6) | c;
    }
    if( (**p & 0xf8) == 0xf0 ) {
        int a = *((*p)++) & 0x07;
        int b = *((*p)++) & 0x3f;
        int c = *((*p)++) & 0x3f;
        int d = *((*p)++) & 0x3f;
        return (a << 18) | (b << 12) | (c << 8) | d;
    }
    return 0;
}
array(uint32_t) strutf32( const char *utf8 ) {
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(int) list[16] = {0};

    slot = (slot+1) % 16;
    array_resize(list[slot], 0);

    //int worstlen = strlen(utf8) + 1; array_reserve(out, worstlen);
    while( *utf8 ) {
        uint32_t unicode = extract_utf32( &utf8 );
        array_push(list[slot], unicode);
    }
    return list[slot];
}

#line 0
#line 1 "fwk_compat.c"
//-----------------------------------------------------------------------------
// compat (unix & stdio.h)

#ifdef __TINYC__ // add missing symbols
#ifdef _WIN32
const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;        /* :: */
const struct in6_addr in6addr_loopback;   /* ::1 */
#endif
#endif

#ifdef _WIN32
#include <io.h>
#if is(mingw)
#include <unistd.h>
#endif
#else
#include <unistd.h>
#include <sched.h>    // CPU_ZERO, CPU_COUNT
#include <sys/ioctl.h>
#endif

#ifdef _WIN32
//#define alloca        _alloca
#define atoi64        _atoi64
#define popen         _popen
#define pclose        _pclose
//#define strncasecmp   _strnicmp
#define mkdir(p,m)    mkdir(p)
#define chdir         ifdef(msc, _chdir, chdir)
#if is(msc) || is(tcc)
#define ftruncate     _chsize_s
#endif
#else // gcc
//#include <alloca.h> // mingw64 does not have it
#include <strings.h> // strncasecmp
#define atoi64        atoll
//#define strstri       strcasestr
//#define strcmpi       strcasecmp
#endif

#if   defined MAX_PATH
#define MAX_PATHFILE MAX_PATH
#elif defined PATH_MAX
#define MAX_PATHFILE PATH_MAX
#else
#define MAX_PATHFILE 260
#endif

#ifdef _WIN32 // _MSC_VER and __MINGW64__
#include <stdio.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
FILE *fmemopen(void *buf, size_t len, const char *type) {
    int fd = -1;
    char temppath[MAX_PATHFILE - 14], filename[MAX_PATHFILE + 1];
    if( GetTempPathA(sizeof(temppath), temppath) )
    if( GetTempFileNameA(temppath, "fwk_temp", 0, filename) )
    if( !_sopen_s(&fd, filename, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE) )
    for( FILE *fp = fd != -1 ? _fdopen(fd, "w+b") : NULL; fp; )
    return fwrite(buf, len, 1, fp), rewind(fp), unlink(filename), fp; // no need to _close. fclose(on the returned FILE*) also _closes the file descriptor.
    return fd != -1 ? _close(fd), NULL : NULL;
}
#endif



#ifndef _MSC_VER
#define tmpfile file_temp
#endif

#define tmpnam file_tempname


#if 0
static
const char *pathfile_from_handle(FILE *fp) {
#if is(win32)
    int fd = fileno(fp);
    HANDLE handle = (HANDLE)_get_osfhandle( fd ); // <io.h>
    DWORD size = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS);
    wchar_t name[MAX_PATHFILE] = L"";
    size = GetFinalPathNameByHandleW(handle, name, size, VOLUME_NAME_DOS);
    name[size] = L'\0';
    return wchar16to8(name + 4); // skip \\?\ header
#else
    // In Linux, you can use readlink on /proc/self/fd/NNN where NNN is the file descriptor
    // In OSX:
    //     #include <sys/syslimits.h>
    //     #include <fcntl.h>
    //     char filePath[MAX_PATHFILE];
    //     if (fcntl(fd, F_GETPATH, filePath) != -1) {
    //         // do something with the file path
    //     }
    return 0;
#endif
}
#endif

// -----------------------------------------------------------------------------
// new C macros

#define local            __thread // ifdef(msc, __declspec(thread), __thread) // __STDC_VERSION_ >= 201112L: _Thread_local

#define cast(T)          ifdef(c, void *, decltype(T))
#define literal(T)       ifdef(c, T, (T))

// -----------------------------------------------------------------------------

void fwk_init();
static void fwk_pre_init_subsystems();
static void fwk_post_init_subsystems();
static void fwk_pre_swap_subsystems();
#line 0
#line 1 "fwk_profile.c"
#if WITH_PROFILE
profiler_t profiler;
#endif
#line 0

#define  atexit(...) // hack to boost exit time. there are no critical systems that need to shutdown properly
#line 1 "fwk_audio.c"
// @fixme: really shutdown audio & related threads before quitting. drwav crashes.

// encapsulate drwav,drmp3,stbvoribs and some buffer with the sts_mixer_stream_t
enum { UNK, WAV, OGG, MP1, MP3 };
typedef struct {
    int type;
    union {
        drwav wav;
        stb_vorbis *ogg;
        void *opaque;
        drmp3           mp3_;
    };
    sts_mixer_stream_t  stream;             // mixer stream
    union {
    int32_t             data[4096*2];       // static sample buffer
    float               dataf[4096*2];
    };
} mystream_t;

static void stereo_float_to_mono( int channels, float *buffer, int samples ) {
    if( channels > 1 ) {
        float *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (float)(mix / channels);
        }
    }
}
static void stereo_s16_to_mono( int channels, short *buffer, int samples ) {
    if( channels > 1 ) {
        short *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (short)(mix / channels);
        }
    }
}

// the callback to refill the (stereo) stream data
static void refill_stream(sts_mixer_sample_t* sample, void* userdata) {
    mystream_t* stream = (mystream_t*)userdata;
    switch( stream->type ) {
        default:
        break; case WAV: {
            int sl = sample->length / 2; /*sample->channels*/;
            if (drwav_read_pcm_frames_s16(&stream->wav, sl, (short*)stream->data) < sl) {
                drwav_seek_to_pcm_frame(&stream->wav, 0);
            }
        }
        break; case MP3: {
            int sl = sample->length / 2; /*sample->channels*/;
            if (drmp3_read_pcm_frames_f32(&stream->mp3_, sl, stream->dataf) < sl) {
                drmp3_seek_to_pcm_frame(&stream->mp3_, 0);
            }
        }
        break; case OGG: {
            stb_vorbis *ogg = (stb_vorbis*)stream->ogg;
            if( stb_vorbis_get_samples_short_interleaved(ogg, 2, (short*)stream->data, sample->length) == 0 )  {
                stb_vorbis_seek(stream->ogg, 0);
            }
        }
    }
}

// load a (stereo) stream
static bool load_stream(mystream_t* stream, const char *filename) {
    int error;
    int HZ = 44100;
    stream->type = UNK;
    if( stream->type == UNK && (stream->ogg = stb_vorbis_open_filename(filename, &error, NULL)) ) {
        stb_vorbis_info info = stb_vorbis_get_info(stream->ogg);
        if( info.channels != 2 ) { puts("cannot stream ogg file. stereo required."); goto end; }
        stream->type = OGG;
        stream->stream.sample.frequency = info.sample_rate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    if( stream->type == UNK && (drwav_init_file(&stream->wav, filename, NULL))) {
        if( stream->wav.channels != 2 ) { puts("cannot stream wav file. stereo required."); goto end; }
        stream->type = WAV;
        stream->stream.sample.frequency = stream->wav.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    drmp3_config mp3_cfg = { 2, HZ };
    if( stream->type == UNK && (drmp3_init_file(&stream->mp3_, filename, NULL/*&mp3_cfg*/) != 0) ) {
        stream->type = MP3;
        stream->stream.sample.frequency = stream->mp3_.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_FLOAT;
    }

    if( stream->type == UNK ) {
        return false;
    }

    end:;
    stream->stream.userdata = stream;
    stream->stream.callback = refill_stream;
    stream->stream.sample.length = sizeof(stream->data) / sizeof(stream->data[0]);
    stream->stream.sample.data = stream->data;
    refill_stream(&stream->stream.sample, stream);

    return true;
}

// load a (mono) sample
static bool load_sample(sts_mixer_sample_t* sample, const char *filename) {
    int error;
    int channels = 0;
    if( !channels ) for( drwav w = {0}, *wav = &w; wav && drwav_init_file(wav, filename, NULL); wav = 0 ) {
        channels = wav->channels;
        sample->frequency = wav->sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = wav->totalPCMFrameCount;
        sample->data = REALLOC(0, sample->length * sizeof(short) * channels);
        drwav_read_pcm_frames_s16(wav, sample->length, (short*)sample->data);
        drwav_uninit(wav);
    }
    if( !channels ) for( stb_vorbis *ogg = stb_vorbis_open_filename(filename, &error, NULL); ogg; ogg = 0 ) {
        stb_vorbis_info info = stb_vorbis_get_info(ogg);
        channels = info.channels;
        sample->frequency = info.sample_rate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = (int)stb_vorbis_stream_length_in_samples(ogg);
        stb_vorbis_close(ogg);

        short *buffer;
        int sample_rate;
        stb_vorbis_decode_filename(filename, &channels, &sample_rate, (short **)&buffer);
        sample->data = buffer;
    }
    drmp3_config mp3_cfg = { 2, 44100 };
    drmp3_uint64 mp3_fc;
    if( !channels ) for( short *fbuf = 0; fbuf = drmp3_open_file_and_read_pcm_frames_s16(filename, &mp3_cfg, &mp3_fc, NULL); ) {
        channels = mp3_cfg.channels;
        sample->frequency = mp3_cfg.sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = mp3_fc; //  / sizeof(float) / mp3_cfg.channels;
        sample->data = fbuf;
        break;
    }
    if( !channels ) {
        int inputSize = file_size( filename );
        void *inputData = file_read( filename );
        if( inputData ) {
            short *output = 0;
            int outputSize, hz, mp1channels;
            bool ok = jo_read_mp1(inputData, inputSize, &output, &outputSize, &hz, &mp1channels);
            if( ok ) {
                channels = mp1channels;
                sample->frequency = hz;
                sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
                sample->length = outputSize / sizeof(int16_t) / channels;
                sample->data = REALLOC(0, sample->length * sizeof(int16_t) * channels );
                memcpy( sample->data, output, outputSize );
            }
            REALLOC( inputData, 0 );
        }
    }

#if 0
    if( !channels ) {
        //loadPreset(1, 0);
        //SaveSettings("test.sfxr");
        LoadSettings(filename);
        ExportWAV("sfxr.wav");
        return load_sample(sample, "sfxr.wav");
    }
#endif

    if( !channels ) {
        return false;
    }

    if( channels > 1 ) {
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_FLOAT ) {
            stereo_float_to_mono( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(float));
        }
        else
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_16 ) {
            stereo_s16_to_mono( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(short));
        }
        else {
            puts("error!");
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

static ma_device  device;
static ma_context context;
static sts_mixer_t mixer;

// This is the function that's used for sending more data to the device for playback.
static ma_uint32 audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    int len = frameCount;
    sts_mixer_mix_audio(&mixer, pOutput, len / (sizeof(int32_t) / 4));
    return len / (sizeof(int32_t) / 4);
}

void audio_drop(void) {
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
}

int audio_init( int flags ) {
    atexit(audio_drop);

    // init sts_mixer
    sts_mixer_init(&mixer, 44100, STS_MIXER_SAMPLE_FORMAT_32);

    // The prioritization of backends can be controlled by the application. You need only specify the backends
    // you care about. If the context cannot be initialized for any of the specified backends ma_context_init()
    // will fail.
    ma_backend backends[] = {
        ma_backend_wasapi, // Higest priority.
        ma_backend_dsound,
        ma_backend_winmm,
        ma_backend_pulseaudio,
        ma_backend_alsa,
        ma_backend_oss,
        ma_backend_jack,
        ma_backend_opensl,
        //ma_backend_openal,
        //ma_backend_sdl,
        ma_backend_null    // Lowest priority.
    };

    if (ma_context_init(backends, countof(backends), NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.");
        return false;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback); // Or ma_device_type_capture or ma_device_type_duplex.
    config.playback.pDeviceID = NULL; // &myPlaybackDeviceID; // Or NULL for the default playback device.
    config.playback.format    = ma_format_s32;
    config.playback.channels  = 2;
    config.sampleRate         = 44100;
    config.dataCallback       = (void*)audio_callback; //< @r-lyeh add void* cast
    config.pUserData          = NULL;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.");
        ma_context_uninit(&context);
        return false;
    }

    ma_device_start(&device);
    return true;
}

typedef struct audio_handle {
    bool is_clip;
    bool is_stream;
    union {
    sts_mixer_sample_t clip;
    mystream_t         stream;
    };
} audio_handle;

static array(audio_handle*) audio_instances;

audio_t audio_clip( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_clip = load_sample( &a->clip, vfs_find(pathfile) );
    array_push(audio_instances, a);
    return a;
}
audio_t audio_stream( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_stream = load_stream( &a->stream, vfs_find(pathfile) );
    array_push(audio_instances, a);
    return a;
}


static float volume_clip = 1, volume_stream = 1, volume_master = 1;
float audio_volume_clip(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_clip = gain * gain;
        // patch all live clips
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].sample ) // is_sample?
                mixer.voices[i].gain = volume_clip;
        }
    return sqrt( volume_clip );
}
float audio_volume_stream(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_stream = gain * gain;
        // patch all live streams
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].stream ) // is_stream?
                mixer.voices[i].gain = volume_stream;
        }
    return sqrt( volume_stream );
}
float audio_volume_master(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_master = gain * gain;
        // patch global mixer
        mixer.gain = volume_master;
    return sqrt( volume_master );
}

int audio_play_gain_pitch_pan( audio_t a, int flags, float gain, float pitch, float pan ) {
    if( flags & AUDIO_IGNORE_MIXER_GAIN ) {
        // do nothing, gain used as-is
    } else {
        // apply mixer gains on top
        gain += a->is_clip ? volume_clip : volume_stream;
    }

    // gain: [0..+1], pitch: (0..N], pan: [-1..+1]

    if( a->is_clip ) {
        int voice = sts_mixer_play_sample(&mixer, &a->clip, gain, pitch, pan);
        if( voice == -1 ) return 0; // all voices busy
    }
    if( a->is_stream ) {
        int voice = sts_mixer_play_stream(&mixer, &a->stream.stream, gain);
        if( voice == -1 ) return 0; // all voices busy
    }
    return 1;
}

int audio_play_gain_pitch( audio_t a, int flags, float gain, float pitch ) {
    return audio_play_gain_pitch_pan(a, flags, gain, pitch, 0);
}

int audio_play_gain( audio_t a, int flags, float gain ) {
    return audio_play_gain_pitch(a, flags, gain, 1.f);
}

int audio_play( audio_t a, int flags ) {
    return audio_play_gain(a, flags & ~AUDIO_IGNORE_MIXER_GAIN, 0.f);
}

// -----------------------------------------------------------------------------
// audio queue

#ifndef AUDIO_QUEUE_BUFFERING_MS
#define AUDIO_QUEUE_BUFFERING_MS 100
#endif

#ifndef AUDIO_MAX_QUEUE
#define AUDIO_MAX_QUEUE 2048
#endif

typedef struct audio_queue_t {
    void *data;
    int cursor;
    int avail;
} audio_queue_t;

static thread_mutex_t queue_mutex;
static audio_queue_t audio_queues[AUDIO_MAX_QUEUE] = {0};
static int audio_queue_rd = 0, audio_queue_wr = AUDIO_MAX_QUEUE/2, audio_queue_voice = -1;
static void audio_queue_callback(sts_mixer_sample_t* sample, void* userdata) {
    char *p = sample->data;
    int bytes = sample->length * 4; // * 8; // 2ch * sizeof float

    if( audio_queue_rd == audio_queue_wr ) {
        PRINTF("WARNING: audio buffers exhausted, please increase buffering (%d>>%d)\n", audio_queue_rd, audio_queue_wr);
        audio_queue_wr = (audio_queue_wr + AUDIO_MAX_QUEUE / 2) % AUDIO_MAX_QUEUE;
        // sleep_ms(10);
        return;
    }

    while( bytes > 0 ) {
        thread_mutex_lock(&queue_mutex);

            audio_queue_t* front = &audio_queues[audio_queue_rd];

            int bytes_to_copy = bytes;
            if( bytes_to_copy > front->avail ) bytes_to_copy = front->avail;

if(front->data)
            memcpy(p, &((char*)front->data)[front->cursor], bytes_to_copy);
            p += bytes_to_copy;
            front->cursor += bytes_to_copy;
            front->avail -= bytes_to_copy;
            bytes -= bytes_to_copy;

            if( front->avail <= 0 ) {
                audio_queue_rd = (audio_queue_rd + 1) % AUDIO_MAX_QUEUE;
            }

        thread_mutex_unlock(&queue_mutex);
    }
}
void audio_queue_clear() {
    thread_mutex_lock(&queue_mutex);
//sts_mixer_stop_voice(&mixer, audio_queue_voice);
//audio_queue_voice = -1;
    for (int i = 0; i < AUDIO_MAX_QUEUE; ++i ) {
        audio_queues[i].data = REALLOC(audio_queues[i].data, 0);
    }
    audio_queue_rd = 0;
    audio_queue_wr = AUDIO_MAX_QUEUE/2;
    thread_mutex_unlock(&queue_mutex);
}
int audio_queue( const void *samples, int num_samples, int flags ) {
    float gain = 1; // [0..1]
    float pitch = 1; // (0..N]
    float pan = 0; // [-1..1]

    int bits = flags & AUDIO_16 ? 16 : 32;
    int channels = flags & AUDIO_2CH ? 2 : 1;
    int bytes_per_sample = channels * (bits / 8);
    int bytes = num_samples * bytes_per_sample;
    assert( channels == 2 );

    if( audio_queue_voice < 0 ) {
        static sts_mixer_stream_t q = { 0 };
        q.callback = audio_queue_callback;
        q.sample.frequency = flags & AUDIO_44KHZ ? 44100 : 22050;
        q.sample.audio_format = flags & AUDIO_FLOAT ? STS_MIXER_SAMPLE_FORMAT_FLOAT : STS_MIXER_SAMPLE_FORMAT_16;
        q.sample.length = q.sample.frequency / (1000 / AUDIO_QUEUE_BUFFERING_MS);
        int bytes = q.sample.length /* * channels*/ * bytes_per_sample;
        q.sample.data = memset(REALLOC(q.sample.data, bytes), 0, bytes);
        audio_queue_voice = sts_mixer_play_stream(&mixer, &q, gain * 1.f);
    }

    do_once thread_mutex_init(&queue_mutex);
    thread_mutex_lock(&queue_mutex);
        audio_queue_t *aq = &audio_queues[audio_queue_wr = (audio_queue_wr+1) % AUDIO_MAX_QUEUE];
        aq->cursor = 0;
        aq->avail = bytes;
        aq->data = REALLOC(aq->data, bytes);
        assert(aq->data);
        if( samples ) memcpy(aq->data, samples, bytes);
        else          memset(aq->data,       0, bytes);
    thread_mutex_unlock(&queue_mutex);

    return audio_queue_voice;
}
#line 0
#line 1 "fwk_collide.c"
/* poly */
poly poly_alloc(int cnt) {
    poly p = {0};
    p.cnt = cnt;
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * cnt); // array_resize(p.verts, cnt);
    return p;
}

void poly_free(poly *p) {
    REALLOC(p->verts, 0); // array_free(p->verts);
    poly z = {0};
    *p = z;
}

/* plane */
vec4 plane4(vec3 p, vec3 n) {
    return vec34(n, -dot3(n,p));
}

/* pyramid */
poly pyramid(vec3 from, vec3 to, float size) {
    /* calculate axis */
    vec3 up, right, forward = norm3( sub3(to, from) );
    ortho3(&right, &up, forward);

    /* calculate extend */
    vec3 xext = scale3(right, size);
    vec3 yext = scale3(up, size);
    vec3 nxext = scale3(right, -size);
    vec3 nyext = scale3(up, -size);

    /* calculate base vertices */
    poly p = {0};
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * (5+1)); p.cnt = 5; /*+1 for diamond case*/ // array_resize(p.verts, 5+1); p.cnt = 5;
    p.verts[0] = add3(add3(from, xext), yext); /*a*/
    p.verts[1] = add3(add3(from, xext), nyext); /*b*/
    p.verts[2] = add3(add3(from, nxext), nyext); /*c*/
    p.verts[3] = add3(add3(from, nxext), yext); /*d*/
    p.verts[4] = to; /*r*/
    return p;
}

/* pyramid */
poly diamond(vec3 from, vec3 to, float size) {
    vec3 mid = add3(from, scale3(sub3(to, from), 0.5f));
    poly p = pyramid(mid, to, size);
    p.verts[5] = from; p.cnt = 6;
    return p;
}

// ---

static void transform_(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        i[&r->x]  = i[&v.x] * r33[i*3+0];
        i[&r->x] += i[&v.x] * r33[i*3+1];
        i[&r->x] += i[&v.x] * r33[i*3+2];
        i[&r->x] += i[&t3.x];
    }
}
static void transformS(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transform_(v, tmp, r33, t3);
}
static void transformT(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        float p = i[&v.x] - i[&t3.x];
        i[&r->x]  = p * r33[0*3+i];
        i[&r->x] += p * r33[1*3+i];
        i[&r->x] += p * r33[2*3+i];
    }
}
static void transformST(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transformT(v, tmp, r33, t3);
}

/* ============================================================================
 *
 *                                  COLLISION
 *
 * =========================================================================== */

static local hit hits[16] = {0};
static local int hit_index = -1;
#define hit_next() &hits[ (++hit_index) & 15 ]

static float line_closest_line_(float *t1, float *t2, vec3 *c1, vec3 *c2, line l, line m) {
    vec3 r, d1, d2;
    d1 = sub3(l.b, l.a); /* direction vector segment s1 */
    d2 = sub3(m.b, m.a); /* direction vector segment s2 */
    r = sub3(l.a, m.a);

    float i = dot3(d1, d1);
    float e = dot3(d2, d2);
    float f = dot3(d2, r);

    if (i <= C_EPSILON && e <= C_EPSILON) {
        /* both segments degenerate into points */
        vec3 d12;
        *t1 = *t2 = 0.0f;
        *c1 = l.a;
        *c2 = m.a;
        d12 = sub3(*c1, *c2);
        return dot3(d12,d12);
    }
    if (i > C_EPSILON) {
        float c = dot3(d1,r);
        if (e > C_EPSILON) {
            /* non-degenerate case */
            float b = dot3(d1,d2);
            float denom = i*e - b*b;

            /* compute closest point on L1/L2 if not parallel else pick any t2 */
            if (denom != 0.0f)
                *t1 = clampf(0.0f, (b*f - c*e) / denom, 1.0f);
            else *t1 = 0.0f;

            /* cmpute point on L2 closest to S1(s) */
            *t2 = (b*(*t1) + f) / e;
            if (*t2 < 0.0f) {
                *t2 = 0.0f;
                *t1 = clampf(0.0f, -c/i, 1.0f);
            } else if (*t2 > 1.0f) {
                *t2 = 1.0f;
                *t1 = clampf(0.0f, (b-c)/i, 1.0f);
            }
        } else {
            /* second segment degenerates into a point */
            *t1 = clampf(0.0f, -c/i, 1.0f);
            *t2 = 0.0f;
        }
    } else {
        /* first segment degenerates into a point */
        *t2 = clampf(0.0f, f/e, 1.0f);
        *t1 = 0.0f;
    }
    /* calculate closest points */
    vec3 n, d12;
    n = scale3(d1, *t1);
    *c1 = add3(l.a, n);
    n = scale3(d2, *t2);
    *c2 = add3(m.a, n);

    /* calculate squared distance */
    d12 = sub3(*c1, *c2);
    return dot3(d12,d12);
}

vec3 line_closest_point(line l, vec3 p) {
    vec3 ab = sub3(l.b,l.a), pa = sub3(p,l.a);
    float t = dot3(pa,ab) / dot3(ab,ab);
    return add3(l.a, scale3(ab, t < 0 ? 0 : t > 1 ? 1 : t));
}
float line_distance2_point(line l, vec3 p) {
    vec3 ab = sub3(l.a,l.b), ap = sub3(l.a,p), bp = sub3(l.b,p);
    /* handle cases p proj outside ab */
    float e = dot3(ap,ab); if (e <= 0) return dot3(ap,ap);
    float f = dot3(ab,ab); if (e >= f) return dot3(bp,bp);
    return dot3(ap,ap) - (e*e)/f;
}
float ray_test_plane(ray r, vec4 plane) {
    /* Ray: P = origin + rd * t
     * Plane: plane_normal * P + d = 0
     *
     * Substitute:
     *      normal * (origin + rd*t) + d = 0
     *
     * Solve for t:
     *      plane_normal * origin + plane_normal * rd*t + d = 0
     *      -(plane_normal*rd*t) = plane_normal * origin + d
     *
     *                  plane_normal * origin + d
     *      t = -1 * -------------------------
     *                  plane_normal * rd
     *
     * Result:
     *      Behind: t < 0
     *      Infront: t >= 0
     *      Parallel: t = 0
     *      Intersection point: ro + rd * t
     */
    vec3 p = ptr3(&plane.x);
    float n = -(dot3(p,r.p) + plane.w);
    if (fabs(n) < 0.0001f) return 0.0f;
    return n/(dot3(p,r.d));
}
float ray_test_triangle(ray r, triangle tr) {
    float t = 0;
    vec3 di0, di1, di2;
    vec3 d21, d02, in;
    vec3 n, d10, d20;
    vec3 in0, in1, in2;

    /* calculate triangle normal */
    d10 = sub3(tr.p1,tr.p0);
    d20 = sub3(tr.p2,tr.p0);
    d21 = sub3(tr.p2,tr.p1);
    d02 = sub3(tr.p0,tr.p2);
    n = cross3(d10,d20);

    /* check for plane intersection */
    vec4 p = plane4(tr.p0, n);
    t = ray_test_plane(r, p);
    if (t <= 0.0f) return t;

    /* intersection point */
    in = scale3(r.d,t);
    in = add3(in,r.p);

    /* check if point inside triangle in plane */
    di0 = sub3(in, tr.p0);
    di1 = sub3(in, tr.p1);
    di2 = sub3(in, tr.p2);

    in0 = cross3(d10, di0);
    in1 = cross3(d21, di1);
    in2 = cross3(d02, di2);

    if (dot3(in0,n) < 0.0f)
        return -1;
    if (dot3(in1,n) < 0.0f)
        return -1;
    if (dot3(in2,n) < 0.0f)
        return -1;
    return t;
}
int ray_test_sphere(float *t0, float *t1, ray r, sphere s) {
    vec3 a;
    float tc,td,d2,r2;
    a = sub3(s.c,r.p);
    tc = dot3(r.d,a);
    if (tc < 0) return 0;

    r2 = s.r*s.r;
    d2 = dot3(a,a) - tc*tc;
    if (d2 > r2) return 0;
    td = sqrtf(r2 - d2);

    *t0 = tc - td;
    *t1 = tc + td;
    return 1;
}
int ray_test_aabb(float *t0, float *t1, ray r, aabb a) {
    float t0x = (a.min.x - r.p.x) / r.d.x;
    float t0y = (a.min.y - r.p.y) / r.d.y;
    float t0z = (a.min.z - r.p.z) / r.d.z;
    float t1x = (a.max.x - r.p.x) / r.d.x;
    float t1y = (a.max.y - r.p.y) / r.d.y;
    float t1z = (a.max.z - r.p.z) / r.d.z;

    float tminx = minf(t0x, t1x);
    float tminy = minf(t0y, t1y);
    float tminz = minf(t0z, t1z);
    float tmaxx = maxf(t0x, t1x);
    float tmaxy = maxf(t0y, t1y);
    float tmaxz = maxf(t0z, t1z);
    if (tminx > tmaxy || tminy > tmaxx)
        return 0;

    *t0 = maxf(tminx, tminy);
    *t1 = minf(tmaxy, tmaxx);
    if (*t0 > tmaxz || tminz> *t1)
        return 0;

    *t0 = maxf(*t0, tminz);
    *t1 = minf(*t1, tmaxz);
    return 1;
}
vec3 sphere_closest_point(sphere s, vec3 p) {
    vec3 d = norm3(sub3(p, s.c));
    return add3(s.c, scale3(d,s.r));
}
int sphere_test_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    if (dot3(d,d) > r*r)
        return 0;
    return 1;
}
hit *sphere_hit_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    float d2 = dot3(d,d);
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->depth = r - l;
    d = scale3(m->normal, b.r);
    m->contact_point = sub3(b.c, d);
    return m;
}
int sphere_test_aabb(sphere s, aabb a) {
    return aabb_test_sphere(a, s);
}
hit *sphere_hit_aabb(sphere s, aabb a) {
    /* find closest aabb point to sphere center point */
    vec3 ap = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, ap);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    hit *m = hit_next();
    /* calculate distance vector between sphere and aabb center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(ac, s.c);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    m->normal = d;
    m->contact_point = scale3(m->normal, s.r);
    m->contact_point = add3(s.c, m->contact_point);

    /* calculate penetration depth */
    vec3 sp = sphere_closest_point(s, ap);
    d = sub3(sp, ap);
    m->depth = sqrtf(dot3(d,d)) - l;
    return m;
}
int sphere_test_capsule(sphere s, capsule c) {
    return capsule_test_sphere(c, s);
}
hit *sphere_hit_capsule(sphere s, capsule c) {
#if 0
        // original code
        /* find closest capsule point to sphere center point */
        hit *m = hit_next();
        vec3 cp = capsule_closest_point(c, s.c);
        m->normal = sub3(cp, s.c);
        float d2 = dot3(m->normal, m->normal);
        if (d2 > s.r*s.r) return 0;

        /* normalize hit normal vector */
        m->normal = norm3(m->normal);

        /* calculate penetration depth */
        m->depth = d2 - s.r*s.r;
        m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
        m->contact_point = add3(s.c, scale3(m->normal, s.r));
        return m;
#else
        // aproximation of I would expect this function to return instead
        vec3 l = sub3(c.a,c.b); float len = len3(l);
        vec3 d = norm3(l);
        ray r = ray(add3(c.a,scale3(d,-2*len)), d);
        s.r += c.r;
        hit *h = ray_hit_sphere(r, s);
        if(!h) return 0;
        s.r -= c.r;
        h->contact_point = add3(s.c,scale3(norm3(sub3(h->contact_point,s.c)),s.r));
        return h;
#endif
}
int sphere_test_poly(sphere s, poly p) {
    return poly_test_sphere(p, s);
}
void aabb_rebalance_transform(aabb *b, aabb a, mat33 m, vec3 t) {
    for (int i = 0; i < 3; ++i) {
        i[&b->min.x] = i[&b->max.x] = i[&t.x];
        for (int j = 0; j < 3; ++j) {
            float e = m[i*3+j] * j[&a.min.x];
            float f = m[i*3+j] * j[&a.max.x];
            if (e < f) {
                i[&b->min.x] += e;
                i[&b->max.x] += f;
            } else {
                i[&b->min.x] += f;
                i[&b->max.x] += e;
            }
        }
    }
}
vec3 aabb_closest_point(aabb a, vec3 p) {
    vec3 res;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) v = i[&a.min.x];
        if (v > i[&a.max.x]) v = i[&a.max.x];
        i[&res.x] = v;
    }
    return res;
}
float aabb_distance2_point(aabb a, vec3 p) {
    float r = 0;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) r += (i[&a.min.x]-v) * (i[&a.min.x]-v);
        if (v > i[&a.max.x]) r += (v-i[&a.max.x]) * (v-i[&a.max.x]);
    } return r;
}
int aabb_contains_point(aabb a, vec3 p) {
    if (p.x < a.min.x || p.x > a.max.x) return 0;
    if (p.y < a.min.y || p.y > a.max.y) return 0;
    if (p.z < a.min.z || p.z > a.max.z) return 0;
    return 1;
}
int aabb_test_aabb(aabb a, aabb b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return 0;
    return 1;
}
hit *aabb_hit_aabb(aabb a, aabb b) {
    if (!aabb_test_aabb(a, b))
        return 0;

    hit *m = hit_next();
    /* calculate distance vector between both aabb center points */
    vec3 ac, bc, d;
    ac = sub3(a.max, a.min);
    bc = sub3(b.max, b.min);

    ac = scale3(ac, 0.5f);
    bc = scale3(bc, 0.5f);

    ac = add3(a.min, ac);
    bc = add3(b.min, bc);
    d = sub3(bc, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate contact point */
    m->normal = d;
    m->contact_point = aabb_closest_point(a, bc);
    d = sub3(m->contact_point, ac);

    /* calculate penetration depth */
    float r2 = dot3(d,d);
    float r = sqrtf(r2);
    m->depth = r - l;
    return m;
}
int aabb_test_sphere(aabb a, sphere s) {
    /* compute squared distance between sphere center and aabb */
    float d2 = aabb_distance2_point(a, s.c);
    /* intersection if distance is smaller/equal sphere radius*/
    return d2 <= s.r*s.r;
}
hit *aabb_hit_sphere(aabb a, sphere s) {
    /* find closest aabb point to sphere center point */
    hit *m = hit_next();
    m->contact_point = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, m->contact_point);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    /* calculate distance vector between aabb and sphere center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(s.c, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate penetration depth */
    m->normal = d;
    d = sub3(m->contact_point, ac);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int aabb_test_capsule(aabb a, capsule c) {
    return capsule_test_aabb(c, a);
}
hit *aabb_hit_capsule(aabb a, capsule c) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(cp, ac);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = ap;
    return m;
}
int aabb_test_poly(aabb a, poly p) {
    return poly_test_aabb(p, a);
}
float capsule_distance2_point(capsule c, vec3 p) {
    float d2 = line_distance2_point(line(c.a,c.b), p);
    return d2 - (c.r*c.r);
}
vec3 capsule_closest_point(capsule c, vec3 p) {
    /* calculate closest point to internal capsule segment */
    vec3 pp = line_closest_point(line(c.a,c.b), p);

    /* extend point out by radius in normal direction */
    vec3 d = norm3(sub3(p,pp));
    return add3(pp, scale3(d, c.r));
}
int capsule_test_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    return d2 <= r*r;
}
hit *capsule_hit_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    /* calculate normal from both closest points for each segement */
    vec3 cp, d;
    m->normal = sub3(c2, c1);
    m->normal = norm3(m->normal);

    /* calculate contact point from closest point and depth */
    m->contact_point = capsule_closest_point(a, c2);
    cp = capsule_closest_point(b, c1);
    d = sub3(c1, cp);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int capsule_test_sphere(capsule c, sphere s) {
    /* squared distance bwetween sphere center and capsule line segment */
    float d2 = line_distance2_point(line(c.a,c.b), s.c);
    float r = s.r + c.r;
    return d2 <= r * r;
}
hit *capsule_hit_sphere(capsule c, sphere s) {
    /* find closest capsule point to sphere center point */
    hit *m = hit_next();
    m->contact_point = capsule_closest_point(c, s.c);
    m->normal = sub3(s.c, m->contact_point);
    float d2 = dot3(m->normal, m->normal);
    if (d2 > s.r*s.r) return 0;

    /* normalize hit normal vector */
    float l = d2 != 0.0f ? sqrtf(d2): 1;
    float linv = 1.0f/l;
    m->normal = scale3(m->normal, linv);

    /* calculate penetration depth */
    m->depth = d2 - s.r*s.r;
    m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
    return m;
}
int capsule_test_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = scale3(sub3(a.max, a.min), 0.5f);

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 p = capsule_closest_point(c, ac);
    return aabb_contains_point(a, p);
}
hit *capsule_hit_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(ac, cp);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = cp;
    return m;
}
int capsule_test_poly(capsule c, poly p) {
    return poly_test_capsule(p, c);
}
int line_support(vec3 *support, vec3 d, vec3 a, vec3 b) {
    int i = 0;
    float adot = dot3(a, d);
    float bdot = dot3(b, d);
    if (adot < bdot) {
        *support = b;
        i = 1;
    } else *support = a;
    return i;
}
int poly_support(vec3 *support, vec3 d, poly p) {
    int imax = 0;
    float dmax = dot3(*p.verts, d);
    for (int i = 1; i < p.cnt; ++i) {
        /* find vertex with max dot product in direction d */
        float dot = dot3(p.verts[i], d);
        if (dot < dmax) continue;
        imax = i, dmax = dot;
    } *support = p.verts[imax];
    return imax;
}
int poly_hit_sphere(struct gjk_result *res,
    poly p,
    sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, p);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_test_sphere(poly p, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere(&res, p, s);
}
int poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere_transform(&res, p, pos3, rot33, s);
}
int poly_hit_capsule(struct gjk_result *res, poly p, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support s = {0};
    s.a = *p.verts;
    s.b = c.a;
    d = sub3(s.b, s.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &s, &d)) {
        vec3 n = scale3(d, -1);
        s.aid = poly_support(&s.a, n, p);
        s.bid = line_support(&s.b, d, c.a, c.b);
        d = sub3(s.b, s.a);
    }
    /* check distance between closest points */
    assert(gsx.iter < gsx.max_iter);
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule(poly p, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule(&res, p, c);
}
int poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = c.a;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        gs.bid = line_support(&gs.b, d, c.a, c.b);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule_transform(&res, p, pos3, rot33, c);
}
int poly_hit_poly_transform(struct gjk_result *res,
    poly a, vec3 at3, mat33 ar33,
    poly b, vec3 bt3, mat33 br33) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    transformS(&gs.a, ar33, at3);
    transformS(&gs.b, br33, bt3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        /* transform direction */
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, ar33, at3);
        vec3 db; transformT(&db, d, br33, bt3);
        /* run support function on tranformed directions  */
        gs.aid = poly_support(&gs.a, da, a);
        gs.bid = poly_support(&gs.b, db, b);
        /* calculate distance vector on transformed points */
        transformS(&gs.a, ar33, at3);
        transformS(&gs.b, br33, bt3);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_hit_poly(struct gjk_result *res,
    poly a,
    poly b) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, a);
        gs.bid = poly_support(&gs.b, d, b);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_test_poly(poly a, poly b) {
    struct gjk_result res;
    return poly_hit_poly(&res, a, b);
}
int poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33,
    poly b, vec3 bpos3, mat33 brot33) {
    struct gjk_result res;
    return poly_hit_poly_transform(&res, a, apos3, arot33, b, bpos3, brot33);
}
int poly_hit_aabb(struct gjk_result *res, poly p, aabb a) {
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly(res, p, poly(&box[0], 8));
}
int poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a) {
    vec3 zero = {0};
    vec3 id[3] = {{1,0,0},{0,1,0},{0,0,1}};
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly_transform(res, p, pos3, rot33, poly(&box[0], 8), zero, &id[0].x);
}
int poly_test_aabb(poly p, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb(&res, p, a);
}
int poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb_transform(&res, p, apos3, arot33, a);
}

/* ============================================================================
 *
 *                              COLLISION VOLUME
 *
 * =========================================================================== */

hit *ray_hit_plane(ray r, plane p) {
    vec4 pf = plane4(p.p, p.n);
    float t = ray_test_plane(r, pf);
    if (t <= 0.0f) return 0;
    hit *o = hit_next();
    o->p = add3(r.p, scale3(r.d, t));
    o->t0 = o->t1 = t;
    o->n = scale3(p.n, -1.0f);
    return o;
}
hit *ray_hit_triangle(ray r, triangle tr) {
    float t = ray_test_triangle(r, tr);
    if (t <= 0) return 0;

    hit *o = hit_next();
    o->t0 = o->t1 = t;
    o->p = add3(r.p, scale3(r.d, t));
    o->n = norm3(cross3(sub3(tr.p1,tr.p0),sub3(tr.p2,tr.p0)));
    return o;
}
hit *ray_hit_sphere(ray r, sphere s) {
    hit *o = hit_next();
    if (!ray_test_sphere(&o->t0, &o->t1, r, s))
        return 0;
    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    o->n = norm3(sub3(o->p, s.c));
    return o;
}
hit *ray_hit_aabb(ray r, aabb a) {
    hit *o = hit_next();

    vec3 pnt, ext, c;
    float d, min;
    if (!ray_test_aabb(&o->t0, &o->t1, r, a))
        return 0;

    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    ext = sub3(a.max, a.min);
    c = add3(a.min, scale3(ext,0.5f));
    pnt = sub3(o->p, c);

    min = fabs(ext.x - fabs(pnt.x));
    o->n = scale3(vec3(1,0,0), signf(pnt.x));
    d = fabs(ext.y - fabs(pnt.y));
    if (d < min) {
        min = d;
        o->n = scale3(vec3(0,1,0), signf(pnt.y));
    }
    d = fabs(ext.z - fabs(pnt.z));
    if (d < min)
        o->n = scale3(vec3(0,0,1), signf(pnt.z));
    return o;
}

frustum frustum_build(mat44 pv) {
    frustum f;
    f.l = vec4(pv[ 3]+pv[ 0], pv[ 7]+pv[ 4], pv[11]+pv[ 8], pv[15]+pv[12]);
    f.r = vec4(pv[ 3]-pv[ 0], pv[ 7]-pv[ 4], pv[11]-pv[ 8], pv[15]-pv[12]);
    f.t = vec4(pv[ 3]-pv[ 1], pv[ 7]-pv[ 5], pv[11]-pv[ 9], pv[15]-pv[13]);
    f.b = vec4(pv[ 3]+pv[ 1], pv[ 7]+pv[ 5], pv[11]+pv[ 9], pv[15]+pv[13]);
    f.n = vec4(pv[ 3]+pv[ 2], pv[ 7]+pv[ 6], pv[11]+pv[10], pv[15]+pv[14]);
    f.f = vec4(pv[ 3]-pv[ 2], pv[ 7]-pv[ 6], pv[11]-pv[10], pv[15]-pv[14]);
    for (int i = 0; i < 6; i++) f.pl[i] = div4(f.pl[i], len3(f.pl[i].xyz));
    return f;
}
int frustum_test_sphere(frustum f, sphere s) {
    for(int i = 0; i < 6; i++) {
        if((dot3(f.pl[i].xyz, s.c) + f.pl[i].w + s.r) < 0) return 0;
    }
    return 1;
}
int frustum_test_aabb(frustum f, aabb a) {
    for(int i = 0; i < 6; i++) {
        vec3 v = vec3(f.pl[i].x > 0 ? a.max.x : a.min.x, f.pl[i].y > 0 ? a.max.y : a.min.y, f.pl[i].z > 0 ? a.max.z : a.min.z);
        if((dot3(f.pl[i].xyz, v) + f.pl[i].w) < 0) return 0;
    }
    return 1;
}
#line 0
#line 1 "fwk_cooker.c"
// data pipeline
// - rlyeh, public domain.
// ----------------------------------------------------------------------------
// @todo: threads should steal workloads from job queue
// @todo: restore errno/errorlevel checks
// @todo: +=, -=, that_asset.ini
// @todo: @dae FLAGS+=-U
// @todo: SF2_SOUNDBANK=TOOLS/soundbank.sf2
// @fixme: leaks (worth?)
// -----------------------------------------------------------------------------

const char *ART = "art/";
const char *TOOLS = ifdef(win32, "art/tools/", "art//tools/");
const char *FWK_INI = "fwk.ini";

typedef struct cook_script_t {
    char *infile;     // free after use
    char *finalfile;  // free after use. can be either infile or a totally different file
    char *script;
    int compress_level;
} cook_script_t;

static
cook_script_t cook_script(const char *rules, const char *infile, const char *outfile) {
    // by default, assume:
    // - no script is going to be generated (empty script)
    // - if no script is going to be generated, output is in fact input file.
    // - no compression is going to be required.
    cook_script_t cs = { 0 };

    // reuse script heap from last call if possible (optimization)
    static __thread char *script = 0;
    if(script) script[0] = 0;

    // reuse parsing maps if possible (optimization)
    static __thread map(char*, char*) symbols = 0;
    static __thread map(char*, char*) groups = 0;

    if(!symbols) map_init(symbols, less_str, hash_str);
    if(!groups)  map_init(groups, less_str, hash_str);

    map_find_or_add(symbols, "INFILE", STRDUP(infile));
    map_find_or_add(symbols, "INPUT", STRDUP(infile));
    map_find_or_add(symbols, "OUTPUT", STRDUP(outfile));
    map_find_or_add(symbols, "TOOLS", STRDUP(TOOLS));
    map_find_or_add(symbols, "PROGRESS", STRDUP(va("%03d", cooker_progress())));

    // start parsing. parsing is enabled by default
    int enabled = 1;
    array(char*)lines = strsplit(rules, "\r\n");
    for( int i = 0, end = array_count(lines); i < end; ++i ) {
        // skip blanks
        int blanks = strspn(lines[i], " \t");
        char *line = lines[i] + blanks;

        // discard full comments
        if( line[0] == ';' ) continue;
        // truncate inline comments
        if( strstr(line, ";") ) *strstr(line, ";") = 0;
        // trim ending spaces
        char *eos = line + strlen(line); while(eos > line && eos[-1] == ' ' ) *--eos = 0;
        // discard non-specific lines
        if( line[0] == '@' ) {
            int with_wine = flag("--with-wine") && !!strstr(line, "@win");
            int parse = 0
                | ifdef(win32, !!strstr(line, "@win"), 0)
                | ifdef(linux, !!strstr(line, "@lin") ? 1 : with_wine, 0)
                | ifdef(osx,   !!strstr(line, "@osx") ? 1 : with_wine, 0);

            if( !parse ) continue;

            line = strchr(line+1, ' ');
            if(!line) continue;
            line += strspn(line, " \t");
        }
        // execute `shell` commands
        if( line[0] == '`' ) {
            char *eos = strrchr(++line, '`');
            if( eos ) *eos = 0;

                // replace all symbols
                char* nl = STRDUP(line);
                for each_map(symbols, char*, key, char*, val) {
                    strrepl(&nl, key, val);
                }
                lines[i] = line = nl; // @fixme:leak

            static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
            thread_mutex_lock( &lock );
                system(line); // strcatf(&script, "%s\n", line);
            thread_mutex_unlock( &lock );

            continue;
        }
        // process [sections]
        if( line[0] == '[' ) {
            enabled = 1;
            int is_cook = !!strstr(line, "[cook]");
            int is_compress = !!strstr(line, "[compress]");
            if( !is_cook && !is_compress ) {
                // remove hint cook tag if present. that's informative only.
                if(strbegi(line, "[cook ") ) memcpy(line+1, "    ", 4);

                // start parsing expressions like `[media && !avi && mp3]`
                array(char*) tags = strsplit(line, " []&");

                // let's check whether INPUT belongs to tags above
                char **INPUT = map_find(symbols, "INPUT");
                bool found_in_set = true;

                for( int i = 0, end = array_count(tags); i < end; ++i) { char *tag = tags[i];
                    bool negate = false;
                    while(*tag == '!') negate ^= 1, ++tag;

                    // find tag in groups map
                    // either a group or an extension
                    char **is_group = map_find(groups, tag);
                    if( is_group ) {
                        char *list = *is_group;
                        char *INPUT_EXT = file_ext(infile); INPUT_EXT = strrchr(INPUT_EXT, '.'); // .ext1.ext -> .ext
                        char *ext = INPUT_EXT; ext += ext[0] == '.'; // dotless
                        bool in_list = strbegi(list, ext) || strstri(list, va(",%s,",ext)) || strendi(list, va(",%s",ext));
                        if( !in_list ^ negate ) { found_in_set = false; break; }
                    } else {
                        char *ext = va(".%s", tag);
                        bool found = !!strendi(*INPUT, ext);
                        if( !found ^ negate ) { found_in_set = false; break; }
                    }
                }
                //
                enabled = found_in_set ? 1 : 0;
            }
        }
        // either SYMBOL=, group=, or regular script line
        if( enabled && line[0] != '[' ) {
            enum { group, symbol, regular } type = regular;
            int tokenlen = strspn(line, "-+_.|0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
            char *token = va("%.*s", tokenlen, line);
            char *equal = strchr(line, '=');
            if( equal ) {
                if( equal == &line[tokenlen] ) { // if key=value expression found
                    // discriminate: symbols are uppercase and never begin with digits. groups are [0-9]+[|][a-z].
                    type = strcmp(strupper(token), token) || isdigit(token[0]) ? group : symbol;
                }
            }
            if( type == group  ) map_find_or_add(groups, token, STRDUP(equal+1));
            if( type == symbol ) {
                // @todo: perform the replacement/union/intersection on set here
                bool is_add = strendi(token, "+");
                bool is_del = strendi(token, "-");

                // if present, remove last sign from token -> (FLAGS1+)=, (FLAGS1-)=
                if(is_add || is_del) token[strlen(token) - 1] = 0;

                map_find_or_add(symbols, token, STRDUP(equal+1));
            }
            // for each_map(symbols, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
            // for each_map(groups, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
            // if( type != regular ) printf("%s found >> %s\n", type == group ? "group" : "symbol", line);

            if( type == regular ) {
                char** INPUT = map_find(symbols, "INPUT");
                char** OUTPUT = map_find(symbols, "OUTPUT");

                // parse return code
                char *has_errorlevel = strstr(line, "=="); //==N form
                int errorlevel = has_errorlevel ? atoi(has_errorlevel + 2) : 0;
                if( has_errorlevel ) memcpy(has_errorlevel, "   ", 3);

                // detect if newer extension is present, and thus update OUTPUT if needed
                char *newer_extension = strstr(line, "->"); if(newer_extension) {
                    *newer_extension = 0;
                    newer_extension += 2 + strspn(newer_extension + 2, " ");

                    strcatf(&*OUTPUT, ".%s", newer_extension);
                }

                // replace all symbols
                char* nl = STRDUP(line);
                for each_map(symbols, char*, key, char*, val) {
                    strrepl(&nl, key, val);
                }
                lines[i] = line = nl; // @fixme:leak

                // convert slashes
                ifdef(win32,
                    strswap(line, "/", "\\")
                , // else
                    strswap(line, "\\", "/")
                );

                // append line
                strcatf(&script, "%s\n", line);

                // handle return code here
                // if(has_errorlevel)
                // strcatf(&script, "IF NOT '%%ERRORLEVEL%%'=='%d' echo ERROR!\n", errorlevel);

                // rename output->input for further chaining, in case it is needed
                if( newer_extension ) {
                    *INPUT[0] = 0;
                    strcatf(&*INPUT, "%s", *OUTPUT);
                }
            }
        }
    }

    // compression
    char* ext = file_ext(infile); ext = strrchr(ext, '.'); ext += ext[0] == '.'; // dotless INPUT_EXT

    char** OUTPUT = map_find(symbols, "OUTPUT");
    char* belongs_to = 0;
    for each_map(groups, char*, key, char*, val) {
        if( !isdigit(key[0]) ) {
            char *comma = va(",%s,", ext);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                belongs_to = key;
                //goto break1; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
        }
    }
    break1:;
    char *compression = 0;
    for each_map(groups, char*, key, char*, val) {
        if( isdigit(key[0]) ) {
            char *comma = va(",%s,", ext);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                compression = key;
                //goto break2; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
            comma = va(",%s,", belongs_to);
            if( strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                compression = key;
                //goto break2; // each_map() macro is made of multiple for(;;)s. goto needed; you cant escape with single break.
            }
        }
    }
    break2:;

    cs.compress_level = 0;
    if( compression ) {
        // last chance to optionally override the compressor at command-line level
        static const char *user_compression, **init = 0;
        if( !init ) *(init = &user_compression) = option("--with-compressor", "");
        if( user_compression[0] ) compression = (char*)user_compression;

        /**/ if(strstr(compression, "PPP"))  cs.compress_level = atoi(compression) | PPP;
        else if(strstr(compression, "ULZ"))  cs.compress_level = atoi(compression) | ULZ;
        else if(strstr(compression, "LZ4"))  cs.compress_level = atoi(compression) | LZ4X;
        else if(strstr(compression, "CRSH")) cs.compress_level = atoi(compression) | CRSH;
        else if(strstr(compression, "DEFL")) cs.compress_level = atoi(compression) /*| DEFL*/;
        //else if(strstr(compression, "LZP"))  cs.compress_level = atoi(compression) | LZP1; // not supported
        else if(strstr(compression, "LZMA")) cs.compress_level = atoi(compression) | LZMA;
        else if(strstr(compression, "BALZ")) cs.compress_level = atoi(compression) | BALZ;
        else if(strstr(compression, "LZW"))  cs.compress_level = atoi(compression) | LZW3;
        else if(strstr(compression, "LZSS")) cs.compress_level = atoi(compression) | LZSS;
        else if(strstr(compression, "BCM"))  cs.compress_level = atoi(compression) | BCM;
        else                                 cs.compress_level = atoi(compression) /*| DEFL*/;
    }

    // if script was generated...
    if( script && script[0]) {
        // update outfile
        cs.finalfile = *OUTPUT;

        // amalgamate script
        array(char*) lines = strsplit(script, "\r\n");

        #if is(win32)
            char *joint = strjoin(lines, " && ");
            cs.script = joint;
        #else
            if( flag("--with-wine") ) {
                // dear linux/osx/bsd users:
                // tools going wrong for any reason? cant compile them maybe?
                // small hack to use win32 pipeline tools instead
                char *joint = strjoin(lines, " && wine " );
                cs.script = va("wine %s", TOOLS, joint);
            } else {
                char *joint = strjoin(lines, " && " );
                cs.script = va("export LD_LIBRARY_PATH=%s && %s", TOOLS, joint);
            }
        #endif
    } else {
        // ... else bypass infile->outfile
        char** INFILE = map_find(symbols, "INFILE");
        cs.finalfile = *INFILE;

        // and return an empty script
        cs.script = "";
    }

    map_clear(symbols);
    map_clear(groups);
    return cs;
}

// ----------------------------------------------------------------------------

struct fs {
    char *fname, status;
    uint64_t stamp;
    uint64_t bytes;
};

static array(struct fs) fs_now;
static local array(char*) added;
static local array(char*) changed;
static local array(char*) deleted;
static local array(char*) uncooked;

static
array(struct fs) zipscan_filter(int threadid, int numthreads) {
    // iterate all previously scanned files
    array(struct fs) fs = 0;
    for( int i = 0, end = array_count(fs_now); i < end; ++i ) {
        const char *fname = fs_now[i].fname;

        // skip if list item does not belong to this thread bucket
        uint64_t hash = hash_str(fname);
        unsigned bucket = (hash % numthreads);
        if(bucket != threadid) continue;

        array_push(fs, fs_now[i]);
    }
    return fs;
}

static
int zipscan_diff( zip* old, array(struct fs) now ) {
    array_free(added);
    array_free(changed);
    array_free(deleted);
    array_free(uncooked);

    // if not zipfile is present, all files are new and must be added
    if( !old ) {
        for( int i = 0; i < array_count(now); ++i ) {
            array_push(uncooked, STRDUP(now[i].fname));
        }
        return 1;
    }

    // compare for new & changed files
    for( int i = 0; i < array_count(now); ++i ) {
        int found = zip_find(old, now[i].fname);
        if( found < 0 ) {
            array_push(added, STRDUP(now[i].fname));
            array_push(uncooked, STRDUP(now[i].fname));
        } else {
            uint64_t oldsize = atoi64(zip_comment(old,found)); // zip_size(old, found); returns sizeof processed asset. return original size of unprocessed asset, which we store in comment section
            uint64_t oldstamp = atoi64(zip_modt(old, found)+20);
            if( oldsize != now[i].bytes || abs(oldstamp - now[i].stamp) > 1 ) { // @fixme: should use hash instead. hashof(tool) ^ hashof(args used) ^ hashof(rawsize) ^ hashof(rawdate)
                printf("%s:\t%llu vs %llu, %llu vs %llu\n", now[i].fname, (uint64_t)oldsize,(uint64_t)now[i].bytes, (uint64_t)oldstamp,(uint64_t)now[i].stamp);
                array_push(changed, STRDUP(now[i].fname));
                array_push(uncooked, STRDUP(now[i].fname));
            }
        }
    }
    // compare for deleted files
    for( int i = 0; i < zip_count(old); ++i ) {
        char *oldname = zip_name(old, i);
        int idx = zip_find(old, oldname); // find latest versioned file in zip
        unsigned oldsize = zip_size(old, idx);
        if (!oldsize) continue;

        struct fs *found = 0; // zipscan_locate(now, oldname);
        for(int j = 0; j < array_count(now); ++j) {
            if( !strcmp(now[j].fname,oldname)) {
                found = &now[j];
                break;
            }
        }

        if( !found ) {
            array_push(deleted, STRDUP(oldname));
        }
    }
    return 1;
}

// ----------------------------------------------------------------------------

typedef struct cook_worker {
    const char **files;
    const char *rules;
    int threadid, numthreads;
    thread_ptr_t self;
    volatile int progress;
} cook_worker;

static cook_worker jobs[100] = {0};

static
int cook(void *userdata) {
    cook_worker *job = (cook_worker*)userdata;

    // start progress
    volatile int *progress = &job->progress;
    *progress = 0;

    // scan disk from fs_now snapshot
    array(struct fs) filtered = zipscan_filter(job->threadid, job->numthreads);
    //printf("Scanned: %d items found\n", array_count(now));

    // prepare out tempname
    char COOKER_TMPFILE[64]; snprintf(COOKER_TMPFILE, 64, "temp_%02d", job->threadid);

    // prepare zip
    char zipfile[64]; snprintf(zipfile, 64, ".art[%d].zip", job->threadid);
    if( file_size(zipfile) == 0 ) unlink(zipfile);

    // populate added/deleted/changed arrays by examining current disk vs last cache
    zip *z;
    {
        z = zip_open(zipfile, "r+b");
        zipscan_diff(z, filtered);
        if( z ) zip_close(z);

        fflush(0);

        z = zip_open(zipfile, "a+b");
        if( !z ) {
            unlink(zipfile);
            z = zip_open(zipfile, "a+b"); // try again
            if(!z) PANIC("cannot open file for updating: %s", zipfile);
        }
    }

    // deleted files
    for( int i = 0, end = array_count(deleted); i < end; ++i ) {
        printf("Deleting %03d%% %s\n", (i+1) == end ? 100 : (i * 100) / end, deleted[i]);
        FILE* out = fopen(COOKER_TMPFILE, "wb"); fclose(out);
        FILE* in = fopen(COOKER_TMPFILE, "rb");
        char *comment = "0";
        zip_append_file(z, deleted[i], comment, in, 0);
        fclose(in);
    }
    // added or changed files
    for( int i = 0, end = array_count(uncooked); i < end; ++i ) {
        *progress = ((i+1) == end ? 90 : (i * 90) / end); // (i+i>0) * 100.f / end;

        // start cook
        const char *fname = uncooked[i]; //job->files[j];
        int inlen = file_size(fname);

        // generate a cooking script for this asset
        cook_script_t cs = cook_script(job->rules, fname, COOKER_TMPFILE);
        // puts(cs.script);

        // log to batch file for forensic purposes, if explicitly requested
        static __thread bool logging = 0, *init = 0; if(!init) *(init = &logging) = !!flag("--cook-debug");
        if( logging ) {
            FILE *logfile = fopen(va("cook%d.cmd",job->threadid), "a+t");
            if( logfile ) { fprintf(logfile, "@rem %s\n%s\n", fname, cs.script); fclose(logfile); }
            // maybe log fprintf(logfile, "@rem %*.s\n", 4096, os_exec_output()); ?
        }

        // invoke cooking script and recap status
        int rc = cs.script[0] ? os_exec(cs.script) : 0;
        int outlen = file_size(cs.finalfile);
        int failed = cs.script[0] ? rc || !outlen : 0;

        // print errors, or...
        if( failed ) {
            PRINTF("Import failed: %s while executing:\n%s\n%s\nReturned: %d (%#x)\n", fname, cs.script, os_exec_output(), rc, rc);
        }
        // ...process only if included. may include optional compression.
        else if( cs.compress_level >= 0 ) {
            FILE *in = fopen(cs.finalfile, "rb");

                static __thread char cwd[MAX_PATHFILE] = {0};
                if(!cwd[0]) snprintf(cwd, MAX_PATHFILE-1, "%s", app_path()), strswap(cwd, "\\", "/");
                if(strbegi(fname, cwd)) fname += strlen(cwd);
                //puts(fname);

                char *comment = va("%d", inlen);
                if( !zip_append_file(z, fname, comment, in, cs.compress_level) ) {
                    PANIC("failed to add processed file into %s: %s", zipfile, fname);
                }

            fclose(in);
        }
    }

    zip_close(z);

    // end progress
    *progress = 100;
    return 1;
}

static
int cook_async( void *userptr ) {
    while(!window_handle()) sleep_ms(100); // wait for window handle to be created

    int ret = cook(userptr);
    thread_exit( ret );
    return ret;
}

bool cooker_start( const char *masks, int flags ) {
    const char *rules = file_read(FWK_INI);
    if(rules[0] == 0) return false;

    if( strstr(rules, "ART=") ) {
        ART = STRDUP( strstr(rules, "ART=") + 4 ); // @leak
        char *r = strchr( ART, '\r' ); if(r) *r = 0;
        char *n = strchr( ART, '\n' ); if(n) *n = 0;
        char *s = strchr( ART, ';' );  if(s) *s = 0;
        char *w = strchr( ART, ' ' );  if(w) *w = 0;
        strcat((char*)ART, "/");
    }

    // get normalized current working directory (absolute)
    char cwd[MAX_PATHFILE] = {0};
    getcwd(cwd, sizeof(cwd));
    for(int i = 0; cwd[i]; ++i) if(cwd[i] == '\\') cwd[i] = '/';

    // scan disk
    const char **list = file_list(ART, "**");
    // inspect disk
    for( int i = 0; list[i]; ++i ) {
        const char *fname = list[i];

        // also, skip folders and internal files like .art.zip
        // if( file_directory(fname) ) continue;
        // if( fname[0] == '.' ) continue;
        if( fname[0] == '.' ) continue; // discard system dirs and hidden files
        if( strstri(fname, TOOLS) ) continue; // discard tools folder
        if( !file_ext(fname)[0] ) continue; // discard extensionless entries
        if( !file_size(fname)) continue; // skip dirs and empty files

        // exclude vc c/c++ .obj files. they're not 3d wavefront .obj files
        if( strstr(fname, ".obj") || strstr(fname, ".OBJ") ) {
            char header[4] = {0};
            for( FILE *in = fopen(fname, "rb"); in; fclose(in), in = NULL) {
                fread(header, 1, 2, in);
            }
            if( !memcmp(header, "\x64\x86", 2) ) continue;
        }

        // @todo: normalize path & rebase here (absolute to local)
        // [...]
        // fi.normalized = ; tolower->to_underscore([]();:+ )->remove_extra_underscores

        // make buffer writable
        char buffer[MAX_PATHFILE];
        snprintf(buffer, MAX_PATHFILE, "%s", fname);

        // normalize path
        for(int i = 0; buffer[i]; ++i) if(buffer[i] == '\\') buffer[i] = '/';

        // rebase from absolute to relative
        char *buf = buffer; int cwdlen = strlen(cwd);
        if( !strncmp(buf, cwd, cwdlen) ) buf += cwdlen;
        while(buf[0] == '/') ++buf;

        if( file_name(buf)[0] == '.' ) continue; // skip system files

        struct fs fi = {0};
        fi.fname = STRDUP(buf);
        fi.bytes = file_size(buf);
        fi.stamp = file_stamp(buf); // human-readable base10 timestamp

        array_push(fs_now, fi);
    }

    // spawn all the threads
    int num_jobs = cooker_jobs();
    for( int i = 0, end = num_jobs; i < end; ++i ) {
        jobs[i].threadid = i;
        jobs[i].numthreads = end;
        jobs[i].files = list;
        jobs[i].rules = rules;
        jobs[i].progress = -1;
        if( num_jobs > 1 && (flags & COOKER_ASYNC) ) {
            jobs[i].self = thread_init(cook_async, &jobs[i], "cook_async()", 0/*STACK_SIZE*/);
        } else {
            return !!cook(&jobs[0]);
        }
    }
    return true;
}

void cooker_stop() {
    // join all threads
    int num_jobs = cooker_jobs();
    for( int i = 1; i < num_jobs; ++i ) {
        if(jobs[i].self) thread_join(jobs[i].self);
    }
    // remove all temporary outfiles
    const char **temps = file_list("./", "temp_*");
    for( int i = 0; temps[i]; ++i ) unlink(temps[i]);
}

int cooker_progress() {
    int count = 0, sum = 0;
    for( int i = 0, end = cooker_jobs(); i < end; ++i ) {
        if( jobs[i].progress >= 0 ) {
            sum += jobs[i].progress;
            ++count;
        }
    }
    return sum / (count+!count);
}

int cooker_jobs() {
    int num_jobs = optioni("--with-jobs", ifdef(tcc, 1, cpu_cores())), max_jobs = countof(jobs);
    return clampi(num_jobs, 0, max_jobs);
}

void cooker_config( const char *art_path, const char *tools_path, const char *fwk_ini ) { // @todo: test run-from-"bin/" case on Linux.
    const char *rules = file_read(fwk_ini ? fwk_ini : FWK_INI);
    if( rules && rules[0] != 0 ) {
        if( strstr(rules, "ART=") ) {
            ART = STRDUP( strstr(rules, "ART=") + 4 ); // @leak
            char *r = strchr( ART, '\r' ); if(r) *r = 0;
            char *n = strchr( ART, '\n' ); if(n) *n = 0;
            char *s = strchr( ART, ';' );  if(s) *s = 0;
            char *w = strchr( ART, ' ' );  if(w) *w = 0;
            if( ART[strlen(ART) - 1] != '/' ) strcat((char*)ART, "/");
        }
        if( strstr(rules, "TOOLS=") ) {
            TOOLS = STRDUP( strstr(rules, "TOOLS=") + 6 ); // @leak
            char *r = strchr( TOOLS, '\r' ); if(r) *r = 0;
            char *n = strchr( TOOLS, '\n' ); if(n) *n = 0;
            char *s = strchr( TOOLS, ';' );  if(s) *s = 0;
            char *w = strchr( TOOLS, ' ' );  if(w) *w = 0;
            if( TOOLS[strlen(TOOLS) - 1] != '/' ) strcat((char*)TOOLS, "/");
        }
    }

    ART = STRDUP( art_path ? art_path : ART );
    TOOLS = STRDUP( tools_path ? tools_path : TOOLS );
    FWK_INI = STRDUP( fwk_ini ? fwk_ini : FWK_INI );

    assert( ART[strlen(ART) - 1] == '/' );
    assert( TOOLS[strlen(TOOLS) - 1] == '/' );
    assert( FWK_INI[strlen(FWK_INI) - 1] != '/' );
}

// ----------------------------------------------------------------------------

#ifdef FWK_COOKER_STANDALONE
int main(int argc, char **argv) {
    double timer = -clock() / CLOCKS_PER_SEC;

   char *rules = file_read("fwk.ini");

    if( argc > 1 && argv[1][0] != '-' ) {
        cook_script_t cs = cook_script(rules, argv[1], "a.out");
        puts(cs.script);
        exit(1);
    }

    cooker_start("**", 0);
    cooker_stop();

    printf("Ok, %5.2fs.", timer += clock() / CLOCKS_PER_SEC);
}
#endif
#line 0
#line 1 "fwk_data.c"
static array(json5) roots;
static array(char*) sources;

bool data_push(const char *source) {
    char *source_rw = STRDUP(source);
    json5 root = {0};
    char *error = json5_parse(&root, source_rw, 0);
    if( error ) {
        FREE(source_rw);
        return false;
    } else {
        array_push(sources, source_rw);
        array_push(roots, root);
        return true;
    }
}

bool data_pop() {
    if( array_count(roots) > 0 ) {
        FREE(*array_back(sources));
        array_pop(sources);

        json5_free(array_back(roots));
        array_pop(roots);
        return true;
    }
    return false;
}

json5* data_node(const char *keypath) {
    json5 *j = array_back(roots), *r = j;
    for each_substring( keypath, "/[.]", key ) {
        r = 0;
        /**/ if( j->type == JSON5_ARRAY ) r = j = &j->array[atoi(key)];
        else if( j->type == JSON5_OBJECT )
        for( int i = 0; !r && i < j->count; ++i ) {
            if( j->nodes[i].name && !strcmp(j->nodes[i].name, key) ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        if( !j ) break;
    }
    return r;
}

int (data_count)(const char *keypath) {
    json5* j = data_node(keypath);
    return j ? j->count : 0;
}

data_t *data_find(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = data_node(key);
    if( !j ) return NULL;

    static __thread int slot = 0;
    static __thread data_t buf[128] = {0};
    slot = (slot+1) % 128;

    data_t *v = &buf[slot];
    v->i = j ? j->integer : 0;
    if(!v->p && type == 's') v->s = ""; // if_null_string
    return v;
}

data_t data_get(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = data_node(key);

    data_t v = {0};
    v.i = j ? j->integer : 0;
    if(!v.p && type == 's') v.s = ""; // if_null_string
    return v;
}

#line 0
#line 1 "fwk_dll.c"
#ifdef _WIN32
#   include <winsock2.h>
#   define dlopen(name,mode)    (void*)( (name) ? LoadLibraryA(name) : GetModuleHandle(NULL))
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)handle, symbol )
#   define dlclose(handle)      0
#else
#   include <dlfcn.h>
#endif

void* dll(const char *filename, const char *symbol) {
/*
    char *buf, *base = file_name(filename);
    if( file_exists(buf = va("%s", base)) ||
        file_exists(buf = va("%s.dll", base)) ||
        file_exists(buf = va("%s.so", base)) ||
        file_exists(buf = va("lib%s.so", base)) ||
        file_exists(buf = va("%s.dylib", base)) ) {
        filename = buf;
    }
*/
    void *dll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    dll = dll ? dlsym(dll, symbol) : 0;
    return dll;
}

#if 0 // demo: cl demo.c /LD && REM dll
EXPORT int add2(int a, int b) {
    return a + b;
}
int (*adder)() = dll("demo.dll", "add2");
printf("%d\n", adder(2,3));
#endif
#line 0
#line 1 "fwk_file.c"
// -----------------------------------------------------------------------------
// file

char *file_name(const char *pathfile) {
    char *s = strrchr(pathfile, '/'), *t = strrchr(pathfile, '\\');
    return va("%s", s > t ? s+1 : t ? t+1 : pathfile);
}
char *file_path(const char *pathfile) {
    return va("%.*s", (int)(strlen(pathfile)-strlen(file_name(pathfile))), pathfile);
}
char *file_load(const char *filename, int *len) { // @todo: fix leaks
    FILE *fp = filename[0] ? fopen(filename, "rb") : NULL;
    if( fp ) {
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        char *bin = REALLOC(0, sz+1);
        fread(bin,sz,1,fp);
        fclose(fp);
        bin[sz] = 0;
        if(len) *len = (int)sz;
        return bin;
    }
    if (len) *len = 0;
    return 0;
}
char *file_read(const char *filename) { // @todo: fix leaks
    return file_load(filename, NULL);
}
bool file_move(const char *src, const char *dst) {
    bool ok = file_exist(src) && !file_exist(dst) && 0 == rename(src, dst);
    return ok;
}
bool file_delete(const char *pathfile) {
    if( file_exist(pathfile) ) {
        for( int i = 0; i < 10; ++i ) {
            bool ok = 0 == unlink(pathfile);
            if( ok ) return true;
            sleep_ms(10);
        }
        return false;
    }
    return true;
}
uint64_t file_stamp(const char *fname) {
    time_t mtime = (time_t)file_stamp_epoch(fname);
    struct tm *ti = localtime(&mtime);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
static bool file_stat(const char *fname, struct stat *st) {
    // remove ending slashes. win32+tcc does not like them.
    int l = strlen(fname), m = l;
    while( l && (fname[l-1] == '/' || fname[l-1] == '\\') ) --l;
    fname = l == m ? fname : va("%.*s", l, fname);
    return stat(fname, st) >= 0;
}
uint64_t file_stamp_epoch(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_mtime;
}
uint64_t file_size(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_size;
}
bool file_directory( const char *pathfile ) {
    struct stat st;
    return !file_stat(pathfile, &st) ? 0 : S_IFDIR == ( st.st_mode & S_IFMT );
}
bool file_exist(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? false : true;
}
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
#if is(win32)
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '/') copy[i] = '\\'; if(copy[i] == '\'') copy[i] = '\"'; }
#else
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '\\') copy[i] = '/'; if(copy[i] == '\"') copy[i] = '\''; }
#endif
    return copy;
}
#if 0
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
    // lowercases+digits+underscores+slashes only. anything else is truncated.
    for( ; *name ; ++name ) {
        /**/ if( *name >= 'a' && *name <= 'z' ) *s++ = *name;
        else if( *name >= 'A' && *name <= 'Z' ) *s++ = *name - 'A' + 'a';
        else if( *name >= '0' && *name <= '9' ) *s++ = *name;
        else if( *name == '/' || *name == '\\') *s++ = '/';
        else if( *name <= ' ' || *name == '.' ) *s++ = '_';
    } *s++ = 0;
    // remove dupe slashes
    for( name = s = copy, c = '/'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    // remove dupe underlines
    for( name = s = copy, c = '_'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    return copy;
}
char *file_normalize_with_folder(const char *name) {
    char *s = file_normalize(name);
    char *slash = strrchr(s, '/'); if(slash) *slash = 0;
    char *penultimate = strrchr(s, '/'); if(slash) *slash = '/';
    return penultimate ? penultimate+1 : /*slash ? slash+1 :*/ s;
}
#endif
char *file_ext(const char *name) {
    char *b = file_name(name), *s = strchr(b, '.'); //strrchr(name, '.');
    return va("%s", s ? s : "" ); // , name );
}

char *file_id(const char *pathfile) {
    char *dir = file_path(pathfile); for(int i=0;dir[i];++i) dir[i]=tolower(dir[i]);
    char *base = file_name(pathfile); for(int i=0;base[i];++i) base[i]=tolower(base[i]);
#if 0 // extensionless, larry.mid and larry.txt will collide, diffuse.tga and diffuse.png will match
char *ext = strchr(base, '.'); if (ext) ext[0] = '\0'; // remove all extensions
#else // extensionless for audio/images only (materials: diffuse.tga and diffuse.png will match)
char *ext = strrchr(base, '.'); //if (ext) ext[0] = '\0'; // remove all extensions
    if(ext) if( strstr(".jpg.png.bmp.tga.hdr"".", ext) || strstr(".ogg.mp3.wav.mod.xm.flac"".", ext) || strstr(".mp4.ogv.avi.mkv.wmv.mpg.mpeg"".", ext) ) {
        ext = strchr(base, '.');
        ext[0] = '\0'; //strcpy(ext, "_xxx");
    }
#endif

// if (!dir[0]) return base;
    char *stem = va("%s/%s", dir, base); // file_name(dir);

    // /path2/path1/file2_file1 -> file1_file2/path1/path2
    int len = 0;
    int ids_count = 0;
    char ids[64][64] = { 0 };
    // split path stems
    for each_substring(stem, "/\\", key) {
        int tokens_count = 0;
        char* tokens[64] = { 0 };
        // split tokens
        for each_substring(key, "[]()_ ", it) {
            tokens[tokens_count++] = va("%s", it);
        }
        // sort alphabetically
        if( tokens_count > 1 ) qsort(tokens, tokens_count, sizeof(char *), strcmp_qsort);
        // concat sorted token1_token2_...
        char built[256]; *built = 0;
        for( int i = 0; i < tokens_count; ++i ) {
            strlcat(built, "_", 256);
            strlcat(built, tokens[i], 256);
        }
        snprintf( ids[ ids_count ], 64, "%s", &built[1] );
        len += strlen( ids[ ids_count++ ] );
    }
    // concat in inverse order: file/path1/path2/...
    char buffer[512]; buffer[0] = 0;
    for( int it = ids_count; --it >= 0; ) {
        strcat(buffer, ids[it]);
        strcat(buffer, "/");
    }
    return va("%s", buffer);
}
const char** file_list(const char *cwd, const char *masks) {
    ASSERT(strendi(cwd, "/"), "Error: dirs like '%s' must end with slash", cwd);

    static local array(char*) list = 0;
    const char *arg0 = cwd; // app_path();
    int larg0 = strlen(arg0);

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_free(list);

    for each_substring(masks,";",it) {
        int recurse = !!strstr(it, "**");
        #if is(win32)
        char *glob = va("dir %s/b/o:n \"%s\\%s\" 2> NUL", recurse ? "/s":"", cwd, it);
        #else // linux, osx
        char *glob = va("find %s -type f -iname \"%s\" | sort", cwd, it); // @fixme: add non-recursive support
        #endif
        for( FILE *in = popen(glob, "r"); in; pclose(in), in = 0) {
            char buf[1024], *line = buf;
            while( fgets(buf, sizeof(buf), in) ) {
                // clean up
                if( strstr(line, arg0) ) line = buf + larg0;
                if( !memcmp(line, "./", 2) ) line += 2;
                int len = strlen(line); while( len > 0 && line[len-1] < 32 ) line[--len] = 0;
                for(int i = 0; i < len; ++i ) if(line[i] == '\\') line[i] = '/';
                // do not insert system folders/files
                if( strstr(line, "/.") ) continue;
                // insert copy
                #if is(win32)
                char *copy = STRDUP(line); // full path already provided
                #else
                char *copy = STRDUP(va("%s%s", cwd, line)); // need to prepend path
                #endif
                array_push(list, copy);
            }
        }
    }
    array_push(list, 0); // terminator
    return (const char**)list;
}

bool file_copy(const char *src, const char *dst) {
    int ok = 0, BUFSIZE = 1 << 20; // 1 MiB
    static local char *buffer = 0; do_once buffer = REALLOC(0, BUFSIZE);
    for( FILE *in = fopen(src, "rb"); in; fclose(in), in = 0) {
        for( FILE *out = fopen(dst, "wb"); out; fclose(out), out = 0, ok = 1) {
            for( int n; !!(n = fread( buffer, 1, BUFSIZE, in )); ){
                if(fwrite( buffer, 1, n, out ) != n)
                return fclose(in), fclose(out), false;
            }
        }
    }
    return ok;
}

char* file_tempname() {
    static __thread int id;
    return va("%s/fwk-temp.%s.%p.%d", app_temp(), getenv(ifdef(win32, "username", "USER")), &id, rand());
}

FILE *file_temp(void) {
    const char *fname = file_tempname();
    FILE *fp = fopen(fname, "w+b");
    if( fp ) unlink(fname);
    return fp;
}

// -----------------------------------------------------------------------------
// archives

enum { is_zip, is_tar, is_pak, is_dir };

typedef struct archive_dir {
    char* path;
    union {
        int type;
        int size; // for cache only
    };
    union {
        void *archive;
        void *data; // for cache only
        zip* zip_archive;
        tar* tar_archive;
        pak* pak_archive;
    };
    struct archive_dir *next;
} archive_dir;

static archive_dir *dir_mount;
static archive_dir *dir_cache; enum { MAX_CACHED_FILES = 16 }; // @todo: should we cache the cooked contents instead? ie, stbi() result instead of file.png?

struct vfs_entry {
    const char *name;
    const char *id;
    unsigned size;
};
array(struct vfs_entry) vfs_entries;

bool vfs_mount(const char *path) {
    zip *z = NULL; tar *t = NULL; pak *p = NULL; dir *d = NULL;
    int is_folder = ('/' == path[strlen(path)-1]);
    if( is_folder ) d = dir_open(path, "rb");
    if( is_folder && !d ) return 0;
    if( !is_folder ) z = zip_open(path, "rb");
    if( !is_folder && !z ) t = tar_open(path, "rb");
    if( !is_folder && !z && !t ) p = pak_open(path, "rb");
    if( !is_folder && !z && !t && !p ) return 0;

    // normalize input -> "././" to ""
    while (path[0] == '.' && path[1] == '/') path += 2;
    path = STRDUP(path);
    if( z || t || p ) {
    // save local path for archives, so we can subtract those from upcoming requests
    if(strrchr(path,'/')) strrchr(path,'/')[1] = '\0';
    } else if(d) 0[(char*)path] = 0;

    // append to mounted archives
    archive_dir *prev = dir_mount, zero = {0};
    *(dir_mount = REALLOC(0, sizeof(archive_dir))) = zero;
    dir_mount->next = prev;
    dir_mount->path = (char*)path;
    dir_mount->archive = z ? (void*)z : t ? (void*)t : p ? (void*)p : (void*)d;
    dir_mount->type = is_folder ? is_dir : z ? is_zip : t ? is_tar : p ? is_pak : -1;
    ASSERT(dir_mount->type >= 0 && dir_mount->type < 4);

    // append list of files to internal listing
    for( archive_dir *dir = dir_mount; dir ; dir = 0 ) { // for(archive_dir *dir = dir_mount; dir; dir = dir->next) {
        assert(dir->type >= 0 && dir->type < 4);
        unsigned (*fn_count[4])(void*) = {(void*)zip_count, (void*)tar_count, (void*)pak_count, (void*)dir_count};
        char*    (*fn_name[4])(void*, unsigned index) = {(void*)zip_name, (void*)tar_name, (void*)pak_name, (void*)dir_name};
        unsigned (*fn_size[4])(void*, unsigned index) = {(void*)zip_size, (void*)tar_size, (void*)pak_size, (void*)dir_size};

        for( unsigned idx = 0, end = fn_count[dir->type](dir->archive); idx < end; ++idx ) {
            assert(idx < end);
            const char *filename = STRDUP( fn_name[dir->type](dir->archive, idx) );
            const char *fileid = STRDUP( file_id(filename) );
            unsigned filesize = fn_size[dir->type](dir->archive, idx);
            // printf("%u) %s %u [%s]\n", idx, filename, filesize, fileid);
            // append to list
            array_push(vfs_entries, (struct vfs_entry){filename, fileid, filesize});
        }
    }

    return 1;
}
const char** vfs_list(const char *masks) {
    static local array(char*) list = 0;

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_free(list);

    for each_substring(masks,";",it) {
        it = va("*/%s", it);
        // int recurse = !!strstr(it, "**"); // @fixme: support non-recursive
        for( unsigned i = 0; i < array_count(vfs_entries); ++i ) {
            const char *name = vfs_entries[i].name;
            if( strmatch(name, it) ) {
                // insert copy
                char *copy = STRDUP(name);
                array_push(list, copy);
            }
        }
    }

    // sort alphabetically
    qsort(list, array_count(list), sizeof(char *), strcmp_qsort);

    array_push(list, 0); // terminator
    return (const char**)list;
}

static
char *vfs_unpack(const char *pathfile, int *size) { // must free() after use
    // @todo: add cache here
    char *data = NULL;
    for(archive_dir *dir = dir_mount; dir && !data; dir = dir->next) {
        if( dir->type == is_dir ) {
#if 1 // sandboxed
            char buf[512];
            snprintf(buf, sizeof(buf), "%s%s", dir->path, pathfile);
            data = file_load(buf, size);
#endif
        } else {
            int      (*fn_find[3])(void *, const char *) = {(void*)zip_find, (void*)tar_find, (void*)pak_find};
            void*    (*fn_unpack[3])(void *, unsigned) = {(void*)zip_extract, (void*)tar_extract, (void*)pak_extract};
            unsigned (*fn_size[3])(void *, unsigned) = {(void*)zip_size, (void*)tar_size, (void*)pak_size};

            const char* cleanup = pathfile + strbegi(pathfile, dir->path) * strlen(dir->path);
            while (cleanup[0] == '/') ++cleanup;
            int index = fn_find[dir->type](dir->archive, cleanup);
            data = fn_unpack[dir->type](dir->archive, index);
            if( size ) *size = fn_size[dir->type](dir->archive, index);
        }
        // printf("%c trying %s in %s ...\n", data ? 'Y':'N', pathfile, dir->path);
    }
    //wait_ms(1000); // <-- simulate slow hdd
    return data;
}

const char *vfs_resolve(const char *pathfile) {
    // we dont resolve absolute paths. they dont belong to the vfs
    if( pathfile[0] == '/' || pathfile[0] == '\\' || pathfile[1] == ':' ) return pathfile;

    // find best match
    char* id = file_id(pathfile);
    for (int i = array_count(vfs_entries); --i >= 0; ) {
        if (strbegi(vfs_entries[i].id, id) ) {
            return vfs_entries[i].name;
        }
    }

    return pathfile;
}

char* vfs_load(const char *pathfile, int *size_out) { // @todo: fix leaks
    if (!pathfile[0]) return file_load(pathfile, size_out);
    if (pathfile[0] == '/' || pathfile[1] == ':') return file_load(pathfile, size_out);

    //{
    // exclude garbage from material names
    // @todo: exclude double slashs in paths
    char *base = file_name(pathfile); if(strchr(base,'+')) base = strchr(base, '+')+1;
    char *folder = file_path(pathfile);
    pathfile = va("%s%s", folder, base);

    // solve virtual path
    pathfile = va("%s", vfs_resolve(pathfile));
    base = file_name(pathfile);
    folder = file_path(pathfile);
    //}

    int size = 0;
    void *ptr = 0;

    // clean pathfile
    while (pathfile[0] == '.' && pathfile[1] == '/') pathfile += 2;
    while (pathfile[0] == '/') ++pathfile;

    const char *lookup_id = /*file_normalize_with_folder*/(pathfile);

    // search (last item)
    static char last_item[256] = { 0 };
    static void *last_ptr = 0;
    static int   last_size = 0;
    if( !strcmpi(lookup_id, last_item)) {
        ptr = last_ptr;
        size = last_size;
    }

    // search (cache)
    if( !ptr ) {
        ptr = cache_lookup(lookup_id, &size);
    }

    if( ptr ) {
        PRINTF("Loading VFS (%s)%s (cached)\n", folder, base);
    } else {
        PRINTF("Loading VFS (%s)%s\n", folder, base);
    }

    // search (mounted disks)
    if( !ptr ) {
        ptr = vfs_unpack(pathfile, &size);
        if( ptr ) {
            cache_insert(lookup_id, ptr, size);
        }
    }

    if( ptr && size )
    if( ptr != last_ptr) {
        snprintf(last_item, 256, "%s", lookup_id);
        last_ptr = ptr;
        last_size = size;
    }

    if(!ptr) {
        PRINTF("Loading %s (not found)\n", pathfile);
    }

    if( size_out ) *size_out = ptr ? size : 0;
    return ptr;
}
char* vfs_read(const char *pathfile) {
    return vfs_load(pathfile, NULL);
}
int vfs_size(const char *pathfile) {
    int sz;
    return vfs_load(pathfile, &sz), sz;
}


FILE* vfs_handle(const char *pathfile) { // preferred way, will clean descriptors at exit
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    FILE *fp = fmemopen(buf ? buf : "", buf ? sz : 0, "a+b");
    ASSERT( fp, "cannot create tempfile" );
    return fp;
}
const char *vfs_find(const char *pathfile) {
#if 0
    FILE* fp = vfs_handle(pathfile);
    return fp ? pathfile_from_handle(fp) : "";
#else
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    if( !buf ) return "";

    // pool of temp files. recycles after every loop
    enum { MAX_TEMP_FILES = 16 };
    static local char temps[MAX_TEMP_FILES][MAX_PATHFILE] = {0};
    static local int i = 0;
    if( temps[i]) unlink(temps[i]);
    i = (i + 1) % MAX_TEMP_FILES;
    if(!temps[i][0]) snprintf(temps[i], MAX_PATHFILE, "%s", tmpnam(0));
    char *name = temps[i];

    FILE *tmp = fopen(name, "wb"); //unlink(name);
    ASSERT( tmp, "cannot create tempfile %s", name );
    fwrite(buf ? buf : "", 1, buf ? sz : 0, tmp);
    fclose(tmp);
    return name;
#endif
}


// -----------------------------------------------------------------------------
// cache

void* cache_lookup(const char *pathfile, int *size) { // find key->value
    if( !MAX_CACHED_FILES ) return 0;
    for(archive_dir *dir = dir_cache; dir; dir = dir->next) {
        if( !strcmp(dir->path, pathfile) ) {
            if(size) *size = dir->size;
            return dir->data;
        }
    }
    return 0;
}
void* cache_insert(const char *pathfile, void *ptr, int size) { // append key/value; return LRU or NULL
    if( !MAX_CACHED_FILES ) return 0;
    assert( ptr );
    assert( size );

    // append to cache
    archive_dir zero = {0}, *old = dir_cache;
    *(dir_cache = REALLOC(0, sizeof(archive_dir))) = zero;
    dir_cache->next = old;
    dir_cache->path = STRDUP(pathfile);
    dir_cache->size = size;
    dir_cache->data = REALLOC(0, size+1);
    memcpy(dir_cache->data, ptr, size); size[(char*)dir_cache->data] = 0; // copy+terminator

    // keep cached files within limits
    static int added = 0;
    if( added < MAX_CACHED_FILES ) {
        ++added;
    } else {
        // remove oldest cache entry
        for( archive_dir *prev = dir_cache, *dir = prev; dir ; prev = dir, dir = dir->next ) {
            if( !dir->next ) {
                prev->next = 0; // break link
                void *data = dir->data;
                dir->path = REALLOC(dir->path, 0);
                dir->data = REALLOC(dir->data, 0);
                dir = REALLOC(dir, 0);
                return data;
            }
        }
    }
    return 0;
}
#line 0
#line 1 "fwk_font.c"
// font framework. original code by Vassvik (UNLICENSED)
// - rlyeh, public domain.
//
// [x] embedded default font (bm mini).
// [x] oversampling, texture dimensions.
// [x] utf8, unicode ranges.
// [x] markup opcodes.
// [x] faces (italic, bold, regular, cjk), colors and sizes.
// [x] unicode ranges from dear-imgui (@ocornut allowed to mit-0 relicense the data tables).
// [*] alignment. kinda hacky. revisit some day.
// [ ] underlining, soft/hard shadows, outlines.
// [ ] clip/wrap/overflow regions.
// [ ] text-shaping, text-layout.
// [ ] text-wrapping.
// [ ] optimizations.
//
// ## language families that could be merged on a single texture alias
// - EU+EL+RU
// - AR+HE+RU
// - TH+VI+TW
// - ZH
// - JP
// - KR

// -----------------------------------------------------------------------------

// bm-mini.zip (public domain font)
// http://bitmapmania.m78.com
// cooz@m78.com

static const char bm_mini_ttf[] = {
/*000000*/ 0x00,0x01,0x00,0x00,0x00,0x0e,0x00,0x30,0x00,0x03,0x00,0xb0,0x4f,0x53,0x2f,0x32,
/*000010*/ 0x80,0x00,0x6d,0x88,0x00,0x00,0x4e,0x04,0x00,0x00,0x00,0x4e,0x63,0x6d,0x61,0x70,
/*000020*/ 0xf1,0x89,0xe8,0x81,0x00,0x00,0x45,0x54,0x00,0x00,0x02,0x28,0x63,0x76,0x74,0x20,
/*000030*/ 0x5a,0x9b,0xfa,0x82,0x00,0x00,0x03,0xe8,0x00,0x00,0x00,0x32,0x66,0x70,0x67,0x6d,
/*000040*/ 0x83,0x33,0xc2,0x4f,0x00,0x00,0x03,0xd4,0x00,0x00,0x00,0x14,0x67,0x6c,0x79,0x66,
/*000050*/ 0x05,0xa3,0xba,0x6b,0x00,0x00,0x04,0x64,0x00,0x00,0x3c,0xec,0x68,0x64,0x6d,0x78,
/*000060*/ 0x0d,0xad,0x0b,0x14,0x00,0x00,0x47,0x7c,0x00,0x00,0x06,0x88,0x68,0x65,0x61,0x64,
/*000070*/ 0xce,0xfe,0xc6,0xae,0x00,0x00,0x4e,0x54,0x00,0x00,0x00,0x36,0x68,0x68,0x65,0x61,
/*000080*/ 0x05,0x15,0x02,0xee,0x00,0x00,0x4e,0x8c,0x00,0x00,0x00,0x24,0x68,0x6d,0x74,0x78,
/*000090*/ 0xae,0x38,0x00,0xf4,0x00,0x00,0x42,0xe0,0x00,0x00,0x01,0x8c,0x6c,0x6f,0x63,0x61,
/*0000a0*/ 0x00,0x0b,0xe4,0xba,0x00,0x00,0x41,0x50,0x00,0x00,0x01,0x90,0x6d,0x61,0x78,0x70,
/*0000b0*/ 0x00,0xfe,0x01,0x4d,0x00,0x00,0x4e,0xb0,0x00,0x00,0x00,0x20,0x6e,0x61,0x6d,0x65,
/*0000c0*/ 0xd6,0xe2,0x1a,0x1f,0x00,0x00,0x00,0xec,0x00,0x00,0x02,0xe5,0x70,0x6f,0x73,0x74,
/*0000d0*/ 0x09,0x85,0x09,0xff,0x00,0x00,0x44,0x6c,0x00,0x00,0x00,0xe8,0x70,0x72,0x65,0x70,
/*0000e0*/ 0xc9,0x0f,0xd2,0x13,0x00,0x00,0x04,0x1c,0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x15,
/*0000f0*/ 0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x76,0x00,0x3b,0x00,0x00,
/*000100*/ 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0e,0x00,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,
/*000110*/ 0x00,0x02,0x00,0x04,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x4e,
/*000120*/ 0x01,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x14,0x00,0xd6,0x00,0x00,
/*000130*/ 0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x48,0x01,0x83,0x00,0x00,0x00,0x00,0x00,0x00,
/*000140*/ 0x00,0x06,0x00,0x10,0x01,0xd3,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3b,
/*000150*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x07,0x00,0xb1,0x00,0x01,
/*000160*/ 0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0xc6,0x00,0x01,0x00,0x00,0x00,0x00,
/*000170*/ 0x00,0x03,0x00,0x27,0x00,0xea,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0a,
/*000180*/ 0x00,0xcc,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x24,0x01,0x5f,0x00,0x01,
/*000190*/ 0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x08,0x01,0xcb,0x00,0x03,0x00,0x01,0x04,0x09,
/*0001a0*/ 0x00,0x00,0x00,0x76,0x00,0x3b,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x01,0x00,0x0e,
/*0001b0*/ 0x00,0xb8,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x02,0x00,0x04,0x00,0xc8,0x00,0x03,
/*0001c0*/ 0x00,0x01,0x04,0x09,0x00,0x03,0x00,0x4e,0x01,0x11,0x00,0x03,0x00,0x01,0x04,0x09,
/*0001d0*/ 0x00,0x04,0x00,0x14,0x00,0xd6,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x05,0x00,0x48,
/*0001e0*/ 0x01,0x83,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x06,0x00,0x10,0x01,0xd3,0x43,0x6f,
/*0001f0*/ 0x70,0x79,0x72,0x69,0x67,0x68,0x74,0x20,0x28,0x43,0x29,0x20,0x32,0x30,0x30,0x31,
/*000200*/ 0x20,0x42,0x69,0x74,0x6d,0x61,0x70,0x4d,0x61,0x6e,0x69,0x61,0x20,0x2f,0x20,0x43,
/*000210*/ 0x4f,0x4f,0x5a,0x2e,0x20,0x41,0x6c,0x6c,0x20,0x72,0x69,0x67,0x68,0x74,0x73,0x20,
/*000220*/ 0x72,0x65,0x73,0x65,0x72,0x76,0x65,0x64,0x2e,0x00,0x43,0x00,0x6f,0x00,0x70,0x00,
/*000230*/ 0x79,0x00,0x72,0x00,0x69,0x00,0x67,0x00,0x68,0x00,0x74,0x00,0x20,0x00,0x28,0x00,
/*000240*/ 0x43,0x00,0x29,0x00,0x20,0x00,0x32,0x00,0x30,0x00,0x30,0x00,0x31,0x00,0x20,0x00,
/*000250*/ 0x42,0x00,0x69,0x00,0x74,0x00,0x6d,0x00,0x61,0x00,0x70,0x00,0x4d,0x00,0x61,0x00,
/*000260*/ 0x6e,0x00,0x69,0x00,0x61,0x00,0x20,0x00,0x2f,0x00,0x20,0x00,0x43,0x00,0x4f,0x00,
/*000270*/ 0x4f,0x00,0x5a,0x00,0x2e,0x00,0x20,0x00,0x41,0x00,0x6c,0x00,0x6c,0x00,0x20,0x00,
/*000280*/ 0x72,0x00,0x69,0x00,0x67,0x00,0x68,0x00,0x74,0x00,0x73,0x00,0x20,0x00,0x72,0x00,
/*000290*/ 0x65,0x00,0x73,0x00,0x65,0x00,0x72,0x00,0x76,0x00,0x65,0x00,0x64,0x00,0x2e,0x42,
/*0002a0*/ 0x4d,0x20,0x6d,0x69,0x6e,0x69,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,0x6d,0x00,0x69,
/*0002b0*/ 0x00,0x6e,0x00,0x69,0x41,0x38,0x00,0x41,0x00,0x38,0x42,0x4d,0x20,0x6d,0x69,0x6e,
/*0002c0*/ 0x69,0x20,0x41,0x38,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,0x6d,0x00,0x69,0x00,0x6e,
/*0002d0*/ 0x00,0x69,0x00,0x20,0x00,0x41,0x00,0x38,0x4d,0x61,0x63,0x72,0x6f,0x6d,0x65,0x64,
/*0002e0*/ 0x69,0x61,0x20,0x46,0x6f,0x6e,0x74,0x6f,0x67,0x72,0x61,0x70,0x68,0x65,0x72,0x20,
/*0002f0*/ 0x34,0x2e,0x31,0x4a,0x20,0x42,0x4d,0x20,0x6d,0x69,0x6e,0x69,0x20,0x41,0x38,0x00,
/*000300*/ 0x4d,0x00,0x61,0x00,0x63,0x00,0x72,0x00,0x6f,0x00,0x6d,0x00,0x65,0x00,0x64,0x00,
/*000310*/ 0x69,0x00,0x61,0x00,0x20,0x00,0x46,0x00,0x6f,0x00,0x6e,0x00,0x74,0x00,0x6f,0x00,
/*000320*/ 0x67,0x00,0x72,0x00,0x61,0x00,0x70,0x00,0x68,0x00,0x65,0x00,0x72,0x00,0x20,0x00,
/*000330*/ 0x34,0x00,0x2e,0x00,0x31,0x00,0x4a,0x00,0x20,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,
/*000340*/ 0x6d,0x00,0x69,0x00,0x6e,0x00,0x69,0x00,0x20,0x00,0x41,0x00,0x38,0x4d,0x61,0x63,
/*000350*/ 0x72,0x6f,0x6d,0x65,0x64,0x69,0x61,0x20,0x46,0x6f,0x6e,0x74,0x6f,0x67,0x72,0x61,
/*000360*/ 0x70,0x68,0x65,0x72,0x20,0x34,0x2e,0x31,0x4a,0x20,0x30,0x31,0x2e,0x31,0x2e,0x32,
/*000370*/ 0x37,0x00,0x4d,0x00,0x61,0x00,0x63,0x00,0x72,0x00,0x6f,0x00,0x6d,0x00,0x65,0x00,
/*000380*/ 0x64,0x00,0x69,0x00,0x61,0x00,0x20,0x00,0x46,0x00,0x6f,0x00,0x6e,0x00,0x74,0x00,
/*000390*/ 0x6f,0x00,0x67,0x00,0x72,0x00,0x61,0x00,0x70,0x00,0x68,0x00,0x65,0x00,0x72,0x00,
/*0003a0*/ 0x20,0x00,0x34,0x00,0x2e,0x00,0x31,0x00,0x4a,0x00,0x20,0x00,0x30,0x00,0x31,0x00,
/*0003b0*/ 0x2e,0x00,0x31,0x00,0x2e,0x00,0x32,0x00,0x37,0x42,0x4d,0x6d,0x69,0x6e,0x69,0x41,
/*0003c0*/ 0x38,0x00,0x42,0x00,0x4d,0x00,0x6d,0x00,0x69,0x00,0x6e,0x00,0x69,0x00,0x41,0x00,
/*0003d0*/ 0x38,0x00,0x00,0x00,0x40,0x01,0x00,0x2c,0x76,0x45,0x20,0xb0,0x03,0x25,0x45,0x23,
/*0003e0*/ 0x61,0x68,0x18,0x23,0x68,0x60,0x44,0x2d,0xff,0x39,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0003f0*/ 0x00,0x62,0x00,0xc5,0x00,0x62,0x00,0xc5,0x01,0x29,0x01,0x8d,0x01,0x8e,0x01,0x2a,
/*000400*/ 0x5a,0x67,0x12,0x06,0xd2,0xb8,0x6a,0x18,0xf8,0x2a,0x61,0xa3,0x0e,0x40,0xee,0xd2,
/*000410*/ 0x80,0x3a,0x27,0x55,0xa2,0x87,0x00,0x01,0x00,0x0d,0x00,0x00,0x40,0x11,0x0b,0x0b,
/*000420*/ 0x0a,0x0a,0x09,0x09,0x08,0x08,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,0x01,0x8d,
/*000430*/ 0xb8,0x01,0xff,0x85,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,
/*000440*/ 0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0xb3,0x05,0x04,0x46,
/*000450*/ 0x00,0x2b,0xb3,0x07,0x06,0x46,0x00,0x2b,0xb1,0x04,0x04,0x45,0x68,0x44,0xb1,0x06,
/*000460*/ 0x06,0x45,0x68,0x44,0x00,0x02,0x00,0x32,0x00,0x00,0x01,0x5e,0x02,0x58,0x00,0x03,
/*000470*/ 0x00,0x07,0x00,0x55,0x40,0x1f,0x01,0x08,0x08,0x40,0x09,0x02,0x07,0x04,0x04,0x01,
/*000480*/ 0x00,0x06,0x05,0x04,0x03,0x02,0x05,0x04,0x06,0x00,0x07,0x06,0x06,0x01,0x02,0x01,
/*000490*/ 0x03,0x00,0x01,0x00,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,0x10,0xfd,
/*0004a0*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,
/*0004b0*/ 0x01,0x49,0x68,0xb9,0x00,0x00,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*0004c0*/ 0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x33,0x11,0x21,0x11,0x27,0x33,0x11,
/*0004d0*/ 0x23,0x32,0x01,0x2c,0xfa,0xc8,0xc8,0x02,0x58,0xfd,0xa8,0x32,0x01,0xf4,0x00,0x02,
/*0004e0*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x07,0x00,0x53,0x40,0x20,
/*0004f0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x01,0x00,0x06,0x05,0x02,0x03,0x01,0x04,0x07,0x04,
/*000500*/ 0x03,0x03,0x00,0x07,0x06,0x06,0x04,0x05,0x04,0x01,0x03,0x02,0x03,0x01,0x01,0x46,
/*000510*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*000520*/ 0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,
/*000530*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,
/*000540*/ 0x59,0x37,0x23,0x11,0x33,0x11,0x23,0x35,0x33,0x63,0x62,0x62,0x62,0x62,0xc9,0x01,
/*000550*/ 0x8e,0xfd,0xaa,0x62,0x00,0x02,0x00,0x01,0x01,0x91,0x01,0x2b,0x02,0x57,0x00,0x03,
/*000560*/ 0x00,0x07,0x00,0x50,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,0x02,0x01,0x04,0x03,
/*000570*/ 0x00,0x07,0x04,0x04,0x06,0x05,0x05,0x04,0x01,0x03,0x00,0x07,0x06,0x03,0x03,0x02,
/*000580*/ 0x03,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x01,
/*000590*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*0005a0*/ 0x05,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,
/*0005b0*/ 0xff,0xc0,0x38,0x59,0x01,0x23,0x35,0x33,0x07,0x23,0x35,0x33,0x01,0x2b,0x62,0x62,
/*0005c0*/ 0xc8,0x62,0x62,0x01,0x91,0xc6,0xc6,0xc6,0x00,0x02,0x00,0x01,0x00,0x65,0x01,0xf3,
/*0005d0*/ 0x02,0x57,0x00,0x1b,0x00,0x1f,0x00,0xb3,0x40,0x5e,0x01,0x20,0x20,0x40,0x21,0x00,
/*0005e0*/ 0x1b,0x18,0x17,0x14,0x13,0x0e,0x0d,0x0a,0x09,0x00,0x06,0x05,0x04,0x07,0x1f,0x12,
/*0005f0*/ 0x11,0x03,0x1e,0x04,0x10,0x0f,0x0c,0x0b,0x08,0x05,0x07,0x1d,0x04,0x03,0x03,0x1c,
/*000600*/ 0x04,0x1a,0x19,0x16,0x02,0x01,0x05,0x15,0x1b,0x1a,0x06,0x00,0x1f,0x0b,0x0a,0x03,
/*000610*/ 0x1c,0x06,0x09,0x08,0x05,0x04,0x01,0x05,0x00,0x0d,0x0c,0x06,0x0e,0x1e,0x19,0x18,
/*000620*/ 0x03,0x1d,0x06,0x17,0x16,0x13,0x12,0x0f,0x05,0x0e,0x07,0x06,0x03,0x03,0x02,0x15,
/*000630*/ 0x14,0x11,0x03,0x10,0x03,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,
/*000640*/ 0x2f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,
/*000650*/ 0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,
/*000660*/ 0x17,0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,
/*000670*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x20,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000680*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x20,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*000690*/ 0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,0x33,0x35,
/*0006a0*/ 0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x01,0xf3,0x65,0x61,0x67,
/*0006b0*/ 0x61,0x64,0x64,0x64,0x64,0x62,0x67,0x61,0x64,0x64,0x64,0xc8,0x62,0xc9,0x64,0x64,
/*0006c0*/ 0x64,0x65,0x61,0x67,0x61,0x64,0x64,0x64,0x65,0x61,0x67,0x03,0x62,0x62,0x00,0x01,
/*0006d0*/ 0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0xf3,0x00,0x13,0x00,0x88,0x40,0x42,0x01,0x14,
/*0006e0*/ 0x14,0x40,0x15,0x00,0x13,0x0a,0x09,0x00,0x04,0x03,0x04,0x05,0x0e,0x06,0x05,0x03,
/*0006f0*/ 0x0d,0x04,0x0c,0x0b,0x08,0x03,0x07,0x10,0x0f,0x04,0x12,0x02,0x01,0x03,0x11,0x07,
/*000700*/ 0x06,0x03,0x03,0x02,0x0b,0x00,0x09,0x08,0x01,0x03,0x00,0x07,0x12,0x11,0x10,0x0d,
/*000710*/ 0x03,0x0c,0x13,0x12,0x0f,0x0e,0x0b,0x05,0x0a,0x02,0x05,0x04,0x01,0x01,0x09,0x46,
/*000720*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x10,0xfd,0x17,
/*000730*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*000740*/ 0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*000750*/ 0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,
/*000760*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x15,0x23,0x35,0x23,0x35,0x23,0x35,0x33,
/*000770*/ 0x35,0x33,0x15,0x33,0x35,0x33,0x15,0x33,0x01,0xf3,0x65,0x64,0x61,0x64,0x64,0x64,
/*000780*/ 0x62,0x67,0x61,0x64,0xc9,0x64,0x64,0x65,0x64,0xc5,0x64,0x64,0x64,0x65,0x00,0x03,
/*000790*/ 0x00,0x01,0x00,0x01,0x02,0xbb,0x02,0x57,0x00,0x23,0x00,0x27,0x00,0x2b,0x00,0xde,
/*0007a0*/ 0x40,0x80,0x01,0x2c,0x2c,0x40,0x2d,0x00,0x20,0x1f,0x08,0x07,0x04,0x0a,0x09,0x1e,
/*0007b0*/ 0x1d,0x04,0x18,0x17,0x0c,0x03,0x0b,0x29,0x28,0x16,0x15,0x0e,0x05,0x0d,0x04,0x0f,
/*0007c0*/ 0x2b,0x2a,0x14,0x13,0x10,0x05,0x0f,0x04,0x12,0x11,0x1a,0x06,0x05,0x03,0x19,0x04,
/*0007d0*/ 0x27,0x26,0x1c,0x04,0x03,0x05,0x1b,0x25,0x24,0x22,0x02,0x01,0x05,0x21,0x04,0x23,
/*0007e0*/ 0x00,0x27,0x24,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x1d,0x1c,0x11,0x10,0x0d,0x05,
/*0007f0*/ 0x0c,0x06,0x0e,0x21,0x0f,0x0e,0x03,0x20,0x06,0x26,0x25,0x23,0x22,0x1f,0x1e,0x0b,
/*000800*/ 0x07,0x06,0x09,0x0a,0x2a,0x29,0x17,0x16,0x13,0x05,0x12,0x06,0x14,0x2b,0x28,0x19,
/*000810*/ 0x03,0x18,0x02,0x1b,0x1a,0x15,0x03,0x14,0x03,0x09,0x08,0x03,0x03,0x02,0x01,0x01,
/*000820*/ 0x11,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*000830*/ 0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*000840*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,
/*000850*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,
/*000860*/ 0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x11,0x00,0x2c,0x49,0x68,0x61,
/*000870*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x2c,0xff,0xc0,0x38,0x59,0x25,0x23,
/*000880*/ 0x15,0x23,0x35,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x23,
/*000890*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x15,0x23,0x15,0x33,0x35,0x33,
/*0008a0*/ 0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x35,0x23,0x15,0x02,0xbb,0x65,0x61,0x64,0x67,
/*0008b0*/ 0x61,0x64,0x67,0x61,0x64,0x64,0x62,0x64,0x67,0x61,0x64,0x67,0x61,0x64,0x64,0x62,
/*0008c0*/ 0xfe,0xd2,0x62,0x65,0x64,0x65,0x63,0xc8,0xc6,0xca,0x64,0x65,0x61,0x64,0x65,0x63,
/*0008d0*/ 0xc8,0xc6,0xca,0x64,0x65,0x61,0x62,0x62,0x01,0x2c,0x62,0x62,0x00,0x03,0x00,0x01,
/*0008e0*/ 0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x1b,0x00,0x1f,0x00,0x25,0x00,0xce,0x40,0x76,
/*0008f0*/ 0x01,0x26,0x26,0x40,0x27,0x00,0x1a,0x19,0x1b,0x04,0x03,0x03,0x00,0x04,0x05,0x10,
/*000900*/ 0x0f,0x0c,0x03,0x0b,0x04,0x09,0x25,0x24,0x1f,0x1e,0x12,0x11,0x0e,0x0d,0x0a,0x09,
/*000910*/ 0x09,0x04,0x13,0x23,0x22,0x1d,0x1c,0x18,0x17,0x14,0x07,0x13,0x04,0x15,0x21,0x20,
/*000920*/ 0x16,0x08,0x07,0x05,0x15,0x04,0x06,0x05,0x02,0x03,0x01,0x03,0x02,0x06,0x04,0x25,
/*000930*/ 0x20,0x0b,0x0a,0x07,0x05,0x06,0x06,0x04,0x1e,0x1d,0x15,0x14,0x11,0x05,0x10,0x06,
/*000940*/ 0x12,0x22,0x21,0x06,0x0c,0x24,0x23,0x19,0x18,0x0d,0x01,0x00,0x07,0x0c,0x06,0x17,
/*000950*/ 0x0f,0x0e,0x03,0x16,0x1f,0x1c,0x1b,0x03,0x1a,0x02,0x13,0x12,0x03,0x09,0x08,0x05,
/*000960*/ 0x03,0x04,0x01,0x01,0x0b,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,
/*000970*/ 0x3f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,
/*000980*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*000990*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,
/*0009a0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0b,0x00,0x26,0x49,0x68,0x61,0xb0,0x40,
/*0009b0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x26,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,
/*0009c0*/ 0x15,0x23,0x35,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,
/*0009d0*/ 0x15,0x33,0x15,0x23,0x15,0x33,0x35,0x33,0x25,0x35,0x23,0x15,0x13,0x35,0x23,0x35,
/*0009e0*/ 0x23,0x15,0x01,0xf3,0x64,0x64,0x62,0x67,0xc5,0x64,0x64,0x64,0x64,0x62,0x64,0x64,
/*0009f0*/ 0xcb,0x61,0xfe,0xd4,0x62,0xc6,0x64,0x62,0x01,0x2d,0xcb,0x61,0x64,0x64,0x65,0xc5,
/*000a00*/ 0x67,0x61,0x64,0x65,0x61,0x66,0x64,0x02,0x62,0x62,0xfe,0xd4,0x61,0x65,0xc6,0x00,
/*000a10*/ 0x00,0x01,0x00,0x01,0x01,0x91,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x3f,0x40,0x13,
/*000a20*/ 0x01,0x04,0x04,0x40,0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x01,0x00,0x03,0x02,0x03,
/*000a30*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2f,0x3c,0xfd,
/*000a40*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,
/*000a50*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,
/*000a60*/ 0x33,0x63,0x62,0x62,0x01,0x91,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,
/*000a70*/ 0x02,0x57,0x00,0x0b,0x00,0x67,0x40,0x2d,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x07,0x06,
/*000a80*/ 0x0b,0x04,0x03,0x03,0x00,0x04,0x05,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,
/*000a90*/ 0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x03,0x05,
/*000aa0*/ 0x04,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,
/*000ab0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,
/*000ac0*/ 0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*000ad0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x13,0x23,0x11,
/*000ae0*/ 0x33,0x15,0x23,0x35,0x23,0x11,0x33,0x35,0x33,0xc7,0x64,0x64,0x62,0x64,0x64,0x62,
/*000af0*/ 0x01,0xf5,0xfe,0x6d,0x61,0x65,0x01,0x8d,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*000b00*/ 0x00,0xc7,0x02,0x57,0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*000b10*/ 0x0b,0x0a,0x02,0x01,0x04,0x08,0x07,0x04,0x03,0x03,0x0a,0x09,0x06,0x03,0x05,0x04,
/*000b20*/ 0x0b,0x00,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x07,0x06,0x06,0x08,0x09,0x08,0x03,
/*000b30*/ 0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,
/*000b40*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,
/*000b50*/ 0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,
/*000b60*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x37,
/*000b70*/ 0x23,0x15,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0x15,0x33,0xc7,0x65,0x61,0x64,0x64,
/*000b80*/ 0x62,0x64,0x65,0x64,0x62,0x01,0x92,0x62,0x65,0x00,0x00,0x01,0x00,0x01,0x01,0x2d,
/*000b90*/ 0x01,0x2b,0x02,0x57,0x00,0x13,0x00,0x83,0x40,0x40,0x01,0x14,0x14,0x40,0x15,0x00,
/*000ba0*/ 0x12,0x11,0x13,0x04,0x03,0x03,0x00,0x04,0x05,0x08,0x07,0x04,0x0e,0x0d,0x0a,0x03,
/*000bb0*/ 0x09,0x10,0x0f,0x0c,0x03,0x0b,0x04,0x06,0x05,0x02,0x03,0x01,0x07,0x06,0x06,0x11,
/*000bc0*/ 0x0d,0x0c,0x01,0x00,0x05,0x10,0x09,0x08,0x05,0x03,0x04,0x13,0x12,0x0f,0x03,0x0e,
/*000bd0*/ 0x03,0x0b,0x03,0x02,0x03,0x0a,0x02,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,
/*000be0*/ 0x17,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x01,0x2f,0x17,
/*000bf0*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,0x00,
/*000c00*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000c10*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,
/*000c20*/ 0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,0x33,0x35,0x33,0x01,
/*000c30*/ 0x2b,0x64,0x64,0x62,0x67,0x61,0x64,0x64,0x62,0x67,0x61,0x01,0xf5,0x67,0x61,0x64,
/*000c40*/ 0x64,0x62,0x66,0x62,0x64,0x64,0x00,0x01,0x00,0x01,0x00,0x65,0x01,0x2b,0x01,0x8f,
/*000c50*/ 0x00,0x0b,0x00,0x66,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x00,0x08,0x01,
/*000c60*/ 0x06,0x05,0x08,0x03,0x0a,0x09,0x02,0x03,0x01,0x04,0x08,0x07,0x04,0x03,0x03,0x0b,
/*000c70*/ 0x07,0x06,0x03,0x0a,0x06,0x05,0x04,0x01,0x03,0x00,0x03,0x02,0x09,0x08,0x02,0x01,
/*000c80*/ 0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*000c90*/ 0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,
/*000ca0*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*000cb0*/ 0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*000cc0*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x2b,0x65,0x61,0x64,0x64,0x62,0x64,0xc9,0x64,
/*000cd0*/ 0x65,0x61,0x64,0x65,0x00,0x01,0x00,0x01,0xff,0x9d,0x00,0xc7,0x00,0x63,0x00,0x07,
/*000ce0*/ 0x00,0x53,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,0x02,0x01,0x04,0x04,0x03,0x06,
/*000cf0*/ 0x05,0x04,0x07,0x00,0x05,0x04,0x06,0x02,0x07,0x06,0x03,0x02,0x01,0x00,0x01,0x01,
/*000d00*/ 0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x3c,0x10,0xfd,0x3c,
/*000d10*/ 0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*000d20*/ 0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*000d30*/ 0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0xc7,0x65,0x61,
/*000d40*/ 0x64,0x62,0x01,0x64,0x62,0x64,0x00,0x01,0x00,0x01,0x00,0xc9,0x01,0x2b,0x01,0x2b,
/*000d50*/ 0x00,0x03,0x00,0x3d,0x40,0x11,0x01,0x04,0x04,0x40,0x05,0x00,0x03,0x02,0x01,0x00,
/*000d60*/ 0x03,0x02,0x01,0x00,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,
/*000d70*/ 0x01,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,
/*000d80*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,
/*000d90*/ 0x59,0x25,0x21,0x35,0x21,0x01,0x2b,0xfe,0xd6,0x01,0x2a,0xc9,0x62,0x00,0x00,0x01,
/*000da0*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x00,0x63,0x00,0x03,0x00,0x3f,0x40,0x13,0x01,0x04,
/*000db0*/ 0x04,0x40,0x05,0x00,0x03,0x00,0x04,0x02,0x01,0x03,0x02,0x01,0x00,0x01,0x01,0x01,
/*000dc0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x00,
/*000dd0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000de0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x35,0x33,0x63,
/*000df0*/ 0x62,0x62,0x01,0x62,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,
/*000e00*/ 0x00,0x69,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x04,0x03,0x04,0x06,0x05,0x08,
/*000e10*/ 0x07,0x04,0x02,0x01,0x0a,0x09,0x04,0x0b,0x00,0x01,0x00,0x07,0x0a,0x07,0x03,0x02,
/*000e20*/ 0x03,0x06,0x07,0x04,0x0b,0x0a,0x03,0x09,0x08,0x02,0x05,0x04,0x01,0x01,0x05,0x46,
/*000e30*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*000e40*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,
/*000e50*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000e60*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,
/*000e70*/ 0x23,0x35,0x33,0x35,0x33,0x35,0x33,0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x62,0x01,
/*000e80*/ 0x91,0xc8,0xc8,0xc6,0xc8,0xc8,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*000e90*/ 0x00,0x0b,0x00,0x11,0x00,0x17,0x00,0x95,0x40,0x4d,0x01,0x18,0x18,0x40,0x19,0x00,
/*000ea0*/ 0x15,0x14,0x05,0x05,0x11,0x10,0x05,0x00,0x17,0x16,0x0f,0x0e,0x08,0x07,0x04,0x07,
/*000eb0*/ 0x03,0x04,0x06,0x05,0x13,0x12,0x0d,0x0c,0x0a,0x02,0x01,0x07,0x09,0x04,0x0b,0x00,
/*000ec0*/ 0x17,0x12,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,0x0a,0x07,0x05,0x06,
/*000ed0*/ 0x06,0x08,0x11,0x0c,0x06,0x13,0x14,0x13,0x10,0x03,0x0f,0x06,0x16,0x15,0x09,0x08,
/*000ee0*/ 0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*000ef0*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*000f00*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,
/*000f10*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,0x68,0x61,0xb0,
/*000f20*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*000f30*/ 0x23,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x33,0x35,0x13,
/*000f40*/ 0x35,0x23,0x15,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0xc6,0x61,
/*000f50*/ 0x65,0x62,0x64,0x65,0x64,0x65,0x01,0x8d,0x64,0x65,0x61,0x62,0xc6,0x64,0xfe,0xd4,
/*000f60*/ 0xc6,0x64,0x62,0x00,0x00,0x01,0x00,0x65,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x07,
/*000f70*/ 0x00,0x54,0x40,0x20,0x01,0x08,0x08,0x40,0x09,0x00,0x04,0x03,0x05,0x00,0x06,0x05,
/*000f80*/ 0x02,0x03,0x01,0x04,0x07,0x00,0x03,0x02,0x06,0x05,0x04,0x07,0x06,0x03,0x01,0x00,
/*000f90*/ 0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,
/*000fa0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*000fb0*/ 0xb9,0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*000fc0*/ 0x00,0x08,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x35,0x33,0x35,0x33,0x01,0x2b,
/*000fd0*/ 0x62,0x64,0x64,0x62,0x01,0x01,0x91,0x61,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*000fe0*/ 0x01,0x8f,0x02,0x57,0x00,0x19,0x00,0xa4,0x40,0x53,0x01,0x1a,0x1a,0x40,0x1b,0x00,
/*000ff0*/ 0x12,0x11,0x06,0x03,0x05,0x04,0x14,0x13,0x0a,0x03,0x09,0x04,0x03,0x04,0x16,0x15,
/*001000*/ 0x0c,0x03,0x0b,0x0e,0x0d,0x04,0x02,0x01,0x18,0x17,0x10,0x03,0x0f,0x04,0x19,0x08,
/*001010*/ 0x07,0x03,0x00,0x13,0x12,0x01,0x03,0x00,0x06,0x19,0x18,0x0d,0x03,0x02,0x03,0x0c,
/*001020*/ 0x06,0x0b,0x0a,0x05,0x03,0x04,0x07,0x06,0x06,0x08,0x15,0x14,0x11,0x03,0x10,0x06,
/*001030*/ 0x16,0x17,0x16,0x03,0x0f,0x0e,0x02,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,
/*001040*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,
/*001050*/ 0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,
/*001060*/ 0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*001070*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x1a,0x49,0x68,0x61,0xb0,0x40,
/*001080*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x1a,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,
/*001090*/ 0x15,0x23,0x15,0x21,0x15,0x21,0x35,0x33,0x35,0x33,0x35,0x33,0x35,0x23,0x15,0x23,
/*0010a0*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x8f,0x65,0x64,0x63,0x01,0x2c,0xfe,0x72,0x65,
/*0010b0*/ 0x64,0x63,0xcb,0x61,0x64,0xc6,0x64,0x01,0x91,0x64,0x64,0x67,0x61,0xc6,0x64,0x64,
/*0010c0*/ 0x66,0x64,0x62,0x64,0x65,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0010d0*/ 0x00,0x1b,0x00,0xa4,0x40,0x57,0x01,0x1c,0x1c,0x40,0x1d,0x00,0x14,0x13,0x10,0x0f,
/*0010e0*/ 0x08,0x04,0x03,0x07,0x07,0x04,0x12,0x11,0x06,0x03,0x05,0x1b,0x18,0x17,0x03,0x00,
/*0010f0*/ 0x04,0x09,0x0c,0x0b,0x04,0x1a,0x19,0x16,0x0e,0x0d,0x0a,0x09,0x02,0x01,0x09,0x15,
/*001100*/ 0x1b,0x1a,0x07,0x00,0x09,0x08,0x06,0x02,0x07,0x06,0x06,0x05,0x01,0x00,0x03,0x04,
/*001110*/ 0x0b,0x0a,0x06,0x0c,0x19,0x18,0x11,0x03,0x10,0x06,0x17,0x16,0x13,0x0f,0x0e,0x05,
/*001120*/ 0x12,0x15,0x14,0x03,0x0d,0x0c,0x02,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,
/*001130*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*001140*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*001150*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*001160*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x1c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001170*/ 0x37,0xb9,0x00,0x1c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,
/*001180*/ 0x15,0x33,0x35,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*001190*/ 0x15,0x23,0x15,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x64,0x64,0xcb,0x61,0x64,
/*0011a0*/ 0xc6,0x64,0x64,0x64,0x65,0x64,0x65,0x61,0x64,0xcb,0x61,0x66,0x64,0x62,0x64,0x65,
/*0011b0*/ 0x61,0x67,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x6a,
/*0011c0*/ 0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x00,0x0a,0x09,0x04,0x03,0x03,0x04,
/*0011d0*/ 0x0c,0x0b,0x02,0x03,0x01,0x08,0x07,0x04,0x06,0x05,0x0d,0x09,0x08,0x03,0x0c,0x06,
/*0011e0*/ 0x05,0x04,0x01,0x03,0x00,0x0b,0x0a,0x07,0x03,0x06,0x03,0x03,0x02,0x01,0x01,0x05,
/*0011f0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*001200*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,
/*001210*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001220*/ 0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*001230*/ 0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0x61,0xc8,0x62,0x67,0x61,
/*001240*/ 0x64,0xc9,0xc8,0xc8,0x01,0x8e,0xfe,0xd4,0x01,0x2c,0xfe,0xd3,0x00,0x01,0x00,0x01,
/*001250*/ 0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7b,0x40,0x38,0x01,0x10,0x10,0x40,
/*001260*/ 0x11,0x00,0x02,0x01,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x0a,0x09,0x03,0x00,0x0c,
/*001270*/ 0x0b,0x04,0x08,0x07,0x04,0x03,0x03,0x0f,0x0e,0x07,0x00,0x05,0x01,0x00,0x03,0x04,
/*001280*/ 0x06,0x02,0x07,0x06,0x06,0x0d,0x0c,0x0b,0x0a,0x06,0x08,0x09,0x08,0x03,0x03,0x02,
/*001290*/ 0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*0012a0*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,
/*0012b0*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*0012c0*/ 0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*0012d0*/ 0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x35,0x21,0x11,0x21,0x15,
/*0012e0*/ 0x21,0x15,0x33,0x15,0x33,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2c,0xfe,0xd4,0x01,0x8e,
/*0012f0*/ 0xfe,0xd4,0xc8,0x64,0x65,0x64,0x62,0xca,0x01,0x2a,0x62,0x67,0x64,0x00,0x00,0x03,
/*001300*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x13,0x00,0x17,0x00,0x8a,
/*001310*/ 0x40,0x49,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x16,0x13,0x12,0x08,0x07,0x04,0x07,
/*001320*/ 0x03,0x04,0x06,0x05,0x15,0x14,0x11,0x10,0x0e,0x0d,0x0a,0x02,0x01,0x09,0x09,0x04,
/*001330*/ 0x0f,0x0c,0x0b,0x03,0x00,0x17,0x14,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x12,0x11,
/*001340*/ 0x0b,0x0a,0x07,0x05,0x06,0x06,0x08,0x16,0x0f,0x0e,0x03,0x15,0x06,0x13,0x10,0x0d,
/*001350*/ 0x03,0x0c,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*001360*/ 0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*001370*/ 0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*001380*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001390*/ 0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*0013a0*/ 0x11,0x33,0x35,0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x13,0x35,
/*0013b0*/ 0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0x64,0x64,0xc6,0xc6,0xc6,
/*0013c0*/ 0x65,0x64,0x65,0x01,0x8d,0x64,0x65,0x61,0x67,0x67,0x62,0x62,0xfe,0xd4,0xc6,0xc6,
/*0013d0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7d,0x40,0x38,
/*0013e0*/ 0x01,0x10,0x10,0x40,0x11,0x00,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x02,0x01,
/*0013f0*/ 0x0a,0x09,0x04,0x0f,0x00,0x0c,0x0b,0x04,0x0e,0x0d,0x0d,0x0c,0x01,0x03,0x00,0x07,
/*001400*/ 0x0e,0x07,0x03,0x02,0x03,0x06,0x07,0x04,0x0b,0x0a,0x06,0x0e,0x0f,0x0e,0x03,0x09,
/*001410*/ 0x08,0x02,0x05,0x04,0x01,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,
/*001420*/ 0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*001430*/ 0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,
/*001440*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,
/*001450*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,
/*001460*/ 0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x21,0x01,0x8f,0x65,0x64,0x61,
/*001470*/ 0x65,0x63,0xcb,0x61,0x01,0x8e,0x01,0x91,0xc8,0xc8,0xc6,0xc8,0x66,0x64,0xc6,0x00,
/*001480*/ 0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x13,0x00,0x17,0x00,0x1b,
/*001490*/ 0x00,0x94,0x40,0x52,0x01,0x1c,0x1c,0x40,0x1d,0x00,0x1b,0x1a,0x17,0x16,0x0c,0x0b,
/*0014a0*/ 0x08,0x07,0x04,0x09,0x03,0x04,0x0a,0x09,0x06,0x03,0x05,0x19,0x18,0x15,0x14,0x12,
/*0014b0*/ 0x11,0x0e,0x02,0x01,0x09,0x0d,0x04,0x13,0x10,0x0f,0x03,0x00,0x1b,0x18,0x05,0x04,
/*0014c0*/ 0x01,0x05,0x00,0x06,0x02,0x16,0x15,0x0f,0x0e,0x0b,0x05,0x0a,0x06,0x0c,0x1a,0x19,
/*0014d0*/ 0x13,0x12,0x07,0x05,0x06,0x06,0x17,0x14,0x11,0x09,0x08,0x05,0x10,0x0d,0x0c,0x03,
/*0014e0*/ 0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,
/*0014f0*/ 0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,
/*001500*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*001510*/ 0xb9,0x00,0x05,0x00,0x1c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*001520*/ 0x00,0x1c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,
/*001530*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x13,0x35,
/*001540*/ 0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0x64,0x64,0xc6,0x64,0x64,0x64,0x64,0xc6,
/*001550*/ 0xc6,0xc6,0x65,0x64,0x65,0xc5,0x67,0x61,0x64,0x65,0x61,0x67,0x67,0x62,0x62,0xfe,
/*001560*/ 0xd4,0xc6,0xc6,0x00,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,
/*001570*/ 0x00,0x13,0x00,0x17,0x00,0x8a,0x40,0x49,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x16,
/*001580*/ 0x13,0x12,0x0c,0x0b,0x08,0x07,0x04,0x09,0x03,0x04,0x0a,0x09,0x06,0x03,0x05,0x15,
/*001590*/ 0x14,0x11,0x10,0x0e,0x02,0x01,0x07,0x0d,0x04,0x0f,0x00,0x17,0x14,0x05,0x01,0x00,
/*0015a0*/ 0x05,0x04,0x06,0x06,0x12,0x11,0x0f,0x0e,0x0b,0x05,0x0a,0x06,0x0c,0x13,0x09,0x08,
/*0015b0*/ 0x03,0x10,0x06,0x16,0x15,0x07,0x03,0x06,0x0d,0x0c,0x03,0x03,0x02,0x01,0x01,0x05,
/*0015c0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*0015d0*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,
/*0015e0*/ 0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,
/*0015f0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,
/*001600*/ 0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*001610*/ 0x07,0x35,0x23,0x15,0x17,0x35,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0x64,0x64,
/*001620*/ 0xc6,0x64,0x64,0xc6,0xc6,0xc6,0x65,0x64,0x65,0x61,0x67,0xc5,0x64,0x65,0xc5,0xc6,
/*001630*/ 0xc6,0xc8,0x62,0x62,0x00,0x02,0x00,0x01,0x00,0x65,0x00,0x63,0x01,0xf3,0x00,0x03,
/*001640*/ 0x00,0x07,0x00,0x54,0x40,0x20,0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,
/*001650*/ 0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x01,0x00,0x06,0x02,0x07,0x06,0x06,0x04,0x03,
/*001660*/ 0x02,0x05,0x04,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,0x10,
/*001670*/ 0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*001680*/ 0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001690*/ 0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x35,0x33,
/*0016a0*/ 0x63,0x62,0x62,0x62,0x62,0x01,0x91,0x62,0xfe,0x72,0x62,0x00,0x00,0x02,0x00,0x01,
/*0016b0*/ 0x00,0x01,0x00,0x63,0x01,0xf3,0x00,0x03,0x00,0x07,0x00,0x55,0x40,0x21,0x01,0x08,
/*0016c0*/ 0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x07,
/*0016d0*/ 0x06,0x07,0x04,0x01,0x00,0x06,0x02,0x03,0x02,0x05,0x04,0x01,0x01,0x01,0x46,0x76,
/*0016e0*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,
/*0016f0*/ 0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,
/*001700*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,
/*001710*/ 0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0x63,0x62,0x62,0x62,0x62,0x01,0x91,
/*001720*/ 0x62,0xfe,0x0e,0xc6,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x13,
/*001730*/ 0x00,0x8a,0x40,0x42,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x13,0x08,0x07,0x03,
/*001740*/ 0x00,0x04,0x09,0x12,0x11,0x0a,0x09,0x06,0x02,0x01,0x07,0x05,0x04,0x0b,0x0c,0x0b,
/*001750*/ 0x04,0x03,0x03,0x04,0x0e,0x0d,0x01,0x00,0x06,0x12,0x07,0x06,0x06,0x08,0x05,0x04,
/*001760*/ 0x06,0x0b,0x0a,0x0d,0x0c,0x06,0x0f,0x03,0x02,0x03,0x0e,0x13,0x12,0x11,0x10,0x02,
/*001770*/ 0x09,0x08,0x01,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,
/*001780*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,
/*001790*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,
/*0017a0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,
/*0017b0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,
/*0017c0*/ 0x15,0x33,0x15,0x33,0x15,0x23,0x35,0x23,0x35,0x23,0x35,0x33,0x35,0x33,0x35,0x33,
/*0017d0*/ 0x01,0x2b,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0x65,0x63,0x62,0x01,0x91,0x64,0x67,
/*0017e0*/ 0x64,0x61,0x65,0x64,0x61,0x64,0x64,0x00,0x00,0x02,0x00,0x01,0x00,0x65,0x01,0x2b,
/*0017f0*/ 0x01,0x8f,0x00,0x03,0x00,0x07,0x00,0x54,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,
/*001800*/ 0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x01,0x00,0x06,0x02,0x07,0x06,0x06,0x04,
/*001810*/ 0x05,0x04,0x03,0x02,0x02,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,
/*001820*/ 0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
/*001830*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,
/*001840*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x01,0x21,0x35,0x21,
/*001850*/ 0x11,0x21,0x35,0x21,0x01,0x2b,0xfe,0xd6,0x01,0x2a,0xfe,0xd6,0x01,0x2a,0x01,0x2d,
/*001860*/ 0x62,0xfe,0xd6,0x62,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x13,
/*001870*/ 0x00,0x8e,0x40,0x44,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x04,0x03,0x03,0x04,
/*001880*/ 0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,0x07,0x04,0x02,0x01,0x12,0x11,0x0a,
/*001890*/ 0x03,0x09,0x04,0x13,0x00,0x13,0x12,0x06,0x00,0x09,0x01,0x00,0x03,0x08,0x06,0x02,
/*0018a0*/ 0x07,0x03,0x02,0x03,0x06,0x06,0x04,0x0b,0x0a,0x06,0x11,0x10,0x0d,0x0c,0x06,0x0e,
/*0018b0*/ 0x0f,0x0e,0x05,0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,
/*0018c0*/ 0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0018d0*/ 0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,
/*0018e0*/ 0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x14,0x49,
/*0018f0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*001900*/ 0x25,0x23,0x15,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,
/*001910*/ 0x15,0x33,0x15,0x33,0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x64,0x64,0x62,0x64,0x64,
/*001920*/ 0xc9,0x64,0x64,0x62,0x64,0x67,0x63,0x62,0x65,0x64,0x00,0x02,0x00,0x01,0x00,0x01,
/*001930*/ 0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x13,0x00,0x86,0x40,0x40,0x01,0x14,0x14,0x40,
/*001940*/ 0x15,0x00,0x02,0x01,0x05,0x03,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x00,0x08,0x07,
/*001950*/ 0x04,0x0a,0x09,0x13,0x10,0x04,0x12,0x11,0x0c,0x0b,0x04,0x05,0x03,0x05,0x01,0x00,
/*001960*/ 0x03,0x04,0x06,0x03,0x02,0x09,0x08,0x06,0x0f,0x0e,0x0b,0x07,0x06,0x05,0x0a,0x13,
/*001970*/ 0x12,0x06,0x10,0x11,0x10,0x01,0x0d,0x0c,0x03,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,
/*001980*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,
/*001990*/ 0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,
/*0019a0*/ 0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,
/*0019b0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*0019c0*/ 0x01,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*0019d0*/ 0x03,0x23,0x35,0x33,0x01,0x8f,0x65,0xc5,0xc8,0xcb,0x61,0x64,0xc6,0x64,0xc8,0x62,
/*0019e0*/ 0x62,0x01,0x2d,0x64,0x62,0xca,0x64,0x62,0x64,0x65,0xfe,0x0f,0x62,0x00,0x00,0x03,
/*0019f0*/ 0x00,0x01,0xff,0x9d,0x01,0xf3,0x01,0xf3,0x00,0x0b,0x00,0x0f,0x00,0x15,0x00,0x8a,
/*001a00*/ 0x40,0x42,0x01,0x16,0x16,0x40,0x17,0x00,0x14,0x13,0x12,0x11,0x13,0x12,0x08,0x07,
/*001a10*/ 0x04,0x05,0x03,0x04,0x06,0x05,0x11,0x10,0x0d,0x0c,0x0a,0x05,0x09,0x04,0x0b,0x00,
/*001a20*/ 0x0f,0x0e,0x04,0x15,0x02,0x01,0x03,0x14,0x0f,0x0c,0x06,0x00,0x0e,0x0d,0x06,0x15,
/*001a30*/ 0x10,0x09,0x08,0x03,0x02,0x0b,0x0a,0x07,0x03,0x06,0x02,0x05,0x04,0x01,0x03,0x00,
/*001a40*/ 0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x2f,
/*001a50*/ 0x3c,0x2f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,
/*001a60*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x2e,0x2e,0x31,
/*001a70*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x16,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001a80*/ 0x38,0x11,0x37,0xb9,0x00,0x16,0xff,0xc0,0x38,0x59,0x25,0x21,0x15,0x23,0x35,0x23,
/*001a90*/ 0x11,0x33,0x35,0x21,0x15,0x33,0x03,0x35,0x23,0x15,0x37,0x35,0x21,0x11,0x33,0x11,
/*001aa0*/ 0x01,0xf3,0xfe,0xd3,0x61,0x64,0x64,0x01,0x2a,0x64,0x64,0x62,0x62,0xfe,0xd6,0x62,
/*001ab0*/ 0x01,0x64,0x65,0x01,0x8d,0x64,0x65,0xfe,0xd7,0x62,0x62,0xc8,0x62,0xfe,0x72,0x01,
/*001ac0*/ 0x2c,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,
/*001ad0*/ 0x00,0x6b,0x40,0x30,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x08,0x04,0x03,0x05,
/*001ae0*/ 0x07,0x04,0x06,0x05,0x0b,0x02,0x01,0x03,0x0a,0x04,0x09,0x00,0x0d,0x0a,0x06,0x03,
/*001af0*/ 0x02,0x0c,0x0b,0x07,0x03,0x06,0x06,0x08,0x09,0x08,0x03,0x05,0x04,0x01,0x03,0x00,
/*001b00*/ 0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,
/*001b10*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*001b20*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,
/*001b30*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,
/*001b40*/ 0x23,0x15,0x23,0x11,0x33,0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0x62,0xca,0x62,
/*001b50*/ 0x64,0x01,0x2a,0x64,0xc6,0x01,0xc8,0xc8,0x01,0xf2,0x64,0xfe,0xd6,0xc6,0xc6,0x00,
/*001b60*/ 0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x0f,0x00,0x13,
/*001b70*/ 0x00,0x82,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x13,0x12,0x0f,0x03,0x0e,0x04,
/*001b80*/ 0x04,0x03,0x11,0x10,0x0d,0x0c,0x0a,0x09,0x06,0x02,0x01,0x09,0x05,0x04,0x0b,0x08,
/*001b90*/ 0x07,0x03,0x00,0x13,0x10,0x01,0x03,0x00,0x06,0x02,0x0e,0x07,0x06,0x03,0x0d,0x06,
/*001ba0*/ 0x04,0x12,0x0b,0x0a,0x03,0x11,0x06,0x0f,0x0c,0x09,0x03,0x08,0x05,0x04,0x03,0x03,
/*001bb0*/ 0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,
/*001bc0*/ 0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,
/*001bd0*/ 0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*001be0*/ 0x03,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,
/*001bf0*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x11,0x21,0x15,0x33,0x15,0x23,0x15,0x33,
/*001c00*/ 0x27,0x35,0x23,0x15,0x13,0x35,0x23,0x15,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2a,0x64,
/*001c10*/ 0x64,0x64,0x64,0xc6,0xc6,0xc6,0x65,0x64,0x02,0x56,0x65,0x61,0x67,0x67,0x62,0x62,
/*001c20*/ 0xfe,0xd4,0xc6,0xc6,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,
/*001c30*/ 0x00,0x64,0x40,0x2a,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x07,0x06,0x0b,0x04,0x03,0x00,
/*001c40*/ 0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,
/*001c50*/ 0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x03,0x05,0x04,0x01,0x01,0x07,0x46,0x76,0x2f,
/*001c60*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*001c70*/ 0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,
/*001c80*/ 0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*001c90*/ 0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,0x11,0x21,0x15,0x21,0x35,0x23,0x11,0x33,
/*001ca0*/ 0x35,0x21,0x01,0x8f,0xfe,0xd4,0x01,0x2c,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x01,0xf5,
/*001cb0*/ 0xfe,0x6e,0x62,0x65,0x01,0x8d,0x64,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*001cc0*/ 0x02,0x57,0x00,0x07,0x00,0x0b,0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*001cd0*/ 0x0b,0x0a,0x04,0x04,0x03,0x09,0x08,0x06,0x02,0x01,0x05,0x05,0x04,0x07,0x00,0x0b,
/*001ce0*/ 0x08,0x01,0x03,0x00,0x06,0x02,0x0a,0x07,0x06,0x03,0x09,0x06,0x04,0x05,0x04,0x03,
/*001cf0*/ 0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,
/*001d00*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,
/*001d10*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*001d20*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*001d30*/ 0x21,0x11,0x21,0x15,0x33,0x03,0x11,0x23,0x11,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2a,
/*001d40*/ 0x64,0x64,0xc6,0x65,0x64,0x02,0x56,0x65,0xfe,0x73,0x01,0x8e,0xfe,0x72,0x00,0x01,
/*001d50*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x6d,0x40,0x2f,0x01,0x0e,
/*001d60*/ 0x0e,0x40,0x0f,0x00,0x0d,0x08,0x07,0x04,0x03,0x00,0x0c,0x0b,0x06,0x05,0x02,0x05,
/*001d70*/ 0x01,0x04,0x0a,0x09,0x0b,0x0a,0x01,0x03,0x00,0x06,0x0c,0x05,0x04,0x06,0x03,0x02,
/*001d80*/ 0x07,0x06,0x06,0x08,0x0d,0x0c,0x03,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,
/*001d90*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,
/*001da0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,
/*001db0*/ 0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*001dc0*/ 0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x01,0x21,0x15,0x33,0x15,0x23,0x15,
/*001dd0*/ 0x21,0x15,0x21,0x11,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0xc8,0xc8,0x01,0x2c,0xfe,
/*001de0*/ 0x72,0x64,0x01,0x2a,0x01,0xf5,0x67,0x61,0xca,0x62,0x01,0xf2,0x64,0x00,0x00,0x01,
/*001df0*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x62,0x40,0x29,0x01,0x0c,
/*001e00*/ 0x0c,0x40,0x0d,0x00,0x0b,0x04,0x03,0x00,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,
/*001e10*/ 0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x05,0x04,0x06,0x03,0x02,0x0b,0x0a,
/*001e20*/ 0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*001e30*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,
/*001e40*/ 0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,
/*001e50*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,
/*001e60*/ 0x15,0x33,0x15,0x23,0x11,0x23,0x11,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0xc8,0xc8,
/*001e70*/ 0x62,0x64,0x01,0x2a,0x01,0xf5,0x67,0x61,0xfe,0xd4,0x01,0xf2,0x64,0x00,0x00,0x01,
/*001e80*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7c,0x40,0x39,0x01,0x10,
/*001e90*/ 0x10,0x40,0x11,0x00,0x03,0x02,0x0e,0x0d,0x05,0x00,0x0a,0x06,0x05,0x02,0x01,0x05,
/*001ea0*/ 0x09,0x04,0x04,0x03,0x0c,0x0b,0x04,0x0f,0x08,0x07,0x03,0x00,0x0b,0x0a,0x06,0x00,
/*001eb0*/ 0x09,0x05,0x04,0x03,0x08,0x06,0x06,0x0d,0x0c,0x06,0x0e,0x0f,0x0e,0x02,0x07,0x06,
/*001ec0*/ 0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*001ed0*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*001ee0*/ 0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,
/*001ef0*/ 0x49,0x68,0xb9,0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001f00*/ 0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,0x33,0x35,0x21,
/*001f10*/ 0x15,0x21,0x11,0x33,0x35,0x23,0x35,0x33,0x01,0x8f,0xfe,0xd6,0x64,0x64,0x01,0x2a,
/*001f20*/ 0xfe,0xd4,0xca,0x64,0xc6,0x01,0x65,0x01,0x8d,0x64,0x62,0xfe,0x6e,0xcb,0x61,0x00,
/*001f30*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x65,0x40,0x2c,
/*001f40*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0a,0x09,0x02,0x03,0x01,0x04,0x0b,0x00,0x08,0x07,
/*001f50*/ 0x04,0x03,0x03,0x04,0x06,0x05,0x03,0x02,0x06,0x08,0x09,0x08,0x02,0x0b,0x0a,0x07,
/*001f60*/ 0x03,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,
/*001f70*/ 0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*001f80*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,
/*001f90*/ 0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,
/*001fa0*/ 0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x35,0x33,0x01,
/*001fb0*/ 0x8f,0x62,0xca,0x62,0x62,0xca,0x62,0x01,0x01,0x2c,0xfe,0xd4,0x02,0x56,0xc8,0xc8,
/*001fc0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x6c,0x40,0x30,
/*001fd0*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x08,0x07,0x03,0x00,0x08,0x09,0x06,0x05,0x02,
/*001fe0*/ 0x03,0x01,0x08,0x03,0x04,0x03,0x04,0x0a,0x09,0x0b,0x0a,0x03,0x03,0x02,0x06,0x00,
/*001ff0*/ 0x09,0x08,0x05,0x03,0x04,0x06,0x06,0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x01,0x46,
/*002000*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*002010*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,
/*002020*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*002030*/ 0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x33,0x11,0x23,
/*002040*/ 0x35,0x21,0x15,0x23,0x11,0x33,0x01,0x2b,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x64,0x64,
/*002050*/ 0x01,0x62,0x01,0x92,0x62,0x62,0xfe,0x6d,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002060*/ 0x02,0x57,0x00,0x09,0x00,0x61,0x40,0x27,0x01,0x0a,0x0a,0x40,0x0b,0x00,0x04,0x03,
/*002070*/ 0x02,0x01,0x08,0x07,0x05,0x00,0x06,0x05,0x04,0x09,0x00,0x05,0x01,0x00,0x03,0x04,
/*002080*/ 0x06,0x02,0x07,0x06,0x06,0x08,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,
/*002090*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*0020a0*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,
/*0020b0*/ 0x68,0xb9,0x00,0x03,0x00,0x0a,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0020c0*/ 0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x11,0x23,0x35,
/*0020d0*/ 0x33,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2c,0x64,0xc6,0x65,0x64,0x62,0x01,0x92,0x62,
/*0020e0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x76,0x40,0x37,
/*0020f0*/ 0x01,0x10,0x10,0x40,0x11,0x00,0x0f,0x0e,0x0e,0x0d,0x0a,0x09,0x02,0x05,0x01,0x04,
/*002100*/ 0x0f,0x0c,0x0b,0x03,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x0d,0x0c,0x07,
/*002110*/ 0x06,0x03,0x02,0x06,0x08,0x09,0x08,0x02,0x0b,0x0a,0x07,0x03,0x06,0x03,0x05,0x04,
/*002120*/ 0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,
/*002130*/ 0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,
/*002140*/ 0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*002150*/ 0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,
/*002160*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x35,0x33,
/*002170*/ 0x15,0x23,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xca,0x62,0x64,0x64,0x01,0x01,
/*002180*/ 0x2c,0xfe,0xd4,0x02,0x56,0xc8,0xc8,0xc6,0x67,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*002190*/ 0x01,0x8f,0x02,0x57,0x00,0x07,0x00,0x53,0x40,0x1f,0x01,0x08,0x08,0x40,0x09,0x00,
/*0021a0*/ 0x03,0x02,0x07,0x00,0x06,0x02,0x01,0x03,0x05,0x04,0x04,0x03,0x07,0x06,0x06,0x00,
/*0021b0*/ 0x05,0x04,0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*0021c0*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x2e,0x2e,
/*0021d0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,
/*0021e0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,
/*0021f0*/ 0x33,0x11,0x21,0x01,0x8f,0xfe,0xd6,0x64,0x62,0x01,0x2c,0x01,0x65,0x01,0xf1,0xfe,
/*002200*/ 0x0c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x13,0x00,0x8a,
/*002210*/ 0x40,0x43,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x05,0x00,0x12,0x11,0x02,0x03,
/*002220*/ 0x01,0x04,0x13,0x00,0x04,0x03,0x04,0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,
/*002230*/ 0x07,0x04,0x0a,0x09,0x07,0x06,0x03,0x03,0x02,0x06,0x11,0x0d,0x0c,0x03,0x10,0x05,
/*002240*/ 0x04,0x06,0x0e,0x0f,0x0e,0x02,0x13,0x12,0x0b,0x03,0x0a,0x03,0x09,0x08,0x01,0x03,
/*002250*/ 0x00,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*002260*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,
/*002270*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,
/*002280*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*002290*/ 0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x15,0x23,
/*0022a0*/ 0x35,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x35,0x33,0x01,0xf3,
/*0022b0*/ 0x62,0x67,0x61,0x67,0x61,0x62,0x64,0x67,0x64,0x61,0x01,0x01,0x90,0x64,0x64,0xfe,
/*0022c0*/ 0x70,0x02,0x56,0x65,0x63,0x64,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*0022d0*/ 0x02,0x57,0x00,0x0f,0x00,0x79,0x40,0x38,0x01,0x10,0x10,0x40,0x11,0x00,0x0c,0x04,
/*0022e0*/ 0x03,0x03,0x0b,0x05,0x07,0x0e,0x0d,0x02,0x03,0x01,0x04,0x0f,0x00,0x0a,0x09,0x06,
/*0022f0*/ 0x03,0x05,0x04,0x08,0x07,0x03,0x02,0x06,0x0c,0x05,0x04,0x06,0x0b,0x0a,0x0d,0x0c,
/*002300*/ 0x02,0x0f,0x0e,0x09,0x03,0x08,0x03,0x07,0x06,0x01,0x03,0x00,0x01,0x01,0x07,0x46,
/*002310*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,
/*002320*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*002330*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,
/*002340*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,
/*002350*/ 0x23,0x11,0x23,0x35,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x01,
/*002360*/ 0x8f,0x62,0x64,0x67,0x61,0x62,0x64,0x67,0x61,0x01,0x01,0x2d,0x63,0xfe,0x70,0x02,
/*002370*/ 0x56,0x65,0x63,0xc8,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,
/*002380*/ 0x00,0x0f,0x00,0x6f,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,0x0f,0x0e,0x08,0x07,
/*002390*/ 0x04,0x05,0x03,0x04,0x06,0x05,0x0d,0x0c,0x0a,0x02,0x01,0x05,0x09,0x04,0x0b,0x00,
/*0023a0*/ 0x0f,0x0c,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,0x0a,0x07,0x05,0x06,
/*0023b0*/ 0x06,0x08,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0023c0*/ 0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,
/*0023d0*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,
/*0023e0*/ 0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,
/*0023f0*/ 0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x03,
/*002400*/ 0x11,0x23,0x11,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0xc6,0x65,0x64,0x65,
/*002410*/ 0x01,0x8d,0x64,0x65,0xfe,0x73,0x01,0x8e,0xfe,0x72,0x00,0x02,0x00,0x01,0x00,0x01,
/*002420*/ 0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6b,0x40,0x30,0x01,0x0e,0x0e,0x40,
/*002430*/ 0x0f,0x00,0x0d,0x04,0x03,0x03,0x0c,0x04,0x06,0x05,0x0b,0x0a,0x08,0x02,0x01,0x05,
/*002440*/ 0x07,0x04,0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x03,0x02,0x0c,0x09,0x08,0x03,
/*002450*/ 0x0b,0x06,0x06,0x07,0x06,0x03,0x05,0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,
/*002460*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x01,0x2f,
/*002470*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*002480*/ 0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*002490*/ 0x0e,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,0x23,0x11,0x21,0x15,0x33,0x07,
/*0024a0*/ 0x35,0x23,0x15,0x01,0x8f,0x65,0xc7,0x62,0x01,0x2a,0x64,0x64,0xc6,0x01,0x2d,0x64,
/*0024b0*/ 0xc8,0x02,0x56,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,
/*0024c0*/ 0x02,0x57,0x00,0x09,0x00,0x0f,0x00,0x13,0x00,0x85,0x40,0x42,0x01,0x14,0x14,0x40,
/*0024d0*/ 0x15,0x00,0x11,0x0f,0x0e,0x03,0x10,0x05,0x00,0x13,0x12,0x0d,0x0c,0x06,0x05,0x02,
/*0024e0*/ 0x07,0x01,0x04,0x04,0x03,0x0b,0x0a,0x08,0x03,0x07,0x04,0x09,0x00,0x13,0x03,0x02,
/*0024f0*/ 0x03,0x10,0x06,0x00,0x0c,0x0b,0x09,0x08,0x05,0x05,0x04,0x06,0x06,0x0e,0x0d,0x06,
/*002500*/ 0x12,0x0f,0x0a,0x03,0x11,0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,
/*002510*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,
/*002520*/ 0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*002530*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x14,0x49,0x68,
/*002540*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,
/*002550*/ 0x21,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x03,0x11,0x23,0x15,0x33,0x15,0x07,
/*002560*/ 0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x64,0xc6,0x64,0x64,0xc6,0x64,0x02,0x62,
/*002570*/ 0x01,0x65,0x01,0x8d,0x64,0x65,0xfe,0xd7,0x01,0x2a,0xc5,0x65,0x64,0x62,0x62,0x00,
/*002580*/ 0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x11,0x00,0x7c,
/*002590*/ 0x40,0x3c,0x01,0x12,0x12,0x40,0x13,0x00,0x11,0x04,0x03,0x03,0x10,0x04,0x06,0x05,
/*0025a0*/ 0x0f,0x0e,0x0c,0x0b,0x08,0x02,0x01,0x07,0x07,0x04,0x0d,0x0a,0x09,0x03,0x00,0x0d,
/*0025b0*/ 0x0c,0x07,0x00,0x11,0x0e,0x0b,0x03,0x0a,0x06,0x03,0x02,0x10,0x09,0x08,0x03,0x0f,
/*0025c0*/ 0x06,0x06,0x07,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,
/*0025d0*/ 0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*0025e0*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*0025f0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,
/*002600*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x12,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,
/*002610*/ 0x15,0x23,0x11,0x21,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x01,0x8f,
/*002620*/ 0x62,0xca,0x62,0x01,0x2a,0x64,0x64,0x64,0x64,0xc6,0x01,0xc8,0xc8,0x02,0x56,0x65,
/*002630*/ 0xc5,0x67,0x67,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*002640*/ 0x00,0x17,0x00,0xa3,0x40,0x53,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x10,0x0f,0x03,
/*002650*/ 0x00,0x04,0x05,0x16,0x06,0x05,0x02,0x01,0x05,0x15,0x04,0x07,0x14,0x08,0x07,0x03,
/*002660*/ 0x13,0x04,0x09,0x12,0x0e,0x0d,0x0a,0x09,0x05,0x11,0x04,0x0c,0x0b,0x04,0x03,0x03,
/*002670*/ 0x17,0x16,0x06,0x00,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x15,0x14,0x06,0x07,0x06,
/*002680*/ 0x09,0x08,0x06,0x13,0x12,0x0b,0x0a,0x06,0x0c,0x11,0x0d,0x0c,0x03,0x10,0x06,0x0e,
/*002690*/ 0x0f,0x0e,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*0026a0*/ 0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,
/*0026b0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,
/*0026c0*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*0026d0*/ 0x68,0xb9,0x00,0x03,0x00,0x18,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0026e0*/ 0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x35,0x23,0x35,
/*0026f0*/ 0x23,0x35,0x23,0x35,0x33,0x35,0x21,0x15,0x21,0x15,0x33,0x15,0x33,0x15,0x33,0x01,
/*002700*/ 0x8f,0x65,0xfe,0xd7,0x01,0x2c,0x64,0x64,0x64,0x64,0x01,0x2a,0xfe,0xd4,0x64,0x64,
/*002710*/ 0x64,0x65,0x64,0x62,0x67,0x64,0x64,0x61,0x64,0x62,0x67,0x64,0x64,0x00,0x00,0x01,
/*002720*/ 0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x07,0x00,0x53,0x40,0x1f,0x01,0x08,
/*002730*/ 0x08,0x40,0x09,0x00,0x07,0x06,0x05,0x00,0x02,0x01,0x04,0x04,0x03,0x05,0x04,0x01,
/*002740*/ 0x03,0x00,0x06,0x06,0x07,0x06,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,
/*002750*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2e,
/*002760*/ 0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x08,0x49,0x68,
/*002770*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x01,
/*002780*/ 0x23,0x11,0x23,0x11,0x23,0x35,0x21,0x01,0xf3,0xc8,0x62,0xc8,0x01,0xf2,0x01,0xf5,
/*002790*/ 0xfe,0x0c,0x01,0xf4,0x62,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0027a0*/ 0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x05,0x04,0x02,0x01,
/*0027b0*/ 0x05,0x03,0x08,0x04,0x03,0x03,0x07,0x04,0x06,0x05,0x0a,0x09,0x04,0x0b,0x00,0x09,
/*0027c0*/ 0x01,0x00,0x03,0x08,0x06,0x02,0x0b,0x0a,0x07,0x03,0x06,0x03,0x03,0x02,0x01,0x01,
/*0027d0*/ 0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0027e0*/ 0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,
/*0027f0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,
/*002800*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*002810*/ 0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x62,0x65,
/*002820*/ 0x64,0x65,0x01,0xf1,0xfe,0x0c,0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002830*/ 0x02,0x57,0x00,0x0f,0x00,0x79,0x40,0x37,0x01,0x10,0x10,0x40,0x11,0x00,0x04,0x03,
/*002840*/ 0x05,0x07,0x0a,0x09,0x06,0x03,0x05,0x04,0x08,0x07,0x0c,0x0b,0x04,0x02,0x01,0x0e,
/*002850*/ 0x0d,0x04,0x0f,0x00,0x0d,0x01,0x00,0x03,0x0c,0x06,0x02,0x0b,0x03,0x02,0x03,0x0a,
/*002860*/ 0x06,0x05,0x04,0x0f,0x0e,0x09,0x03,0x08,0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,
/*002870*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*002880*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*002890*/ 0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,
/*0028a0*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,
/*0028b0*/ 0x23,0x15,0x23,0x15,0x23,0x15,0x23,0x11,0x33,0x11,0x33,0x35,0x33,0x11,0x33,0x01,
/*0028c0*/ 0x8f,0x65,0x64,0x64,0x61,0x62,0x67,0x64,0x61,0x01,0x2d,0x64,0x64,0x64,0x02,0x56,
/*0028d0*/ 0xfe,0x70,0x64,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,
/*0028e0*/ 0x00,0x13,0x00,0x87,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x0f,0x0e,0x09,0x08,
/*0028f0*/ 0x02,0x01,0x04,0x03,0x06,0x05,0x04,0x07,0x0c,0x08,0x07,0x03,0x0b,0x04,0x0a,0x09,
/*002900*/ 0x0e,0x0d,0x04,0x10,0x04,0x03,0x03,0x0f,0x12,0x11,0x04,0x13,0x00,0x11,0x10,0x0d,
/*002910*/ 0x05,0x04,0x01,0x00,0x07,0x0c,0x06,0x02,0x13,0x12,0x0b,0x03,0x0a,0x03,0x07,0x06,
/*002920*/ 0x03,0x03,0x02,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,
/*002930*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,
/*002940*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x2e,0x2e,
/*002950*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,
/*002960*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*002970*/ 0x23,0x15,0x23,0x35,0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,
/*002980*/ 0xf3,0x65,0x61,0x67,0x61,0x64,0x62,0x67,0x61,0x66,0x62,0x65,0x64,0x64,0x64,0x65,
/*002990*/ 0x01,0xf1,0xfe,0x0c,0x01,0x90,0xfe,0x70,0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,
/*0029a0*/ 0x01,0x8f,0x02,0x57,0x00,0x13,0x00,0x87,0x40,0x44,0x01,0x14,0x14,0x40,0x15,0x00,
/*0029b0*/ 0x12,0x11,0x06,0x02,0x01,0x05,0x05,0x04,0x13,0x04,0x03,0x03,0x00,0x10,0x0f,0x0c,
/*0029c0*/ 0x0b,0x08,0x05,0x07,0x04,0x0e,0x0d,0x0a,0x03,0x09,0x0d,0x0c,0x01,0x03,0x00,0x07,
/*0029d0*/ 0x0e,0x03,0x02,0x07,0x04,0x0b,0x0a,0x07,0x03,0x06,0x07,0x10,0x11,0x10,0x02,0x13,
/*0029e0*/ 0x12,0x0f,0x03,0x0e,0x03,0x09,0x08,0x05,0x03,0x04,0x01,0x01,0x09,0x46,0x76,0x2f,
/*0029f0*/ 0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*002a00*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,
/*002a10*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,
/*002a20*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,
/*002a30*/ 0x23,0x15,0x33,0x15,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,
/*002a40*/ 0x33,0x35,0x33,0x01,0x8f,0x64,0x64,0x62,0xca,0x62,0x64,0x64,0x62,0xca,0x62,0x01,
/*002a50*/ 0x91,0xcb,0xc5,0xc8,0xc8,0xc6,0xca,0xc6,0xc8,0xc8,0x00,0x01,0x00,0x01,0x00,0x01,
/*002a60*/ 0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x76,0x40,0x36,0x01,0x10,0x10,0x40,0x11,0x00,
/*002a70*/ 0x09,0x08,0x02,0x01,0x05,0x03,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x00,0x0c,0x08,
/*002a80*/ 0x07,0x04,0x03,0x05,0x0b,0x04,0x0a,0x09,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x0d,
/*002a90*/ 0x0c,0x06,0x07,0x06,0x0f,0x0e,0x0b,0x03,0x0a,0x03,0x03,0x02,0x01,0x01,0x09,0x46,
/*002aa0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,
/*002ab0*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,
/*002ac0*/ 0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x10,0x49,0x68,0x61,
/*002ad0*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,
/*002ae0*/ 0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,
/*002af0*/ 0x65,0xc5,0xc8,0xc8,0x64,0x62,0xca,0x62,0x65,0x64,0x62,0x67,0x64,0x01,0x29,0xfe,
/*002b00*/ 0xd4,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x13,
/*002b10*/ 0x00,0x8e,0x40,0x43,0x01,0x14,0x14,0x40,0x15,0x00,0x06,0x05,0x04,0x12,0x11,0x0a,
/*002b20*/ 0x03,0x09,0x04,0x03,0x04,0x0c,0x0b,0x0e,0x0d,0x04,0x02,0x01,0x10,0x0f,0x04,0x13,
/*002b30*/ 0x08,0x07,0x03,0x00,0x01,0x00,0x07,0x12,0x0d,0x03,0x02,0x03,0x0c,0x06,0x0b,0x0a,
/*002b40*/ 0x05,0x03,0x04,0x07,0x06,0x06,0x08,0x11,0x10,0x06,0x12,0x13,0x12,0x03,0x0f,0x0e,
/*002b50*/ 0x02,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*002b60*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*002b70*/ 0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,
/*002b80*/ 0x17,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,
/*002b90*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*002ba0*/ 0x01,0x23,0x15,0x23,0x15,0x23,0x15,0x21,0x15,0x21,0x35,0x33,0x35,0x33,0x35,0x33,
/*002bb0*/ 0x35,0x21,0x35,0x21,0x01,0x8f,0x65,0x64,0x63,0x01,0x2c,0xfe,0x72,0x65,0x64,0x63,
/*002bc0*/ 0xfe,0xd4,0x01,0x8e,0x01,0x91,0x64,0x64,0x67,0x61,0xc6,0x64,0x64,0x66,0x62,0x00,
/*002bd0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,0x02,0x57,0x00,0x07,0x00,0x59,0x40,0x23,
/*002be0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,0x00,0x05,0x01,0x06,0x05,0x04,
/*002bf0*/ 0x02,0x01,0x07,0x06,0x06,0x00,0x05,0x04,0x06,0x02,0x03,0x02,0x03,0x01,0x00,0x01,
/*002c00*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,
/*002c10*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*002c20*/ 0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*002c30*/ 0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,0x15,0x23,0x11,0x33,0xc7,
/*002c40*/ 0xc6,0xc6,0x64,0x64,0x01,0x02,0x56,0x62,0xfe,0x6d,0x00,0x01,0x00,0x01,0x00,0x01,
/*002c50*/ 0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x6d,0x40,0x2f,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*002c60*/ 0x0b,0x00,0x04,0x01,0x0a,0x02,0x01,0x03,0x09,0x04,0x03,0x08,0x04,0x03,0x03,0x07,
/*002c70*/ 0x04,0x06,0x05,0x0b,0x0a,0x07,0x00,0x03,0x02,0x07,0x09,0x08,0x05,0x04,0x07,0x06,
/*002c80*/ 0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*002c90*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*002ca0*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*002cb0*/ 0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*002cc0*/ 0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x35,0x23,0x35,0x33,0x15,0x33,0x15,
/*002cd0*/ 0x33,0x01,0x2b,0x62,0x64,0x64,0x62,0x64,0x64,0x01,0xc9,0xc7,0xc6,0xc9,0xc8,0x00,
/*002ce0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,0x02,0x57,0x00,0x07,0x00,0x59,0x40,0x23,
/*002cf0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x06,0x05,0x02,0x03,0x01,0x05,0x00,0x04,0x03,0x04,
/*002d00*/ 0x07,0x00,0x03,0x02,0x06,0x00,0x05,0x04,0x06,0x06,0x07,0x06,0x03,0x01,0x00,0x01,
/*002d10*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,
/*002d20*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*002d30*/ 0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*002d40*/ 0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0xc7,
/*002d50*/ 0xc6,0x64,0x64,0xc6,0x01,0x62,0x01,0x92,0x62,0x00,0x00,0x01,0x00,0x01,0x01,0x91,
/*002d60*/ 0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x69,0x40,0x2d,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*002d70*/ 0x0b,0x00,0x04,0x01,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x0a,0x02,0x01,0x03,
/*002d80*/ 0x09,0x0b,0x0a,0x06,0x00,0x07,0x06,0x03,0x03,0x02,0x06,0x08,0x05,0x04,0x01,0x03,
/*002d90*/ 0x00,0x09,0x08,0x03,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x17,
/*002da0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,
/*002db0*/ 0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,
/*002dc0*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,
/*002dd0*/ 0x59,0x01,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x2b,0x62,
/*002de0*/ 0x67,0x61,0x64,0x62,0x64,0x01,0x91,0x64,0x64,0x62,0x64,0x65,0x00,0x01,0x00,0x01,
/*002df0*/ 0x00,0x01,0x01,0x8f,0x00,0x63,0x00,0x03,0x00,0x3e,0x40,0x12,0x01,0x04,0x04,0x40,
/*002e00*/ 0x05,0x00,0x03,0x02,0x01,0x00,0x03,0x02,0x01,0x00,0x01,0x01,0x01,0x46,0x76,0x2f,
/*002e10*/ 0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,
/*002e20*/ 0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*002e30*/ 0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x21,0x01,0x8f,0xfe,0x72,
/*002e40*/ 0x01,0x8e,0x01,0x62,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x07,
/*002e50*/ 0x00,0x0b,0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x0a,0x06,0x05,
/*002e60*/ 0x02,0x05,0x01,0x04,0x04,0x03,0x09,0x08,0x04,0x07,0x00,0x0b,0x03,0x02,0x03,0x08,
/*002e70*/ 0x06,0x00,0x0a,0x09,0x05,0x03,0x04,0x06,0x06,0x07,0x06,0x02,0x01,0x00,0x01,0x01,
/*002e80*/ 0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*002e90*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,
/*002ea0*/ 0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*002eb0*/ 0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x35,0x33,0x35,
/*002ec0*/ 0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x64,0xc6,0x01,
/*002ed0*/ 0x65,0xc5,0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002ee0*/ 0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,0x0f,0x00,
/*002ef0*/ 0x0d,0x06,0x05,0x03,0x0c,0x04,0x04,0x03,0x0b,0x0a,0x08,0x02,0x01,0x05,0x07,0x04,
/*002f00*/ 0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x02,0x0c,0x09,0x08,0x03,0x0b,0x06,0x06,
/*002f10*/ 0x07,0x06,0x02,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,
/*002f20*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*002f30*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*002f40*/ 0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*002f50*/ 0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x11,0x33,0x15,0x33,0x15,0x33,
/*002f60*/ 0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xfe,0xd7,0x62,0xc8,0x64,0x64,0xc6,0x65,0x64,
/*002f70*/ 0x02,0x56,0xc8,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002f80*/ 0x01,0x8f,0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x04,
/*002f90*/ 0x03,0x00,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,0x07,0x07,0x06,0x07,0x08,
/*002fa0*/ 0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x02,0x05,0x04,
/*002fb0*/ 0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*002fc0*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,
/*002fd0*/ 0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*002fe0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,0x15,
/*002ff0*/ 0x21,0x15,0x21,0x35,0x23,0x35,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0x01,0x2c,0xfe,
/*003000*/ 0xd6,0x64,0x64,0x01,0x2a,0x01,0x2d,0xca,0x62,0x65,0xc5,0x64,0x00,0x02,0x00,0x01,
/*003010*/ 0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,
/*003020*/ 0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x06,0x05,0x02,0x05,0x01,0x04,0x04,0x03,0x0b,0x08,
/*003030*/ 0x07,0x03,0x0a,0x04,0x09,0x00,0x0d,0x03,0x02,0x03,0x0a,0x06,0x00,0x0c,0x0b,0x05,
/*003040*/ 0x03,0x04,0x06,0x06,0x09,0x08,0x03,0x07,0x06,0x02,0x01,0x00,0x01,0x01,0x03,0x46,
/*003050*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*003060*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*003070*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003080*/ 0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x35,0x33,
/*003090*/ 0x35,0x33,0x35,0x33,0x03,0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x65,0xc7,0x62,
/*0030a0*/ 0x64,0xc6,0x01,0x65,0xc5,0x64,0xc8,0xfe,0x0e,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,
/*0030b0*/ 0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0d,0x00,0x11,0x00,0x89,0x40,0x43,0x01,0x12,
/*0030c0*/ 0x12,0x40,0x13,0x00,0x0c,0x0b,0x05,0x05,0x0d,0x04,0x03,0x03,0x00,0x05,0x01,0x0f,
/*0030d0*/ 0x0e,0x02,0x03,0x01,0x04,0x05,0x11,0x10,0x0a,0x09,0x06,0x05,0x05,0x04,0x08,0x07,
/*0030e0*/ 0x07,0x06,0x07,0x08,0x11,0x0e,0x01,0x03,0x00,0x06,0x0c,0x03,0x02,0x06,0x04,0x10,
/*0030f0*/ 0x0f,0x0d,0x0c,0x09,0x05,0x08,0x06,0x0a,0x0b,0x0a,0x02,0x05,0x04,0x01,0x01,0x07,
/*003100*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*003110*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*003120*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*003130*/ 0x00,0x07,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*003140*/ 0x12,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x33,0x15,0x21,0x35,0x23,0x35,0x33,0x35,
/*003150*/ 0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0xc8,0xc8,0xfe,0xd6,0x64,0x64,0xc6,
/*003160*/ 0x64,0xc8,0x62,0xc9,0x67,0x61,0x65,0xc5,0x64,0x65,0x61,0x62,0x62,0x00,0x00,0x01,
/*003170*/ 0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x68,0x40,0x2d,0x01,0x0c,
/*003180*/ 0x0c,0x40,0x0d,0x00,0x04,0x03,0x05,0x07,0x0b,0x00,0x05,0x09,0x0a,0x09,0x06,0x05,
/*003190*/ 0x02,0x05,0x01,0x04,0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x05,0x04,0x06,
/*0031a0*/ 0x03,0x02,0x0b,0x0a,0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0031b0*/ 0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,
/*0031c0*/ 0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*0031d0*/ 0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,
/*0031e0*/ 0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,0x23,0x11,0x23,0x11,0x33,0x35,0x33,
/*0031f0*/ 0x01,0x2b,0xc8,0x64,0x65,0x61,0x64,0xc6,0x01,0xf5,0x67,0x61,0xfe,0xd4,0x01,0xf2,
/*003200*/ 0x64,0x00,0x00,0x02,0x00,0x01,0xff,0x39,0x01,0x8f,0x01,0x8f,0x00,0x0d,0x00,0x11,
/*003210*/ 0x00,0x7a,0x40,0x3b,0x01,0x12,0x12,0x40,0x13,0x00,0x11,0x10,0x0c,0x0b,0x08,0x07,
/*003220*/ 0x04,0x07,0x03,0x04,0x0a,0x09,0x0f,0x06,0x05,0x02,0x01,0x05,0x0e,0x04,0x0d,0x00,
/*003230*/ 0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x11,0x09,0x08,0x03,0x0e,0x06,0x07,0x06,0x10,
/*003240*/ 0x0f,0x0b,0x03,0x0a,0x06,0x0c,0x0d,0x0c,0x02,0x03,0x02,0x00,0x01,0x09,0x46,0x76,
/*003250*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*003260*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*003270*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,
/*003280*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x12,0xff,0xc0,0x38,0x59,0x05,0x23,0x15,0x23,
/*003290*/ 0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,
/*0032a0*/ 0x65,0xc5,0xc8,0xc8,0x64,0x64,0x01,0x2a,0x64,0xc6,0x63,0x64,0x62,0x67,0x64,0xc5,
/*0032b0*/ 0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0032c0*/ 0x00,0x0b,0x00,0x65,0x40,0x2b,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x0a,0x0a,0x09,
/*0032d0*/ 0x02,0x03,0x01,0x04,0x0b,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x03,0x02,
/*0032e0*/ 0x06,0x08,0x09,0x08,0x02,0x07,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,
/*0032f0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*003300*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,
/*003310*/ 0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*003320*/ 0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,
/*003330*/ 0x33,0x15,0x33,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xc8,0x64,0x01,0x01,0x2c,
/*003340*/ 0xfe,0xd4,0x02,0x56,0xc8,0x65,0x00,0x02,0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,
/*003350*/ 0x00,0x03,0x00,0x07,0x00,0x54,0x40,0x21,0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,
/*003360*/ 0x03,0x03,0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x01,0x00,0x06,0x02,0x07,0x06,0x02,
/*003370*/ 0x05,0x04,0x01,0x03,0x02,0x03,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*003380*/ 0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*003390*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*0033a0*/ 0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,
/*0033b0*/ 0x11,0x33,0x63,0x62,0x62,0x62,0x62,0x01,0xf5,0x62,0xfd,0xaa,0x01,0x8e,0x00,0x02,
/*0033c0*/ 0x00,0x01,0xff,0x39,0x00,0xc7,0x02,0x57,0x00,0x03,0x00,0x0b,0x00,0x68,0x40,0x2d,
/*0033d0*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0a,0x09,0x02,0x03,0x01,0x04,0x0b,0x04,0x03,0x03,
/*0033e0*/ 0x00,0x06,0x05,0x04,0x08,0x07,0x01,0x00,0x06,0x02,0x09,0x05,0x04,0x03,0x08,0x06,
/*0033f0*/ 0x06,0x0b,0x0a,0x02,0x07,0x06,0x00,0x03,0x02,0x03,0x01,0x07,0x46,0x76,0x2f,0x37,
/*003400*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,
/*003410*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*003420*/ 0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003430*/ 0x00,0x0c,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x15,0x23,0x35,0x33,
/*003440*/ 0x11,0x33,0xc7,0x62,0x62,0x65,0x61,0x64,0x62,0x01,0xf5,0x62,0xfd,0x46,0x64,0x62,
/*003450*/ 0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x79,
/*003460*/ 0x40,0x39,0x01,0x10,0x10,0x40,0x11,0x00,0x0e,0x0d,0x0a,0x09,0x02,0x05,0x01,0x04,
/*003470*/ 0x0f,0x0c,0x0b,0x03,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x0f,0x0e,0x07,
/*003480*/ 0x00,0x03,0x02,0x06,0x08,0x0d,0x09,0x08,0x03,0x0c,0x06,0x0a,0x0b,0x0a,0x02,0x07,
/*003490*/ 0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0034a0*/ 0x3f,0x17,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,
/*0034b0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,
/*0034c0*/ 0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*0034d0*/ 0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x15,0x23,0x11,
/*0034e0*/ 0x33,0x11,0x33,0x35,0x33,0x15,0x23,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xcb,
/*0034f0*/ 0x61,0x64,0x64,0x01,0xc8,0xc8,0x02,0x56,0xfe,0xd4,0x64,0x62,0x67,0x00,0x00,0x01,
/*003500*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x40,0x40,0x14,0x01,0x04,
/*003510*/ 0x04,0x40,0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x03,0x02,0x03,0x01,0x00,0x01,0x01,
/*003520*/ 0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,
/*003530*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,
/*003540*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,
/*003550*/ 0x63,0x62,0x62,0x01,0x02,0x56,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0x8f,
/*003560*/ 0x00,0x0d,0x00,0x6b,0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x0c,0x0b,
/*003570*/ 0x02,0x03,0x01,0x04,0x0d,0x00,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x0a,0x09,
/*003580*/ 0x07,0x06,0x03,0x03,0x02,0x06,0x0a,0x0b,0x0a,0x02,0x09,0x08,0x05,0x04,0x01,0x05,
/*003590*/ 0x00,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,
/*0035a0*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,
/*0035b0*/ 0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x0e,0x49,0x68,
/*0035c0*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,
/*0035d0*/ 0x23,0x11,0x23,0x11,0x23,0x11,0x23,0x11,0x23,0x11,0x21,0x15,0x33,0x01,0xf3,0x62,
/*0035e0*/ 0x67,0x61,0x67,0x61,0x01,0x8e,0x64,0x01,0x01,0x2c,0xfe,0xd4,0x01,0x2c,0xfe,0xd4,
/*0035f0*/ 0x01,0x8e,0x65,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x09,
/*003600*/ 0x00,0x5c,0x40,0x25,0x01,0x0a,0x0a,0x40,0x0b,0x00,0x09,0x08,0x08,0x07,0x02,0x03,
/*003610*/ 0x01,0x04,0x09,0x00,0x04,0x03,0x04,0x06,0x05,0x03,0x02,0x06,0x06,0x07,0x06,0x02,
/*003620*/ 0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,
/*003630*/ 0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*003640*/ 0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0a,0x49,0x68,0x61,
/*003650*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x23,
/*003660*/ 0x11,0x23,0x11,0x23,0x11,0x21,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x01,0x2a,0x64,
/*003670*/ 0x01,0x01,0x2c,0xfe,0xd4,0x01,0x8e,0x65,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*003680*/ 0x01,0x8f,0x00,0x0b,0x00,0x0f,0x00,0x6f,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,
/*003690*/ 0x0f,0x0e,0x08,0x07,0x04,0x05,0x03,0x04,0x06,0x05,0x0d,0x0c,0x0a,0x02,0x01,0x05,
/*0036a0*/ 0x09,0x04,0x0b,0x00,0x0f,0x0c,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,
/*0036b0*/ 0x0a,0x07,0x05,0x06,0x06,0x08,0x09,0x08,0x02,0x03,0x02,0x01,0x01,0x05,0x46,0x76,
/*0036c0*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0036d0*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*0036e0*/ 0x68,0xb9,0x00,0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0036f0*/ 0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,
/*003700*/ 0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,
/*003710*/ 0xc6,0x65,0x64,0x65,0xc5,0x64,0x65,0xc5,0xc6,0xc6,0x00,0x02,0x00,0x01,0xff,0x39,
/*003720*/ 0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,
/*003730*/ 0x0f,0x00,0x0d,0x04,0x03,0x03,0x0c,0x04,0x06,0x05,0x0b,0x0a,0x08,0x02,0x01,0x05,
/*003740*/ 0x07,0x04,0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x02,0x0c,0x09,0x08,0x03,0x0b,
/*003750*/ 0x06,0x06,0x07,0x06,0x02,0x05,0x04,0x00,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,
/*003760*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*003770*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*003780*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*003790*/ 0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x15,0x23,0x11,0x21,
/*0037a0*/ 0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xc7,0x62,0x01,0x2a,0x64,0x64,0xc6,
/*0037b0*/ 0x65,0x64,0xc8,0x02,0x56,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,0xff,0x39,
/*0037c0*/ 0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,
/*0037d0*/ 0x0f,0x00,0x0d,0x0c,0x08,0x07,0x04,0x05,0x03,0x04,0x06,0x05,0x0b,0x02,0x01,0x03,
/*0037e0*/ 0x0a,0x04,0x09,0x00,0x0d,0x05,0x04,0x03,0x0a,0x06,0x02,0x0c,0x0b,0x07,0x03,0x06,
/*0037f0*/ 0x06,0x08,0x09,0x08,0x02,0x03,0x02,0x01,0x01,0x00,0x00,0x01,0x05,0x46,0x76,0x2f,
/*003800*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*003810*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*003820*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*003830*/ 0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x05,0x23,0x35,0x23,0x35,0x23,0x35,0x33,
/*003840*/ 0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0x62,0xc8,0x64,0x64,0x01,0x2a,0x64,0xc6,
/*003850*/ 0xc7,0xc8,0x65,0xc5,0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*003860*/ 0x01,0x8f,0x01,0x8f,0x00,0x0b,0x00,0x65,0x40,0x2b,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*003870*/ 0x02,0x01,0x05,0x05,0x0a,0x09,0x05,0x0b,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,
/*003880*/ 0x05,0x03,0x02,0x06,0x09,0x01,0x00,0x03,0x08,0x0b,0x0a,0x07,0x03,0x06,0x02,0x05,
/*003890*/ 0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,
/*0038a0*/ 0x17,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,
/*0038b0*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*0038c0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,
/*0038d0*/ 0x23,0x15,0x23,0x11,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0xc9,0x64,0x61,0x62,0x67,
/*0038e0*/ 0xc5,0x01,0x2d,0x64,0xc8,0x01,0x8e,0x64,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*0038f0*/ 0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x85,0x40,0x3f,0x01,0x10,0x10,0x40,0x11,0x00,
/*003900*/ 0x0a,0x09,0x08,0x05,0x0f,0x0c,0x0b,0x03,0x00,0x05,0x05,0x08,0x07,0x04,0x03,0x03,
/*003910*/ 0x05,0x0d,0x0e,0x0d,0x06,0x03,0x05,0x04,0x02,0x01,0x0f,0x0e,0x06,0x00,0x05,0x01,
/*003920*/ 0x00,0x03,0x04,0x06,0x02,0x07,0x06,0x06,0x08,0x0d,0x09,0x08,0x03,0x0c,0x06,0x0a,
/*003930*/ 0x0b,0x0a,0x02,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*003940*/ 0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,
/*003950*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*003960*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,
/*003970*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*003980*/ 0x21,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x21,0x15,0x23,0x15,0x33,0x01,0x8f,0x65,
/*003990*/ 0xfe,0xd7,0xc8,0xc8,0x64,0x01,0x2a,0xc8,0xc8,0x65,0x64,0x62,0x67,0x61,0x64,0x62,
/*0039a0*/ 0x67,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x0b,0x00,0x64,
/*0039b0*/ 0x40,0x2a,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x03,0x02,0x0b,0x08,0x07,0x03,0x00,0x05,
/*0039c0*/ 0x01,0x0a,0x09,0x06,0x02,0x01,0x05,0x05,0x04,0x04,0x03,0x0b,0x0a,0x06,0x00,0x09,
/*0039d0*/ 0x08,0x06,0x07,0x06,0x05,0x04,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,
/*0039e0*/ 0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*0039f0*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*003a00*/ 0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,
/*003a10*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x11,0x33,0x15,0x33,0x15,0x23,0x15,0x33,
/*003a20*/ 0x01,0x2b,0xc6,0x64,0x62,0xc8,0xc8,0xc8,0x01,0x65,0x01,0x8d,0x65,0x61,0xca,0x00,
/*003a30*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x5c,0x40,0x25,
/*003a40*/ 0x01,0x0a,0x0a,0x40,0x0b,0x00,0x03,0x02,0x06,0x02,0x01,0x03,0x05,0x04,0x04,0x03,
/*003a50*/ 0x08,0x07,0x04,0x09,0x00,0x07,0x06,0x06,0x00,0x09,0x08,0x05,0x03,0x04,0x02,0x01,
/*003a60*/ 0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,
/*003a70*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,
/*003a80*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0a,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003a90*/ 0x38,0x11,0x37,0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,0x33,
/*003aa0*/ 0x11,0x33,0x11,0x33,0x01,0x8f,0xfe,0xd6,0x64,0x62,0xca,0x62,0x01,0x65,0x01,0x29,
/*003ab0*/ 0xfe,0xd4,0x01,0x2c,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0b,
/*003ac0*/ 0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x02,0x01,0x05,0x03,0x08,0x04,
/*003ad0*/ 0x03,0x03,0x07,0x04,0x06,0x05,0x0a,0x09,0x04,0x0b,0x00,0x09,0x05,0x04,0x01,0x00,
/*003ae0*/ 0x05,0x08,0x07,0x02,0x0b,0x0a,0x07,0x03,0x06,0x02,0x03,0x02,0x01,0x01,0x05,0x46,
/*003af0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*003b00*/ 0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*003b10*/ 0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*003b20*/ 0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x15,
/*003b30*/ 0x33,0x35,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x62,0xc9,0xc8,0xc8,0xc6,0xc8,
/*003b40*/ 0xc8,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0x8f,0x00,0x0d,0x00,0x6b,
/*003b50*/ 0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x03,0x02,0x06,0x02,0x01,0x03,0x05,0x04,
/*003b60*/ 0x04,0x03,0x08,0x07,0x04,0x0a,0x09,0x0c,0x0b,0x04,0x0d,0x00,0x0b,0x0a,0x07,0x03,
/*003b70*/ 0x06,0x06,0x00,0x0d,0x0c,0x09,0x08,0x05,0x05,0x04,0x02,0x01,0x00,0x01,0x01,0x03,
/*003b80*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*003b90*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,
/*003ba0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,
/*003bb0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,
/*003bc0*/ 0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0xf3,0xfe,0x72,0x64,0x62,0x67,
/*003bd0*/ 0x61,0x67,0x61,0x01,0x65,0x01,0x29,0xfe,0xd4,0x01,0x2c,0xfe,0xd4,0x01,0x2c,0x00,
/*003be0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x13,0x00,0x89,0x40,0x45,
/*003bf0*/ 0x01,0x14,0x14,0x40,0x15,0x00,0x0c,0x0b,0x05,0x01,0x12,0x11,0x06,0x02,0x01,0x05,
/*003c00*/ 0x05,0x04,0x13,0x04,0x03,0x03,0x00,0x10,0x0f,0x08,0x03,0x07,0x04,0x0e,0x0d,0x0a,
/*003c10*/ 0x03,0x09,0x07,0x06,0x07,0x10,0x11,0x10,0x0d,0x0c,0x01,0x05,0x00,0x06,0x0e,0x0b,
/*003c20*/ 0x0a,0x03,0x03,0x02,0x06,0x04,0x13,0x12,0x0f,0x03,0x0e,0x02,0x09,0x08,0x05,0x03,
/*003c30*/ 0x04,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*003c40*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,
/*003c50*/ 0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*003c60*/ 0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*003c70*/ 0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,0x23,0x35,0x23,0x15,
/*003c80*/ 0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0x64,0x64,0x62,
/*003c90*/ 0xcb,0x61,0x64,0x64,0x62,0xcb,0x61,0x01,0x2d,0xcb,0x61,0x64,0x64,0x62,0xca,0x62,
/*003ca0*/ 0x64,0x64,0x00,0x01,0x00,0x01,0xff,0x39,0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x76,
/*003cb0*/ 0x40,0x36,0x01,0x10,0x10,0x40,0x11,0x00,0x09,0x08,0x02,0x01,0x05,0x03,0x0e,0x0d,
/*003cc0*/ 0x06,0x03,0x05,0x04,0x0f,0x00,0x0c,0x08,0x07,0x04,0x03,0x05,0x0b,0x04,0x0a,0x09,
/*003cd0*/ 0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x0d,0x0c,0x06,0x07,0x06,0x0f,0x0e,0x0b,0x03,
/*003ce0*/ 0x0a,0x02,0x03,0x02,0x00,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,
/*003cf0*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,
/*003d00*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,
/*003d10*/ 0xb9,0x00,0x09,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003d20*/ 0x00,0x10,0xff,0xc0,0x38,0x59,0x05,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x23,
/*003d30*/ 0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0xc5,0xc8,0xc8,0x64,0x62,0xca,0x62,
/*003d40*/ 0x63,0x64,0x62,0x67,0x64,0x01,0x29,0xfe,0xd4,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,
/*003d50*/ 0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x7c,0x40,0x38,0x01,0x10,0x10,0x40,
/*003d60*/ 0x11,0x00,0x0f,0x0a,0x09,0x08,0x07,0x02,0x01,0x00,0x04,0x03,0x04,0x0d,0x0e,0x0d,
/*003d70*/ 0x06,0x03,0x05,0x04,0x0c,0x0b,0x0d,0x05,0x04,0x03,0x0c,0x07,0x08,0x0f,0x0e,0x03,
/*003d80*/ 0x03,0x02,0x06,0x00,0x0b,0x0a,0x07,0x03,0x06,0x06,0x08,0x09,0x08,0x02,0x01,0x00,
/*003d90*/ 0x01,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,
/*003da0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*003db0*/ 0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,
/*003dc0*/ 0xb9,0x00,0x01,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003dd0*/ 0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x21,
/*003de0*/ 0x15,0x23,0x15,0x23,0x15,0x33,0x01,0x8f,0xfe,0x72,0x65,0x63,0xc8,0x01,0x8e,0x65,
/*003df0*/ 0x63,0xc8,0x01,0x62,0x64,0x66,0x62,0x62,0x64,0x67,0x00,0x01,0x00,0x01,0xff,0x9d,
/*003e00*/ 0x01,0x2b,0x02,0x57,0x00,0x13,0x00,0x89,0x40,0x42,0x01,0x14,0x14,0x40,0x15,0x00,
/*003e10*/ 0x10,0x0f,0x13,0x08,0x07,0x03,0x00,0x04,0x09,0x12,0x11,0x0a,0x09,0x06,0x02,0x01,
/*003e20*/ 0x07,0x05,0x04,0x0b,0x0c,0x0b,0x04,0x03,0x03,0x04,0x0e,0x0d,0x05,0x04,0x07,0x0b,
/*003e30*/ 0x0a,0x11,0x10,0x01,0x03,0x00,0x06,0x12,0x07,0x06,0x06,0x08,0x0d,0x0c,0x06,0x0f,
/*003e40*/ 0x03,0x02,0x03,0x0e,0x09,0x08,0x13,0x12,0x03,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,
/*003e50*/ 0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,
/*003e60*/ 0x3c,0x2f,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*003e70*/ 0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x14,
/*003e80*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,
/*003e90*/ 0x59,0x01,0x23,0x15,0x23,0x15,0x33,0x15,0x33,0x15,0x23,0x35,0x23,0x35,0x23,0x35,
/*003ea0*/ 0x33,0x35,0x33,0x35,0x33,0x01,0x2b,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0x65,0x63,
/*003eb0*/ 0x62,0x01,0xf5,0xc8,0x67,0xc8,0x61,0x65,0xc8,0x61,0xc8,0x64,0x00,0x01,0x00,0x01,
/*003ec0*/ 0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x40,0x40,0x14,0x01,0x04,0x04,0x40,
/*003ed0*/ 0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x03,0x02,0x03,0x01,0x00,0x01,0x01,0x01,0x46,
/*003ee0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x00,0x31,
/*003ef0*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003f00*/ 0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,0x63,0x62,
/*003f10*/ 0x62,0x01,0x02,0x56,0x00,0x01,0x00,0x01,0xff,0x9d,0x01,0x2b,0x02,0x57,0x00,0x13,
/*003f20*/ 0x00,0x8a,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x04,0x03,0x03,0x04,
/*003f30*/ 0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,0x07,0x04,0x02,0x01,0x12,0x11,0x0a,
/*003f40*/ 0x03,0x09,0x04,0x13,0x00,0x0b,0x0a,0x07,0x11,0x10,0x13,0x12,0x06,0x09,0x08,0x01,
/*003f50*/ 0x03,0x00,0x07,0x06,0x06,0x04,0x0d,0x0c,0x06,0x0e,0x05,0x04,0x0f,0x0e,0x03,0x03,
/*003f60*/ 0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,
/*003f70*/ 0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x01,
/*003f80*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*003f90*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,
/*003fa0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,
/*003fb0*/ 0x15,0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,0x15,0x33,0x15,0x33,
/*003fc0*/ 0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0xc9,0xc8,0x64,0x62,
/*003fd0*/ 0xc8,0x67,0xc7,0x62,0x65,0xc8,0x00,0x01,0x00,0x01,0x01,0x91,0x01,0x8f,0x02,0x57,
/*003fe0*/ 0x00,0x0f,0x00,0x73,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,0x02,0x01,0x04,0x03,
/*003ff0*/ 0x06,0x05,0x04,0x08,0x07,0x0c,0x04,0x03,0x03,0x0b,0x04,0x0a,0x09,0x0e,0x0d,0x04,
/*004000*/ 0x0f,0x00,0x0d,0x0c,0x09,0x05,0x04,0x01,0x00,0x07,0x08,0x06,0x02,0x07,0x06,0x03,
/*004010*/ 0x03,0x02,0x0f,0x0e,0x0b,0x03,0x0a,0x03,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,
/*004020*/ 0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,
/*004030*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*004040*/ 0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*004050*/ 0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x35,0x23,0x15,0x23,0x35,
/*004060*/ 0x33,0x35,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0x65,0x61,0x67,0x61,0x64,0x62,0x67,
/*004070*/ 0x61,0x01,0xf5,0x64,0x64,0x64,0x62,0x64,0x64,0x64,0x00,0x01,0x00,0x01,0x00,0x01,
/*004080*/ 0x01,0xf3,0x02,0x57,0x00,0x17,0x00,0x95,0x40,0x4a,0x01,0x18,0x18,0x40,0x19,0x00,
/*004090*/ 0x17,0x0e,0x0d,0x0a,0x09,0x04,0x03,0x00,0x06,0x02,0x01,0x03,0x05,0x04,0x07,0x12,
/*0040a0*/ 0x0c,0x0b,0x08,0x07,0x05,0x11,0x04,0x10,0x0f,0x14,0x13,0x04,0x16,0x15,0x0d,0x0c,
/*0040b0*/ 0x01,0x03,0x00,0x06,0x16,0x0b,0x0a,0x03,0x03,0x02,0x06,0x09,0x08,0x05,0x03,0x04,
/*0040c0*/ 0x15,0x14,0x11,0x03,0x10,0x03,0x17,0x16,0x13,0x12,0x0f,0x05,0x0e,0x02,0x07,0x06,
/*0040d0*/ 0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x3f,0x17,
/*0040e0*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,
/*0040f0*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
/*004100*/ 0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x18,0x49,0x68,0x61,0xb0,
/*004110*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,
/*004120*/ 0x33,0x15,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,
/*004130*/ 0x33,0x35,0x33,0x15,0x33,0x01,0xf3,0xc8,0xc8,0xc9,0x61,0xc8,0xc8,0xc8,0x64,0x62,
/*004140*/ 0x67,0x61,0x64,0x01,0x2d,0x67,0x61,0x64,0x65,0x61,0x67,0x61,0xc8,0xc8,0xc8,0xc9,
/*004150*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0x7a,
/*004160*/ 0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x64,0x00,0x00,0x02,0x6a,
/*004170*/ 0x00,0x00,0x03,0x2a,0x00,0x00,0x04,0x78,0x00,0x00,0x05,0xac,0x00,0x00,0x06,0x04,
/*004180*/ 0x00,0x00,0x06,0x96,0x00,0x00,0x07,0x26,0x00,0x00,0x07,0xe2,0x00,0x00,0x08,0x70,
/*004190*/ 0x00,0x00,0x08,0xe2,0x00,0x00,0x09,0x3a,0x00,0x00,0x09,0x90,0x00,0x00,0x0a,0x22,
/*0041a0*/ 0x00,0x00,0x0b,0x00,0x00,0x00,0x0b,0x76,0x00,0x00,0x0c,0x62,0x00,0x00,0x0d,0x4e,
/*0041b0*/ 0x00,0x00,0x0d,0xe8,0x00,0x00,0x0e,0x9a,0x00,0x00,0x0f,0x6c,0x00,0x00,0x10,0x1c,
/*0041c0*/ 0x00,0x00,0x11,0x00,0x00,0x00,0x11,0xd0,0x00,0x00,0x12,0x48,0x00,0x00,0x12,0xc0,
/*0041d0*/ 0x00,0x00,0x13,0x84,0x00,0x00,0x14,0x00,0x00,0x00,0x14,0xc6,0x00,0x00,0x15,0x8a,
/*0041e0*/ 0x00,0x00,0x16,0x5e,0x00,0x00,0x16,0xfc,0x00,0x00,0x17,0xc0,0x00,0x00,0x18,0x54,
/*0041f0*/ 0x00,0x00,0x18,0xea,0x00,0x00,0x19,0x8a,0x00,0x00,0x1a,0x1a,0x00,0x00,0x1a,0xcc,
/*004200*/ 0x00,0x00,0x1b,0x5c,0x00,0x00,0x1b,0xf4,0x00,0x00,0x1c,0x7c,0x00,0x00,0x1d,0x26,
/*004210*/ 0x00,0x00,0x1d,0x9e,0x00,0x00,0x1e,0x64,0x00,0x00,0x1f,0x10,0x00,0x00,0x1f,0xb6,
/*004220*/ 0x00,0x00,0x20,0x54,0x00,0x00,0x21,0x1c,0x00,0x00,0x21,0xd2,0x00,0x00,0x22,0xba,
/*004230*/ 0x00,0x00,0x23,0x32,0x00,0x00,0x23,0xc4,0x00,0x00,0x24,0x72,0x00,0x00,0x25,0x36,
/*004240*/ 0x00,0x00,0x25,0xf6,0x00,0x00,0x26,0xa0,0x00,0x00,0x27,0x6c,0x00,0x00,0x27,0xe6,
/*004250*/ 0x00,0x00,0x28,0x7c,0x00,0x00,0x28,0xf6,0x00,0x00,0x29,0x88,0x00,0x00,0x29,0xe0,
/*004260*/ 0x00,0x00,0x2a,0x74,0x00,0x00,0x2b,0x14,0x00,0x00,0x2b,0xa8,0x00,0x00,0x2c,0x48,
/*004270*/ 0x00,0x00,0x2d,0x0a,0x00,0x00,0x2d,0x9e,0x00,0x00,0x2e,0x52,0x00,0x00,0x2e,0xe2,
/*004280*/ 0x00,0x00,0x2f,0x5a,0x00,0x00,0x2f,0xee,0x00,0x00,0x30,0x9a,0x00,0x00,0x30,0xf2,
/*004290*/ 0x00,0x00,0x31,0x90,0x00,0x00,0x32,0x14,0x00,0x00,0x32,0xb6,0x00,0x00,0x33,0x56,
/*0042a0*/ 0x00,0x00,0x33,0xf6,0x00,0x00,0x34,0x86,0x00,0x00,0x35,0x3e,0x00,0x00,0x35,0xcc,
/*0042b0*/ 0x00,0x00,0x36,0x50,0x00,0x00,0x36,0xde,0x00,0x00,0x37,0x7c,0x00,0x00,0x38,0x3e,
/*0042c0*/ 0x00,0x00,0x38,0xe8,0x00,0x00,0x39,0x96,0x00,0x00,0x3a,0x58,0x00,0x00,0x3a,0xb0,
/*0042d0*/ 0x00,0x00,0x3b,0x72,0x00,0x00,0x3c,0x16,0x00,0x00,0x3c,0xec,0x00,0x00,0x3c,0xec,
/*0042e0*/ 0x01,0x90,0x00,0x32,0x00,0x00,0x00,0x00,0x01,0x2c,0x00,0x00,0x01,0x2c,0x00,0x00,
/*0042f0*/ 0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,0x02,0x58,0x00,0x01,0x02,0x58,0x00,0x01,
/*004300*/ 0x03,0x20,0x00,0x01,0x02,0x58,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x2c,0x00,0x01,
/*004310*/ 0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0x2c,0x00,0x01,
/*004320*/ 0x01,0x90,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004330*/ 0x01,0xf4,0x00,0x65,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004340*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004350*/ 0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,
/*004360*/ 0x01,0x90,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,
/*004370*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004380*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004390*/ 0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043a0*/ 0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043b0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,
/*0043c0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043d0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,
/*0043e0*/ 0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043f0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004400*/ 0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,
/*004410*/ 0x01,0x2c,0x00,0x01,0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,0x02,0x58,0x00,0x01,
/*004420*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004430*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004440*/ 0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004450*/ 0x01,0xf4,0x00,0x01,0x01,0x90,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,
/*004460*/ 0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0x90,0x00,0x00,0x00,0x02,0x00,0x00,
/*004470*/ 0x00,0x00,0x00,0x00,0xff,0x7b,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004480*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x00,0x00,
/*004490*/ 0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,
/*0044a0*/ 0x00,0x09,0x00,0x0a,0x00,0x0b,0x00,0x0c,0x00,0x0d,0x00,0x0e,0x00,0x0f,0x00,0x10,
/*0044b0*/ 0x00,0x11,0x00,0x12,0x00,0x13,0x00,0x14,0x00,0x15,0x00,0x16,0x00,0x17,0x00,0x18,
/*0044c0*/ 0x00,0x19,0x00,0x1a,0x00,0x1b,0x00,0x1c,0x00,0x1d,0x00,0x1e,0x00,0x1f,0x00,0x20,
/*0044d0*/ 0x00,0x21,0x00,0x22,0x00,0x23,0x00,0x24,0x00,0x25,0x00,0x26,0x00,0x27,0x00,0x28,
/*0044e0*/ 0x00,0x29,0x00,0x2a,0x00,0x2b,0x00,0x2c,0x00,0x2d,0x00,0x2e,0x00,0x2f,0x00,0x30,
/*0044f0*/ 0x00,0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,0x37,0x00,0x38,
/*004500*/ 0x00,0x39,0x00,0x3a,0x00,0x3b,0x00,0x3c,0x00,0x3d,0x00,0x3e,0x00,0x3f,0x00,0x40,
/*004510*/ 0x00,0x41,0x00,0x42,0x00,0x44,0x00,0x45,0x00,0x46,0x00,0x47,0x00,0x48,0x00,0x49,
/*004520*/ 0x00,0x4a,0x00,0x4b,0x00,0x4c,0x00,0x4d,0x00,0x4e,0x00,0x4f,0x00,0x50,0x00,0x51,
/*004530*/ 0x00,0x52,0x00,0x53,0x00,0x54,0x00,0x55,0x00,0x56,0x00,0x57,0x00,0x58,0x00,0x59,
/*004540*/ 0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,0x5e,0x00,0x5f,0x00,0x60,0x00,0x61,
/*004550*/ 0x00,0x96,0x00,0xac,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x24,
/*004560*/ 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x03,0x00,0x01,0x00,0x00,0x01,0x24,
/*004570*/ 0x00,0x00,0x01,0x06,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,
/*004580*/ 0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004590*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
/*0045a0*/ 0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,
/*0045b0*/ 0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,
/*0045c0*/ 0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,
/*0045d0*/ 0x3d,0x3e,0x3f,0x40,0x41,0x42,0x00,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,
/*0045e0*/ 0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,
/*0045f0*/ 0x5c,0x5d,0x5e,0x5f,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004600*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004610*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00,0x00,0x61,0x00,0x00,0x00,0x00,
/*004620*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004630*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004640*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
/*004650*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004660*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004670*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x01,0x04,0x00,0x00,0x00,0x0c,
/*004680*/ 0x00,0x08,0x00,0x02,0x00,0x04,0x00,0x5f,0x00,0x7e,0x00,0xa0,0x00,0xa5,0x20,0x10,
/*004690*/ 0xff,0xff,0x00,0x00,0x00,0x20,0x00,0x61,0x00,0xa0,0x00,0xa5,0x20,0x10,0xff,0xff,
/*0046a0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0c,0x00,0x8a,
/*0046b0*/ 0x00,0xc4,0x00,0xc4,0x00,0xc4,0xff,0xff,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,
/*0046c0*/ 0x00,0x07,0x00,0x08,0x00,0x09,0x00,0x0a,0x00,0x0b,0x00,0x0c,0x00,0x0d,0x00,0x0e,
/*0046d0*/ 0x00,0x0f,0x00,0x10,0x00,0x11,0x00,0x12,0x00,0x13,0x00,0x14,0x00,0x15,0x00,0x16,
/*0046e0*/ 0x00,0x17,0x00,0x18,0x00,0x19,0x00,0x1a,0x00,0x1b,0x00,0x1c,0x00,0x1d,0x00,0x1e,
/*0046f0*/ 0x00,0x1f,0x00,0x20,0x00,0x21,0x00,0x22,0x00,0x23,0x00,0x24,0x00,0x25,0x00,0x26,
/*004700*/ 0x00,0x27,0x00,0x28,0x00,0x29,0x00,0x2a,0x00,0x2b,0x00,0x2c,0x00,0x2d,0x00,0x2e,
/*004710*/ 0x00,0x2f,0x00,0x30,0x00,0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,
/*004720*/ 0x00,0x37,0x00,0x38,0x00,0x39,0x00,0x3a,0x00,0x3b,0x00,0x3c,0x00,0x3d,0x00,0x3e,
/*004730*/ 0x00,0x3f,0x00,0x40,0x00,0x41,0x00,0x42,0x00,0x43,0x00,0x44,0x00,0x45,0x00,0x46,
/*004740*/ 0x00,0x47,0x00,0x48,0x00,0x49,0x00,0x4a,0x00,0x4b,0x00,0x4c,0x00,0x4d,0x00,0x4e,
/*004750*/ 0x00,0x4f,0x00,0x50,0x00,0x51,0x00,0x52,0x00,0x53,0x00,0x54,0x00,0x55,0x00,0x56,
/*004760*/ 0x00,0x57,0x00,0x58,0x00,0x59,0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,0x5e,
/*004770*/ 0x00,0x5f,0x00,0x60,0x00,0x62,0x00,0x61,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x10,
/*004780*/ 0x00,0x00,0x00,0x68,0x09,0x09,0x05,0x00,0x03,0x03,0x02,0x05,0x07,0x07,0x09,0x07,
/*004790*/ 0x02,0x03,0x03,0x05,0x05,0x03,0x05,0x02,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
/*0047a0*/ 0x06,0x06,0x06,0x02,0x02,0x05,0x05,0x05,0x06,0x07,0x06,0x06,0x06,0x06,0x06,0x06,
/*0047b0*/ 0x06,0x06,0x05,0x06,0x06,0x06,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x06,0x06,
/*0047c0*/ 0x07,0x06,0x06,0x06,0x03,0x05,0x03,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,
/*0047d0*/ 0x06,0x02,0x03,0x06,0x02,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x07,
/*0047e0*/ 0x06,0x06,0x06,0x05,0x02,0x05,0x06,0x07,0x05,0x00,0x00,0x00,0x0a,0x0a,0x05,0x00,
/*0047f0*/ 0x04,0x04,0x03,0x05,0x08,0x08,0x0a,0x08,0x03,0x04,0x04,0x05,0x05,0x04,0x05,0x03,
/*004800*/ 0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x03,0x03,0x05,0x05,0x05,
/*004810*/ 0x06,0x08,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x06,0x08,0x06,
/*004820*/ 0x06,0x06,0x06,0x06,0x06,0x08,0x06,0x06,0x08,0x06,0x06,0x06,0x04,0x05,0x04,0x05,
/*004830*/ 0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x03,0x04,0x06,0x03,0x08,0x06,0x06,
/*004840*/ 0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x08,0x06,0x06,0x06,0x05,0x03,0x05,0x06,0x08,
/*004850*/ 0x05,0x00,0x00,0x00,0x0b,0x0b,0x06,0x00,0x04,0x04,0x03,0x06,0x08,0x08,0x0b,0x08,
/*004860*/ 0x03,0x04,0x04,0x06,0x06,0x04,0x06,0x03,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x07,
/*004870*/ 0x07,0x07,0x07,0x03,0x03,0x06,0x06,0x06,0x07,0x08,0x07,0x07,0x07,0x07,0x07,0x07,
/*004880*/ 0x07,0x07,0x06,0x07,0x07,0x07,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x08,0x07,0x07,
/*004890*/ 0x08,0x07,0x07,0x07,0x04,0x06,0x04,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x07,
/*0048a0*/ 0x07,0x03,0x04,0x07,0x03,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x07,0x07,0x08,
/*0048b0*/ 0x07,0x07,0x07,0x06,0x03,0x06,0x07,0x08,0x06,0x00,0x00,0x00,0x0c,0x0c,0x06,0x00,
/*0048c0*/ 0x05,0x05,0x03,0x06,0x09,0x09,0x0c,0x09,0x03,0x05,0x05,0x06,0x06,0x05,0x06,0x03,
/*0048d0*/ 0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x03,0x03,0x06,0x06,0x06,
/*0048e0*/ 0x08,0x09,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x08,0x09,0x08,
/*0048f0*/ 0x08,0x08,0x08,0x08,0x08,0x09,0x08,0x08,0x09,0x08,0x08,0x08,0x05,0x06,0x05,0x06,
/*004900*/ 0x08,0x08,0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x03,0x05,0x08,0x03,0x09,0x08,0x08,
/*004910*/ 0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x09,0x08,0x08,0x08,0x06,0x03,0x06,0x08,0x09,
/*004920*/ 0x06,0x00,0x00,0x00,0x0d,0x0d,0x07,0x00,0x05,0x05,0x03,0x07,0x0a,0x0a,0x0d,0x0a,
/*004930*/ 0x03,0x05,0x05,0x07,0x07,0x05,0x07,0x03,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
/*004940*/ 0x08,0x08,0x08,0x03,0x03,0x07,0x07,0x07,0x08,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,
/*004950*/ 0x08,0x08,0x07,0x08,0x08,0x08,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x0a,0x08,0x08,
/*004960*/ 0x0a,0x08,0x08,0x08,0x05,0x07,0x05,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x07,0x08,
/*004970*/ 0x08,0x03,0x05,0x08,0x03,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x07,0x08,0x08,0x0a,
/*004980*/ 0x08,0x08,0x08,0x07,0x03,0x07,0x08,0x0a,0x07,0x00,0x00,0x00,0x0e,0x0e,0x07,0x00,
/*004990*/ 0x05,0x05,0x04,0x07,0x0b,0x0b,0x0e,0x0b,0x04,0x05,0x05,0x07,0x07,0x05,0x07,0x04,
/*0049a0*/ 0x07,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x04,0x04,0x07,0x07,0x07,
/*0049b0*/ 0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x09,0x0b,0x09,
/*0049c0*/ 0x09,0x09,0x09,0x09,0x09,0x0b,0x09,0x09,0x0b,0x09,0x09,0x09,0x05,0x07,0x05,0x07,
/*0049d0*/ 0x09,0x09,0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x04,0x05,0x09,0x04,0x0b,0x09,0x09,
/*0049e0*/ 0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x0b,0x09,0x09,0x09,0x07,0x04,0x07,0x09,0x0b,
/*0049f0*/ 0x07,0x00,0x00,0x00,0x0f,0x0f,0x08,0x00,0x06,0x06,0x04,0x08,0x0b,0x0b,0x0f,0x0b,
/*004a00*/ 0x04,0x06,0x06,0x08,0x08,0x06,0x08,0x04,0x08,0x09,0x09,0x09,0x09,0x09,0x09,0x09,
/*004a10*/ 0x09,0x09,0x09,0x04,0x04,0x08,0x08,0x08,0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,
/*004a20*/ 0x09,0x09,0x08,0x09,0x09,0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x0b,0x09,0x09,
/*004a30*/ 0x0b,0x09,0x09,0x09,0x06,0x08,0x06,0x08,0x09,0x09,0x09,0x09,0x09,0x09,0x08,0x09,
/*004a40*/ 0x09,0x04,0x06,0x09,0x04,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x08,0x09,0x09,0x0b,
/*004a50*/ 0x09,0x09,0x09,0x08,0x04,0x08,0x09,0x0b,0x08,0x00,0x00,0x00,0x10,0x10,0x08,0x00,
/*004a60*/ 0x06,0x06,0x04,0x08,0x0c,0x0c,0x10,0x0c,0x04,0x06,0x06,0x08,0x08,0x06,0x08,0x04,
/*004a70*/ 0x08,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x04,0x04,0x08,0x08,0x08,
/*004a80*/ 0x0a,0x0c,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x0a,0x0c,0x0a,
/*004a90*/ 0x0a,0x0a,0x0a,0x0a,0x0a,0x0c,0x0a,0x0a,0x0c,0x0a,0x0a,0x0a,0x06,0x08,0x06,0x08,
/*004aa0*/ 0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x04,0x06,0x0a,0x04,0x0c,0x0a,0x0a,
/*004ab0*/ 0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x0c,0x0a,0x0a,0x0a,0x08,0x04,0x08,0x0a,0x0c,
/*004ac0*/ 0x08,0x00,0x00,0x00,0x11,0x11,0x09,0x00,0x06,0x06,0x04,0x09,0x0d,0x0d,0x11,0x0d,
/*004ad0*/ 0x04,0x06,0x06,0x09,0x09,0x06,0x09,0x04,0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
/*004ae0*/ 0x0b,0x0b,0x0b,0x04,0x04,0x09,0x09,0x09,0x0b,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
/*004af0*/ 0x0b,0x0b,0x09,0x0b,0x0b,0x0b,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0d,0x0b,0x0b,
/*004b00*/ 0x0d,0x0b,0x0b,0x0b,0x06,0x09,0x06,0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,
/*004b10*/ 0x0b,0x04,0x06,0x0b,0x04,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0d,
/*004b20*/ 0x0b,0x0b,0x0b,0x09,0x04,0x09,0x0b,0x0d,0x09,0x00,0x00,0x00,0x12,0x12,0x09,0x00,
/*004b30*/ 0x07,0x07,0x05,0x09,0x0e,0x0e,0x12,0x0e,0x05,0x07,0x07,0x09,0x09,0x07,0x09,0x05,
/*004b40*/ 0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x05,0x05,0x09,0x09,0x09,
/*004b50*/ 0x0b,0x0e,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0b,0x0e,0x0b,
/*004b60*/ 0x0b,0x0b,0x0b,0x0b,0x0b,0x0e,0x0b,0x0b,0x0e,0x0b,0x0b,0x0b,0x07,0x09,0x07,0x09,
/*004b70*/ 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x05,0x07,0x0b,0x05,0x0e,0x0b,0x0b,
/*004b80*/ 0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0e,0x0b,0x0b,0x0b,0x09,0x05,0x09,0x0b,0x0e,
/*004b90*/ 0x09,0x00,0x00,0x00,0x13,0x13,0x0a,0x00,0x07,0x07,0x05,0x0a,0x0e,0x0e,0x13,0x0e,
/*004ba0*/ 0x05,0x07,0x07,0x0a,0x0a,0x07,0x0a,0x05,0x0a,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,
/*004bb0*/ 0x0c,0x0c,0x0c,0x05,0x05,0x0a,0x0a,0x0a,0x0c,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,
/*004bc0*/ 0x0c,0x0c,0x0a,0x0c,0x0c,0x0c,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0e,0x0c,0x0c,
/*004bd0*/ 0x0e,0x0c,0x0c,0x0c,0x07,0x0a,0x07,0x0a,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0a,0x0c,
/*004be0*/ 0x0c,0x05,0x07,0x0c,0x05,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0a,0x0c,0x0c,0x0e,
/*004bf0*/ 0x0c,0x0c,0x0c,0x0a,0x05,0x0a,0x0c,0x0e,0x0a,0x00,0x00,0x00,0x14,0x14,0x0a,0x00,
/*004c00*/ 0x08,0x08,0x05,0x0a,0x0f,0x0f,0x14,0x0f,0x05,0x08,0x08,0x0a,0x0a,0x08,0x0a,0x05,
/*004c10*/ 0x0a,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x05,0x05,0x0a,0x0a,0x0a,
/*004c20*/ 0x0d,0x0f,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x0d,0x0f,0x0d,
/*004c30*/ 0x0d,0x0d,0x0d,0x0d,0x0d,0x0f,0x0d,0x0d,0x0f,0x0d,0x0d,0x0d,0x08,0x0a,0x08,0x0a,
/*004c40*/ 0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x05,0x08,0x0d,0x05,0x0f,0x0d,0x0d,
/*004c50*/ 0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x0f,0x0d,0x0d,0x0d,0x0a,0x05,0x0a,0x0d,0x0f,
/*004c60*/ 0x0a,0x00,0x00,0x00,0x15,0x15,0x0b,0x00,0x08,0x08,0x05,0x0b,0x10,0x10,0x15,0x10,
/*004c70*/ 0x05,0x08,0x08,0x0b,0x0b,0x08,0x0b,0x05,0x0b,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,
/*004c80*/ 0x0d,0x0d,0x0d,0x05,0x05,0x0b,0x0b,0x0b,0x0d,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,
/*004c90*/ 0x0d,0x0d,0x0b,0x0d,0x0d,0x0d,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x10,0x0d,0x0d,
/*004ca0*/ 0x10,0x0d,0x0d,0x0d,0x08,0x0b,0x08,0x0b,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0b,0x0d,
/*004cb0*/ 0x0d,0x05,0x08,0x0d,0x05,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0b,0x0d,0x0d,0x10,
/*004cc0*/ 0x0d,0x0d,0x0d,0x0b,0x05,0x0b,0x0d,0x10,0x0b,0x00,0x00,0x00,0x16,0x16,0x0b,0x00,
/*004cd0*/ 0x08,0x08,0x06,0x0b,0x11,0x11,0x16,0x11,0x06,0x08,0x08,0x0b,0x0b,0x08,0x0b,0x06,
/*004ce0*/ 0x0b,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x06,0x06,0x0b,0x0b,0x0b,
/*004cf0*/ 0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x0e,0x11,0x0e,
/*004d00*/ 0x0e,0x0e,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x08,0x0b,0x08,0x0b,
/*004d10*/ 0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x06,0x08,0x0e,0x06,0x11,0x0e,0x0e,
/*004d20*/ 0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x0b,0x06,0x0b,0x0e,0x11,
/*004d30*/ 0x0b,0x00,0x00,0x00,0x17,0x17,0x0c,0x00,0x09,0x09,0x06,0x0c,0x11,0x11,0x17,0x11,
/*004d40*/ 0x06,0x09,0x09,0x0c,0x0c,0x09,0x0c,0x06,0x0c,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,
/*004d50*/ 0x0e,0x0e,0x0e,0x06,0x06,0x0c,0x0c,0x0c,0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,
/*004d60*/ 0x0e,0x0e,0x0c,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,
/*004d70*/ 0x11,0x0e,0x0e,0x0e,0x09,0x0c,0x09,0x0c,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0c,0x0e,
/*004d80*/ 0x0e,0x06,0x09,0x0e,0x06,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0c,0x0e,0x0e,0x11,
/*004d90*/ 0x0e,0x0e,0x0e,0x0c,0x06,0x0c,0x0e,0x11,0x0c,0x00,0x00,0x00,0x18,0x18,0x0c,0x00,
/*004da0*/ 0x09,0x09,0x06,0x0c,0x12,0x12,0x18,0x12,0x06,0x09,0x09,0x0c,0x0c,0x09,0x0c,0x06,
/*004db0*/ 0x0c,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x06,0x06,0x0c,0x0c,0x0c,
/*004dc0*/ 0x0f,0x12,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,0x12,0x0f,
/*004dd0*/ 0x0f,0x0f,0x0f,0x0f,0x0f,0x12,0x0f,0x0f,0x12,0x0f,0x0f,0x0f,0x09,0x0c,0x09,0x0c,
/*004de0*/ 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x06,0x09,0x0f,0x06,0x12,0x0f,0x0f,
/*004df0*/ 0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x12,0x0f,0x0f,0x0f,0x0c,0x06,0x0c,0x0f,0x12,
/*004e00*/ 0x0c,0x00,0x00,0x00,0x00,0x00,0x01,0xaf,0x01,0x90,0x00,0x05,0x00,0x01,0x02,0x30,
/*004e10*/ 0x02,0x08,0x00,0x00,0x00,0x72,0x02,0x30,0x02,0x08,0x00,0x00,0x01,0x6b,0x00,0x28,
/*004e20*/ 0x00,0xcf,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004e30*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x6c,
/*004e40*/ 0x74,0x73,0x00,0x40,0x00,0x20,0x20,0x10,0x02,0x58,0xff,0x38,0x00,0x00,0x02,0x58,
/*004e50*/ 0x00,0xc7,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0xe3,0xe5,0x12,0x31,
/*004e60*/ 0x5f,0x0f,0x3c,0xf5,0x00,0x00,0x03,0x20,0x00,0x00,0x00,0x00,0xb6,0x97,0xc2,0x82,
/*004e70*/ 0x00,0x00,0x00,0x00,0xb6,0x97,0xc2,0x82,0x00,0x00,0xff,0x39,0x02,0xbb,0x02,0x58,
/*004e80*/ 0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
/*004e90*/ 0x02,0x58,0xff,0x38,0x00,0x00,0x03,0x20,0x00,0x00,0x00,0x32,0x02,0xbb,0x00,0x01,
/*004ea0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,
/*004eb0*/ 0x00,0x01,0x00,0x00,0x00,0x63,0x00,0x2c,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x02,
/*004ec0*/ 0x00,0x08,0x00,0x40,0x00,0x0a,0x00,0x00,0x00,0x84,0x00,0xde,0x00,0x01,0x00,0x01
};

static const unsigned bm_mini_ttf_length = (unsigned)sizeof(bm_mini_ttf);

// -----------------------------------------------------------------------------

// The following data tables are coming from Dear Imgui.
// Re-licensed under permission as MIT-0.
//
// @todo: 0x3100, 0x312F, FONT_TW, // Bopomofo
//        0xE000, 0xEB4C, FONT_EM, // Private use (emojis)

static const unsigned table_common[] = {
    0x0020, 0x00FF, // Basic Latin + Latin Supplement
    0xFFFD, 0xFFFD, // Invalid
    0,
};

static const unsigned table_cyrillic[] = {
    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
    0x2DE0, 0x2DFF, // Cyrillic Extended-A
    0xA640, 0xA69F, // Cyrillic Extended-B
    0,
};

static const unsigned table_vietnamese[] = {
    0x0102, 0x0103,
    0x0110, 0x0111,
    0x0128, 0x0129,
    0x0168, 0x0169,
    0x01A0, 0x01A1,
    0x01AF, 0x01B0,
    0x1EA0, 0x1EF9,
    0,
};

static const unsigned table_thai[] = {
    0x0E00, 0x0E7F, // Thai
    0x2010, 0x205E, // General Punctuation (smaller set than full CJK 2000-206F)
    0,
};

static const unsigned table_korean[] = {
    0x1100, 0x11FF, // Hangul Jamo
    0x3131, 0x3163, // Korean alphabets (no 3130-318F?)
    0xAC00, 0xD7A3, // Korean characters (no AC00-D7AF?)
    0,
};

static const unsigned table_chinese_punctuation[] = {
    0x2000, 0x206F, // General CJK Punctuation (Thai uses 2010-205E)
    0,
};

static const unsigned table_chinese_japanese_common[] = {
    0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
    0x31F0, 0x31FF, // Katakana Phonetic Extensions
    0xFF00, 0xFFEF, // Half-width characters (Romaji)
    0,
};

static const unsigned table_chinese_full[] = {
    0x4e00, 0x9FAF, // CJK Ideograms
    0,
};

static const unsigned table_eastern_europe[] = {
    0x0100, 0x017F, // Latin_Extended-A
    0x0180, 0x024F, // Latin_Extended-B
    0x0250, 0x02AF, // International Phonetic Alphabet (IPA)
    0x02B0, 0x02FF, // Spacing modifier letters (and quotes)
    0x0300, 0x036F, // Fix IPA missing glyphs (?)
    0x16A0, 0x16F0, // Old Norse / Runic
    0x1E00, 0x1EFF, // Latin_Extended_Additional
    0x2000, 0x206F, // General punctuaction
//  0x2C60, 0x2C7F, // Latin Extended-C (Uighur New Script, the Uralic Phonetic Alphabet, Shona, and Claudian Latin)
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

static const unsigned table_western_europe[] = {
    0x0370, 0x03FF, // Greek and Coptic
    0x10A0, 0x10FF, // Modern Georgian, Svan, and Mingrelian
    0x1F00, 0x1FFF, // fix ancient greek glyphs (?)
    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
    0x2DE0, 0x2DFF, // Cyrillic Extended-A
    0xA640, 0xA69F, // Cyrillic Extended-B
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

static const unsigned table_middle_east[] = {
    0x0590, 0x05FF, // Hebrew, Yiddish, Ladino, and other Jewish diaspora languages.
    0x0600, 0x06FF, // Arabic script and Arabic-Indic digits
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

// Store 2500 regularly used characters for Simplified Chinese. Table below covers 97.97% of all characters used during the month in July, 1987.
// [ref] https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
static const unsigned short packed_table_chinese[] = { // starts with 0x4E00
    0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
    1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
    2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
    1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
    3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
    1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
    1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
    2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
    27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
    3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
    1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
    176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
    5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
    1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
    6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
    2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
    2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
    2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
    3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
    3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
    3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
    1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
    140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
    5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
    2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
    4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
    2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
    4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
    3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
    3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
    2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
    5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
    3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
    1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
    4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
    4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
    26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
    3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
    2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
    10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
};

// 2999 japanese ideograms = 2136 Joyo (for common use) + 863 Jinmeiyo (for personal name) Kanji code points.
// [ref] https://github.com/ocornut/imgui/pull/3627 - Missing 1 Joyo Kanji: U+20B9F (Kun'yomi: Shikaru, On'yomi: Shitsu,shichi).
static const unsigned short packed_table_japanese[] = { // starts with 0x4E00
    0,1,2,4,1,1,1,1,2,1,3,3,2,2,1,5,3,5,7,5,6,1,2,1,7,2,6,3,1,8,1,1,4,1,1,18,2,11,2,6,2,1,2,1,5,1,2,1,3,1,2,1,2,3,3,1,1,2,3,1,1,1,12,7,9,1,4,5,1,
    1,2,1,10,1,1,9,2,2,4,5,6,9,3,1,1,1,1,9,3,18,5,2,2,2,2,1,6,3,7,1,1,1,1,2,2,4,2,1,23,2,10,4,3,5,2,4,10,2,4,13,1,6,1,9,3,1,1,6,6,7,6,3,1,2,11,3,
    2,2,3,2,15,2,2,5,4,3,6,4,1,2,5,2,12,16,6,13,9,13,2,1,1,7,16,4,7,1,19,1,5,1,2,2,7,7,8,2,6,5,4,9,18,7,4,5,9,13,11,8,15,2,1,1,1,2,1,2,2,1,2,2,8,
    2,9,3,3,1,1,4,4,1,1,1,4,9,1,4,3,5,5,2,7,5,3,4,8,2,1,13,2,3,3,1,14,1,1,4,5,1,3,6,1,5,2,1,1,3,3,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,1,1,1,12,3,3,9,5,
    2,6,1,5,6,1,2,3,18,2,4,14,4,1,3,6,1,1,6,3,5,5,3,2,2,2,2,12,3,1,4,2,3,2,3,11,1,7,4,1,2,1,3,17,1,9,1,24,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,2,4,15,1,
    1,2,1,1,2,1,5,2,5,20,2,5,9,1,10,8,7,6,1,1,1,1,1,1,6,2,1,2,8,1,1,1,1,5,1,1,3,1,1,1,1,3,1,1,12,4,1,3,1,1,1,1,1,10,3,1,7,5,13,1,2,3,4,6,1,1,30,
    2,9,9,1,15,38,11,3,1,8,24,7,1,9,8,10,2,1,9,31,2,13,6,2,9,4,49,5,2,15,2,1,10,2,1,1,1,2,2,6,15,30,35,3,14,18,8,1,16,10,28,12,19,45,38,1,3,2,3,
    13,2,1,7,3,6,5,3,4,3,1,5,7,8,1,5,3,18,5,3,6,1,21,4,24,9,24,40,3,14,3,21,3,2,1,2,4,2,3,1,15,15,6,5,1,1,3,1,5,6,1,9,7,3,3,2,1,4,3,8,21,5,16,4,
    5,2,10,11,11,3,6,3,2,9,3,6,13,1,2,1,1,1,1,11,12,6,6,1,4,2,6,5,2,1,1,3,3,6,13,3,1,1,5,1,2,3,3,14,2,1,2,2,2,5,1,9,5,1,1,6,12,3,12,3,4,13,2,14,
    2,8,1,17,5,1,16,4,2,2,21,8,9,6,23,20,12,25,19,9,38,8,3,21,40,25,33,13,4,3,1,4,1,2,4,1,2,5,26,2,1,1,2,1,3,6,2,1,1,1,1,1,1,2,3,1,1,1,9,2,3,1,1,
    1,3,6,3,2,1,1,6,6,1,8,2,2,2,1,4,1,2,3,2,7,3,2,4,1,2,1,2,2,1,1,1,1,1,3,1,2,5,4,10,9,4,9,1,1,1,1,1,1,5,3,2,1,6,4,9,6,1,10,2,31,17,8,3,7,5,40,1,
    7,7,1,6,5,2,10,7,8,4,15,39,25,6,28,47,18,10,7,1,3,1,1,2,1,1,1,3,3,3,1,1,1,3,4,2,1,4,1,3,6,10,7,8,6,2,2,1,3,3,2,5,8,7,9,12,2,15,1,1,4,1,2,1,1,
    1,3,2,1,3,3,5,6,2,3,2,10,1,4,2,8,1,1,1,11,6,1,21,4,16,3,1,3,1,4,2,3,6,5,1,3,1,1,3,3,4,6,1,1,10,4,2,7,10,4,7,4,2,9,4,3,1,1,1,4,1,8,3,4,1,3,1,
    6,1,4,2,1,4,7,2,1,8,1,4,5,1,1,2,2,4,6,2,7,1,10,1,1,3,4,11,10,8,21,4,6,1,3,5,2,1,2,28,5,5,2,3,13,1,2,3,1,4,2,1,5,20,3,8,11,1,3,3,3,1,8,10,9,2,
    10,9,2,3,1,1,2,4,1,8,3,6,1,7,8,6,11,1,4,29,8,4,3,1,2,7,13,1,4,1,6,2,6,12,12,2,20,3,2,3,6,4,8,9,2,7,34,5,1,18,6,1,1,4,4,5,7,9,1,2,2,4,3,4,1,7,
    2,2,2,6,2,3,25,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,5,3,4,4,3,2,1,1,4,1,2,1,1,3,1,11,1,6,3,1,7,3,6,2,8,8,6,9,3,4,11,3,2,10,12,2,5,11,1,6,4,5,
    3,1,8,5,4,6,6,3,5,1,1,3,2,1,2,2,6,17,12,1,10,1,6,12,1,6,6,19,9,6,16,1,13,4,4,15,7,17,6,11,9,15,12,6,7,2,1,2,2,15,9,3,21,4,6,49,18,7,3,2,3,1,
    6,8,2,2,6,2,9,1,3,6,4,4,1,2,16,2,5,2,1,6,2,3,5,3,1,2,5,1,2,1,9,3,1,8,6,4,8,11,3,1,1,1,1,3,1,13,8,4,1,3,2,2,1,4,1,11,1,5,2,1,5,2,5,8,6,1,1,7,
    4,3,8,3,2,7,2,1,5,1,5,2,4,7,6,2,8,5,1,11,4,5,3,6,18,1,2,13,3,3,1,21,1,1,4,1,4,1,1,1,8,1,2,2,7,1,2,4,2,2,9,2,1,1,1,4,3,6,3,12,5,1,1,1,5,6,3,2,
    4,8,2,2,4,2,7,1,8,9,5,2,3,2,1,3,2,13,7,14,6,5,1,1,2,1,4,2,23,2,1,1,6,3,1,4,1,15,3,1,7,3,9,14,1,3,1,4,1,1,5,8,1,3,8,3,8,15,11,4,14,4,4,2,5,5,
    1,7,1,6,14,7,7,8,5,15,4,8,6,5,6,2,1,13,1,20,15,11,9,2,5,6,2,11,2,6,2,5,1,5,8,4,13,19,25,4,1,1,11,1,34,2,5,9,14,6,2,2,6,1,1,14,1,3,14,13,1,6,
    12,21,14,14,6,32,17,8,32,9,28,1,2,4,11,8,3,1,14,2,5,15,1,1,1,1,3,6,4,1,3,4,11,3,1,1,11,30,1,5,1,4,1,5,8,1,1,3,2,4,3,17,35,2,6,12,17,3,1,6,2,
    1,1,12,2,7,3,3,2,1,16,2,8,3,6,5,4,7,3,3,8,1,9,8,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,4,3,7,5,8,3,3,3,3,3,3,1,23,10,3,1,2,2,6,3,1,16,1,16,
    22,3,10,4,11,6,9,7,7,3,6,2,2,2,4,10,2,1,1,2,8,7,1,6,4,1,3,3,3,5,10,12,12,2,3,12,8,15,1,1,16,6,6,1,5,9,11,4,11,4,2,6,12,1,17,5,13,1,4,9,5,1,11,
    2,1,8,1,5,7,28,8,3,5,10,2,17,3,38,22,1,2,18,12,10,4,38,18,1,4,44,19,4,1,8,4,1,12,1,4,31,12,1,14,7,75,7,5,10,6,6,13,3,2,11,11,3,2,5,28,15,6,18,
    18,5,6,4,3,16,1,7,18,7,36,3,5,3,1,7,1,9,1,10,7,2,4,2,6,2,9,7,4,3,32,12,3,7,10,2,23,16,3,1,12,3,31,4,11,1,3,8,9,5,1,30,15,6,12,3,2,2,11,19,9,
    14,2,6,2,3,19,13,17,5,3,3,25,3,14,1,1,1,36,1,3,2,19,3,13,36,9,13,31,6,4,16,34,2,5,4,2,3,3,5,1,1,1,4,3,1,17,3,2,3,5,3,1,3,2,3,5,6,3,12,11,1,3,
    1,2,26,7,12,7,2,14,3,3,7,7,11,25,25,28,16,4,36,1,2,1,6,2,1,9,3,27,17,4,3,4,13,4,1,3,2,2,1,10,4,2,4,6,3,8,2,1,18,1,1,24,2,2,4,33,2,3,63,7,1,6,
    40,7,3,4,4,2,4,15,18,1,16,1,1,11,2,41,14,1,3,18,13,3,2,4,16,2,17,7,15,24,7,18,13,44,2,2,3,6,1,1,7,5,1,7,1,4,3,3,5,10,8,2,3,1,8,1,1,27,4,2,1,
    12,1,2,1,10,6,1,6,7,5,2,3,7,11,5,11,3,6,6,2,3,15,4,9,1,1,2,1,2,11,2,8,12,8,5,4,2,3,1,5,2,2,1,14,1,12,11,4,1,11,17,17,4,3,2,5,5,7,3,1,5,9,9,8,
    2,5,6,6,13,13,2,1,2,6,1,2,2,49,4,9,1,2,10,16,7,8,4,3,2,23,4,58,3,29,1,14,19,19,11,11,2,7,5,1,3,4,6,2,18,5,12,12,17,17,3,3,2,4,1,6,2,3,4,3,1,
    1,1,1,5,1,1,9,1,3,1,3,6,1,8,1,1,2,6,4,14,3,1,4,11,4,1,3,32,1,2,4,13,4,1,2,4,2,1,3,1,11,1,4,2,1,4,4,6,3,5,1,6,5,7,6,3,23,3,5,3,5,3,3,13,3,9,10,
    1,12,10,2,3,18,13,7,160,52,4,2,2,3,2,14,5,4,12,4,6,4,1,20,4,11,6,2,12,27,1,4,1,2,2,7,4,5,2,28,3,7,25,8,3,19,3,6,10,2,2,1,10,2,5,4,1,3,4,1,5,
    3,2,6,9,3,6,2,16,3,3,16,4,5,5,3,2,1,2,16,15,8,2,6,21,2,4,1,22,5,8,1,1,21,11,2,1,11,11,19,13,12,4,2,3,2,3,6,1,8,11,1,4,2,9,5,2,1,11,2,9,1,1,2,
    14,31,9,3,4,21,14,4,8,1,7,2,2,2,5,1,4,20,3,3,4,10,1,11,9,8,2,1,4,5,14,12,14,2,17,9,6,31,4,14,1,20,13,26,5,2,7,3,6,13,2,4,2,19,6,2,2,18,9,3,5,
    12,12,14,4,6,2,3,6,9,5,22,4,5,25,6,4,8,5,2,6,27,2,35,2,16,3,7,8,8,6,6,5,9,17,2,20,6,19,2,13,3,1,1,1,4,17,12,2,14,7,1,4,18,12,38,33,2,10,1,1,
    2,13,14,17,11,50,6,33,20,26,74,16,23,45,50,13,38,33,6,6,7,4,4,2,1,3,2,5,8,7,8,9,3,11,21,9,13,1,3,10,6,7,1,2,2,18,5,5,1,9,9,2,68,9,19,13,2,5,
    1,4,4,7,4,13,3,9,10,21,17,3,26,2,1,5,2,4,5,4,1,7,4,7,3,4,2,1,6,1,1,20,4,1,9,2,2,1,3,3,2,3,2,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,3,2,10,3,5,3,4,4,
    3,4,16,1,6,1,10,2,4,2,1,1,2,10,11,2,2,3,1,24,31,4,10,10,2,5,12,16,164,15,4,16,7,9,15,19,17,1,2,1,1,5,1,1,1,1,1,3,1,4,3,1,3,1,3,1,2,1,1,3,3,7,
    2,8,1,2,2,2,1,3,4,3,7,8,12,92,2,10,3,1,3,14,5,25,16,42,4,7,7,4,2,21,5,27,26,27,21,25,30,31,2,1,5,13,3,22,5,6,6,11,9,12,1,5,9,7,5,5,22,60,3,5,
    13,1,1,8,1,1,3,3,2,1,9,3,3,18,4,1,2,3,7,6,3,1,2,3,9,1,3,1,3,2,1,3,1,1,1,2,1,11,3,1,6,9,1,3,2,3,1,2,1,5,1,1,4,3,4,1,2,2,4,4,1,7,2,1,2,2,3,5,13,
    18,3,4,14,9,9,4,16,3,7,5,8,2,6,48,28,3,1,1,4,2,14,8,2,9,2,1,15,2,4,3,2,10,16,12,8,7,1,1,3,1,1,1,2,7,4,1,6,4,38,39,16,23,7,15,15,3,2,12,7,21,
    37,27,6,5,4,8,2,10,8,8,6,5,1,2,1,3,24,1,16,17,9,23,10,17,6,1,51,55,44,13,294,9,3,6,2,4,2,2,15,1,1,1,13,21,17,68,14,8,9,4,1,4,9,3,11,7,1,1,1,
    5,6,3,2,1,1,1,2,3,8,1,2,2,4,1,5,5,2,1,4,3,7,13,4,1,4,1,3,1,1,1,5,5,10,1,6,1,5,2,1,5,2,4,1,4,5,7,3,18,2,9,11,32,4,3,3,2,4,7,11,16,9,11,8,13,38,
    32,8,4,2,1,1,2,1,2,4,4,1,1,1,4,1,21,3,11,1,16,1,1,6,1,3,2,4,9,8,57,7,44,1,3,3,13,3,10,1,1,7,5,2,7,21,47,63,3,15,4,7,1,16,1,1,2,8,2,3,42,15,4,
    1,29,7,22,10,3,78,16,12,20,18,4,67,11,5,1,3,15,6,21,31,32,27,18,13,71,35,5,142,4,10,1,2,50,19,33,16,35,37,16,19,27,7,1,133,19,1,4,8,7,20,1,4,
    4,1,10,3,1,6,1,2,51,5,40,15,24,43,22928,11,1,13,154,70,3,1,1,7,4,10,1,2,1,1,2,1,2,1,2,2,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,
    3,2,1,1,1,1,2,1,1,
};

// -----------------------------------------------------------------------------

static const char mv_vs_source[] = "//" FILELINE /*#version 330 core\n\*/"\
\n\
in vec2 vertexPosition;\n\
in vec4 instanceGlyph;\n\
\n\
uniform sampler2D sampler_font;\n\
uniform sampler2D sampler_meta;\n\
\n\
uniform float offset_firstline; // ascent - descent - linegap/2\n\
uniform float scale_factor;     // scaling factor proportional to font size\n\
uniform vec2 string_offset;     // offset of upper-left corner\n\
\n\
uniform vec2 res_meta;   // 96x2 \n\
uniform vec2 res_bitmap; // 512x256\n\
uniform vec2 resolution; // screen resolution\n\
\n\
out vec2 uv;\n\
out float color_index; // for syntax highlighting\n\
\n\
void main() { \
    // (xoff, yoff, xoff2, yoff2), from second row of texture\n\
    vec4 q2 = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.75))*vec4(res_bitmap, res_bitmap);\n\
\n\
    vec2 p = vertexPosition*(q2.zw - q2.xy) + q2.xy; // offset and scale it properly relative to baseline\n\
    p *= vec2(1.0, -1.0);                            // flip y, since texture is upside-down\n\
    p.y -= offset_firstline;                         // make sure the upper-left corner of the string is in the upper-left corner of the screen\n\
    p *= scale_factor;                               // scale relative to font size\n\
    p += instanceGlyph.xy + string_offset;           // move glyph into the right position\n\
    p *= 2.0/resolution;                             // to NDC\n\
    p += vec2(-1.0, 1.0);                            // move to upper-left corner instead of center\n\
\n\
    gl_Position = vec4(p, 0.0, 1.0);\n\
\n\
    // (x0, y0, x1-x0, y1-y0), from first row of texture\n\
    vec4 q = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.25));\n\
\n\
    // send the correct uv's in the font atlas to the fragment shader\n\
    uv = q.xy + vertexPosition*q.zw;\n\
    color_index = instanceGlyph.w;\n\
}\n";

static const char mv_fs_source[] = "//" FILELINE /*#version 330 core\n\*/"\
\n\
in vec2 uv;\n\
in float color_index;\n\
\n\
uniform sampler2D sampler_font;\n\
uniform sampler1D sampler_colors;\n\
uniform float num_colors;\n\
\n\
out vec4 outColor;\n\
\n\
void main() {\
    vec3 col = texture(sampler_colors, (color_index+0.5)/num_colors).rgb;\n\
    float s = texture(sampler_font, uv).r;\n\
    outColor = vec4(col, s);\n\
}\n";

enum { FONT_MAX_COLORS = 256};
enum { FONT_MAX_STRING_LEN = 40000 }; // more glyphs than any reasonable person would show on the screen at once. you can only fit 20736 10x10 rects in a 1920x1080 window

static unsigned font_palette[FONT_MAX_COLORS] = {
    RGB4(248, 248, 242, 255), // foreground color
    RGB4(249,  38, 114, 255), // operator
    RGB4(174, 129, 255, 255), // numeric
    RGB4(102, 217, 239, 255), // function
    RGB4(249,  38, 114, 255), // keyword
    RGB4(117, 113,  94, 255), // comment
    RGB4(102, 217, 239, 255), // type
    RGB4( 73,  72,  62, 255), // background color
    RGB4( 39,  40,  34, 255), // clear color
};

typedef struct font_t {
    bool initialized;

    //char filename[256];

    // character info
    // filled up by stb_truetype.h
    stbtt_packedchar *cdata;
    unsigned num_glyphs;
    unsigned *cp2iter;
    unsigned *iter2cp;

    // font info and data
    int height;      // bitmap height
    int width;       // bitmap width
    float font_size; // font size in pixels (matches scale[0+1] size below)
    float scale[7];  // user defined font scale (match H1..H6 tags)

    // displacement info
    float ascent;   // max distance above baseline for all glyphs
    float descent;  // max distance below baseline for all glyphs
    float linegap;  // distance betwen ascent of next line and descent of current line
    float linedist; // distance between the baseline of two lines (ascent - descent + linegap)

    // opengl stuff
    GLuint vao;
    GLuint program;

    // font bitmap texture
    // generated using stb_truetype.h
    GLuint texture_fontdata;

    // metadata texture.
    // first row contains information on which parts of the bitmap correspond to a glyph.
    // the second row contain information about the relative displacement of the glyph relative to the cursor position
    GLuint texture_offsets;

    // color texture
    // used to color each glyph individually, e.g. for syntax highlighting
    GLuint texture_colors;

    // vbos
    GLuint vbo_quad;      // vec2: simply just a regular [0,1]x[0,1] quad
    GLuint vbo_instances; // vec4: (char_pos_x, char_pos_y, char_index, color_index)
} font_t;

static font_t fonts[8] = {0};

static
void font_init() {
    do_once {
        for( int i = 0; i < 6; ++i ) {
            font_face_from_mem(FONT_FACE1 + i, bm_mini_ttf,0, 48, 0);
        }
    }
}

// Remap color within all existing color textures
void font_color(const char *tag, uint32_t color) {
    do_once font_init();

    unsigned index = *tag - FONT_COLOR1[0];
    if( index < FONT_MAX_COLORS ) {
        font_palette[index] = color;

        for( int i = 0; i < 8; ++i ) {
            font_t *f = &fonts[i];
            if( f->initialized ) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_1D, f->texture_colors);
                glTexSubImage1D(GL_TEXTURE_1D, 0, 0, FONT_MAX_COLORS, GL_BGRA, GL_UNSIGNED_BYTE, font_palette);
            }
        }
    }
}

void font_scales(const char *tag, float h1, float h2, float h3, float h4, float h5, float h6) {
    do_once font_init();

    unsigned index = *tag - FONT_FACE1[0];
    if( index >= 8 ) return;

    font_t *f = &fonts[index];
    if (!f->initialized) return;

    f->scale[0] = h1 / f->font_size;
    f->scale[1] = h1 / f->font_size;
    f->scale[2] = h2 / f->font_size;
    f->scale[3] = h3 / f->font_size;
    f->scale[4] = h4 / f->font_size;
    f->scale[5] = h5 / f->font_size;
    f->scale[6] = h6 / f->font_size;
}

// 1. Compile the shaders.
// 1. Call stb_truetype.h routines to read and parse a .ttf file.
// 1. Create a bitmap that is uploaded to the gpu using opengl.
// 1. Calculate and save a bunch of useful variables and put them in the global font variable.
void font_face_from_mem(const char *tag, const unsigned char *ttf_buffer, unsigned ttf_len, float font_size, unsigned flags) {
    flags |= FONT_ASCII; // ensure this minimal range [0020-00FF] is always in

    unsigned index = *tag - FONT_FACE1[0];

    if( index >= 8 ) return;
    if( !ttf_buffer /*|| !ttf_len*/ ) return;
    if( font_size <= 0 || font_size > 72 ) return;

    font_t *f = &fonts[index];
    f->initialized = 1;

    // load .ttf into a bitmap using stb_truetype.h
    int dim = flags & FONT_4096 ? 4096 : flags & FONT_2048 ? 2048 : flags & FONT_1024 ? 1024 : 512;
    f->width = dim;
    f->height = dim;

    // change size [h1(largest) to h3(regular) to h6(smallest)]
    f->font_size = font_size;
    f->scale[0] = 1.0000f; // H1
    f->scale[1] = 1.0000f; // H1
    f->scale[2] = 0.8000f; // H2
    f->scale[3] = 0.6000f; // H3
    f->scale[4] = 0.5000f; // H4
    f->scale[5] = 0.4000f; // H5
    f->scale[6] = 0.3000f; // H6

    const char *vs_filename = 0, *fs_filename = 0;
    char *vs = vs_filename ? file_read(vs_filename) : mv_vs_source;
    char *fs = fs_filename ? file_read(fs_filename) : mv_fs_source;
    f->program = shader(vs, fs, "vertexPosition,instanceGlyph", "outColor");

    // figure out what ranges we're about to bake
    #define MERGE_TABLE(table) do { \
    for( int i = 0 ; table[i]; i += 2 ) { \
        uint64_t begin = table[i+0], end = table[i+1]; \
        for( int j = begin; j <= end; ++j ) { \
            array_push(sorted, j); \
        } \
    } } while(0)
    #define MERGE_PACKED_TABLE(codepoint_begin, table) do { \
    for( int i = 0, begin = codepoint_begin, end = countof(table); i < end; i++) { \
        array_push(sorted, (unsigned)(begin + table[i])); \
        begin += table[i]; \
    } } while(0)

    array(uint64_t) sorted = 0;
    if(flags & FONT_ASCII) { MERGE_TABLE(table_common); }
    if(flags & FONT_EU)    { MERGE_TABLE(table_eastern_europe); }
    if(flags & FONT_RU)    { MERGE_TABLE(table_western_europe); }
    if(flags & FONT_EL)    { MERGE_TABLE(table_western_europe); }
    if(flags & FONT_AR)    { MERGE_TABLE(table_middle_east); }
    if(flags & FONT_HE)    { MERGE_TABLE(table_middle_east); }
    if(flags & FONT_TH)    { MERGE_TABLE(table_thai); }
    if(flags & FONT_VI)    { MERGE_TABLE(table_vietnamese); }
    if(flags & FONT_KR)    { MERGE_TABLE(table_korean); }
    if(flags & FONT_JP)    { MERGE_TABLE(table_chinese_japanese_common); MERGE_PACKED_TABLE(0x4E00, packed_table_japanese); }
    if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_japanese_common); MERGE_PACKED_TABLE(0x4E00, packed_table_chinese); } // zh-simplified
    if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_punctuation); } // both zh-simplified and zh-full
//  if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_full); } // zh-full
    array_unique(sorted, sort_u64); // sort + unique pass

    // pack and create bitmap
    unsigned char *bitmap = (unsigned char*)MALLOC(f->height*f->width);

        int charCount = 0xFFFF;
        f->cdata = (stbtt_packedchar*)CALLOC(1, sizeof(stbtt_packedchar) * charCount);
        f->iter2cp = (unsigned*)CALLOC( 1, sizeof(unsigned) * charCount );
        for( int i = 0; i < charCount; ++i ) f->iter2cp[i] = 0xFFFD; // default invalid glyph
        f->cp2iter = (unsigned*)CALLOC( 1, sizeof(unsigned) * charCount );
        for( int i = 0; i < charCount; ++i ) f->cp2iter[i] = 0xFFFD; // default invalid glyph

        stbtt_pack_context pc;
        if( !stbtt_PackBegin(&pc, bitmap, f->width, f->height, 0, 1, NULL) ) {
            PANIC("Failed to initialize atlas font");
        }
        stbtt_PackSetOversampling(&pc, flags & FONT_OVERSAMPLE_X ? 2 : 1, flags & FONT_OVERSAMPLE_Y ? 2 : 1);  /*useful on small fonts*/
        int count = 0;
        for( int i = 0, num = array_count(sorted); i < num; ++i ) {

            uint64_t begin = sorted[i], end = sorted[i];
            while( i < (num-1) && (sorted[i+1]-sorted[i]) == 1 ) end = sorted[++i];
            //printf("(%d,%d)", (unsigned)begin, (unsigned)end);

            if( stbtt_PackFontRange(&pc, ttf_buffer, 0, f->font_size, begin, end - begin + 1, (stbtt_packedchar*)f->cdata + begin) ) {
                for( int j = begin; j <= end; ++j ) {
                    // unicode->index runtime lookup
                    f->cp2iter[ j ] = count;
                    f->iter2cp[ count++ ] = j;
                }
            } else {
                PRINTF("!Failed to pack atlas font. Likely out of texture mem.");
            }
        }
        stbtt_PackEnd(&pc);
        f->num_glyphs = count;

    array_free(sorted);

    // calculate vertical font metrics
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

    int a, d, l;
    float s = stbtt_ScaleForPixelHeight(&info, f->font_size);
    stbtt_GetFontVMetrics(&info, &a, &d, &l);

    f->ascent = a * s;
    f->descent = d * s;
    f->linegap = l * s;
    f->linedist = (a - d + l) * s;

    // save some gpu memory by truncating unused vertical space in atlas texture
    {
        int max_y1 = 0;
        for (int i = 0; i < f->num_glyphs; i++) {
            int cp = f->iter2cp[i];
            if( cp == 0xFFFD ) continue;
            stbtt_packedchar *cd = &f->cdata[ cp ];
            if (cd->y1 > max_y1) {
                max_y1 = cd->y1;
            }
        }
        // cut away the unused part of the bitmap
        f->height = max_y1+1;
    }

    PRINTF("Font atlas size %dx%d (GL_R, %5.2fKiB) (%u glyphs)\n", f->width, f->height, f->width * f->height / 1024.f, f->num_glyphs);

    // vao
    glGenVertexArrays(1, &f->vao);
    glBindVertexArray(f->vao);

    // quad vbo setup, used for glyph vertex positions,
    // just uv coordinates that will be stretched accordingly by the glyphs width and height
    float v[] = {0,0,  1,0,  0,1,  0,1,  1,0,  1,1};

    glGenBuffers(1, &f->vbo_quad);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
    glVertexAttribDivisor(0, 0);

    // instance vbo setup: for glyph positions, glyph index and color index
    glGenBuffers(1, &f->vbo_instances);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_instances);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*FONT_MAX_STRING_LEN, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glVertexAttribDivisor(1, 1);
    //glEnable(GL_FRAMEBUFFER_SRGB);

    // setup and upload font bitmap texture
    glGenTextures(1, &f->texture_fontdata);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture_fontdata);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, f->width, f->height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // last chance to inspect the font atlases
    if( flag("--debug-font-atlas") )
    stbi_write_png(va("debug_font_atlas%d.png", index), f->width, f->height, 1, bitmap, 0);

    FREE(bitmap);

    // setup and upload font metadata texture
    // used for lookup in the bitmap texture
    glGenTextures(1, &f->texture_offsets);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f->texture_offsets);

    float *texture_offsets = (float*)MALLOC(sizeof(float)*8*f->num_glyphs);

    // remap larger 0xFFFF unicodes into smaller NUM_GLYPHS glyphs
    for (int i = 0, count = 0; i < f->num_glyphs; i++) {

        unsigned cp = f->iter2cp[ i ];
        if(cp == 0xFFFD) continue;

        stbtt_packedchar *cd = &f->cdata[ cp ];
//      if(cd->x1==cd->x0) { f->iter2cp[i] = f->cp2iter[cp] = 0xFFFD; continue; }

        int k1 = 0*f->num_glyphs + count;
        int k2 = 1*f->num_glyphs + count; ++count;

        texture_offsets[4*k1+0] = cd->x0/(double)f->width;
        texture_offsets[4*k1+1] = cd->y0/(double)f->height;
        texture_offsets[4*k1+2] = (cd->x1-cd->x0)/(double)f->width;
        texture_offsets[4*k1+3] = (cd->y1-cd->y0)/(double)f->height;

        texture_offsets[4*k2+0] = cd->xoff/(double)f->width;
        texture_offsets[4*k2+1] = cd->yoff/(double)f->height;
        texture_offsets[4*k2+2] = cd->xoff2/(double)f->width;
        texture_offsets[4*k2+3] = cd->yoff2/(double)f->height;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, f->num_glyphs, 2, 0, GL_RGBA, GL_FLOAT, texture_offsets);

    FREE(texture_offsets);

    // setup color texture
    glGenTextures(1, &f->texture_colors);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, f->texture_colors);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, FONT_MAX_COLORS, 0, GL_BGRA, GL_UNSIGNED_BYTE, font_palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    // upload constant uniforms
    glUseProgram(f->program);
    glUniform1i(glGetUniformLocation(f->program, "sampler_font"), 0);
    glUniform1i(glGetUniformLocation(f->program, "sampler_meta"), 1);
    glUniform1i(glGetUniformLocation(f->program, "sampler_colors"), 2);

    glUniform2f(glGetUniformLocation(f->program, "res_bitmap"), f->width, f->height);
    glUniform2f(glGetUniformLocation(f->program, "res_meta"), f->num_glyphs, 2);
    glUniform1f(glGetUniformLocation(f->program, "num_colors"), FONT_MAX_COLORS);
    glUniform1f(glGetUniformLocation(f->program, "offset_firstline"), f->linedist-f->linegap);
}

void font_face(const char *tag, const char *filename_ttf, float font_size, unsigned flags) {
    do_once font_init();

    const unsigned char *buffer = file_read(filename_ttf);
    if(!buffer) buffer = vfs_read(filename_ttf);
    font_face_from_mem(tag, buffer,0, font_size, flags);
}

static
void font_draw_cmd(font_t *f, const float *glyph_data, int glyph_idx, float factor, vec2 offset) {
    // Backup GL state
    GLint last_program, last_vertex_array;
    GLint last_texture0, last_texture1, last_texture2;
    GLint last_blend_src, last_blend_dst;
    GLint last_blend_equation_rgb, last_blend_equation_alpha;

    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture0);
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture1);
    glActiveTexture(GL_TEXTURE2);
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &last_texture2);

    glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    glGetIntegerv(GL_BLEND_EQUATION_RGB,   &last_blend_equation_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);

    GLboolean last_enable_blend      = glIsEnabled(GL_BLEND);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);

    // Setup render state: alpha-blending enabled, no depth testing and bind textures
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture_fontdata);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f->texture_offsets);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, f->texture_colors);

    // update bindings
    glBindVertexArray(f->vao);

    // update uniforms
    glUseProgram(f->program);
    glUniform1f(glGetUniformLocation(f->program, "scale_factor"), factor);
    glUniform2fv(glGetUniformLocation(f->program, "string_offset"), 1, &offset.x);

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    glUniform2f(glGetUniformLocation(f->program, "resolution"), dims[2], dims[3]);

    // actual uploading
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_instances);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4*4*glyph_idx, glyph_data);

    // actual drawing
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, glyph_idx);

    // Restore modified GL state
    glUseProgram(last_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, last_texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, last_texture1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, last_texture2);

    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBindVertexArray(last_vertex_array);
    glBlendFunc(last_blend_src, last_blend_dst);

    (last_enable_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    (last_enable_blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND));
}

// 1. call font_face() if it's the first time it's called.
// 1. parse the string and update the instance vbo, then upload it
// 1. draw the string
static
vec2 font_draw_ex(const char *text, vec2 offset, const char *col, void (*draw_cmd)(font_t *,const float *,int,float,vec2)) {
    do_once font_init();

    // sanity checks
    int len = strlen(text);
    if (len >= FONT_MAX_STRING_LEN) {
        return vec2(0, 0);
    }

    // pre-init
    static __thread float *text_glyph_data, **init = 0;
    if(!init) *(init = &text_glyph_data) = (float*)MALLOC(4 * FONT_MAX_STRING_LEN * sizeof(float));

    // ready
    font_t *f = &fonts[0];
    int S = 3;
    uint32_t color = 0;
    float X = 0, Y = 0, W = 0, L = f->linedist*f->scale[S], LL = L; // LL=largest linedist
    offset.y = -offset.y; // invert y polarity

    // utf8 to utf32
    array(uint32_t) unicode = strutf32(text);

    // parse string
    float *t = text_glyph_data;
    for( int i = 0, end = array_count(unicode); i < end; ++i ) {
        uint32_t ch = unicode[i];

        if( ch == '\n' ) {
            // change cursor, advance y, record largest x as width, increase height
            if( X > W ) W = X;
            X = 0.0;
            Y -= L;
            continue;
        }
        if( ch >= 1 && ch <= 6 ) {
            // flush previous state
            if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);
            t = text_glyph_data;

            // reposition offset to align new baseline
            offset.y += (f->linedist - f->linegap) * ( f->scale[ch] - f->scale[S] );

            // change size
            S = ch;
            L = f->linedist*f->scale[S];
            if(L > LL) LL = L;
            continue;
        }
        if( ch >= 0x10 && ch <= 0x19 ) {
            color = ch - 0x10;
            continue;
        }
        if( ch >= 0x1a && ch <= 0x1f ) {
            if( fonts[ ch - 0x1a ].initialized) {
            // flush previous state
            if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);
            t = text_glyph_data;

            // change face
            f = &fonts[ ch - 0x1a ];
            }
            continue;
        }

        // convert to vbo data
        int cp = ch; // f->cp2iter[ch];
        //if(cp == 0xFFFD) continue;
        *t++ = X;
        *t++ = Y;
        *t++ = f->cp2iter[cp];
        *t++ = col ? col[i] : color;

        X += f->cdata[cp].xadvance*f->scale[S];
    }

    if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);

    //if(strstr(text, "fps")) printf("(%f,%f) (%f) L:%f LINEDIST:%f\n", X, Y, W, L, f->linedist);
    return abs2(vec2(W*W > X*X ? W : X, Y*Y > LL*LL ? Y : LL));
}

void *font_colorize(const char *text, const char *comma_types, const char *comma_keywords) {
    // reallocate memory
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    buf[slot] = REALLOC(buf[slot], strlen(text)+1);
    memset(buf[slot], 0, strlen(text)+1);

    // ready
    char *col = buf[slot];
    char *str = STRDUP(text);

    // split inputs
    array(char*) TYPES = strsplit(comma_types, ", ");
    array(char*) KEYWORDS = strsplit(comma_keywords, ", ");

    // ignored characters
    char delims[] = " ,(){}[];\t\n";
    int num_delims = strlen(delims);

    char operators[] = "/+-*<>=&|";
    int num_operators = strlen(operators);

    struct token {
        char *start, *stop;
        enum {
            TOKEN_OTHER,TOKEN_OPERATOR,TOKEN_NUMERIC,TOKEN_FUNCTION,
            TOKEN_KEYWORD,TOKEN_COMMENT,TOKEN_VARIABLE,TOKEN_UNSET
        } type;
    } tokens[9999]; // hurr
    int num_tokens = 0; // running counter

    char *ptr = str;
    while (*ptr) {
        // skip delimiters
        int is_delim = 0;
        for (int i = 0; i < num_delims; i++) {
            if (*ptr == delims[i]) {
                is_delim = 1;
                break;
            }
        }

        if (is_delim == 1) {
            ptr++;
            continue;
        }

        // found a token!
        char *start = ptr;

        if (*ptr == '/' && *(ptr+1) == '/') {
            // found a line comment, go to end of line or end of file
            while (*ptr != '\n' && *ptr != '\0') {
                ptr++;
            }

            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr;
            tokens[num_tokens].type = TOKEN_COMMENT;
            num_tokens++;

            ptr++;
            continue;
        }

        if (*ptr == '/' && *(ptr+1) == '*') {
            // found a block comment, go to end of line or end of file
            while (!(*ptr == '*' && *(ptr+1) == '/') && *ptr != '\0') {
                ptr++;
            }
            ptr++;

            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr+1;
            tokens[num_tokens].type = TOKEN_COMMENT;
            num_tokens++;

            ptr++;
            continue;
        }

        // check if it's an operator
        int is_operator = 0;
        for (int i = 0; i < num_operators; i++) {
            if (*ptr == operators[i]) {
                is_operator = 1;
                break;
            }
        }

        if (is_operator == 1) {
            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr+1;
            tokens[num_tokens].type = TOKEN_OPERATOR;
            num_tokens++;
            ptr++;
            continue;
        }

        // it's either a name, type, a keyword, a function, or an names separated by an operator without spaces
        while (*ptr) {
            // check whether it's an operator stuck between two names
            int is_operator2 = 0;
            for (int i = 0; i < num_operators; i++) {
                if (*ptr == operators[i]) {
                    is_operator2 = 1;
                    break;
                }
            }

            if (is_operator2 == 1) {
                tokens[num_tokens].start = start;
                tokens[num_tokens].stop = ptr;
                tokens[num_tokens].type = TOKEN_UNSET;
                num_tokens++;
                break;
            }

            // otherwise go until we find the next delimiter
            int is_delim2 = 0;
            for (int i = 0; i < num_delims; i++) {
                if (*ptr == delims[i]) {
                    is_delim2 = 1;
                    break;
                }
            }

            if (is_delim2 == 1) {
                tokens[num_tokens].start = start;
                tokens[num_tokens].stop = ptr;
                tokens[num_tokens].type = TOKEN_UNSET;
                num_tokens++;
                ptr++;
                break;
            }

            // did not find delimiter, check next char
            ptr++;
        }
    }

    // determine the types of the unset tokens, i.e. either
    // a name, a type, a keyword, or a function
    int num_keywords = array_count(KEYWORDS);
    int num_types = array_count(TYPES);

    for (int i = 0; i < num_tokens; i++) {
        // TOKEN_OPERATOR and TOKEN_COMMENT should already be set, so skip those
        if (tokens[i].type != TOKEN_UNSET) {
            continue;
        }

        char end_char = *tokens[i].stop;

        // temporarily null terminate at end of token, restored after parsing
        *tokens[i].stop = '\0';

        // parse

        // if it's a keyword
        int is_keyword = 0;
        for (int j = 0; j < num_keywords; j++) {
            if (strcmp(tokens[i].start, KEYWORDS[j]) == 0) {
                is_keyword = 1;
                break;
            }
        }
        if (is_keyword == 1) {
            tokens[i].type = TOKEN_KEYWORD;
            *tokens[i].stop = end_char;
            continue;
        }

        // Check if it's a function
        float f;
        if (end_char == '(') {
            tokens[i].type = TOKEN_FUNCTION;
            *tokens[i].stop = end_char;
            continue;
        }

        // or if it's a numeric value. catches both integers and floats
        if (sscanf(tokens[i].start, "%f", &f) == 1) {
            tokens[i].type = TOKEN_NUMERIC;
            *tokens[i].stop = end_char;
            continue;
        }

        // if it's a variable type
        int is_type = 0;
        for (int j = 0; j < num_types; j++) {
            if (strcmp(tokens[i].start, TYPES[j]) == 0) {
                is_type = 1;
                break;
            }
        }
        if (is_type == 1) {
            tokens[i].type = TOKEN_VARIABLE;
            *tokens[i].stop = end_char;
            continue;
        }

        // otherwise it's a regular variable name
        tokens[i].type = TOKEN_OTHER;
        *tokens[i].stop = end_char;
    }

    // print all tokens and their types
    for (int i = 0; i < num_tokens; i++) {

        for (char *p = tokens[i].start; p != tokens[i].stop; p++) {
            col[(p - str)] = tokens[i].type;
        }
    }

    FREE(str);
    return col;
}

static vec2 gotoxy = {0};

// Return cursor
vec2 font_xy() {
    return gotoxy;
}

// Relocate cursor
void font_goto(float x, float y) {
    gotoxy = vec2(x, y);
}

// Print and linefeed. Text may include markup code
vec2 font_print(const char *text) {
    // @fixme: remove this hack
    if( text[0] == FONT_LEFT[0] ) {
        int l = text[1] == FONT_LEFT[1];
        int c = text[1] == FONT_CENTER[1];
        int r = text[1] == FONT_RIGHT[1];
        if( l || c || r ) {
            vec2 rect = font_rect(text + 2);
            gotoxy.x = l ? 0 : r ? (window_width() - rect.x) : window_width()/2 - rect.x/2;
            return font_print(text + 2);
        }
        int t = text[1] == FONT_TOP[1];
        int b = text[1] == FONT_BOTTOM[1];
        int m = text[1] == FONT_MIDDLE[1];
        int B = text[1] == FONT_BASELINE[1];
        if( t || b || m || B ) {
            vec2 rect = font_rect(text + 2);
            gotoxy.y = t ? 0 : b ? (window_height() - rect.y) : m ? window_height()/2-rect.y/2 : window_height()/2-rect.y/1;
            return font_print(text + 2);
        }
    }

    vec2 dims = font_draw_ex(text, gotoxy, NULL, font_draw_cmd);
    gotoxy.y += strchr(text, '\n') ? dims.y : 0;
    gotoxy.x =  strchr(text, '\n') ? 0 : gotoxy.x + dims.x;
    return dims;
}

// Print a code snippet with syntax highlighting
vec2 font_highlight(const char *text, const void *colors) {
    vec2 dims = font_draw_ex(text, gotoxy, (const char *)colors, font_draw_cmd);
    gotoxy.y += strchr(text, '\n') ? dims.y : 0;
    gotoxy.x =  strchr(text, '\n') ? 0 : gotoxy.x + dims.x;
    return dims;
}

// Calculate the size of a string, in the pixel size specified. Count stray newlines too.
vec2 font_rect(const char *str) {
    return font_draw_ex(str, gotoxy, NULL, NULL);
}
#line 0
#line 1 "fwk_input.c"
// gotta love linux
#ifdef __linux
#undef KEY_ESC
#undef KEY_TICK
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9
#undef KEY_0
#undef KEY_BS
#undef KEY_TAB
#undef KEY_Q
#undef KEY_W
#undef KEY_E
#undef KEY_R
#undef KEY_T
#undef KEY_Y
#undef KEY_U
#undef KEY_I
#undef KEY_O
#undef KEY_P
#undef KEY_CAPS
#undef KEY_A
#undef KEY_S
#undef KEY_D
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_ENTER
#undef KEY_LSHIFT
#undef KEY_Z
#undef KEY_X
#undef KEY_C
#undef KEY_V
#undef KEY_B
#undef KEY_N
#undef KEY_M
#undef KEY_RSHIFT
#undef KEY_UP
#undef KEY_LCTRL
#undef KEY_LALT
#undef KEY_SPACE
#undef KEY_RALT
#undef KEY_RCTRL
#undef KEY_LEFT
#undef KEY_DOWN
#undef KEY_RIGHT
#undef KEY_INS
#undef KEY_HOME
#undef KEY_PGUP
#undef KEY_DEL
#undef KEY_END
#undef KEY_PGDN
#undef KEY_LMETA
#undef KEY_RMETA
#undef KEY_MENU
#undef KEY_PRINT
#undef KEY_PAUSE
#undef KEY_SCROLL
#undef KEY_NUMLOCK
#undef KEY_MINUS
#undef KEY_EQUAL
#undef KEY_LSQUARE
#undef KEY_RSQUARE
#undef KEY_SEMICOLON
#undef KEY_QUOTE
#undef KEY_HASH
#undef KEY_BAR
#undef KEY_COMMA
#undef KEY_DOT
#undef KEY_SLASH
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#undef KEY_PAD1
#undef KEY_PAD2
#undef KEY_PAD3
#undef KEY_PAD4
#undef KEY_PAD5
#undef KEY_PAD6
#undef KEY_PAD7
#undef KEY_PAD8
#undef KEY_PAD9
#undef KEY_PAD0
#undef KEY_PADADD
#undef KEY_PADSUB
#undef KEY_PADMUL
#undef KEY_PADDIV
#undef KEY_PADDOT
#undef KEY_PADENTER
#undef MOUSE_L
#undef MOUSE_M
#undef MOUSE_R
#undef GAMEPAD_CONNECTED
#undef GAMEPAD_A
#undef GAMEPAD_B
#undef GAMEPAD_X
#undef GAMEPAD_Y
#undef GAMEPAD_UP
#undef GAMEPAD_DOWN
#undef GAMEPAD_LEFT
#undef GAMEPAD_RIGHT
#undef GAMEPAD_MENU
#undef GAMEPAD_START
#undef GAMEPAD_LB
#undef GAMEPAD_RB
#undef GAMEPAD_LTHUMB
#undef GAMEPAD_RTHUMB
#undef WINDOW_BLUR
#undef WINDOW_FOCUS
#undef WINDOW_CLOSE
#undef WINDOW_MINIMIZE
#undef WINDOW_MAXIMIZE
#undef WINDOW_FULLSCREEN
#undef WINDOW_WINDOWED
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPADY
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPADY
#undef GAMEPAD_LT
#undef GAMEPAD_RT
#undef GAMEPAD_BATTERY
#undef MOUSE
#undef MOUSE
#undef MOUSE_Y
#undef MOUSE_W
#undef TOUCH_X1
#undef TOUCH_Y1
#undef TOUCH_X2
#undef TOUCH_Y2
#undef WINDOW_RESIZE
#undef WINDOW_RESIZE
#undef WINDOW_RESIZEY
#undef WINDOW_ORIENTATION
#undef WINDOW_BATTERY
#undef GAMEPAD_GUID
#undef GAMEPAD_NAME
#endif

static int controller_id = 0;

static struct controller_t {
    const char* strings[2];
    float floats[7+3+4+4];
    char  bits[104+3+15+7];
} controller[4] = {0}, frame[60] = {0};

static struct controller_t *input_logger(int position, int advance) {
    static int cycle = 0;
    cycle += advance;
    position += cycle;
    position = position >= 0 ? position % 60 : 60-1 + ((position+1) % 60);
    return &frame[position];
}

void input_mappings() {
    unsigned char* mappings = vfs_read("gamecontrollerdb.txt");
    if( mappings ) { glfwUpdateGamepadMappings(mappings); /*REALLOC(mappings, 0);*/ }
}

void input_init() {
    do_once {
        input_mappings();
    }
    void input_update();
    window_hook(input_update, NULL);
}

void input_update() {
    struct controller_t *c = &controller[0]; // @fixme

    char *bits = &c->bits[0];
    float *floats = &c->floats[-GAMEPAD_LPADX];
    const char **strings = &c->strings[-GAMEPAD_GUID];
    float mouse_wheel_old = floats[MOUSE_W];

    struct controller_t clear = {0};
    *c = clear;
    for( int i = 0; i < countof(c->strings); ++i ) c->strings[i] = "";

    struct GLFWwindow *win = window_handle();
    //  glfwSetInputMode(win, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    floats[MOUSE_X] = mx;
    floats[MOUSE_Y] = my;
    struct nk_glfw* glfw = glfwGetWindowUserPointer(win); // from nuklear, because it is overriding glfwSetScrollCallback()
    floats[MOUSE_W] = !glfw ? 0 : mouse_wheel_old + (float)glfw->scroll.x + (float)glfw->scroll.y;
    bits[MOUSE_L] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bits[MOUSE_M] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    bits[MOUSE_R] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

#if !is(ems) // error: use of undeclared identifier '_GLFWwindow', 'GLFW_GAMEPAD_BUTTON_A' [etc.]
    #define k2(VK,GLFW) [KEY_##VK] = GLFW_KEY_##GLFW
    #define k(VK) k2(VK,VK)
    int table[] = {
        k2(ESC,ESCAPE),
        k2(TICK,GRAVE_ACCENT), k(1),k(2),k(3),k(4),k(5),k(6),k(7),k(8),k(9),k(0), k2(BS,BACKSPACE),
        k(TAB),                k(Q),k(W),k(E),k(R),k(T),k(Y),k(U),k(I),k(O),k(P),
        k2(CAPS,CAPS_LOCK),      k(A),k(S),k(D),k(F),k(G),k(H),k(J),k(K),k(L), k(ENTER),
        k2(LSHIFT,LEFT_SHIFT),       k(Z),k(X),k(C),k(V),k(B),k(N),k(M),  k2(RSHIFT,RIGHT_SHIFT),                      k(UP),
        k2(LCTRL,LEFT_CONTROL),k2(LALT,LEFT_ALT), k(SPACE), k2(RALT,RIGHT_ALT),k2(RCTRL,RIGHT_CONTROL), k(LEFT),k(DOWN),k(RIGHT),

        k(F1),k(F2),k(F3),k(F4),k(F5),k(F6),k(F7),k(F8),k(F9),k(F10),k(F11),k(F12), k2(PRINT,PRINT_SCREEN),k(PAUSE),
        k2(INS,INSERT),k(HOME),k2(PGUP,PAGE_UP), k2(DEL,DELETE),k(END), k2(PGDN,PAGE_DOWN),
    };
    for(int i = 0; i < countof(table); ++i) {
        //if( table[i] ) bits[i] = glfwGetKey(win, table[i] ) == GLFW_PRESS;
        bits[i] = ((_GLFWwindow*)win)->keys[ table[i] ];
    }
    #undef k
    #undef k2

    int jid = GLFW_JOYSTICK_1 + 0; // @fixme
    if( glfwGetGamepadName(jid) ) { // glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid) ) {
        bits[GAMEPAD_CONNECTED] = 1;
        strings[GAMEPAD_GUID] = glfwGetJoystickGUID(jid);
        strings[GAMEPAD_NAME] = glfwGetGamepadName(jid);
        floats[GAMEPAD_BATTERY] = 100; //glfwJoystickCurrentPowerLevel(jid);

        GLFWgamepadstate state;
        if (glfwGetGamepadState(jid, &state)) {
            bits[GAMEPAD_A] = state.buttons[GLFW_GAMEPAD_BUTTON_A]; // cross
            bits[GAMEPAD_B] = state.buttons[GLFW_GAMEPAD_BUTTON_B]; // circle
            bits[GAMEPAD_X] = state.buttons[GLFW_GAMEPAD_BUTTON_X]; // square
            bits[GAMEPAD_Y] = state.buttons[GLFW_GAMEPAD_BUTTON_Y]; // triangle

            bits[GAMEPAD_UP] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP];
            bits[GAMEPAD_DOWN] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN];
            bits[GAMEPAD_LEFT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
            bits[GAMEPAD_RIGHT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];

            bits[GAMEPAD_LB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
            bits[GAMEPAD_RB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
            bits[GAMEPAD_MENU] = state.buttons[GLFW_GAMEPAD_BUTTON_BACK];
            bits[GAMEPAD_START] = state.buttons[GLFW_GAMEPAD_BUTTON_START]; // _GUIDE

            bits[GAMEPAD_LTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
            bits[GAMEPAD_RTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB];

            floats[GAMEPAD_LT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]); // [-1..+1] -> [0..1]
            floats[GAMEPAD_RT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]); // [-1..+1] -> [0..1]

            floats[GAMEPAD_LPADX] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            floats[GAMEPAD_LPADY] = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

            floats[GAMEPAD_RPADX] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            floats[GAMEPAD_RPADY] = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        }
    }
#endif

    *input_logger(0,+1) = controller[0];
}

int input_use(int id) {
    return controller_id >= 0 && controller_id <= 3 ? controller_id = id, 1 : 0;
}

float input_frame( int vk, int frame ) {
    if( controller_id > 0 ) return 0; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    if(vk < GAMEPAD_LPADX) return c->bits[vk]; // if in bits...
    if(vk < GAMEPAD_GUID) return c->floats[vk - GAMEPAD_LPADX]; // if in floats...
    return 0.f; // NAN?
}
vec2 input_frame2( int vk, int frame ) {
    return vec2( input_frame(vk, frame), input_frame(vk+1, frame) );
}
const char *input_frames( int vk, int frame ) {
    if( controller_id > 0 ) return ""; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    return vk >= GAMEPAD_GUID ? c->strings[vk - GAMEPAD_GUID] : ""; // if in strings...
}

// --- sugars

float input_diff( int vk ) {
    return input_frame(vk, 0) - input_frame(vk, -1);
}
vec2 input_diff2( int vk ) {
    return vec2( input_diff(vk), input_diff(vk+1) );
}
float input( int vk ) {
    return input_frame( vk, 0 );
}
vec2 input2( int vk ) {
    return vec2( input_frame(vk, 0), input_frame(vk+1, 0) );
}

// --- events

const float MS2FRAME = 0.06f; // 60 hz/1000 ms

int event( int vk ) {
    float v = input_frame(vk,0);
    return (v * v) > 0;
}
int input_chord2( int vk1, int vk2 ) {
    return event(vk1) && event(vk2);
}
int input_chord3( int vk1, int vk2, int vk3 ) {
    return event(vk1) && input_chord2(vk2, vk3);
}
int input_chord4( int vk1, int vk2, int vk3, int vk4 ) {
    return event(vk1) && input_chord3(vk2, vk3, vk4);
}
int input_down( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) > 0;
}
int input_held( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) > 0;
}
int input_up( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) <= 0;
}
int input_idle( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) <= 0;
}
int input_repeat( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) > 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) > 0;
}
int input_click( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) <= 0;
}
int input_click2( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME *3/4) > 0
    && input_frame(vk,-ms * MS2FRAME *2/4) <= 0 && input_frame(vk,-ms * MS2FRAME *1/4) > 0 && input_frame(vk,0) <= 0;
}

#undef MS2FRAME

// --- filters

float input_filter_positive( float v ) {
    return ( v + 1 ) * 0.5f;
}
vec2  input_filter_positive2( vec2 v ) {
    return scale2(inc2(v,1), 0.5f);
}

vec2 input_filter_deadzone( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float mag = sqrt( v.x*v.x + v.y*v.y );
    float nx = v.x / mag, ny = v.y / mag, k = (mag - deadzone) / (1 - deadzone);
    if( k > 1 ) k = 1; // clamp
    // k = k * k; // uncomment for a smoother curve
    return mag < deadzone ? vec2(0, 0) : vec2(nx * k, ny * k);
}

vec2 input_filter_deadzone_4way( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float v0 = v.x*v.x < deadzone*deadzone ? 0 : v.x;
    float v1 = v.y*v.y < deadzone*deadzone ? 0 : v.y;
    return vec2(v0, v1);
}

// ----------------------------------------------------------------------------

void input_demo() {
    if( ui_begin("Keyboard",0) ) {
        ui_const_bool("[Key 1]", input(KEY_1));
        ui_const_bool("[Key 2]", input(KEY_2));
        ui_const_bool("[Key 3]", input(KEY_3));
        ui_const_bool("[Key 4]", input(KEY_4));
        ui_const_bool("[Key 5]", input(KEY_5));
        ui_const_bool("[Key 6]", input(KEY_6));
        ui_const_bool("[Key 7]", input(KEY_7));
        ui_separator();
        ui_const_bool("[Key 1] Down event", input_down(KEY_1) );
        ui_const_bool("[Key 2] Held event", input_held(KEY_2) );
        ui_const_bool("[Key 3] Up event", input_up(KEY_3) );
        ui_const_bool("[Key 4] Idle event", input_idle(KEY_4) );
        ui_const_bool("[Key 5] Click event", input_click(KEY_5,500) );
        ui_const_bool("[Key 6] Click2 event", input_click2(KEY_6,1000) );
        ui_const_bool("[Key 7] Repeat event", input_repeat(KEY_7,750) );
        ui_end();
    }
    if( ui_begin("Mouse",0) ) {
        ui_const_float("X", input(MOUSE_X));
        ui_const_float("Y", input(MOUSE_Y));
        ui_separator();
        ui_const_float("Wheel", input(MOUSE_W));
        ui_separator();
        ui_const_bool("Left", input(MOUSE_L));
        ui_const_bool("Middle", input(MOUSE_M));
        ui_const_bool("Right", input(MOUSE_R));
        ui_end();
    }
    if( ui_begin("GamePad",0) ) {
        static int gamepad_id = 0;
        const char *list[] = {"1","2","3","4"};
        ui_list("Gamepad", list, 4, &gamepad_id);

        input_use(gamepad_id);

        ui_const_string("Name", input_frames(GAMEPAD_NAME,0));
        ui_const_bool("Connected", input(GAMEPAD_CONNECTED));

        ui_separator();

        ui_const_bool("A", input(GAMEPAD_A) );
        ui_const_bool("B", input(GAMEPAD_B) );
        ui_const_bool("X", input(GAMEPAD_X) );
        ui_const_bool("Y", input(GAMEPAD_Y) );
        ui_const_bool("Up", input(GAMEPAD_UP) );
        ui_const_bool("Down", input(GAMEPAD_DOWN) );
        ui_const_bool("Left", input(GAMEPAD_LEFT) );
        ui_const_bool("Right", input(GAMEPAD_RIGHT) );
        ui_const_bool("Menu", input(GAMEPAD_MENU) );
        ui_const_bool("Start", input(GAMEPAD_START) );

        ui_separator();

        ui_const_float("Left pad x", input(GAMEPAD_LPADX) );
        ui_const_float("Left pad y", input(GAMEPAD_LPADY) );
        ui_const_float("Left trigger", input(GAMEPAD_LT) );
        ui_const_bool("Left bumper", input(GAMEPAD_LB) );
        ui_const_bool("Left thumb", input(GAMEPAD_LTHUMB) );

        vec2 v = input_filter_deadzone( input2(GAMEPAD_LPADX), 0.1f );
        ui_const_float("Filtered pad x", v.x);
        ui_const_float("Filtered pad y", v.y);

        ui_separator();

        ui_const_float("Right pad x", input(GAMEPAD_RPADX) );
        ui_const_float("Right pad y", input(GAMEPAD_RPADY) );
        ui_const_float("Right trigger", input(GAMEPAD_RT) );
        ui_const_bool("Right bumper", input(GAMEPAD_RB) );
        ui_const_bool("Right thumb", input(GAMEPAD_RTHUMB) );

        vec2 w = input_filter_deadzone( input2(GAMEPAD_RPADX), 0.1f );
        ui_const_float("Filtered pad x", w.x);
        ui_const_float("Filtered pad y", w.y);

        ui_end();
    }
}
#line 0
#line 1 "fwk_math.c"
// -----------------------------------------------------------------------------
// math framework: rand, ease, vec2, vec3, vec4, quat, mat2, mat33, mat34, mat4
// - rlyeh, public domain
//
// Credits: @ands+@krig+@vurtun (PD), @datenwolf (WTFPL2), @evanw+@barerose (CC0), @sgorsten (Unlicense).

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>

static uint64_t rand_xoro256(uint64_t x256_s[4]) { // xoshiro256+ 1.0 by David Blackman and Sebastiano Vigna (PD)
    const uint64_t result = x256_s[0] + x256_s[3];
    const uint64_t t = x256_s[1] << 17;

    x256_s[2] ^= x256_s[0];
    x256_s[3] ^= x256_s[1];
    x256_s[1] ^= x256_s[2];
    x256_s[0] ^= x256_s[3];

    x256_s[2] ^= t;

    x256_s[3] = (x256_s[3] << 45) | (x256_s[3] >> (64 - 45)); //x256_rotl(x256_s[3], 45);

    return result;
}
static local uint64_t rand_state[4] = {// = splitmix64(0),splitmix64(splitmix64(0)),... x4 times
    UINT64_C(0x9e3779b8bb0b2c64),UINT64_C(0x3c6ef372178960e7),
    UINT64_C(0xdaa66d2b71a12917),UINT64_C(0x78dde6e4d584aef9)
};
void randset(uint64_t x) {
    x = hash_64(x);
    for( int i = 0; i < 4; ++i) {
        // http://xoroshiro.di.unimi.it/splitmix64.c
        uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
        z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
        x = x ^ (z >> 31);
        rand_state[i] = x;
    }
}
uint64_t rand64(void) {
    return rand_xoro256(rand_state);
}
double randf(void) { // [0, 1) interval
    uint64_t u64 = rand64();
    // fastest way to convert in C99 a 64-bit unsigned integer to a 64-bit double
    union { uint64_t i; double d; } u; u.i = UINT64_C(0x3FF) << 52 | u64 >> 12;
    double dbl = u.d - 1.0;
    return 1 - 2.0 * ((float)(dbl / 2));
}
int randi(int mini, int maxi) { // [mini, maxi) interval ; @todo: test randi(-4,4)
    if( mini < maxi ) {
        uint32_t x, r, range = maxi - mini;
        do r = (x = rand64()) % range; while(range > (r-x));
        return mini + r;
    }
    return mini > maxi ? randi(maxi, mini) : mini;
}
double rng(void) { // [0..1) Lehmer RNG "minimal standard"
    static __thread unsigned int seed = 123;
    seed *= 16807;
    return seed / (double)0x100000000ULL;
}

// ----------------------------------------------------------------------------

// @todo: evaluate stb_perlin.h as well
float simplex1( float v ){ return snoise1(v); }
float simplex2( vec2 v ) { return snoise2(v.x,v.y); }
float simplex3( vec3 v ) { return snoise3(v.x,v.y,v.z); }
float simplex4( vec4 v ) { return snoise4(v.x,v.y,v.z,v.w); }

// ----------------------------------------------------------------------------

float ease_linear(float t) { return t; }

float ease_out_sine(float t) { return sinf(t*(C_PI*0.5f)); }
float ease_out_quad(float t) { return -(t*(t-2)); }
float ease_out_cubic(float t) { float f=t-1; return f*f*f+1; }
float ease_out_quart(float t) { float f=t-1; return f*f*f*(1-t)+1; }
float ease_out_quint(float t) { float f=(t-1); return f*f*f*f*f+1; }
float ease_out_expo(float t) { return (t >= 1) ? t : 1-powf(2,-10*t); }
float ease_out_circ(float t) { return sqrtf((2-t)*t); }
float ease_out_back(float t) { float f=1-t; return 1-(f*f*f-f*sinf(f*C_PI)); }
float ease_out_elastic(float t) { return sinf(-13*(C_PI*0.5f)*(t+1))*powf(2,-10*t)+1; }
float ease_out_bounce(float t) { return (t < 4.f/11) ? (121.f*t*t)/16 : (t < 8.f/11) ? (363.f/40*t*t)-(99.f/10*t)+17.f/5 : (t < 9.f/10) ? (4356.f/361*t*t)-(35442.f/1805*t)+16061.f/1805 : (54.f/5*t*t)-(513.f/25*t)+268.f/25; }

float ease_in_sine(float t) { return 1+sinf((t-1)*(C_PI*0.5f)); }
float ease_in_quad(float t) { return t*t; }
float ease_in_cubic(float t) { return t*t*t; }
float ease_in_quart(float t) { return t*t*t*t; }
float ease_in_quint(float t) { return t*t*t*t*t; }
float ease_in_expo(float t) { return (t <= 0) ? t : powf(2,10*(t-1)); }
float ease_in_circ(float t) { return 1-sqrtf(1-(t*t)); }
float ease_in_back(float t) { return t*t*t-t*sinf(t*C_PI); }
float ease_in_elastic(float t) { return sinf(13*(C_PI*0.5f)*t)*powf(2,10*(t-1)); }
float ease_in_bounce(float t) { return 1-ease_out_bounce(1-t); }

float ease_inout_sine(float t) { return 0.5f*(1-cosf(t*C_PI)); }
float ease_inout_quad(float t) { return (t < 0.5f) ? 2*t*t : (-2*t*t)+(4*t)-1; }
float ease_inout_cubic(float t) { float f; return (t < 0.5f) ? 4*t*t*t : (f=(2*t)-2,0.5f*f*f*f+1); }
float ease_inout_quart(float t) { float f; return (t < 0.5f) ? 8*t*t*t*t : (f=(t-1),-8*f*f*f*f+1); }
float ease_inout_quint(float t) { float f; return (t < 0.5f) ? 16*t*t*t*t*t : (f=((2*t)-2),0.5f*f*f*f*f*f+1); }
float ease_inout_expo(float t) { return (t <= 0 || t >= 1) ? t : t < 0.5f ? 0.5f*powf(2,(20*t)-10) : -0.5f*powf(2,(-20*t)+10)+1; }
float ease_inout_circ(float t) { return t < 0.5f ? 0.5f*(1-sqrtf(1-4*(t*t))) : 0.5f*(sqrtf(-((2*t)-3)*((2*t)-1))+1); }
float ease_inout_back(float t) { float f; return t < 0.5f ? (f=2*t,0.5f*(f*f*f-f*sinf(f*C_PI))) : (f=(1-(2*t-1)),0.5f*(1-(f*f*f-f*sinf(f*C_PI)))+0.5f); }
float ease_inout_elastic(float t) { return t < 0.5f ? 0.5f*sinf(13*(C_PI*0.5f)*(2*t))*powf(2,10*((2*t)-1)) : 0.5f*(sinf(-13*(C_PI*0.5f)*((2*t-1)+1))*powf(2,-10*(2*t-1))+2); }
float ease_inout_bounce(float t) { return t < 0.5f ? 0.5f*ease_in_bounce(t*2) : 0.5f*ease_out_bounce(t*2-1)+0.5f; }

float ease_inout_perlin(float t) { float t3=t*t*t,t4=t3*t,t5=t4*t; return 6*t5-15*t4+10*t3; }

float ease_ping_pong(float t, float(*fn1)(float), float(*fn2)(float)) { return t < 0.5 ? fn1(t*2) : 1-fn2((t-0.5)*2); }
float ease_pong_ping(float t, float(*fn1)(float), float(*fn2)(float)) { return 1 - ease_ping_pong(t,fn1,fn2); }

// ----------------------------------------------------------------------------

float deg      (float radians)      { return radians / C_PI * 180.0f; }
float rad      (float degrees)      { return degrees * C_PI / 180.0f; }

int   mini     (int    a, int    b) { return a < b ? a : b; }
int   maxi     (int    a, int    b) { return a > b ? a : b; }
int   absi     (int    a          ) { return a < 0 ? -a : a; }
int   clampi   (int v,int a,int b)  { return maxi(mini(b,v),a); }

float minf     (float  a, float  b) { return a < b ? a : b; }
float maxf     (float  a, float  b) { return a > b ? a : b; }
float absf     (float  a          ) { return a < 0.0f ? -a : a; }
float pmodf    (float  a, float  b) { return (a < 0.0f ? 1.0f : 0.0f) + (float)fmod(a, b); } // positive mod
float signf    (float  a)           { return (a < 0) ? -1.f : 1.f; }
float clampf(float v,float a,float b){return maxf(minf(b,v),a); }
float mixf(float a,float b,float t) { return a*(1-t)+b*t; }

// ----------------------------------------------------------------------------

vec2  ptr2     (const float *a    ) { return vec2(a[0],a[1]); }
//
vec2  neg2     (vec2   a          ) { return vec2(-a.x, -a.y); }
vec2  add2     (vec2   a, vec2   b) { return vec2(a.x + b.x, a.y + b.y); }
vec2  sub2     (vec2   a, vec2   b) { return vec2(a.x - b.x, a.y - b.y); }
vec2  mul2     (vec2   a, vec2   b) { return vec2(a.x * b.x, a.y * b.y); }
vec2  inc2     (vec2   a, float  b) { return vec2(a.x + b, a.y + b); }
vec2  dec2     (vec2   a, float  b) { return vec2(a.x - b, a.y - b); }
vec2  scale2   (vec2   a, float  b) { return vec2(a.x * b, a.y * b); }
vec2  div2     (vec2   a, float  b) { return scale2(a, b ? 1/b : 0.f); }
vec2  pmod2    (vec2   a, float  b) { return vec2(pmodf(a.x, b), pmodf(a.y, b)); }
vec2  min2     (vec2   a, vec2   b) { return vec2(minf(a.x, b.x), minf(a.y, b.y)); }
vec2  max2     (vec2   a, vec2   b) { return vec2(maxf(a.x, b.x), maxf(a.y, b.y)); }
vec2  abs2     (vec2   a          ) { return vec2(absf(a.x), absf(a.y)); }
vec2  floor2   (vec2   a          ) { return vec2(floorf(a.x), floorf(a.y)); }
vec2  fract2   (vec2   a          ) { return sub2(a, floor2(a)); }
vec2  ceil2    (vec2   a          ) { return vec2(ceilf (a.x), ceilf (a.y)); }
float dot2     (vec2   a, vec2   b) { return a.x * b.x + a.y * b.y; }
vec2  refl2    (vec2   a, vec2   b) { return sub2(a, scale2(b, 2*dot2(a,b))); }
float cross2   (vec2   a, vec2   b) { return a.x * b.y - a.y * b.x; } // pseudo cross product
float len2sq   (vec2   a          ) { return a.x * a.x + a.y * a.y; }
float len2     (vec2   a          ) { return sqrtf(len2sq(a)); }
vec2  norm2    (vec2   a          ) { return /*dot(2) == 0 ? a :*/ div2(a, len2(a)); }
int   finite2  (vec2   a          ) { return m_finite(a.x) && m_finite(a.y); }
vec2  mix2  (vec2 a,vec2 b,float t) { return add2(scale2((a),1-(t)), scale2((b), t)); }
vec2  clamp2(vec2 v,float a,float b){ return vec2(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a)); }
// ----------------------------------------------------------------------------

vec3  ptr3     (const float *a    ) { return vec3(a[0],a[1],a[2]); }
vec3  vec23    (vec2   a, float z ) { return vec3(a.x,a.y,z); }
//
vec3  neg3     (vec3   a          ) { return vec3(-a.x, -a.y, -a.z); }
vec3  add3     (vec3   a, vec3   b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
vec3  sub3     (vec3   a, vec3   b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
vec3  mul3     (vec3   a, vec3   b) { return vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
vec3  inc3     (vec3   a, float  b) { return vec3(a.x + b, a.y + b, a.z + b); }
vec3  dec3     (vec3   a, float  b) { return vec3(a.x - b, a.y - b, a.z - b); }
vec3  scale3   (vec3   a, float  b) { return vec3(a.x * b, a.y * b, a.z * b); }
vec3  div3     (vec3   a, float  b) { return scale3(a, b ? 1/b : 0.f); }
vec3  pmod3    (vec3   a, float  b) { return vec3(pmodf(a.x, b), pmodf(a.y, b), pmodf(a.z, b)); }
vec3  min3     (vec3   a, vec3   b) { return vec3(minf(a.x, b.x), minf(a.y, b.y), minf(a.z, b.z)); }
vec3  max3     (vec3   a, vec3   b) { return vec3(maxf(a.x, b.x), maxf(a.y, b.y), maxf(a.z, b.z)); }
vec3  abs3     (vec3   a          ) { return vec3(absf(a.x), absf(a.y), absf(a.z)); }
vec3  floor3   (vec3   a          ) { return vec3(floorf(a.x), floorf(a.y), floorf(a.z)); }
vec3  fract3   (vec3   a          ) { return sub3(a, floor3(a)); }
vec3  ceil3    (vec3   a          ) { return vec3(ceilf (a.x), ceilf (a.y), ceilf (a.z)); }
vec3  cross3   (vec3   a, vec3   b) { return vec3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y); }
float dot3     (vec3   a, vec3   b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
vec3  refl3    (vec3   a, vec3   b) { return sub3(a, scale3(b, 2*dot3(a, b))); }
float len3sq   (vec3   a          ) { return dot3(a,a); }
float len3     (vec3   a          ) { return sqrtf(len3sq(a)); }
vec3  norm3    (vec3   a          ) { return /*dot3(a) == 0 ? a :*/ div3(a, len3(a)); }
vec3  norm3sq  (vec3   a          ) { return /*dot3(a) == 0 ? a :*/ div3(a, len3sq(a)); }
int   finite3  (vec3   a          ) { return finite2(vec2(a.x,a.y)) && m_finite(a.z); }
vec3  mix3  (vec3 a,vec3 b,float t) { return add3(scale3((a),1-(t)), scale3((b), t)); }
vec3  clamp3(vec3 v,float a,float b){ return vec3(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a),maxf(minf(b,v.z),a)); }
//vec3 tricross3 (vec3 a, vec3 b, vec3 c) { return cross3(a,cross3(b,c)); } // useful?
void  ortho3   (vec3 *left, vec3 *up, vec3 v) {
#if 0
    if ((v.z * v.z) > (0.7f * 0.7f)) {
        float sqrlen  = v.y*v.y + v.z*v.z;
        float invlen  = 1.f / sqrtf(sqrlen);
        *up = vec3(0, v.z*invlen, -v.y*invlen);
        *left = vec3(sqrlen*invlen, -v.x*up->z, v.x*up->y);
    } else {
        float sqrlen = v.x*v.x + v.y*v.y;
        float invlen = 1.f / sqrtf(sqrlen);
        *left = vec3(-v.y*invlen, v.x*invlen, 0);
        *up = vec3(-v.z*left->y, v.z*left->x, sqrlen*invlen);
    }
#else
    *left = (v.z*v.z) < (v.x*v.x) ? vec3(v.y,-v.x,0) : vec3(0,-v.z,v.y);
    *up = cross3(*left, v);
#endif
}

#define rotateq3(v,q) transformq(q,v)
vec3 rotatex3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,1,0,0)); }
vec3 rotatey3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,0,1,0)); }
vec3 rotatez3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,0,0,1)); }

// ----------------------------------------------------------------------------

vec4  ptr4     (const float *a    ) { return vec4(a[0],a[1],a[2],a[3]); }
vec4  vec34    (vec3   a, float w ) { return vec4(a.x,a.y,a.z,w); }
//
vec4  neg4     (vec4   a          ) { return vec4(-a.x, -a.y, -a.z, -a.w); }
vec4  add4     (vec4   a, vec4   b) { return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
vec4  sub4     (vec4   a, vec4   b) { return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
vec4  mul4     (vec4   a, vec4   b) { return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
vec4  inc4     (vec4   a, float  b) { return vec4(a.x + b, a.y + b, a.z + b, a.w + b); }
vec4  dec4     (vec4   a, float  b) { return vec4(a.x - b, a.y - b, a.z - b, a.w - b); }
vec4  scale4   (vec4   a, float  b) { return vec4(a.x * b, a.y * b, a.z * b, a.w * b); }
vec4  div4     (vec4   a, float  b) { return scale4(a, b ? 1/b : 0.f); }
vec4  pmod4    (vec4   a, float  b) { return vec4(pmodf(a.x, b), pmodf(a.y, b), pmodf(a.z, b), pmodf(a.w, b)); }
vec4  min4     (vec4   a, vec4   b) { return vec4(minf(a.x, b.x), minf(a.y, b.y), minf(a.z, b.z), minf(a.w, b.w)); }
vec4  max4     (vec4   a, vec4   b) { return vec4(maxf(a.x, b.x), maxf(a.y, b.y), maxf(a.z, b.z), maxf(a.w, b.w)); }
vec4  abs4     (vec4   a          ) { return vec4(absf(a.x), absf(a.y), absf(a.z), absf(a.w)); }
vec4  floor4   (vec4   a          ) { return vec4(floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w)); }
vec4  fract4   (vec4   a          ) { return sub4(a, floor4(a)); }
vec4  ceil4    (vec4   a          ) { return vec4(ceilf (a.x), ceilf (a.y), ceilf (a.z), ceilf (a.w)); }
float dot4     (vec4   a, vec4   b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
vec4  refl4    (vec4   a, vec4   b) { return sub4(a, scale4(b, 2*dot4(a, b))); }
float len4sq   (vec4   a          ) { return dot4(a,a); }
float len4     (vec4   a          ) { return sqrtf(len4sq(a)); }
vec4  norm4    (vec4   a          ) { return /*dot4(a) == 0 ? a :*/ div4(a, len4(a)); }
vec4  norm4sq  (vec4   a          ) { return /*dot4(a) == 0 ? a :*/ div4(a, len4sq(a)); }
int   finite4  (vec4   a          ) { return finite3(vec3(a.x,a.y,a.z)) && m_finite(a.w); }
vec4  mix4  (vec4 a,vec4 b,float t) { return add4(scale4((a),1-(t)), scale4((b), t)); }
vec4  clamp4(vec4 v,float a,float b){ return vec4(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a),maxf(minf(b,v.z),a),maxf(minf(b,v.w),a)); }
// vec4 cross4(vec4 v0, vec4 v1) { return vec34(cross3(v0.xyz, v1.xyz), (v0.w + v1.w) * 0.5f); } // may fail

// ----------------------------------------------------------------------------

quat  idq      (                  ) { return quat(1,0,0,0); } // 0,0,0,1?
quat  ptrq     (const float *a    ) { return quat(a[0],a[1],a[2],a[3]); }
quat  vec3q    (vec3   a, float w ) { return quat(a.x,a.y,a.z,w); }
quat  vec4q    (vec4   a          ) { return quat(a.x,a.y,a.z,a.w); }
//
quat  negq     (quat   a          ) { return quat(-a.x,-a.y,-a.z,-a.w); }
quat  conjq    (quat   a          ) { return quat(-a.x,-a.y,-a.z,a.w); }
quat  addq     (quat   a, quat   b) { return quat(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w); }
quat  subq     (quat   a, quat   b) { return quat(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w); }
quat  mulq     (quat   p, quat   q) { vec3 w = scale3(p.xyz, q.w), r = add3(add3(cross3(p.xyz, q.xyz), w), scale3(q.xyz, p.w)); return quat(r.x,r.y,r.z,p.w*q.w - dot3(p.xyz, q.xyz)); }
quat  scaleq   (quat   a, float  s) { return quat(a.x*s,a.y*s,a.z*s,a.w*s); }
quat  normq    (quat   a          ) { vec4 v = norm4(a.xyzw); return quat(v.x,v.y,v.z,v.w); }
float dotq     (quat   a, quat   b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
quat  mixq(quat a, quat b, float t) { return normq(dotq(a,b) < 0 ? addq(negq(a),scaleq(addq(b,a),t)) : addq(a,scaleq(subq(b,a),t))); }
/* quat lerpq(quat a, quat b, float s) {
    return norm(quat((1-s)*a.x + s*b.x, (1-s)*a.y + s*b.y, (1-s)*a.z + s*b.z, (1-s)*a.w + s*b.w));
}*/
quat slerpq(quat a, quat b, float s) { //ok ?
    float t = acosf(dotq(a,b)), st = sinf(t), wa = sinf((1-s)*t)/st, wb = sinf(s*t)/st;
    return normq(quat(wa*a.x + wb*b.x, wa*a.y + wb*b.y, wa*a.z + wb*b.z, wa*a.w + wb*b.w));
}

quat  rotationq(float deg,float x,float y,float z){ deg=rad(deg)*0.5f; return vec3q(scale3(vec3(x,y,z),sinf(deg)),cosf(deg)); }
quat  mat44q   (mat44 M) {
    float r=0.f;
    int perm[] = { 0, 1, 2, 0, 1 }, *p = perm;
    for(int i = 0; i<3; i++) {
        float m = M[i*4+i];
        if( m < r ) continue;
        m = r;
        p = &perm[i];
    }
    r = sqrtf(1.f + M[p[0]*4+p[0]] - M[p[1]*4+p[1]] - M[p[2]*4+p[2]] );
    return r >= 1e-6 ? quat(1,0,0,0)
    : quat(r/2.f, (M[p[0]*4+p[1]] - M[p[1]*4+p[0]])/(2.f*r), (M[p[2]*4+p[0]] - M[p[0]*4+p[2]])/(2.f*r), (M[p[2]*4+p[1]] - M[p[1]*4+p[2]])/(2.f*r) );
}

vec3 rotate3q_2(vec3 v, quat q) { // rotate vec3 by quat @testme
    vec3 u = {q.x, q.y, q.z}; // extract the vector part of the quaternion
    float s = q.w;            // scalar part
    vec3 cuv = cross3(u, v);
    float duv2 = dot3(u, v) * 2;
    float ss_duu = s*s - dot3(u, u);
    return add3(add3(scale3(u,duv2), scale3(v,ss_duu)), scale3(cuv,2*s));
}

vec3 rotate3q(vec3 v, quat r) { // rotate vec3 by quat @testme
    float num12 = r.x + r.x;
    float num2 = r.y + r.y;
    float num = r.z + r.z;
    float num11 = r.w * num12;
    float num10 = r.w * num2;
    float num9 = r.w * num;
    float num8 = r.x * num12;
    float num7 = r.x * num2;
    float num6 = r.x * num;
    float num5 = r.y * num2;
    float num4 = r.y * num;
    float num3 = r.z * num;
    float num15 = ((v.x * ((1 - num5) - num3)) + (v.y * (num7 - num9))) + (v.z * (num6 + num10));
    float num14 = ((v.x * (num7 + num9)) + (v.y * ((1 - num8) - num3))) + (v.z * (num4 - num11));
    float num13 = ((v.x * (num6 - num10)) + (v.y * (num4 + num11))) + (v.z * ((1 - num8) - num5));
    return vec3(num15, num14, num13);
}

// euler <-> quat
vec3  euler    (quat q) { // bugs? returns PitchYawRoll (PYR) in degrees. ref: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    float sr_cp = 2*(q.x*q.y + q.z*q.w), cr_cp = 1-2*(q.y*q.y + q.z*q.z);
    float sy_cp = 2*(q.x*q.w + q.y*q.z), cy_cp = 1-2*(q.z*q.z + q.w*q.w), sp = 2*(q.x*q.z-q.w*q.y);
    float p = fabs(sp) >= 1 ? copysignf(C_PI / 2, sp) : asinf(sp);
    float y = atan2f(sy_cp, cy_cp);
    float r = atan2f(sr_cp, cr_cp);
    return scale3(vec3(p, y, r), TO_DEG);
}

quat  eulerq   (vec3 pyr_degrees) { // bugs?
#if 0
    quat x = vec3q(vec3(1,0,0),rad(pyr_degrees.x)); // x, not pitch
    quat y = vec3q(vec3(0,1,0),rad(pyr_degrees.y)); // y, not yaw
    quat z = vec3q(vec3(0,0,1),rad(pyr_degrees.z)); // z, not row
    return normq(mulq(mulq(x, y), z));
#else
    float p = rad(pyr_degrees.x), y = rad(pyr_degrees.y), r = rad(pyr_degrees.z);
    float ha = p * 0.5f, hb = r * 0.5f, hc = y * 0.5f;
    float cp = cosf(ha), sp = sinf(ha), cr = cosf(hb), sr = sinf(hb), cy = cosf(hc), sy = sinf(hc);
    return quat(cy*cr*cp + sy*sr*sp, cy*sr*cp - sy*cr*sp, cy*cr*sp + sy*sr*cp, sy*cr*cp - cy*sr*sp);
#endif
}

// ----------------------------------------------------------------------------

void scaling33(mat33 m, float x, float y, float z) { // !!! ok, i guess
    m[0] = x; m[1] = 0; m[2] = 0;
    m[3] = 0; m[4] = y; m[5] = 0;
    m[6] = 0; m[7] = 0; m[8] = z;
}
void scale33(mat33 m, float x, float y, float z) {
#if 0 // original !!! ok, i guess
    m[0] *= x; m[1] *= x; m[2] *= x;
    m[3] *= y; m[4] *= y; m[5] *= y;
    m[6] *= z; m[7] *= z; m[8] *= z;
#else
    m[0] *= x; m[3] *= x; m[6] *= x;
    m[1] *= y; m[4] *= y; m[7] *= y;
    m[2] *= z; m[5] *= z; m[8] *= z;
#endif
}
void id33(mat33 m) {
    scaling33(m, 1,1,1);
}
void extract33(mat33 m, const mat44 m4) {  // extract rot/sca from mat44
    m[0] = m4[0]; m[1] = m4[1]; m[2] = m4[ 2];
    m[3] = m4[4]; m[4] = m4[5]; m[5] = m4[ 6];
    m[6] = m4[8]; m[7] = m4[9]; m[8] = m4[10];
}
void copy33(mat33 m, const mat33 a) {
    for(int i = 0; i < 9; ++i) m[i] = a[i];
}
//
vec3 mulv33(mat33 m, vec3 v) {
    return vec3(m[0]*v.x+m[1]*v.y+m[2]*v.z,m[3]*v.x+m[4]*v.y+m[5]*v.z,m[6]*v.x+m[7]*v.y+m[8]*v.z);
}
void multiply33x2(mat33 m, const mat33 a, const mat33 b) {
    m[0] = a[0]*b[0]+a[1]*b[3]+a[2]*b[6];
    m[1] = a[0]*b[1]+a[1]*b[4]+a[2]*b[7];
    m[2] = a[0]*b[2]+a[1]*b[5]+a[2]*b[8];

    m[3] = a[3]*b[0]+a[4]*b[3]+a[5]*b[6];
    m[4] = a[3]*b[1]+a[4]*b[4]+a[5]*b[7];
    m[5] = a[3]*b[2]+a[4]*b[5]+a[5]*b[8];

    m[6] = a[6]*b[0]+a[7]*b[3]+a[8]*b[6];
    m[7] = a[6]*b[1]+a[7]*b[4]+a[8]*b[7];
    m[8] = a[6]*b[2]+a[7]*b[5]+a[8]*b[8];
}

void rotation33(mat33 m, float degrees, float x,float y,float z) {
    float radians = degrees * C_PI / 180.0f;
    float s = sinf(radians), c = cosf(radians), c1 = 1.0f - c;
    float xy = x*y, yz = y*z, zx = z*x, xs = x*s, ys = y*s, zs = z*s;
    m[0] = c1*x*x+c; m[1] = c1*xy-zs; m[2] = c1*zx+ys;
    m[3] = c1*xy+zs; m[4] = c1*y*y+c; m[5] = c1*yz-xs;
    m[6] = c1*zx-ys; m[7] = c1*yz+xs; m[8] = c1*z*z+c;
}
void rotationq33(mat33 m, quat q) {
#if  0
    float a  = q.w, b  = q.x, c  = q.y, d  = q.z;
    float a2 = a*a, b2 = b*b, c2 = c*c, d2 = d*d;
    m[ 0] = a2 + b2 - c2 - d2; m[ 1] = 2*(b*c + a*d);     m[ 2] = 2*(b*d - a*c);
    m[ 3] = 2*(b*c - a*d);     m[ 4] = a2 - b2 + c2 - d2; m[ 5] = 2*(c*d + a*b);
    m[ 6] = 2*(b*d + a*c);     m[ 7] = 2*(c*d - a*b);     m[ 8] = a2 - b2 - c2 + d2;
#else
    float x2 = q.x*q.x, y2 = q.y*q.y, z2 = q.z*q.z, w2 = q.w*q.w;
    float xz = q.x*q.z, xy = q.x*q.y, yz = q.y*q.z, wz = q.w*q.z, wy = q.w*q.y, wx = q.w*q.x;
    m[0] = 1-2*(y2+z2); m[1] =   2*(xy+wz); m[2] =   2*(xz-wy);
    m[3] =   2*(xy-wz); m[4] = 1-2*(x2+z2); m[5] =   2*(yz+wx);
    m[6] =   2*(xz+wy); m[7] =   2*(yz-wx); m[8] = 1-2*(x2+y2);
#endif
}
void rotate33(mat33 r, float degrees, float x,float y,float z) {
    if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;
    float m[9]; rotate33(m, degrees, x,y,z);
    multiply33x2(r, r, m);
}
void compose33(mat33 m, quat r, vec3 s) { // verify me
    rotationq33(m, r);
    scale33(m, s.x,s.y,s.z);
}

// ----------------------------------------------------------------------------

void id34(mat34 m) { // verify me
    m[ 0] = 1; m[ 1] = 0; m[ 2] = 0; m[ 3] = 0;
    m[ 4] = 0; m[ 5] = 1; m[ 6] = 0; m[ 7] = 0;
    m[ 8] = 0; m[ 9] = 0; m[10] = 1; m[11] = 0;
}
void copy34(mat34 m, const mat34 a) {
    for( int i = 0; i < 12; ++i ) m[i] = a[i];
}
void scale34(mat34 m, float s) {
    for( int i = 0; i < 12; ++i ) m[i] *= s;
}
void add34(mat34 m, mat34 n) {
    for( int i = 0; i < 12; ++i ) m[i] += n[i];
}
void muladd34(mat34 m, mat34 n, float s) {
    for( int i = 0; i < 12; ++i ) m[i] += n[i] * s;
}
void add34x2(mat34 m, mat34 n, mat34 o) {
    for( int i = 0; i < 12; ++i ) m[i] = n[i] + o[i];
}
void lerp34(mat34 m, mat34 n, mat34 o, float alpha) {
    for( int i = 0; i < 12; ++i ) m[i] = n[i] * (1-alpha) + o[i] * alpha;
}
void multiply34x2(mat34 m, const mat34 m0, const mat34 m1) {
    vec4 r0 = { m0[0*4+0], m0[0*4+1], m0[0*4+2], m0[0*4+3] }; // rows
    vec4 r1 = { m0[1*4+0], m0[1*4+1], m0[1*4+2], m0[1*4+3] };
    vec4 r2 = { m0[2*4+0], m0[2*4+1], m0[2*4+2], m0[2*4+3] };

    vec4 c0 = { m1[0*4+0], m1[1*4+0], m1[2*4+0], 0.0f }; // cols
    vec4 c1 = { m1[0*4+1], m1[1*4+1], m1[2*4+1], 0.0f };
    vec4 c2 = { m1[0*4+2], m1[1*4+2], m1[2*4+2], 0.0f };
    vec4 c3 = { m1[0*4+3], m1[1*4+3], m1[2*4+3], 1.0f };

    m[ 0] = dot4(r0, c0); m[ 1] = dot4(r0, c1); m[ 2] = dot4(r0, c2); m[ 3] = dot4(r0, c3);
    m[ 4] = dot4(r1, c0); m[ 5] = dot4(r1, c1); m[ 6] = dot4(r1, c2); m[ 7] = dot4(r1, c3);
    m[ 8] = dot4(r2, c0); m[ 9] = dot4(r2, c1); m[10] = dot4(r2, c2); m[11] = dot4(r2, c3);
}
void multiply34(mat34 m, const mat34 a) {
    mat34 x; copy34(x, m);
    multiply34x2(m, x, a);
}
void multiply34x3(mat34 m, const mat34 a, const mat34 b, const mat34 c) {
    mat34 x;
    multiply34x2(x, a, b);
    multiply34x2(m, x, c);
}
void compose34(mat34 m, vec3 t, quat q, vec3 s) {
    m[ 0] = s.x * (1 - 2 * q.y * q.y - 2 * q.z * q.z);
    m[ 1] = s.y * (    2 * q.x * q.y - 2 * q.w * q.z);
    m[ 2] = s.z * (    2 * q.x * q.z + 2 * q.w * q.y);
    m[ 3] = t.x;
    m[ 4] = s.x * (    2 * q.x * q.y + 2 * q.w * q.z);
    m[ 5] = s.y * (1 - 2 * q.x * q.x - 2 * q.z * q.z);
    m[ 6] = s.z * (    2 * q.y * q.z - 2 * q.w * q.x);
    m[ 7] = t.y;
    m[ 8] = s.x * (    2 * q.x * q.z - 2 * q.w * q.y);
    m[ 9] = s.y * (    2 * q.y * q.z + 2 * q.w * q.x);
    m[10] = s.z * (1 - 2 * q.x * q.x - 2 * q.y * q.y);
    m[11] = t.z;
}
void invert34(mat34 m, const mat34 o) {
    vec3 a = norm3sq(vec3(o[0*4+0], o[1*4+0], o[2*4+0]));
    vec3 b = norm3sq(vec3(o[0*4+1], o[1*4+1], o[2*4+1]));
    vec3 c = norm3sq(vec3(o[0*4+2], o[1*4+2], o[2*4+2]));
    vec3 trans = vec3(o[0*4+3], o[1*4+3], o[2*4+3]);
    vec4 A = vec34(a, -dot3(a, trans));
    vec4 B = vec34(b, -dot3(b, trans));
    vec4 C = vec34(c, -dot3(c, trans));
    m[ 0] = A.x; m[ 1] = A.y; m[ 2] = A.z; m[ 3] = A.w;
    m[ 4] = B.x; m[ 5] = B.y; m[ 6] = B.z; m[ 7] = B.w;
    m[ 8] = C.x; m[ 9] = C.y; m[10] = C.z; m[11] = C.w;
}

// ----------------------------------------------------------------------------

void scaling44(mat44 m, float x, float y, float z);
void id44(mat44 m) {
    scaling44(m, 1,1,1);
}
void identity44(mat44 m) {
    scaling44(m, 1,1,1);
}
void copy44(mat44 m, const mat44 a) {
    for( int i = 0; i < 16; ++i ) m[i] = a[i];
}
void multiply44x2(mat44 m, const mat44 a, const mat44 b) {
    for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
    m[y*4+x] = a[x] * b[y*4]+a[4+x] * b[y*4+1]+a[8+x] * b[y*4+2]+a[12+x] * b[y*4+3];
}
void multiply44x3(mat44 m, const mat44 a, const mat44 b, const mat44 c) {
    mat44 x;
    multiply44x2(x, a, b);
    multiply44x2(m, x, c);
}
void multiply44(mat44 m, const mat44 a) {
    mat44 b; copy44(b, m);
    multiply44x2(m, b, a);
}
// ---
void ortho44(mat44 m, float l, float r, float b, float t, float n, float f) {
    m[ 0] = 2/(r-l);      m[ 1] = 0;            m[ 2] = 0;            m[ 3] = 0;
    m[ 4] = 0;            m[ 5] = 2/(t-b);      m[ 6] = 0;            m[ 7] = 0;
    m[ 8] = 0;            m[ 9] = 0;            m[10] = -2/(f-n);     m[11] = 0;
    m[12] = -(r+l)/(r-l); m[13] = -(t+b)/(t-b); m[14] = -(f+n)/(f-n); m[15] = 1;
}
void frustum44(mat44 m, float l, float r, float b, float t, float n, float f) {
    m[ 0] = 2*n/(r-l);   m[ 1] = 0;           m[ 2] = 0;               m[ 3] = 0;
    m[ 4] = 0;           m[ 5] = 2*n/(t-b);   m[ 6] = 0;               m[ 7] = 0;
    m[ 8] = (r+l)/(r-l); m[ 9] = (t+b)/(t-b); m[10] = -(f+n)/(f-n);    m[11] = -1;
    m[12] = 0;           m[13] = 0;           m[14] = -2*(f*n)/(f-n);  m[15] = 0;
}
void perspective44(mat44 m, float fovy_degrees, float aspect, float nearp, float farp) {
    float y = tanf(fovy_degrees * C_PI / 360) * nearp, x = y * aspect;
    frustum44(m, -x, x, -y, y, nearp, farp);
}
void lookat44(mat44 m, vec3 eye, vec3 center, vec3 up) {
    vec3 f = norm3(sub3(center, eye));
    vec3 r = norm3(cross3(f, up));
    vec3 u = cross3(r, f);
    m[ 0] = r.x;           m[ 1] = u.x;           m[ 2] = -f.x;         m[ 3] = 0;
    m[ 4] = r.y;           m[ 5] = u.y;           m[ 6] = -f.y;         m[ 7] = 0;
    m[ 8] = r.z;           m[ 9] = u.z;           m[10] = -f.z;         m[11] = 0;
    m[12] = -dot3(r, eye); m[13] = -dot3(u, eye); m[14] = dot3(f, eye); m[15] = 1;
}
// ---
void translation44(mat44 m, float x, float y, float z) { // identity4 + translate4
    m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
    m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
    m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = x;    m[13] = y;    m[14] = z;    m[15] = 1.0f;
}
void translate44(mat44 m, float x, float y, float z) { // translate in place
#if 0 // original
    vec4 t = {x, y, z, 0};
    m[12] += dot4(vec4(m[0],m[4],m[ 8],m[12]),t); // row4(M,0)
    m[13] += dot4(vec4(m[1],m[5],m[ 9],m[13]),t); // row4(M,1)
    m[14] += dot4(vec4(m[2],m[6],m[10],m[14]),t); // row4(M,2)
    m[15] += dot4(vec4(m[3],m[7],m[11],m[15]),t); // row4(M,3)
#else
    m[12] += m[ 0]*x + m[ 4]*y + m[ 8]*z;
    m[13] += m[ 1]*x + m[ 5]*y + m[ 9]*z;
    m[14] += m[ 2]*x + m[ 6]*y + m[10]*z;
    m[15] += m[ 3]*x + m[ 7]*y + m[11]*z;
#endif
}
void relocate44(mat44 m, float x, float y, float z) {
    m[12] = x; m[13] = y; m[14] = z;
}
void rotationq44(mat44 m, quat q) {
#if  0
    float a  = q.w, b  = q.x, c  = q.y, d  = q.z;
    float a2 = a*a, b2 = b*b, c2 = c*c, d2 = d*d;
    m[ 0] = a2 + b2 - c2 - d2; m[ 1] = 2*(b*c + a*d);     m[ 2] = 2*(b*d - a*c);     m[ 3] = 0;
    m[ 4] = 2*(b*c - a*d);     m[ 5] = a2 - b2 + c2 - d2; m[ 6] = 2*(c*d + a*b);     m[ 7] = 0;
    m[ 8] = 2*(b*d + a*c);     m[ 9] = 2*(c*d - a*b);     m[10] = a2 - b2 - c2 + d2; m[11] = 0;
    m[12] = 0;                 m[13] = 0;                 m[14] = 0;                 m[15] = 1;
#else
    float x2 = q.x*q.x, y2 = q.y*q.y, z2 = q.z*q.z, w2 = q.w*q.w;
    float xz = q.x*q.z, xy = q.x*q.y, yz = q.y*q.z, wz = q.w*q.z, wy = q.w*q.y, wx = q.w*q.x;
    m[ 0] = 1-2*(y2+z2); m[ 1] =   2*(xy+wz); m[ 2] =   2*(xz-wy); m[ 3] = 0;
    m[ 4] =   2*(xy-wz); m[ 5] = 1-2*(x2+z2); m[ 6] =   2*(yz+wx); m[ 7] = 0;
    m[ 8] =   2*(xz+wy); m[ 9] =   2*(yz-wx); m[10] = 1-2*(x2+y2); m[11] = 0;
    m[12] = 0;           m[13] = 0;           m[14] = 0;           m[15] = 1;
#endif
}
void rotation44(mat44 m, float degrees, float x, float y, float z) {
    //if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;

    float radians = degrees * C_PI / 180.0f;
    float c = cosf(radians), s = sinf(radians), c1 = 1.0f - c;
    m[ 0] = x*x*c1 + c;   m[ 1] = y*x*c1 + z*s; m[ 2] = x*z*c1 - y*s; m[ 3] = 0.0f;
    m[ 4] = x*y*c1 - z*s; m[ 5] = y*y*c1 + c;   m[ 6] = y*z*c1 + x*s; m[ 7] = 0.0f;
    m[ 8] = x*z*c1 + y*s; m[ 9] = y*z*c1 - x*s; m[10] = z*z*c1 + c;   m[11] = 0.0f;
    m[12] = 0.0f;         m[13] = 0.0f;         m[14] = 0.0f;         m[15] = 1.0f;
}
void rotate44(mat44 m, float degrees, float x, float y, float z) { // !!! ok, i guess
    if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;

    float radians = degrees * C_PI / 180.0f;
    float c = cosf(radians), s = -sinf(radians), c1 = 1 - c;
    float m00 = m[ 0],  m01 = m[ 1],  m02 = m[ 2], m03 = m[ 3],
          m04 = m[ 4],  m05 = m[ 5],  m06 = m[ 6], m07 = m[ 7],
          m08 = m[ 8],  m09 = m[ 9],  m10 = m[10], m11 = m[11];

    // rotation matrix
    float r00 = x*x*c1 + c,   r01 = y*x*c1 + z*s, r02 = x*z*c1 - y*s;
    float r04 = x*y*c1 - z*s, r05 = y*y*c1 + c,   r06 = y*z*c1 + x*s;
    float r08 = x*z*c1 + y*s, r09 = y*z*c1 - x*s, r10 = z*z*c1 + c;

    // multiply
    m[ 0] = r00 * m00 + r04 * m04 + r08 * m08;
    m[ 1] = r00 * m01 + r04 * m05 + r08 * m09;
    m[ 2] = r00 * m02 + r04 * m06 + r08 * m10;
    m[ 3] = r00 * m03 + r04 * m07 + r08 * m11;
    m[ 4] = r01 * m00 + r05 * m04 + r09 * m08;
    m[ 5] = r01 * m01 + r05 * m05 + r09 * m09;
    m[ 6] = r01 * m02 + r05 * m06 + r09 * m10;
    m[ 7] = r01 * m03 + r05 * m07 + r09 * m11;
    m[ 8] = r02 * m00 + r06 * m04 + r10 * m08;
    m[ 9] = r02 * m01 + r06 * m05 + r10 * m09;
    m[10] = r02 * m02 + r06 * m06 + r10 * m10;
    m[11] = r02 * m03 + r06 * m07 + r10 * m11;
}
void scaling44(mat44 m, float x, float y, float z) { // !!! ok, i guess
    m[ 0] = x; m[ 1] = 0; m[ 2] = 0; m[ 3] = 0;
    m[ 4] = 0; m[ 5] = y; m[ 6] = 0; m[ 7] = 0;
    m[ 8] = 0; m[ 9] = 0; m[10] = z; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}
void scale44(mat44 m, float x, float y, float z) {
#if 0 // original !!! ok, i guess
    m[ 0] *= x; m[ 1] *= x; m[ 2] *= x; m[ 3] *= x;
    m[ 4] *= y; m[ 5] *= y; m[ 6] *= y; m[ 7] *= y;
    m[ 8] *= z; m[ 9] *= z; m[10] *= z; m[11] *= z;
#else
    m[0] *= x; m[4] *= x; m[8] *= x;
    m[1] *= y; m[5] *= y; m[9] *= y;
    m[2] *= z; m[6] *= z; m[10] *= z;
#endif
}
// ---
void transpose44(mat44 m, const mat44 a) { // M[i][j] = A[j][i];
    m[ 0] = a[0]; m[ 1] = a[4]; m[ 2] = a[ 8]; m[ 3] = a[12];
    m[ 4] = a[1]; m[ 5] = a[5]; m[ 6] = a[ 9]; m[ 7] = a[13];
    m[ 8] = a[2]; m[ 9] = a[6]; m[10] = a[10]; m[11] = a[14];
    m[12] = a[3]; m[13] = a[7]; m[14] = a[11]; m[15] = a[15];
}
float det44(const mat44 M) { // !!! ok, i guess
    float s[6], c[6];
    s[0] = M[0*4+0]*M[1*4+1] - M[1*4+0]*M[0*4+1];
    s[1] = M[0*4+0]*M[1*4+2] - M[1*4+0]*M[0*4+2];
    s[2] = M[0*4+0]*M[1*4+3] - M[1*4+0]*M[0*4+3];
    s[3] = M[0*4+1]*M[1*4+2] - M[1*4+1]*M[0*4+2];
    s[4] = M[0*4+1]*M[1*4+3] - M[1*4+1]*M[0*4+3];
    s[5] = M[0*4+2]*M[1*4+3] - M[1*4+2]*M[0*4+3];

    c[0] = M[2*4+0]*M[3*4+1] - M[3*4+0]*M[2*4+1];
    c[1] = M[2*4+0]*M[3*4+2] - M[3*4+0]*M[2*4+2];
    c[2] = M[2*4+0]*M[3*4+3] - M[3*4+0]*M[2*4+3];
    c[3] = M[2*4+1]*M[3*4+2] - M[3*4+1]*M[2*4+2];
    c[4] = M[2*4+1]*M[3*4+3] - M[3*4+1]*M[2*4+3];
    c[5] = M[2*4+2]*M[3*4+3] - M[3*4+2]*M[2*4+3];

    return ( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
}
bool invert44(mat44 T, const mat44 M) { // !!! ok, i guess
    float s[6], c[6];
    s[0] = M[0*4+0]*M[1*4+1] - M[1*4+0]*M[0*4+1];
    s[1] = M[0*4+0]*M[1*4+2] - M[1*4+0]*M[0*4+2];
    s[2] = M[0*4+0]*M[1*4+3] - M[1*4+0]*M[0*4+3];
    s[3] = M[0*4+1]*M[1*4+2] - M[1*4+1]*M[0*4+2];
    s[4] = M[0*4+1]*M[1*4+3] - M[1*4+1]*M[0*4+3];
    s[5] = M[0*4+2]*M[1*4+3] - M[1*4+2]*M[0*4+3];

    c[0] = M[2*4+0]*M[3*4+1] - M[3*4+0]*M[2*4+1];
    c[1] = M[2*4+0]*M[3*4+2] - M[3*4+0]*M[2*4+2];
    c[2] = M[2*4+0]*M[3*4+3] - M[3*4+0]*M[2*4+3];
    c[3] = M[2*4+1]*M[3*4+2] - M[3*4+1]*M[2*4+2];
    c[4] = M[2*4+1]*M[3*4+3] - M[3*4+1]*M[2*4+3];
    c[5] = M[2*4+2]*M[3*4+3] - M[3*4+2]*M[2*4+3];

    float det = ( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
    if( !det ) return false;
    float idet = 1.0f / det;

    T[0*4+0] = ( M[1*4+1] * c[5] - M[1*4+2] * c[4] + M[1*4+3] * c[3]) * idet;
    T[0*4+1] = (-M[0*4+1] * c[5] + M[0*4+2] * c[4] - M[0*4+3] * c[3]) * idet;
    T[0*4+2] = ( M[3*4+1] * s[5] - M[3*4+2] * s[4] + M[3*4+3] * s[3]) * idet;
    T[0*4+3] = (-M[2*4+1] * s[5] + M[2*4+2] * s[4] - M[2*4+3] * s[3]) * idet;

    T[1*4+0] = (-M[1*4+0] * c[5] + M[1*4+2] * c[2] - M[1*4+3] * c[1]) * idet;
    T[1*4+1] = ( M[0*4+0] * c[5] - M[0*4+2] * c[2] + M[0*4+3] * c[1]) * idet;
    T[1*4+2] = (-M[3*4+0] * s[5] + M[3*4+2] * s[2] - M[3*4+3] * s[1]) * idet;
    T[1*4+3] = ( M[2*4+0] * s[5] - M[2*4+2] * s[2] + M[2*4+3] * s[1]) * idet;

    T[2*4+0] = ( M[1*4+0] * c[4] - M[1*4+1] * c[2] + M[1*4+3] * c[0]) * idet;
    T[2*4+1] = (-M[0*4+0] * c[4] + M[0*4+1] * c[2] - M[0*4+3] * c[0]) * idet;
    T[2*4+2] = ( M[3*4+0] * s[4] - M[3*4+1] * s[2] + M[3*4+3] * s[0]) * idet;
    T[2*4+3] = (-M[2*4+0] * s[4] + M[2*4+1] * s[2] - M[2*4+3] * s[0]) * idet;

    T[3*4+0] = (-M[1*4+0] * c[3] + M[1*4+1] * c[1] - M[1*4+2] * c[0]) * idet;
    T[3*4+1] = ( M[0*4+0] * c[3] - M[0*4+1] * c[1] + M[0*4+2] * c[0]) * idet;
    T[3*4+2] = (-M[3*4+0] * s[3] + M[3*4+1] * s[1] - M[3*4+2] * s[0]) * idet;
    T[3*4+3] = ( M[2*4+0] * s[3] - M[2*4+1] * s[1] + M[2*4+2] * s[0]) * idet;
    return true;
}

vec4 transform444(const mat44, const vec4);
bool unproject44(vec3 *out, vec3 xyd, vec4 viewport, mat44 mvp) {
    // xyd: usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    // src: https://www.khronos.org/opengl/wiki/GluProject_and_gluUnProject_code
    mat44 inv_mvp;
    if( invert44(inv_mvp, mvp) ) {
        vec4 in = vec4( (xyd.x-viewport.x)/viewport.z*2-1, (xyd.y-viewport.y)/viewport.w*2-1, 2*xyd.z-1, 1 );
        vec4 p = transform444(inv_mvp, in);
        if( p.w != 0 ) {
            p.w = 1.f/p.w;
            *out = vec3(p.x*p.w,p.y*p.w,p.z*p.w);
            return true;
        }
    }
    return false;
}

vec3 transform_axis(const coord_system, const AXIS_ENUMS);
void rebase44(mat44 m, const coord_system src_basis, const coord_system dst_basis) {
    vec3 v1 = transform_axis(src_basis, dst_basis.x);
    vec3 v2 = transform_axis(src_basis, dst_basis.y);
    vec3 v3 = transform_axis(src_basis, dst_basis.z);
    m[ 0] = v1.x; m[ 1] = v1.y; m[ 2] = v1.z; m[ 3] = 0;
    m[ 4] = v2.x; m[ 5] = v2.y; m[ 6] = v2.z; m[ 7] = 0;
    m[ 8] = v3.x; m[ 9] = v3.y; m[10] = v3.z; m[11] = 0;
    m[12] =    0; m[13] =    0; m[14] =    0; m[15] = 1;
}


void compose44(mat44 m, vec3 t, quat q, vec3 s) {
#if 0
    // quat to rotation matrix
    m[0] = 1 - 2 * (q.y * q.y + q.z * q.z);
    m[1] = 2 * (q.x * q.y + q.z * q.w);
    m[2] = 2 * (q.x * q.z - q.y * q.w);

    m[4] = 2 * (q.x * q.y - q.z * q.w);
    m[5] = 1 - 2 * (q.x * q.x + q.z * q.z);
    m[6] = 2 * (q.y * q.z + q.x * q.w);

    m[8] = 2 * (q.x * q.z + q.y * q.w);
    m[9] = 2 * (q.y * q.z - q.x * q.w);
    m[10] = 1 - 2 * (q.x * q.x + q.y * q.y);

    // scale matrix
    m[0] *= s.x; m[4] *= s.x; m[8] *= s.x;
    m[1] *= s.y; m[5] *= s.y; m[9] *= s.y;
    m[2] *= s.z; m[6] *= s.z; m[10] *= s.z;

    // set translation
    m[12] = t.x; m[13] = t.y; m[14] = t.z;

    m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
#else
    rotationq44(m,q);
    scale44(m,s.x,s.y,s.z);
    relocate44(m,t.x,t.y,t.z);
//    relocate44(m,t.x,t.y,t.z); // ok?
//    scale44(m,s.x,s.y,s.z); // ok?

    m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
#endif
}

// ----------------------------------------------------------------------------

vec3 transform33(mat33 m, vec3 p) {
    float x = (m[0] * p.x) + (m[4] * p.y) + (m[ 8] * p.z);
    float y = (m[1] * p.x) + (m[5] * p.y) + (m[ 9] * p.z);
    float z = (m[2] * p.x) + (m[6] * p.y) + (m[10] * p.z);
    return vec3(x,y,z);
}

vec4 transform444(const mat44 m, const vec4 p) {
    // remember w = 1 for move in space; w = 0 rotate in space;
    float x = m[0]*p.x + m[4]*p.y + m[ 8]*p.z + m[12]*p.w;
    float y = m[1]*p.x + m[5]*p.y + m[ 9]*p.z + m[13]*p.w;
    float z = m[2]*p.x + m[6]*p.y + m[10]*p.z + m[14]*p.w;
    float w = m[3]*p.x + m[7]*p.y + m[11]*p.z + m[15]*p.w;
    return vec4(x,y,z,w);
}

vec3 transform344(const mat44 m, const vec3 p) {
    vec4 v = transform444(m, vec34(p, 1));
    return scale3(v.xyz, 1.f / v.w);
}

vec3 transformq(const quat q, const vec3 v) {  // !!! ok, i guess
    // [src] https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion (laurent couvidou)
    // Extract the vector part of the quaternion
    vec3 u = vec3(q.x, q.y, q.z);
    // Extract the scalar part of the quaternion
    float s = q.w;
    // Do the math
    vec3 a = scale3(u, 2 * dot3(u,v));
    vec3 b = scale3(v, s*s - dot3(u,u));
    vec3 c = scale3(cross3(u,v), 2*s);
    return add3(a, add3(b,c));
}

vec3 transform_axis(const coord_system basis, const AXIS_ENUMS to) {
    const float dot_table[6][6] = {
        {+1,-1,0,0,0,0},{-1,+1,0,0,0,0},{0,0,+1,-1,0,0},
        {0,0,-1,+1,0,0},{0,0,0,0,+1,-1},{0,0,0,0,-1,+1},
    };
    return vec3( dot_table[basis.x][to], dot_table[basis.y][to], dot_table[basis.z][to] );
}

// A vector is the difference between two points in 3D space, possessing both direction and magnitude
vec3 transform_vector  (const mat44 m, const vec3 vector)   {
    return transform344(m, vector);
}

// A point is a specific location within a 3D space
vec3 transform_point   (const mat44 m, const vec3 p)    { // return (m * vec4{point,1).xyz()/r.w;
    float inv = 1.0f / (m[3+4*0]*p.x + m[3+4*1]*p.y + m[3+4*2]*p.z + m[3+4*3]);
    return vec3(
        (m[0+4*0]*p.x + m[0+4*1]*p.y + m[0+4*2]*p.z + m[0+4*3]) * inv,
        (m[1+4*0]*p.x + m[1+4*1]*p.y + m[1+4*2]*p.z + m[1+4*3]) * inv,
        (m[2+4*0]*p.x + m[2+4*1]*p.y + m[2+4*2]*p.z + m[2+4*3]) * inv
    );
}

// A tangent is a unit-length vector which is parallel to a piece of geometry, such as a surface or a curve
vec3 transform_tangent (const mat44 m, const vec3 tangent)  { return norm3(transform_vector(m, tangent)); }

// A normal is a unit-length bivector which is perpendicular to a piece of geometry, such as a surface or a curve
vec3 transform_normal  (const mat44 m, const vec3 normal)   {
    return transform_tangent(m, normal); // ok?

    mat44 t; transpose44(t,m); mat44 i; invert44(i,t);
    return scale3(norm3(transform_vector(i, normal)), det44(m) < 0 ? -1.f : 1.f);
}

// A quaternion can describe both a rotation and a uniform scaling in 3D space
quat transform_quat     (const mat44 m, const quat q)      {
    vec3 s = scale3(transform_vector(m, q.xyz), det44(m) < 0 ? -1.f : 1.f);
    return quat(s.x,s.y,s.z,q.w);
}

// A matrix can describe a general transformation of homogeneous coordinates in projective space
float* transform_matrix(mat44 out, const mat44 m, const mat44 matrix) {
    mat44 I; invert44(I, m);
    multiply44x3(out, I, matrix, m); // m,matrix,I instead ?
    return out;
}

// Scaling factors are not a vector, they are a compact representation of a scaling matrix
vec3 transform_scaling (const mat44 m, const vec3 scaling) {
    mat44 s; scaling44(s, scaling.x, scaling.y, scaling.z);
    mat44 out; transform_matrix(out, m, s);
    return vec3( out[0], out[5], out[10] );
}

// ----------------------------------------------------------------------------
// !!! for debugging

#include <stdio.h>
void print_( float *m, int ii, int jj ) {
    for( int j = 0; j < jj; ++j ) {
        for( int i = 0; i < ii; ++i ) printf("%8.3f", *m++);
//        puts("");
    }
//    puts("---");
}
void print2( vec2 v ) { print_(&v.x,2,1); }
void print3( vec3 v ) { print_(&v.x,3,1); }
void print4( vec4 v ) { print_(&v.x,4,1); }
void printq( quat q ) { print_(&q.x,4,1); }
void print33( float *m ) { print_(m,3,3); }
void print34( float *m ) { print_(m,3,4); }
void print44( float *m ) { print_(m,4,4); }
#line 0
#line 1 "fwk_memory.c"
size_t dlmalloc_usable_size(void*); // __ANDROID_API__

#if is(bsd) || is(osx) // bsd or osx
#  include <malloc/malloc.h>
#else
#  include <malloc.h>
#endif

#ifndef SYS_REALLOC
#define SYS_REALLOC realloc
#define SYS_MSIZE /* bsd/osx, then win32, then ems/__ANDROID_API__, then __GLIBC__ */ \
    ifdef(osx, malloc_size, ifdef(bsd, malloc_size, \
        ifdef(win32, _msize, \
            ifdef(ems, dlmalloc_usable_size, malloc_usable_size))))
#endif

// xrealloc --------------------------------------------------------------------

void* xrealloc(void* oldptr, size_t size) {
    void *ptr = SYS_REALLOC(oldptr, size);
    if( !ptr && size ) {
        PANIC("Not memory enough (trying to allocate %u bytes)", (unsigned)size);
    }
#if WITH_XREALLOC_POISON
    if( !oldptr && size ) {
        memset(ptr, 0xCD, size);
    }
#endif
    return ptr;
}
size_t xsize(void* p) {
    if( p ) return SYS_MSIZE(p);
    return 0;
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) { // use negative bytes to rewind stack
    static local uint8_t *stack_mem = 0;
    static local uint64_t stack_ptr = 0;
    static local uint64_t stack_max = 0; // watch this var, in case you want to fine tune 4 MiB value below
    if( bytes < 0 ) {
        if( stack_ptr > stack_max ) stack_max = stack_ptr;
        return (stack_ptr = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xsize(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_ptr += bytes) - bytes ];
}

// leaks ----------------------------------------------------------------------

void* watch( void *ptr, int sz ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        for( FILE *fp = fopen(buf, "a+"); fp; fclose(fp), fp = 0 ) {
            fseek(fp, 0L, SEEK_END);
            char *callstack(int);
            const char *cs = callstack( +48 ); // NULL; // =
            fprintf(fp, "Built %s %s\n", __DATE__, __TIME__); // today() instead?
            fprintf(fp, "Memleak address: [%p], size: %d\n%s\n", ptr, sz, cs ? cs : "No callstack.");
        }
    }
    return ptr;
}
void* forget( void *ptr ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        unlink(buf);
    }
    return ptr;
}
#line 0
#line 1 "fwk_network.c"
int download( FILE *out, const char *url ) {
    bool ok = false;
    if( out ) for( https_t *h = https_get(url, NULL); h; https_release(h), h = NULL ) {
        while (!https_process(h)) sleep_ms(1);
        //printf("%d %s\n\n%.*s\n", h->status_code, h->content_type, (int)h->response_size, (char*)h->response_data);
        ok = fwrite(h->response_data, 1, h->response_size, out) == 1;
    }
    return ok;
}

int portname( const char *service_name, unsigned retries ) {
    // Determine port for a given service based on hash of its name.
    // If port cant be reached, client should retry with next hash.
    // Algorithm: fnv1a(name of service) -> splitmix64 num retries -> remap bucket as [min..max] ports.

    // hash64
    uint64_t hash = 14695981039346656037ULL;
    while( *service_name ) {
        hash = ( (unsigned char)*service_name++ ^ hash ) * 0x100000001b3ULL;
    }

    // splitmix64
    for( unsigned i = 0; i < retries; ++i ) {
        uint64_t h = (hash += UINT64_C(0x9E3779B97F4A7C15));
        h = (h ^ (h >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        h = (h ^ (h >> 27)) * UINT64_C(0x94D049BB133111EB);
        h = (h ^ (h >> 31));
        hash = h;
    }

    // See dynamic ports: https://en.wikipedia.org/wiki/Ephemeral_port
    // So, excluded ranges: 32768..60999 (linux), 49152..65535 (freebsd+vista+win7), 1024..5000 (winsrv2003+bsd)
    // Output range: [5001..32724], in 4096 steps
    return ((hash & 0xFFF) * 677 / 100 + 5001);
}

static
void netdump( const void *ptr, int len ) {
    char hexbuf[256] = {0}, strbuf[256] = {0}, *data = (char*)ptr, width = 16;
    for( int jt = 0; jt < len; jt += width ) {
        char *hex = hexbuf, *str = strbuf;
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", (unsigned char)*data);
            str += sprintf( str, "%c", *data >= 32 && *data != '\\' ? *data : '.');
        }
        printf("%06x %-*s%s\n", jt, width*3, hexbuf, strbuf);
    }
}

// -----------------------------------------------------------------------------

#define UDP_DEBUG 0

static int udp_init() {
    do_once {
        int rc = swrapInit(); // atexit(swrapTerminate);
        if( rc ) PANIC("udp_init: swrapInit error");
    }
    return 1;
}

int udp_open(const char *address, const char *port) {
    int fd = swrapSocket(SWRAP_UDP, SWRAP_CONNECT, 0, address, port);
    if( fd == -1 ) PANIC("udp_open: swrapSocket error");
    return fd;
}

int udp_bind(const char *address, const char *port) {
    int fd = swrapSocket(SWRAP_UDP, SWRAP_BIND, 0, address, port);
    if( fd == -1 ) PANIC("udp_bind: swrapSocket error");
    return fd;
}

int udp_send( int fd, const void *buf, int len ) { // @todo: return 1 if succesfully sent instead?
    int rc = swrapSend(fd, (const char *)buf, len);
    if( rc == -1 ) return -1; //PANIC("udp_send: swrapSend error");
    return rc;
}

// use socket to send data to another address
int udp_sendto( int fd, const char *ip, const char *port, const void *buf, int len ) { // return number of bytes sent
#if 0
    int rc = swrapSendTo(fd, struct swrap_addr*, (const char*)buf, len);
    if( rc == -1 ) return -1; //PANIC("udp_send: swrapSend error");
    return rc;
#else
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    // use inet_addr. tcc(win32) wont work otherwise.
    addr.sin_addr.s_addr = inet_addr(ip); // inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(atoi(port));

    int n = sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    return n < 0 ? -1 : n;
#endif
}

int udp_peek( int fd ) { // <0 error, 0 timeout, >0 data
    int rc = swrapSelect(fd, 0.00001);
    if( rc < 0 ) return -1; // PANIC("udp_peek: swrapSelect error");
    if( rc == 0 ) return 0; // timeout
    return 1; //> 0: new data is available
}

int udp_recv( int fd, void *buf, int len ) { // <0 error, 0 orderly shutdown, >0 received bytes
    struct swrap_addr sa = {0};
    int rc = swrapReceiveFrom(fd, &sa, buf, len);
    if( rc < 0 ) return -1; // PANIC("udp_recv: swrapReceiveFrom error");
    if( rc == 0 ) return 0; // orderly shutdown

#if UDP_DEBUG
    char host[128], serv[128];
    int rc2 = swrapAddressInfo(&sa, host, 128, serv, 128 );
    if( rc2 != 0 ) PANIC("swrapAddressInfo error");
    printf("server: recv %d bytes from %s:%s : %.*s\n", rc, host, serv, ec, buf );
    netdump(buf, rc);
#endif

    return rc;
}

// -----------------------------------------------------------------------------

#define TCP_DEBUG 1

#if TCP_DEBUG
set(int) tcp_set;
#endif

void tcp_init(void) {
    do_once {
        udp_init();
        // int rc = swrapInit(); /*atexit(swrapTerminate);*/
        // if( rc ) PANIC("tcp_init: swrapInit error");
#if TCP_DEBUG
        set_init(tcp_set, less_int, hash_int);
#endif
    }
}
int tcp_open(const char *address, const char *port) {
    tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_CONNECT, 0/*|SWRAP_NODELAY*/, address, port);
    return fd;
}
int tcp_bind(const char *interface_, const char *port, int backlog) {
    tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_BIND, 0/*|SWRAP_NODELAY*//*|SWRAP_NOBLOCK*/, interface_, port);
    if( fd >= 0 ) swrapListen(fd, backlog);
    return fd;
}
int tcp_peek(int fd, int(*callback)(int)) {
    struct swrap_addr sa;
    int fd2 = swrapAccept(fd, &sa);
    if( fd2 >= 0 ) return callback(fd2);
    return -1;
}
int tcp_send(int fd, const void *buf, int len) {
    int rc = swrapSend(fd, (const char *)buf, len);
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        printf("send -> %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
int tcp_recv(int fd, void *buf, int len) {
    int rc = swrapReceive(fd, (char*)buf, len);
#if TCP_DEBUG
    if( rc != 0 && set_find(tcp_set, fd) ) {
        printf("recv <- %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
char* tcp_host(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf);
}
char* tcp_port(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf+512);
}
int tcp_close(int fd) {
    swrapClose(fd);
    return 0;
}
int tcp_debug(int fd) {
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        set_erase(tcp_set, fd);
        return 0;
    } else {
        set_insert(tcp_set, fd);
        return 1;
    }
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------------

void network_init() {
    udp_init();
    tcp_init();
}
#line 0
#line 1 "fwk_render.c"
// -----------------------------------------------------------------------------
// opengl

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C
//
void glDebugCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char * message, void * userdata) {
    // whitelisted codes (also: 131169, 131204).
    if( id == 131154 ) return; // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    if( id == 131185 ) return; // Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if( id == 131218 ) return; // Program/shader state performance warning: Vertex shader in program 9 is being recompiled based on GL state.
    if( id == 2 ) return; // INFO: API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change. [ID: 2]

    const char * GL_ERROR_TYPE[] = { "ERROR", "DEPRECATED BEHAVIOR", "UNDEFINED DEHAVIOUR", "PORTABILITY", "PERFORMANCE", "OTHER" };
    const char * GL_ERROR_SOURCE[] = { "API", "WINDOW SYSTEM", "SHADER COMPILER", "THIRD PARTY", "APPLICATION", "OTHER" };
    const char * GL_ERROR_SEVERITY[] = { "HIGH", "MEDIUM", "LOW", "NOTIFICATION" };

    type = type - GL_DEBUG_TYPE_ERROR;
    source = source - GL_DEBUG_SOURCE_API;
    severity = severity == GL_DEBUG_SEVERITY_NOTIFICATION ? 3 : severity - GL_DEBUG_SEVERITY_HIGH;
    if(severity >= 2) return; // do not log low_severity or notifications

    PRINTF( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
//  PANIC( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
}
void glDebugEnable() {
    do_once {
    typedef void (*GLDEBUGPROC)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t, const char *, const void *);
    typedef void (*GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
        void *func = glfwGetProcAddress("glDebugMessageCallback");
    void (*glDebugMessageCallback)(GLDEBUGPROC, const void *) = (GLDEBUGMESSAGECALLBACKPROC)func;
        if( func ) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallback((GLDEBUGPROC)glDebugCallback, NULL);
        }
    }
}

static
void glCopyBackbufferToTexture( texture_t *tex ) { // unused
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width(), window_height(), 0 );
}

void glNewFrame() {
    glViewport(0, 0, window_width(), window_height());
    //glClearColor(0,0,0,1);
    //glClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    // blending defaults
    glEnable(GL_BLEND);

    // culling defaults
//  glEnable(GL_CULL_FACE);
//  glCullFace(GL_BACK);
//  glFrontFace(GL_CCW);

    // depth-testing defaults
    glEnable(GL_DEPTH_TEST);
//  glDepthFunc(GL_LESS);

    // depth-writing defaults
//  glDepthMask(GL_TRUE);

    // seamless cubemaps
//  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

// ----------------------------------------------------------------------------
// embedded shaders (@fixme: promote to files?)

static const char *const fs_0_0_shadowmap_lit =
    // "#version 140 // inverse() requires v140\n"
    "//" FILELINE "\n"

//  "uniform mat4 view = mat4(1.0);\n"
    "uniform vec3 lightPos = vec3(1.0);\n"
    "uniform float doTexture = 1.;\n"
#if VSMCUBE
    "uniform samplerCube shadowMap;\n" // VSMCUBE
#else
    "uniform sampler2D shadowMap;\n" // !VSMCUBE
#endif

    "struct light {\n"
    "    vec3 position; // world-space\n"
    "    vec4 diffuse;\n"
    "    vec4 specular;\n"
    "    float constantAttenuation, linearAttenuation, quadraticAttenuation;\n"
    "};\n"
    "light light0 = light(\n"
    "    lightPos,\n"
    "    vec4(1,1,1,1), // diffuse\n"
    "    vec4(1,1,1,1), // specular\n"
    "    1.0, 0.0, 0.0  // attenuation (const, linear, quad)\n"
    ");\n"

    "// From http://fabiensanglard.net/shadowmappingVSM/index.php\n"
#if VSMCUBE
    "float chebyshevUpperBound(float distance, vec3 dir) {\n"
    "    distance = distance/20 ;\n"
    "    vec2 moments = texture(shadowMap, dir).rg;\n"
#else
    "float chebyshevUpperBound(float distance, vec4 scPostW) {\n"
    "    vec2 moments = texture(shadowMap,scPostW.xy).rg;\n"
#endif
    "    // Surface is fully lit. as the current fragment is before the light occluder\n"
    "    if (distance <= moments.x)\n"
    "        return 1.0;\n"

    "    // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check\n"
    "    // How likely this pixel is to be lit (p_max)\n"
    "    float variance = moments.y - (moments.x*moments.x);\n"
    "    //variance = max(variance, 0.000002);\n"
    "    variance = max(variance, 0.00002);\n"

    "    float d = distance - moments.x;\n"
    "    float p_max = variance / (variance + d*d);\n"

    "    return p_max;\n"
    "}\n"

    "vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 uv, in vec4 sc) {\n"
#ifndef VSMCUBE
    "    return vec4(1.);\n"
#else

    "    vec3 fragment = vec3(vpeye);\n"
    "    vec3 normal   = vec3(normalize(vneye));\n"
    "    vec3 viewDir  = normalize(-fragment);\n"

    "    // Lighting\n"
    "    // Convert to eye-space\n"
    "    vec3 light = vec3(view * vec4(light0.position, 1.0));\n"

#if VSMCUBE
    "    // Vectors\n"
    "    vec3 fragmentToLight     = light - fragment;\n"
    "    vec3 fragmentToLightDir  = normalize(fragmentToLight);\n"

    "    // Shadows\n"
    "    vec4 fragmentToLight_world = inverse(view) * vec4(fragmentToLightDir, 0.0);\n"
    "    float shadowFactor = chebyshevUpperBound(length(fragmentToLight), -fragmentToLight_world.xyz);\n"
#else
    "    // Shadows\n"
    "    vec4 scPostW = sc / sc.w;\n"
    "    scPostW = scPostW * 0.5 + 0.5;\n"

    "    float shadowFactor = 1.0; // Not in shadow\n"

    "    bool outsideShadowMap = sc.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);\n"
    "    if (!outsideShadowMap) {\n"
    "        shadowFactor = chebyshevUpperBound(scPostW.z, scPostW);\n"
    "    }\n"
#endif

    "    vec4 diffColor = vec4(1,1,1,1);\n"
#if VSMCUBE
    "    if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, -fragmentToLight_world.xyz).r), 1.0);\n"
#else
    "    if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, vec2(uv.x, 1.0 - uv.y)).r), 1.0);\n"
#endif

#if 1
    "    vec3 positionToLight = light - fragment;\n"
    "    vec3 lightDir  = normalize(positionToLight);\n"

    "    // Angle between fragment-normal and incoming light\n"
    "    float cosAngIncidence = dot(lightDir, normal);\n"
    "    cosAngIncidence = clamp(cosAngIncidence, 0, 1);\n"

    "    float attenuation = 1.0f;\n"
    "    attenuation = 1.0 / (light0.constantAttenuation + light0.linearAttenuation * length(positionToLight) + light0.quadraticAttenuation * pow(length(positionToLight),2));\n"

    "    vec4 diffuse  = diffColor * light0.diffuse  * cosAngIncidence * attenuation;\n"

    "    vec4 total_lighting;\n"
    "    total_lighting += vec4(0.1, 0.1, 0.1, 1.0) * diffColor; // Ambient\n"
    "    total_lighting += diffuse * shadowFactor; // Diffuse\n"
#else
    "    vec4 total_lighting = diffColor;\n"
#endif
    "    return vec4(clamp(vec3(total_lighting), 0., 1.), 1.0);\n"
#endif
    "}\n";


static const char *const fs_0_0_shadowmap_unlit = "//" FILELINE "\n"
//  "uniform mat4 view = mat4(1.0);\n"
    "uniform vec3 lightPos = vec3(1.0);\n"
    "uniform float doTexture = 0.;\n"
    "uniform sampler2D shadowMap;\n"

    "vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 Texcoord, in vec4 sc) {\n"
    "    return vec4(1.);\n"
    "};\n";


static const char *const vs_3_3_skybox = "//" FILELINE "\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "out vec3 v_direction;\n"

    "void main() {\n"
    "    vec4 position = u_mvp * vec4(att_position, 0.0);\n"
    "    gl_Position = position.xyww;\n"
    "    v_direction = att_position;\n"
    "}\n";

static const char *const fs_3_4_skybox = "//" FILELINE "\n"
    "uniform samplerCube u_cubemap;\n"

    "in vec3 v_direction;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    fragcolor = vec4(texture(u_cubemap, v_direction).rgb, 1.0);\n"
    "}\n";

static const char *const fs_3_4_skybox_rayleigh = "//" FILELINE "\n"
    "uniform vec3 uSunPos = vec3( 0, 0.1, -1 ); // = [0, Math.cos(theta) * 0.3 + 0.2, -1];\n"

    "in vec3 v_direction;\n"
    "out vec4 fragcolor;\n"

    "vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g);\n"

    "void main() {\n"
    "    vec3 color = atmosphere(\n"
    "        normalize(v_direction),         // normalized ray direction\n"
    "        vec3(0,6372e3,0),               // ray origin\n"
    "        uSunPos,                        // position of the sun\n"
    "        22.0,                           // intensity of the sun\n"
    "        6371e3,                         // radius of the planet in meters\n"
    "        6471e3,                         // radius of the atmosphere in meters\n"
    "        vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient\n"
    "        21e-6,                          // Mie scattering coefficient\n"
    "        8e3,                            // Rayleigh scale height\n"
    "        1.2e3,                          // Mie scale height\n"
    "        0.758                           // Mie preferred scattering direction\n"
    "    );\n"

    "    // Apply exposure.\n"
    "    color = 1.0 - exp(-1.0 * color);\n"

    "    fragcolor = vec4(color, 1);\n"
    "}\n"

    "// [src] https://github.com/wwwtyro/glsl-atmosphere by wwwtyro (Unlicensed)\n"
    "// For more information, please refer to <http://unlicense.org>\n"

    "#define PI 3.141592\n"
    "#define iSteps 16\n"
    "#define jSteps 8\n"

    "vec2 rsi(vec3 r0, vec3 rd, float sr) {\n"
    "    // ray-sphere intersection that assumes\n"
    "    // the sphere is centered at the origin.\n"
    "    // No intersection when result.x > result.y\n"
    "    float a = dot(rd, rd);\n"
    "    float b = 2.0 * dot(rd, r0);\n"
    "    float c = dot(r0, r0) - (sr * sr);\n"
    "    float d = (b*b) - 4.0*a*c;\n"
    "    if (d < 0.0) return vec2(1e5,-1e5);\n"
    "    return vec2(\n"
    "        (-b - sqrt(d))/(2.0*a),\n"
    "        (-b + sqrt(d))/(2.0*a)\n"
    "    );\n"
    "}\n"

    "vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {\n"
    "    // Normalize the sun and view directions.\n"
    "    pSun = normalize(pSun);\n"
    "    r = normalize(r);\n"

    "    // Calculate the step size of the primary ray.\n"
    "    vec2 p = rsi(r0, r, rAtmos);\n"
    "    if (p.x > p.y) return vec3(0,0,0);\n"
    "    p.y = min(p.y, rsi(r0, r, rPlanet).x);\n"
    "    float iStepSize = (p.y - p.x) / float(iSteps);\n"

    "    // Initialize the primary ray time.\n"
    "    float iTime = 0.0;\n"

    "    // Initialize accumulators for Rayleigh and Mie scattering.\n"
    "    vec3 totalRlh = vec3(0,0,0);\n"
    "    vec3 totalMie = vec3(0,0,0);\n"

    "    // Initialize optical depth accumulators for the primary ray.\n"
    "    float iOdRlh = 0.0;\n"
    "    float iOdMie = 0.0;\n"

    "    // Calculate the Rayleigh and Mie phases.\n"
    "    float mu = dot(r, pSun);\n"
    "    float mumu = mu * mu;\n"
    "    float gg = g * g;\n"
    "    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);\n"
    "    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));\n"

    "    // Sample the primary ray.\n"
    "    for (int i = 0; i < iSteps; i++) {\n"

    "        // Calculate the primary ray sample position.\n"
    "        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);\n"

    "        // Calculate the height of the sample.\n"
    "        float iHeight = length(iPos) - rPlanet;\n"

    "        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.\n"
    "        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;\n"
    "        float odStepMie = exp(-iHeight / shMie) * iStepSize;\n"

    "        // Accumulate optical depth.\n"
    "        iOdRlh += odStepRlh;\n"
    "        iOdMie += odStepMie;\n"

    "        // Calculate the step size of the secondary ray.\n"
    "        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);\n"

    "        // Initialize the secondary ray time.\n"
    "        float jTime = 0.0;\n"

    "        // Initialize optical depth accumulators for the secondary ray.\n"
    "        float jOdRlh = 0.0;\n"
    "        float jOdMie = 0.0;\n"

    "        // Sample the secondary ray.\n"
    "        for (int j = 0; j < jSteps; j++) {\n"

    "            // Calculate the secondary ray sample position.\n"
    "            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);\n"

    "            // Calculate the height of the sample.\n"
    "            float jHeight = length(jPos) - rPlanet;\n"

    "            // Accumulate the optical depth.\n"
    "            jOdRlh += exp(-jHeight / shRlh) * jStepSize;\n"
    "            jOdMie += exp(-jHeight / shMie) * jStepSize;\n"

    "            // Increment the secondary ray time.\n"
    "            jTime += jStepSize;\n"
    "        }\n"

    "        // Calculate attenuation.\n"
    "        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));\n"

    "        // Accumulate scattering.\n"
    "        totalRlh += odStepRlh * attn;\n"
    "        totalMie += odStepMie * attn;\n"

    "        // Increment the primary ray time.\n"
    "        iTime += iStepSize;\n"

    "    }\n"

    "    // Calculate and return the final color.\n"
    "    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);\n"
    "}\n";


static const char *const vs_332_32 = "//" FILELINE "\n"
    //"uniform mat4 u_model, u_view, u_proj;\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "in vec3 att_normal;\n"
    "in vec2 att_texcoord;\n"
    "out vec3 v_normal;\n"
    "out vec3 v_normal_ws;\n"
    "out vec2 v_texcoord;\n"

        // shadow
        "uniform mat4 model, view, proj;\n"
        "uniform mat4 cameraToShadowProjector;\n" // !VSMCUBE
        "out vec4 vneye;\n"
        "out vec4 vpeye;\n"
        "out vec4 sc;\n" // !VSMCUBE
        "void do_shadow() {\n"
        "    vneye = view * model * vec4(att_normal,   0.0f);\n"
        "    vpeye = view * model * vec4(att_position, 1.0);\n"
        "    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);\n"
        "}\n"

    "void main() {\n"
    //"    gl_Position = proj * view * model * vec4(att_position, 1.0);\n"
    "    gl_Position = u_mvp * vec4(att_position, 1.0);\n"
    "    v_normal = normalize(att_normal);\n"
    "    v_normal_ws = normalize(vec3(model * vec4(att_normal, 0.)));\n" // normal world/model space
    "    v_texcoord = att_texcoord;\n"
    "    do_shadow();\n"
    "}";

static const char *const vs_0_2_fullscreen_quad_A = "//" FILELINE "\n"
    "out vec2 texcoord;\n"

    "void main() {\n"
    "   texcoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
    "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
    "}\n";

static const char *const vs_0_2_fullscreen_quad_B = "//" FILELINE "\n"
    "out vec2 uv;\n"

    "void main() {\n"
    "    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); \n"
    "    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); \n"
    "    gl_Position = vec4(-1.0 + x*2.0, 0.0+(-1.0+y*2.0), 0.0, 1.0);\n" // normal(0+),flipped(0-)
    "    uv = vec2(x, y);\n" // normal(y),flipped(1.0-y)
    "}\n";

static const char *const vs_0_2_fullscreen_quad_B_flipped = "//" FILELINE "\n"
    "out vec2 uv;\n"

    "void main() {\n"
    "    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); \n"
    "    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); \n"
    "    gl_Position = vec4(-1.0 + x*2.0, 0.0-(-1.0+y*2.0), 0.0, 1.0);\n" // normal(0+),flipped(0-)
    "    uv = vec2(x, y);\n" // normal(y),flipped(1.0-y)
    "}\n";

/*
    "out vec2 uv;\n"
    "void main() {\n"
    "   float x = gl_VertexID / 2;\n"
    "   float y = gl_VertexID % 2;\n"
    "   uv = vec2(x, y);\n"
    "   gl_Position = vec4(2.0*uv - 1.0, 0.0, 1.0);\n"
    "}\n";
*/
static const char *const fs_2_4_texel_inv_gamma = "//" FILELINE "\n"
    "uniform sampler2D texture0; /*unit0*/\n"
    "uniform float u_inv_gamma;\n"

    "in vec2 uv;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    vec4 texel = texture( texture0, uv );\n"
    "    fragcolor = texel;\n"
    "    fragcolor.rgb = pow( fragcolor.rgb, vec3( u_inv_gamma ) );\n" // defaults: 1.0/2.2 gamma
    "}\n";

// vertex stride = 4*(3+2+3+4+4+4+4+3) = 108 bytes
static const char *const vs_32344443_332_model = "//" FILELINE "\n"
    "#ifndef MAX_BONES\n"
    "#define MAX_BONES 110\n"
    "#endif\n"
    "uniform mat3x4 vsBoneMatrix[MAX_BONES];\n"
    "uniform bool SKINNED = false;\n"
    // "uniform mat4 M;\n" // RIM
    "uniform mat4 MVP;\n"

    "in vec3 att_position;\n" // @todo: reorder ass2iqe to emit p3 n3 u2 t3 b3 c4B i4 w4 instead
    "in vec2 att_texcoord;\n"
    "in vec3 att_normal;\n"
    "in vec4 att_tangent;\n" // vec3 + bi sign
    "in vec4 att_indexes;\n" // @fixme: ivec4 instead?
    "in vec4 att_weights;\n" // @todo: downgrade from float to byte
    "in vec4 att_color;\n"
    "in vec3 att_bitangent;\n" // @todo: remove? also, ass2iqe might output this
    "out vec3 v_position;\n"
    "out vec3 v_normal, v_normal_ws;\n"
    "out vec2 v_texcoord;\n"


        // shadow
        "uniform mat4 model, view;\n"
        "uniform mat4 cameraToShadowProjector;\n"
        "out vec4 vneye;\n"
        "out vec4 vpeye;\n"
        "out vec4 sc;\n"
        "void do_shadow() {\n"
        "    vneye = view * model * vec4(att_normal,   0.0f);\n"
        "    vpeye = view * model * vec4(att_position, 1.0);\n"
        "    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);\n"
        "}\n"


    "void main() {\n"
    "   vec3 objPos;\n"
    "   if(!SKINNED) {\n"
    "       objPos = att_position;\n"
    "       v_normal = att_normal;\n"
    "   } else {\n"
    "       mat3x4 m = vsBoneMatrix[int(att_indexes.x)] * att_weights.x;\n"
    "       m += vsBoneMatrix[int(att_indexes.y)] * att_weights.y;\n"
    "       m += vsBoneMatrix[int(att_indexes.z)] * att_weights.z;\n"
    "       m += vsBoneMatrix[int(att_indexes.w)] * att_weights.w;\n"
    "       objPos = vec4(att_position, 1.0) * m;\n"
    "       v_normal = vec4(att_normal, 0.0) * m;\n"
    "       //@todo: tangents\n"
    "   }\n"

    //"   vec3 tangent = att_tangent.xyz;\n"
    //"   vec3 bitangent = cross(att_normal, att_tangent.xyz) * att_tangent.w;

    "   v_normal_ws = normalize(vec3(model * vec4(v_normal, 0.)));\n" // normal to world/model space
    "   v_normal = normalize(v_normal);\n"
    "   v_position = att_position;\n"
    "   v_texcoord = att_texcoord;\n"
    "   gl_Position = MVP * vec4( objPos, 1.0 );\n"
    "   do_shadow();\n"
    "}\n";

#if 0
static const char *const fs_32_4_model_basic = "//" FILELINE "\n"
    "uniform sampler2D fsDiffTex;\n"
    "uniform sampler2D fsNormalTex;\n"
    "uniform sampler2D fsPositionTex;\n"
    "uniform mat4 MVP;\n"

    "in vec3 v_normal;\n"
    "in vec2 v_texcoord;\n"
    "out vec4 fragColor;\n"

    "void main() {\n"
    "    vec4 diff = texture(fsDiffTex, v_texcoord).rgba;\n"
    "    vec3 n = normalize(mat3(MVP) * v_normal); // transform normal to eye space\n"
    "    fragColor = diff;// * vec4(v_normal.xyz, 1);\n"
    "}\n";
#endif

static const char *const fs_32_4_model = "//" FILELINE "\n"
    "uniform mat4 model, view;\n"
    "uniform sampler2D u_texture2d;\n"
    "uniform vec3 u_coefficients_sh[9];\n"
    "uniform bool u_textured = true;\n"
    "uniform bool u_lit = false;\n"
    "uniform bool u_matcaps = false;\n"

    "#ifdef RIM\n"
    "in vec3 v_position;\n"
    "#endif\n"
    "in vec3 v_normal, v_normal_ws;\n"
    "in vec2 v_texcoord;\n"
    "out vec4 fragcolor;\n"

        "{{include-shadowmap}}\n"
        "in vec4 vpeye;\n"
        "in vec4 vneye;\n"
        "in vec4 sc;\n"
        "vec4 shadowing() {\n"
        "    return shadowmap(vpeye, vneye, v_texcoord, sc);\n"
        "}\n"

    "void main() {\n"
    "    vec3 n = /*normalize*/(v_normal);\n"

    // SH lighting
    "    vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "        vec3 SHLightResult[9];\n"
    "        SHLightResult[0] =  0.282095f * u_coefficients_sh[0];\n"
    "        SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;\n"
    "        SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;\n"
    "        SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;\n"
    "        SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;\n"
    "        SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;\n"
    "        SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);\n"
    "        SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;\n"
    "        SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);\n"
    "        vec3 result = vec3(0.0);\n"
    "        for (int i = 0; i < 9; ++i)\n"
    "            result += SHLightResult[i];\n"
    "    if( (result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) lit = vec4(result, 1.0);\n"
    "\n"

    // base
    "    vec4 diffuse;\n"
    "    if(u_matcaps) {\n"
    "        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5);\n" // normal (model space) to view space
    "        diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));\n"
    "    } else if(u_textured) {\n"
    "        diffuse = texture(u_texture2d, v_texcoord);\n"
    "    } else {\n"
    "        diffuse = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "    }\n"

    // lighting mix
    "    fragcolor = diffuse * lit * shadowing();\n"

    // rimlight
    "#ifdef RIM\n"
    "   {vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space\n"
    "    vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space\n"
    "    vec3 v = normalize(-p);                  // eye vector\n"
    "    float rim = 1.0 - max(dot(v, n), 0.0);   // rimlight\n"
    "    rim = smoothstep(1.0-0.01, 1.0, rim);    // intensity (0.01)\n"
    "    fragcolor += vec4(0.0, 0.0, rim, 1.0);}  // blue\n"
    "#endif\n"

    "}\n";


static const char *const fs_2_4_texel_ycbr_gamma_saturation = "//" FILELINE "\n"
    "uniform sampler2D u_texture_y;  /*unit0*/\n"
    "uniform sampler2D u_texture_cb; /*unit1*/\n"
    "uniform sampler2D u_texture_cr; /*unit2*/\n"
    "uniform float u_gamma;\n"

    "in vec2 uv;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    float y = texture(u_texture_y, uv).r;\n"
    "    float cb = texture(u_texture_cb, uv).r;\n"
    "    float cr = texture(u_texture_cr, uv).r;\n"

    "    const mat4 to_rgb = mat4(\n"
    "       1.0000, 1.0000, 1.0000, 0.0000,\n"
    "       0.0000, -0.3441, 1.7720, 0.0000,\n"
    "       1.4020, -0.7141, 0.0000, 0.0000,\n"
    "       -0.7010, 0.5291, -0.8860, 1.0000\n"
    "    );\n"
    "    vec4 texel = to_rgb * vec4(y, cb, cr, 1.0);\n"
    /* same as:
    "    vec3 yCbCr = vec3(y,cb-0.5,cr-0.5);\n"
    "    vec4 texel = vec4(  dot( vec3( 1.0,  0.0,      1.402 ),    yCbCr ),\n"
    "                        dot( vec3( 1.0 , -0.34414 , -0.71414 ), yCbCr ),\n"
    "                        dot( vec3( 1.0,  1.772,    0.0 ),      yCbCr ), 1.0);\n"
    */
    "    // gamma correction\n"
    "    texel.rgb = pow(texel.rgb, vec3(1.0 / u_gamma));\n"

    "   // saturation (algorithm from Chapter 16 of OpenGL Shading Language)\n"
    "   if(false) { float saturation = 2.0; const vec3 W = vec3(0.2125, 0.7154, 0.0721);\n"
    "   vec3 intensity = vec3(dot(texel.rgb, W));\n"
    "   texel.rgb = mix(intensity, texel.rgb, saturation); }\n"

    "    fragcolor = vec4(texel.rgb, 1.0);\n"
    "}\n";

static const char *const vs_324_24_sprite = "//" FILELINE "\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_Position;\n"
    "in vec2 att_TexCoord;\n"
    "in vec4 att_Color;\n"
    "out vec2 vTexCoord;\n"
    "out vec4 vColor;\n"

    "void main() {\n"
    "    vColor = att_Color;\n"
    "    vTexCoord = att_TexCoord;\n"
    "    gl_Position = u_mvp * vec4(att_Position, 1.0);\n"
    "}\n";

static const char *const fs_24_4_sprite = "//" FILELINE "\n"
    "uniform sampler2D u_texture;\n"

    "in vec2 vTexCoord;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"

    "// [src] https://www.shadertoy.com/view/MllBWf CC1.0\n"
    "vec4 texture_AA(sampler2D tx, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tx, 0));\n"
    "    uv = uv*res + 0.5;\n"
    "    // tweak fractionnal value of the texture coordinate\n"
    "    vec2 fl = floor(uv);\n"
    "    vec2 fr = fract(uv);\n"
    "    vec2 aa = fwidth(uv)*0.75;\n"
    "    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);\n"
    "    // return value\n"
    "    uv = (fl+fr-0.5) / res;\n"
    "    return texture(tx, uv);\n"
    "}\n"

    "// [src] https://www.shadertoy.com/view/MllBWf CC1.0\n"
    "vec4 texture_AA2( sampler2D tex, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tex,0));\n"
    "    uv = uv*res;\n"
    "    vec2 seam = floor(uv+0.5);\n"
    "    uv = seam + clamp( (uv-seam)/fwidth(uv), -0.5, 0.5);\n"
    "    return texture(tex, uv/res);\n"
    "}\n"

    "// [src] https://www.shadertoy.com/view/ltBfRD\n"
    "vec4 texture_AA3(sampler2D tex, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tex,0));\n"
    "    float width = 2.0;\n"
    "    uv = uv * res;\n"
    "    // ---\n"
    "    vec2 uv_floor = floor(uv + 0.5);\n"
    "    vec2 uv_fract = fract(uv + 0.5);\n"
    "    vec2 uv_aa = fwidth(uv) * width * 0.5;\n"
    "    uv_fract = smoothstep(\n"
    "        vec2(0.5) - uv_aa,\n"
    "        vec2(0.5) + uv_aa,\n"
    "        uv_fract\n"
    "        );\n"
    "    uv = (uv_floor + uv_fract - 0.5) / res;\n"
    "    return texture(tex, uv);\n"
    "}\n"

    "void main() {\n"
    "    vec4 texColor = texture_AA2(u_texture, vTexCoord);\n"
    "if(texColor.a < 0.5) discard;"
    "    fragColor = vColor * texColor;\n"
    "}\n";

static const char *const fs_2_4_preamble = "//" FILELINE "\n"
    "#define texture2D texture\n"
    "#define texture2DLod textureLod\n"
    "#define FRAGCOLOR fragColor\n"
    "#define texcoord uv\n"
    "#define TEXCOORD uv\n"
    "uniform sampler2D iChannel0;\n"
    "uniform sampler2D iChannel1;\n"
    "uniform float iWidth, iHeight, iTime, iFrame, iMousex, iMousey;\n"
    "uniform float iChannelRes0x, iChannelRes0y;\n"
    "uniform float iChannelRes1x, iChannelRes1y;\n"
    "vec2 iResolution = vec2(iWidth, iHeight);\n"
    "vec2 iMouse = vec2(iMousex, iMousey);\n"
    "vec2 iChannelResolution[2] = vec2[2]( vec2(iChannelRes0x, iChannelRes0y),vec2(iChannelRes1x, iChannelRes1y) );\n"
    "float iGlobalTime = iTime;\n"
    "in vec2 texcoord;\n"
    "out vec4 fragColor;\n";

static const char *const fs_main_shadertoy = "//" FILELINE "\n"
    "void mainImage( out vec4 fragColor, in vec2 fragCoord );\n"
    "void main() {\n"
    "   mainImage(fragColor, texcoord.xy * iResolution);\n"
    "}\n";

// ----------------------------------------------------------------------------
// shaders

void shader_print(const char *source) {
    for(int line = 0, i = 0; source[i] > 0; ) {
        printf("\t%03d: ", line+1);
        while( source[i] >= 32 || source[i] == '\t' ) fputc(source[i++], stdout);
        while( source[i] > 0 && source[i] < 32 ) line += source[i++] == '\n';
        puts("");
    }
}

static
GLuint shader_compile( GLenum type, const char *source ) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE ) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
//      ASSERT(length < 2048); char buf[2048] = { 0 };
        char *buf = stack(length+1);
        glGetShaderInfoLog(shader, length, NULL, buf);

        // dump log with line numbers
        shader_print( source );
        PANIC("ERROR: shader_compile(): %s\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);
        return 0;
    }

    return shader;
}



unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor) {
    PRINTF(/*"!"*/"Compiling shader\n");

    //char *vs = vfs_read(file_vs); if(!vs) vs = (char*)file_vs;
    //char *fs = vfs_read(file_fs); if(!fs) fs = (char*)file_fs;

    vs = vs[0] == '#' && vs[1] == 'v' ? vs : va("#version 150\n%s", vs);
    fs = fs[0] == '#' && fs[1] == 'v' ? fs : va("#version 150\n%s", fs);

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
  //GLuint geom = shader_compile(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        // glAttachShader(program, geom);

        for( int i = 0; attribs && attribs[0]; ++i ) {
            char attrib[128] = {0};
            sscanf(attribs, "%127[^,]", attrib);
            while( attribs[0] && attribs[0] != ',' ) { attribs++; }
            while( attribs[0] && attribs[0] == ',' ) { attribs++; break; }
            if(!attrib[0]) continue;
            glBindAttribLocation(program, i, attrib);
            PRINTF("Shader.attribute[%d]=%s\n", i, attrib);
        }

        if(fragcolor)
        glBindFragDataLocation(program, 0, fragcolor);

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
#ifdef DEBUG_SHADER
        if (status != GL_FALSE && program == DEBUG_SHADER) {
#else
        if (status == GL_FALSE) {
#endif
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            // ASSERT(length < 2048); char buf[2048] = { 0 };
            char *buf = stack(length+1);
            glGetProgramInfoLog(program, length, NULL, buf);
            puts("--- vs:");
            shader_print(vs);
            puts("--- fs:");
            shader_print(fs);
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        // glDetachShader(program, vert);
        // glDetachShader(program, frag);
        // glDetachShader(program, geom);

        glDeleteShader(vert);
        glDeleteShader(frag);
        // glDeleteShader(geom);

//#ifdef DEBUG_ANY_SHADER
//        PRINTF("Shader #%d:\n", program);
//        shader_print(vs);
//        shader_print(fs);
//#endif
    }

/*
    if( s->program ) {
        strcatf(&s->name, "// vs (%s)\n%s\n\n\n", file_vs, vs);
        strcatf(&s->name, "// fs (%s)\n%s\n\n\n", file_fs, fs);
    }
*/

    return program;
}

void shader_destroy(unsigned program){
    if( program == ~0u ) return;
    glDeleteProgram(program);
// if(s->name) FREE(s->name), s->name = NULL;
}

static __thread unsigned last_shader = -1;
static
int shader_uniform(const char *name) {
    int ret = glGetUniformLocation(last_shader, name);
    // if( ret < 0 ) PRINTF("!cannot find uniform '%s' in shader program %d\n", name, (int)last_shader );
    return ret;
}
unsigned shader_get_active() { return last_shader; }
unsigned shader_bind(unsigned program) { unsigned ret = last_shader; return glUseProgram(last_shader = program), ret; }
void shader_int(const char *uniform, int i)     { glUniform1i(shader_uniform(uniform), i); }
void shader_float(const char *uniform, float f) { glUniform1f(shader_uniform(uniform), f); }
void shader_vec2(const char *uniform, vec2 v)   { glUniform2fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3(const char *uniform, vec3 v)   { glUniform3fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec4(const char *uniform, vec4 v)   { glUniform4fv(shader_uniform(uniform), 1, &v.x); }
void shader_mat44(const char *uniform, mat44 m) { glUniformMatrix4fv(shader_uniform(uniform), 1, GL_FALSE/*GL_TRUE*/, m); }
void shader_cubemap(const char *sampler, unsigned texture) { glUniform1i(shader_uniform(sampler), 0); glBindTexture(GL_TEXTURE_CUBE_MAP, texture); }
void shader_bool(const char *uniform, bool x) { glUniform1i(shader_uniform(uniform), x); }
void shader_uint(const char *uniform, unsigned x ) { glUniform1ui(shader_uniform(uniform), x); }
void shader_texture(const char *sampler, texture_t t) { shader_texture_unit(sampler, t.id, t.unit); }
void shader_texture_unit(const char *sampler, unsigned id, unsigned unit) {
    // @todo. if tex.h == 1 ? GL_TEXTURE_1D : GL_TEXTURE_2D
    glUniform1i(shader_uniform(sampler), unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void shader_colormap(const char *name, colormap_t c ) {
    // assumes shader uses `struct { vec4 color; bool has_tex } name + sampler2D name_tex;`
    shader_vec4( va("%s.color", name), c.color );
    shader_bool( va("%s.has_tex", name), c.texture != NULL );
    if( c.texture ) shader_texture( va("%s_tex", name), *c.texture );
}

// -----------------------------------------------------------------------------
// colors

uint32_t rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return r << 24 | g << 16 | b << 8 | a;
}
uint32_t bgra( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return rgba(b,g,r,a);
}
float alpha( uint32_t rgba ) {
    return ( rgba & 255 ) / 255.f;
}

// -----------------------------------------------------------------------------
// images

image_t image_create(int x, int y, int flags) {
    int n = 3; // defaults to RGB
    if(flags & IMAGE_R) n = 1;
    if(flags & IMAGE_RG) n = 2;
    if(flags & IMAGE_RGB) n = 3;
    if(flags & IMAGE_RGBA) n = 4;
    image_t img; img.x = x; img.y = y; img.n = n;
    img.pixels = REALLOC(0, x * y * n ); // @fixme: image_destroy() requires stbi allocator to match REALLOC
    return img;
}

image_t image_from_mem(const char *data, int size, int flags) {
    image_t img = {0};
    if( data && size ) {
        stbi_set_flip_vertically_on_load(flags & IMAGE_FLIP ? 1 : 0);

        int n = 0;
        if(flags & IMAGE_R) n = 1;
        if(flags & IMAGE_RG) n = 2;
        if(flags & IMAGE_RGB) n = 3;
        if(flags & IMAGE_RGBA) n = 4;
        if(flags & IMAGE_FLOAT)
        img.pixels = stbi_loadf_from_memory(data, size, &img.x,&img.y,&img.n, n);
        else
        img.pixels = stbi_load_from_memory(data, size, &img.x,&img.y,&img.n, n);
        if( img.pixels ) {
            PRINTF("Loaded image (%dx%d %.*s->%.*s)\n",img.w,img.h,img.n,"RGBA",n?n:img.n,"RGBA");
        } else {
            // PANIC("Error loading image (%s)\n", pathfile);
        }
        img.n = n ? n : img.n;
    }
    return img;
}

image_t image(const char *pathfile, int flags) {
    const char *fname = vfs_find(pathfile);
    // if( !fname[0] ) fname = vfs_find(va("%s.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.tga",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.jpg.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.tga.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.png.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_find(va("%s.tga.jpg",pathfile)); // needed?

    int size = 0;
    char *data = vfs_load(fname, &size);
    return image_from_mem(data, size, flags);
}

void image_destroy(image_t *img) {
    if(img->pixels) stbi_image_free(img->pixels);
    img->pixels = 0; // *img = (image_t){0}; // do not clear fields yet. might be useful in the future.
}

// bilinear interpolation (uv must be in image coords, range [0..w-1,0..h-1])
static
vec3 bilinear(image_t in, vec2 uv) { // image_bilinear_pixel() ?
    float w = in.x, h = in.y, u = uv.x, v = uv.y;
    float u1 = (int)u, v1 = (int)v, u2 = minf(u1+1, w-1), v2 = minf(v1+1, h-1);
    float c1 = u - u1, c2 = v - v1;
    uint8_t *p1 = &in.pixels8[ in.n * (int)(u1 + v1 * in.w) ];
    uint8_t *p2 = &in.pixels8[ in.n * (int)(u2 + v1 * in.w) ];
    uint8_t *p3 = &in.pixels8[ in.n * (int)(u1 + v2 * in.w) ];
    uint8_t *p4 = &in.pixels8[ in.n * (int)(u2 + v2 * in.w) ];
    vec3 A = vec3( p1[0], p1[1], p1[2] );
    vec3 B = vec3( p2[0], p2[1], p2[2] );
    vec3 C = vec3( p3[0], p3[1], p3[2] );
    vec3 D = vec3( p4[0], p4[1], p4[2] );
    return mix3(mix3(A, B, c1), mix3(C, D, c1), c2);
}

// -----------------------------------------------------------------------------
// textures

static
int allocate_texture_unit() {
    static int textureUnit = 0, totalTextureUnits = 0;
    do_once glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &totalTextureUnits);

    ASSERT(textureUnit < totalTextureUnits, "%d texture units exceeded", totalTextureUnits);
    return textureUnit++;
}

unsigned texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, void *pixels, int flags) {
    ASSERT( t && t->id );
    ASSERT( n <= 4 );
    GLuint pixel_types[] = { GL_RED, GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_R32F, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
    GLenum pixel_storage = flags & TEXTURE_FLOAT ? GL_FLOAT : GL_UNSIGNED_BYTE;
    GLuint pixel_type = pixel_types[ n ];
    GLuint texel_type = pixel_types[ n + 5 * !!(flags & TEXTURE_FLOAT) ];
    GLenum wrap = GL_CLAMP_TO_EDGE;
    GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
//    GLfloat color = (flags&7)/7.f, border_color[4] = { color, color, color, 1.f };

    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGB )  pixel_type = GL_BGR;
    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGBA ) pixel_type = GL_BGRA;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGB )  texel_type = GL_SRGB;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGBA ) texel_type = GL_SRGB_ALPHA; // GL_SRGB8_ALPHA8 ?

    if( flags & TEXTURE_BC1 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    if( flags & TEXTURE_BC2 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    if( flags & TEXTURE_BC3 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    if( flags & TEXTURE_DEPTH ) texel_type = pixel_type = GL_DEPTH_COMPONENT; // GL_DEPTH_COMPONENT32

    if( flags & TEXTURE_REPEAT ) wrap = GL_REPEAT;
    if( flags & TEXTURE_BORDER ) wrap = GL_CLAMP_TO_BORDER;
    if( flags & TEXTURE_LINEAR ) min_filter = GL_LINEAR, mag_filter = GL_LINEAR;
    if( flags & TEXTURE_MIPMAPS  ) min_filter = flags & TEXTURE_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR; // : GL_LINEAR_MIPMAP_NEAREST; maybe?
    if( flags & TEXTURE_MIPMAPS  ) mag_filter = flags & TEXTURE_LINEAR ? GL_LINEAR : GL_NEAREST;

    if( 0 ) { // flags & TEXTURE_PREMULTIPLY_ALPHA )
        uint8_t *p = pixels;
        if(n == 2) for( unsigned i = 0; i < 2*w*h; i += 2 ) {
            p[i] = (p[i] * p[i+1] + 128) >> 8;
        }
        if(n == 4) for( unsigned i = 0; i < 4*w*h; i += 4 ) {
            p[i+0] = (p[i+0] * p[i+3] + 128) >> 8;
            p[i+1] = (p[i+1] * p[i+3] + 128) >> 8;
            p[i+2] = (p[i+2] * p[i+3] + 128) >> 8;
        }
    }

GLenum texture_type = t->flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D; // @fixme: test GL_TEXTURE_2D_ARRAY

//glPixelStorei( GL_UNPACK_ALIGNMENT, n < 4 ? 1 : 4 ); // for framebuffer reading
//glActiveTexture(GL_TEXTURE0 + (flags&7));
    glBindTexture(texture_type, t->id);
    glTexImage2D(texture_type, 0, texel_type, w, h, 0, pixel_type, pixel_storage, pixels);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, mag_filter);
#if 0 // only for sampler2DShadow
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif
//  if( flags & TEXTURE_BORDER )  glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, border_color);
    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(texture_type);

if( flags & TEXTURE_MIPMAPS ) {
        GLfloat max_aniso = 0;
//        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
max_aniso = 4;
//        glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, max_aniso);
}

    // glBindTexture(texture_type, 0); // do not unbind. current code expects texture to be bound at function exit

    t->w = w;
    t->h = h;
    t->n = n;
    t->flags = flags;
    t->filename = t->filename ? t->filename : "";

    return t->id;
}

texture_t texture_create(unsigned w, unsigned h, unsigned n, void *pixels, int flags) {
    texture_t texture = {0};
    glGenTextures( 1, &texture.id );
    texture_update( &texture, w, h, n, pixels, flags );
    texture.unit = allocate_texture_unit();
    texture.transparent = texture.n > 3; // @fixme: should be true only if any pixel.a == 0
    return texture;
}

texture_t texture_checker() {
    static texture_t texture = {0};
    if( !texture.id ) {
#if 0
        float pixels[] = { 1,0.5,0.5,1 };
        texture = texture_create(2,2,1, pixels, TEXTURE_FLOAT|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
#else
        uint32_t *pixels = REALLOC(0, 256*256*4);
        for (int y = 0, i = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
#if 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = secret_palette[ y / 8 ] * !!((x ^ y) & 0x8);
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#elif 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = ((x ^ y) & 0x8) ? secret_palette[6] : secret_palette[ 8 + ((x^y) / (256/6)) ];
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#else
                extern const uint32_t secret_palette[32];
                uint32_t lum = (x^y) & 8 ? 128 : (x^y) & 128 ? 192 : 255;
                uint32_t rgb = rgba(lum,lum,lum,255);
                pixels[i++] = rgb;
#endif
            }
        }
        texture = texture_create(256,256,4, pixels, TEXTURE_RGBA|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
        FREE(pixels);
#endif
    }
    return texture;
}

texture_t texture_from_mem(const char *ptr, int len, int flags) {
    image_t img = image_from_mem(ptr, len, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

texture_t texture(const char *pathfile, int flags) {
    // PRINTF("Loading file %s\n", pathfile);
    image_t img = image(pathfile, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        t.filename = STRDUP(file_name(pathfile));
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

void texture_destroy( texture_t *t ) {
    if(t->filename && t->filename[0]) FREE(t->filename), t->filename = 0;
    if(t->id) glDeleteTextures(1, &t->id), t->id = 0;
    *t = (texture_t){0};
}

// ktx texture loader
// - rlyeh, public domain
//
// [ref] https://developer.nvidia.com/astc-texture-compression-for-game-assets
//
// # Compatibility and modes. What to choose.
// - iOS: PVRTC1_4_RGB or PVRTC1_4 (RGBA) with q:pvrtcnormal.
// - Desktop (OSX/Linux/Windows): BC1, BC1a or BC3 with q:normal.
// - Android: ETC2_RGB or ETC2_RGBA with q:etcfast. ASTC_4x4 or ASTC_8x8 with q:astcmedium, as a fallback.

#if 0
enum {
    // for glFormat
    GLFORMAT_RED                 = 0x1903,
    GLFORMAT_RG                  = 0x8227,
    GLFORMAT_RGB                 = 0x1907,
    GLFORMAT_RGBA                = 0x1908,
    //GLFORMAT_ALPHA             = 0x1906, // 8
    //GLFORMAT_LUMINANCE         = 0x1909, // 8
    //GLFORMAT_LUMINANCE_ALPHA   = 0x190A, // 88

    // for glType
    GLTYPE_UNSIGNED_BYTE         = 0x1401,

    // for glInternalFormat: RAW           // @todo: SRGB, SRGBA, SBGR, SBGRA
    UNCOMPRESSED_RGB             = 0x8051, // 888, GL_RGB8_EXT
    UNCOMPRESSED_RGB_565         = 0x8363,
    UNCOMPRESSED_RGBA            = 0x8058, // 8888, GL_RGBA8_EXT
    UNCOMPRESSED_RGBA_4444       = 0x8033,
    UNCOMPRESSED_RGBA_5551       = 0x8034,
    UNCOMPRESSED_BGR             = 0x80E0, // 888
    UNCOMPRESSED_BGRA            = 0x80E1, // 8888

    // for glInternalFormat: S3TC/DXTn/BCn // @todo: BC4,5,6,7*
    COMPRESSED_RGB_BC1           = 0x83F0, // DXT1
    COMPRESSED_RGBA_BC1          = 0x83F1, // DXT1a, BC1a
    COMPRESSED_RGBA_BC2          = 0x83F2, // DXT3
    COMPRESSED_RGBA_BC3          = 0x83F3, // DXT5
    COMPRESSED_RGBA_BC7          = 0x8E8C, // COMPRESSED_RGBA_BPTC_UNORM_ARB
    COMPRESSED_SRGB_BC1          = 0x8C4C,
    COMPRESSED_SRGBA_BC1         = 0x8C4D,
    COMPRESSED_SRGBA_BC2         = 0x8C4E,
    COMPRESSED_SRGBA_BC3         = 0x8C4F,
    // RGB_BC7f  COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    // RGB_BC7uf COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    // RGBA_BC7  COMPRESSED_RGBA_BPTC_UNORM_ARB
    // SRGBA_BC7 COMPRESSED_SRGBA_BPTC_UNORM_ARB

    // for glInternalFormat: ETC2+EAC
    COMPRESSED_R_EAC             = 0x9270, // 4bpp
    COMPRESSED_R_EAC_SIGNED      = 0x9271, // 4bpp. can preserve 0
    COMPRESSED_RG_EAC            = 0x9272, // 8bpp
    COMPRESSED_RG_EAC_SIGNED     = 0x9273, // 8bbp. can preserve 0
    COMPRESSED_RGB_ETC2          = 0x9274, // 4bpp
    COMPRESSED_RGBA_ETC2         = 0x9276, // 4bpp A1
    COMPRESSED_RGBA_ETC2_EAC     = 0x9278, // 8bpp
    COMPRESSED_SRGB_ETC2         = 0x9275, // 4bpp
    COMPRESSED_SRGBA_ETC2        = 0x9277, // 4bpp A1
    COMPRESSED_SRGBA_ETC2_EAC    = 0x9279, // 8bpp

    // for glInternalFormat: PVR
    COMPRESSED_RGB_PVR1_2        = 0x8C01,
    COMPRESSED_RGB_PVR1_4        = 0x8C00,
    COMPRESSED_RGBA_PVR1_2       = 0x8C03,
    COMPRESSED_RGBA_PVR1_4       = 0x8C02,

    COMPRESSED_SRGB_PVR1_2       = 0x8A54, // _EXT
    COMPRESSED_SRGB_PVR1_4       = 0x8A55, // _EXT
    COMPRESSED_SRGBA_PVR1_2      = 0x8A56, // _EXT
    COMPRESSED_SRGBA_PVR1_4      = 0x8A57, // _EXT

    COMPRESSED_RGBA_PVR2_2       = 0x9137,
    COMPRESSED_RGBA_PVR2_4       = 0x9138,
    COMPRESSED_SRGBA_PVR2_2      = 0x93F0,
    COMPRESSED_SRGBA_PVR2_4      = 0x93F1,

    // for glInternalFormat: ASTC
    COMPRESSED_RGBA_ASTC4x4      = 0x93B0, // 8.00bpp
    COMPRESSED_RGBA_ASTC5x4      = 0x93B1, // 6.40bpp
    COMPRESSED_RGBA_ASTC5x5      = 0x93B2, // 5.12bpp
    COMPRESSED_RGBA_ASTC6x5      = 0x93B3, // 4.27bpp
    COMPRESSED_RGBA_ASTC6x6      = 0x93B4, // 3.56bpp
    COMPRESSED_RGBA_ASTC8x5      = 0x93B5, // 3.20bpp
    COMPRESSED_RGBA_ASTC8x6      = 0x93B6, // 2.67bpp
    COMPRESSED_RGBA_ASTC8x8      = 0x93B7, // 2.56bpp
    COMPRESSED_RGBA_ASTC10x5     = 0x93B8, // 2.13bpp
    COMPRESSED_RGBA_ASTC10x6     = 0x93B9, // 2.00bpp
    COMPRESSED_RGBA_ASTC10x8     = 0x93BA, // 1.60bpp
    COMPRESSED_RGBA_ASTC10x10    = 0x93BB, // 1.28bpp
    COMPRESSED_RGBA_ASTC12x10    = 0x93BC, // 1.07bpp
    COMPRESSED_RGBA_ASTC12x12    = 0x93BD, // 0.89bpp

    COMPRESSED_SRGBA_ASTC4x4     = 0x93D0, // 8.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x4     = 0x93D1, // 6.40bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x5     = 0x93D2, // 5.12bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x5     = 0x93D3, // 4.27bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x6     = 0x93D4, // 3.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x5     = 0x93D5, // 3.20bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x6     = 0x93D6, // 2.67bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x8     = 0x93D7, // 2.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x5    = 0x93D8, // 2.13bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x6    = 0x93D9, // 2.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x8    = 0x93DA, // 1.60bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x10   = 0x93DB, // 1.28bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x10   = 0x93DC, // 1.07bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x12   = 0x93DD, // 0.89bpp SRGB8 A8

    // others:
    // COMPRESSED_RED_RGTC1
    // COMPRESSED_SIGNED_RED_RGTC1
    // COMPRESSED_RG_RGTC2
    // COMPRESSED_SIGNED_RG_RGTC2
};
#endif

#pragma pack(push, 1) // not really needed. the struct is exactly 64 bytes, and all members are 32-bit unsigned
typedef struct ktx_header {
    unsigned identifier[3];         // "«KTX 11»\r\n\x1A\n"
    unsigned endianness;            // 0x04030201 if match

    unsigned glType;                // 0 if compressed; otherwise: UNSIGNED_BYTE, UNSIGNED_SHORT_5_6_5, etc.
    unsigned glTypeSize;            // 1 if compressed; otherwise, size in bytes of glType for endianness conversion. not needed.
    unsigned glFormat;              // STENCIL_INDEX, DEPTH_COMPONENT, DEPTH_STENCIL, RED, GREEN, BLUE, RG, RGB, RGBA, BGR, BGRA, RED_INTEGER, GREEN_INTEGER, BLUE_INTEGER, RG_INTEGER, RGB_INTEGER, RGBA_INTEGER, BGR_INTEGER, BGRA_INTEGER,
    unsigned glInternalFormat;      // COMPRESSED_RED, COMPRESSED_RG, COMPRESSED_RGB, COMPRESSED_RGBA, COMPRESSED_SRGB, COMPRESSED_SRGB_ALPHA, COMPRESSED_RED_RGTC1, COMPRESSED_SIGNED_RED_RGTC1, COMPRESSED_RG_RGTC2, COMPRESSED_SIGNED_RG_RGTC2, COMPRESSED_RGBA_BPTC_UNORM, COMPRESSED_SRGB_ALPHA_BPTC_UNORM, COMPRESSED_RGB_BPTC_SIGNED_FLOAT, COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, COMPRESSED_RGB8_ETC2, COMPRESSED_SRGB8_ETC2, COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_RGBA8_ETC2_EAC, COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, COMPRESSED_R11_EAC, COMPRESSED_SIGNED_R11_EAC, COMPRESSED_RG11_EAC, COMPRESSED_SIGNED_RG11_EAC,
    unsigned glBaseInternalFormat;  // DEPTH_COMPONENT, DEPTH_STENCIL, RED, RG, RGB, RGBA, STENCIL_INDEX,

    unsigned width;
    unsigned height;
    unsigned depth;

    unsigned num_surfaces; // >1 for material
    unsigned num_faces;    // =6 for cubemaps (+X,-X,+Y,-Y,+Z,-Z order), 1 otherwise
    unsigned num_mipmaps;  // >1 for mipmaps

    unsigned metadata_size; // length of following header
    // struct ktx_metadata {
    //     unsigned key_and_value_size;
    //     char     key_and_value[key_and_value_size];
    //     char     value_padding[3 - ((key_and_value_size + 3) % 4)];
    // };
    // struct ktx_texture_data {
    //     unsigned size;
    //     char     data[0];
    // } tx;
} ktx_header;
#pragma pack(pop)

typedef struct ktx_texture {
    unsigned width;
    unsigned height;
    unsigned depth;
    unsigned size;
    const char* data;
} ktx_texture;

typedef struct ktx {
    ktx_header hdr;
    const char *error;
} ktx;

static __thread array(ktx_texture) ktx_textures;

static
ktx ktx_load(const void *data, unsigned int len) {
    ktx ctx = {0};

    // check ktx signature
    bool is_ktx = (len > sizeof(ktx_header)) && !memcmp(data, "\xABKTX 11\xBB\r\n\x1A\n", 12);
    if( !is_ktx ) {
        return ctx.error = "ERROR_BAD_KTX_FILE", ctx;
    }

    // copy texture header
    ktx_header *hdr = &ctx.hdr;
    *hdr = *((const ktx_header *)data);

    // sanity checks
    STATIC_ASSERT(sizeof(ktx_header) == (16*4));
    for( int i = 0; i < sizeof(ktx_header)/4; ++i) {
        i[(unsigned*)hdr] = lil32(i[(unsigned*)hdr]);
    }
    if( hdr->endianness != 0x04030201 ) {
        return ctx.error = "ERROR_BAD_ENDIANNESS", ctx;
    }
    if( (hdr->num_faces != 1) && (hdr->num_faces != 6) ) {
        return ctx.error = "ERROR_BAD_NUMBER_OF_FACES", ctx;
    }

    // normalize glInternalFormat if uncompressed.
    if( hdr->glType != 0 ) {
        hdr->glInternalFormat = hdr->glBaseInternalFormat;
    }

    // normalize [1..N] range
    hdr->num_mipmaps += !hdr->num_mipmaps;
    hdr->num_surfaces += !hdr->num_surfaces;
    hdr->num_faces += !hdr->num_faces;

    // basically,
    // for each level in num_mipmaps { UInt32 imageSize;
    //     for each surface in num_surfaces {
    //         for each face in num_faces {
    //             for each slice in depth {
    //                 for each row in height {
    //                     for each pixel in width {
    //                         byte data[size_based_on_pixelformat]
    //         byte facePadding[0-3] }}}
    //     }
    // Byte mipPadding[0-3] }

    array_resize(ktx_textures, hdr->num_mipmaps * hdr->num_surfaces * hdr->num_faces);

    const char *bitmap = ((const char*)data) + sizeof(ktx_header) + hdr->metadata_size;

    for( unsigned m = 0; m < hdr->num_mipmaps; ++m ) {
        for( unsigned s = 0; s < hdr->num_surfaces; ++s ) {
            for( unsigned f = 0; f < hdr->num_faces; ++f ) {
                ktx_texture *t = &ktx_textures[f+s*hdr->num_faces+m*hdr->num_faces*hdr->num_surfaces];

                // set dimensions [1..N]
                t->width  = (hdr->width  >> m) + !(hdr->width  >> m);
                t->height = (hdr->height >> m) + !(hdr->height >> m);
                t->depth  = (hdr->depth  >> m) + !(hdr->depth  >> m);

                // seek to mip
                const char *ptr = bitmap;
                for( int i = 0; i <= m; i++ ) {
                    // if cubemap, *ptr holds unpadded size of single face,
                    // else, *ptr holds size of all surfaces+faces+slices for whole mipmap.
                    unsigned size = lil32(*(unsigned*)ptr);
                    unsigned padding = 3 - ((size + 3) % 4);

                    // seek to data
                    t->data = ptr + 4 + (size * f);

                    // seek to next mipmap
                    ptr = ptr + 4 + (size * hdr->num_faces) + padding;

                    // adjust size
                    t->size = (uintptr_t)(ptr - t->data); // -padding; needed?
                }

                // ensure we're in bounds
                ASSERT(t->data < ((char*)data + len), "%p < %p", t->data, ((char*)data + len));
                ASSERT(((char*)t->data+t->size) <= ((char*)data + len), "%p < %p", (char*)t->data + t->size, ((char*)data + len));
            }
        }
    }

    return ctx;
}

// ---

texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags) {
    ktx ctx = ktx_load(data, len);
    if( ctx.error ) {
        // puts(ctx.error);
        // return texture_checker();
        return texture_from_mem(data, len, flags);
    }

    ktx_header hdr = ctx.hdr;

    // flags
    int target = hdr.num_faces == 6 ? GL_TEXTURE_CUBE_MAP : hdr.depth > 0 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
    int dimensions = target == GL_TEXTURE_3D ? 3 : target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP ? 2 : 1;

    // create texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);

    // filtering
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, hdr.num_mipmaps > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    // wrapping
    if( dimensions > 0 ) glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if( dimensions > 1 ) glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if( dimensions > 2 ) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    // GLenum internalFormat = flags & TEXTURE_SRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; // @fixme

    int bytes = 0;
    enum { border = 0 };
    for( int m = 0; m < hdr.num_mipmaps; ++m ) {
        for( int s = 0; s < hdr.num_surfaces; ++s ) {
            for( int f = 0; f < hdr.num_faces; ++f ) {
                int d3 = target == GL_TEXTURE_3D, compr = hdr.glType == 0, mode = d3+compr*2;
                ktx_texture *t = &ktx_textures[f+s*hdr.num_faces+m*hdr.num_faces*hdr.num_surfaces];
                /**/ if(mode==0) glTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,                   border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==1) glTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,          border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==2) glCompressedTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,         border,t->size,t->data);
                else if(mode==3) glCompressedTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,border,t->size,t->data);
                bytes += t->size;
            }
        }
    }

//    if( !hdr.num_mipmaps )
//    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(target);

    texture_t t = {0};
    t.id = id;
    t.w = ktx_textures[0].width;
    t.h = ktx_textures[0].height;
    t.d = ktx_textures[0].depth;
    // @todo: reconstruct flags

    PRINTF("dims:%dx%dx%d,size:%.2fMiB,mips:%d,layers:%d,faces:%d\n", t.w, t.h, t.d, bytes / 1024.0 / 1024.0, hdr.num_mipmaps, hdr.num_surfaces, hdr.num_faces);
    return t;
}

texture_t texture_compressed(const char *pathfile, unsigned flags) {
    const char *fname = vfs_find(pathfile);

    int size = 0;
    char *data = vfs_load(fname, &size);
    return texture_compressed_from_mem(data, size, flags);
}

// -----------------------------------------------------------------------------
// shadowmaps

shadowmap_t shadowmap(int texture_width) { // = 1024
    shadowmap_t s = {0};
    s.texture_width = texture_width;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.saved_fb);

    glGenFramebuffers(1, &s.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, s.fbo);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &s.texture);
    glBindTexture(GL_TEXTURE_2D, s.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s.texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, s.saved_fb);
    return s;
}

void shadowmap_destroy(shadowmap_t *s) {
    if (s->texture) {
        glDeleteTextures(1, &s->texture);
    }
    if (s->fbo) {
        glDeleteFramebuffers(1, &s->fbo);
    }
    shadowmap_t z = {0};
    *s = z;
}

void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection) {
    copy44(s->proj, projection);
    s->light_position = vec4(aLightPos.x, aLightPos.y, aLightPos.z, 1);
    lookat44(s->mv, aLightPos, aLightAt, aLightUp);

    mat44 bias = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0 };

    // s->shadowmatrix = bias;
    // s->shadowmatrix *= s->proj;
    // s->shadowmatrix *= s->mv;
//  multiply44x3(s->shadowmatrix, s->mv, s->proj, bias);
    multiply44x3(s->shadowmatrix, bias, s->proj, s->mv);

    // mvp = projection * s->mv;
//  multiply44x2(s->mvp, s->mv, projection);
    multiply44x2(s->mvp, projection, s->mv);
}

void shadowmap_begin(shadowmap_t *s) {
    glGetIntegerv(GL_VIEWPORT, &s->saved_viewport[0]);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s->saved_fb);

    glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
    glViewport(0, 0, s->texture_width, s->texture_width);

    glClearDepth(1);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void shadowmap_end(shadowmap_t *s) {
    glViewport(s->saved_viewport[0], s->saved_viewport[1], s->saved_viewport[2], s->saved_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, s->saved_fb);
}

// shadowmap utils

void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar) {
    perspective44(shm_proj, aLightFov, 1.0f, znear, zfar);
}

void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar) {
    ortho44(shm_proj, left, right, bottom, top, znear, zfar);
}

// -----------------------------------------------------------------------------
// fullscreen quads

// usage: bind empty vao & commit call for 6 (quad) or 3 vertices (tri).
// ie, glBindVertexArray(empty_vao); glDrawArrays(GL_TRIANGLES, 0, 3);

void fullscreen_rgb_quad( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vs_0_2_fullscreen_quad_B_flipped;
        const char* fs = fs_2_4_texel_inv_gamma;

        program = shader(vs, fs, "", "fragcolor" );
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, &vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("drawcalls", +1);
    profile_incstat("triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_ycbcr_quad( texture_t textureYCbCr[3], float gamma ) {
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = vs_0_2_fullscreen_quad_B_flipped;
        const char* fs = fs_2_4_texel_ycbr_gamma_saturation;

        program = shader(vs, fs, "", "fragcolor" );
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, &vao );
    }

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("drawcalls", +1);
    profile_incstat("triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

// ----------------------------------------------------------------------------
// sprites

typedef struct sprite_t {
    float cellw, cellh;       // dimensions of any cell in spritesheet
    int frame, ncx, ncy;      // frame in a (num cellx, num celly) spritesheet
    float px, py, pz;         // origin x, y, depth
    float ox, oy, cos, sin;   // offset x, offset y, cos/sin of rotation degree
    float sx, sy;             // scale x,y
    uint32_t rgba;            // vertex color
} sprite_t;

// sprite batching
typedef struct batch_t { array(sprite_t) sprites; mesh_t mesh; int dirty; } batch_t;
typedef map(int, batch_t) batch_group_t; // mapkey is anything that forces a flush. texture_id for now, might be texture_id+program_id soon

// sprite stream
typedef struct sprite_vertex { vec3 pos; vec2 uv; uint32_t rgba; } sprite_vertex;
typedef struct sprite_index  { GLuint triangle[3]; } sprite_index;

#define sprite_vertex(...) M_CAST(sprite_vertex, __VA_ARGS__)
#define sprite_index(...)  M_CAST(sprite_index, __VA_ARGS__)

// sprite impl
static int sprite_count = 0;
static int sprite_program = -1;
static array(sprite_index)  sprite_indices = 0;
static array(sprite_vertex) sprite_vertices = 0;
static batch_group_t sprite_additive_group = {0};
static batch_group_t sprite_translucent_group = {0};

void tile( texture_t texture, float position[3], float rotation, uint32_t color ) {
    float offset[2] = {0,0}, scale[2] = {1,1}, spritesheet[3] = {0,0,0};
    sprite( texture, position, rotation, offset, scale, 0, color, spritesheet );
}

void sprite( texture_t texture, float position[3], float rotation, float offset[2], float scale[2], int is_additive, uint32_t rgba, float spritesheet[3]) {
    const float px = position[0], py = position[1], pz = position[2];
    const float ox = offset[0], oy = offset[1], sx = scale[0], sy = scale[1];
    const float frame = spritesheet[0], xcells = spritesheet[1], ycells = spritesheet[2];

    if (frame < 0) return;
    if (frame > 0 && frame >= (xcells * ycells)) return;

    // no need to queue if alpha or scale are zero
    if( sx && sy && alpha(rgba) ) {
        sprite_t s;
        s.px = px;
        s.py = py;
        s.pz = pz;
        s.frame = frame;
        s.ncx = xcells ? xcells : 1;
        s.ncy = ycells ? ycells : 1;
        s.sx = sx;
        s.sy = sy;
        s.ox = ox * sx;
        s.oy = oy * sy;
        s.cellw = (texture.x * sx / s.ncx);
        s.cellh = (texture.y * sy / s.ncy);
        s.rgba = rgba;
        s.cos = 1;
        s.sin = 0;
        if(rotation) {
            rotation = (rotation + 0) * ((float)C_PI / 180);
            s.cos = cosf(rotation);
            s.sin = sinf(rotation);
        }

        batch_group_t *batches = is_additive ? &sprite_additive_group : &sprite_translucent_group;
#if 0
        batch_t *found = map_find(*batches, texture.id);
        if( !found ) found = map_insert(*batches, texture.id, (batch_t){0});
#else
        batch_t *found = map_find_or_add(*batches, texture.id, (batch_t){0});
#endif

        array_push(found->sprites, s);
    }
}

static void sprite_rebuild_meshes() {
    sprite_count = 0;

    batch_group_t* list[] = { &sprite_additive_group, &sprite_translucent_group };
    for( int l = 0; l < countof(list); ++l) {
        for each_map_ptr(*list[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float cx = (1.0f / it->ncx) - 1e-9f;
                float cy = (1.0f / it->ncy) - 1e-9f;
                int idx = (int)it->frame;
                int px = idx % it->ncx;
                int py = idx / it->ncx;

                float ux = px * cx, uy = py * cy;
                float vx = ux + cx, vy = uy + cy;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_upgrade(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }
}

static void sprite_render_meshes() {
    if( map_count(sprite_additive_group) <= 0 )
    if( map_count(sprite_translucent_group) <= 0 )
        return;

    if( sprite_program < 0 ) {
        sprite_program = shader( vs_324_24_sprite, fs_24_4_sprite,
            "att_Position,att_TexCoord,att_Color",
            "fragColor"
        );
    }

    // use the shader and  bind the texture @ unit 0
    shader_bind(sprite_program);
    glActiveTexture(GL_TEXTURE0);

    // setup rendering state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL); // try to help with zfighting

    // update camera
    // camera_fps(camera_get_active(), 0,0);
    vec3 pos = camera_get_active()->position;
    float zoom = absf(pos.z); if(zoom < 0.1f) zoom = 0.1f; zoom = 1.f / (zoom + !zoom);
    float width  = window_width();
    float height = window_height();

    // set mvp in the uniform. (0,0) is center of screen.
    mat44 mvp2d;
    float zdepth_max = window_height(); // 1;
    float l = pos.x - width  * zoom / 2;
    float r = pos.x + width  * zoom / 2;
    float b = pos.y + height * zoom / 2;
    float t = pos.y - height * zoom / 2;
    ortho44(mvp2d, l,r,b,t, -zdepth_max, +zdepth_max);

    shader_mat44("u_mvp", mvp2d);

    // set (unit 0) in the uniform texture sampler, and render batch
    // for all additive then translucent groups

    if( map_count(sprite_additive_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE );
        for each_map_ptr(sprite_additive_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_additive_group);
    }

    if( map_count(sprite_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_translucent_group);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}

static void sprite_init() {
    map_init(sprite_translucent_group, less_int, hash_int);
    map_init(sprite_additive_group, less_int, hash_int);
}

void sprite_flush() {
    profile("Sprite rebuild") {
    sprite_rebuild_meshes();
    }
    profile("Sprite render") {
    sprite_render_meshes();
    }
}

// -----------------------------------------------------------------------------
// cubemaps

// project cubemap coords into sphere normals
static
vec3 cubemap2polar(int face, int x, int y, int texture_width) {
    float u = (x / (texture_width - 1.f)) * 2 - 1;
    float v = (y / (texture_width - 1.f)) * 2 - 1;
    /**/ if( face == 0 ) return vec3( u, -1, -v);
    else if( face == 1 ) return vec3(-v, -u,  1);
    else if( face == 2 ) return vec3(-1, -u, -v);
    else if( face == 3 ) return vec3(-u,  1, -v);
    else if( face == 4 ) return vec3( v, -u, -1);
    else                 return vec3( 1,  u, -v);
}
// project normal in a sphere as 2d texcoord
static
vec2 polar2uv(vec3 n) {
    n = norm3(n);
    float theta = atan2(n.y, n.x);
    float phi = atan2(n.z, hypot(n.x, n.y));
    float u = (theta + C_PI) / C_PI;
    float v = (C_PI/2 - phi) / C_PI;
    return vec2(u, v);
}

// equirectangular panorama (2:1) to cubemap - in RGB, out RGB
static
void panorama2cubemap_(image_t out[6], const image_t in, int width){
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGB);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}
// equirectangular panorama (2:1) to cubemap - in RGB, out RGBA
void panorama2cubemap(image_t out[6], const image_t in, int width) {
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGBA);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}


cubemap_t cubemap6( const image_t images[6], int flags ) {
    cubemap_t c = {0}, z = {0};

    glGenTextures(1, &c.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c.id);

    int samples = 0;
    for (int i = 0; i < 6; i++) {
        image_t img = images[i]; //image(textures[i], IMAGE_RGB);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, img.w, img.h, 0, img.n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);

        // calculate SH coefficients (@ands)
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < img.h; y += step) {
            unsigned char *p = (unsigned char*)img.pixels + y * img.w * img.n;
            for (int x = 0; x < img.w; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (img.w - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (img.h - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = div3(vec3(p[0], p[1], p[2]), 255.0f * l * l * l); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                c.sh[0] = add3(c.sh[0], scale3(light,  0.282095f));
                c.sh[1] = add3(c.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                c.sh[2] = add3(c.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                c.sh[3] = add3(c.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                c.sh[4] = add3(c.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                c.sh[5] = add3(c.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                c.sh[6] = add3(c.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                c.sh[7] = add3(c.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                c.sh[8] = add3(c.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += img.n * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        c.sh[s] = scale3(c.sh[s], 32.f / samples);
    }

    if( glGenerateMipmap )
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, glGenerateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return c;
}

cubemap_t cubemap( const image_t in, int flags ) {
    ASSERT( in.n == 4 );
    image_t out[6];
    panorama2cubemap(out, in, in.h);
    image_t swap[6] = { out[0],out[3],out[1],out[4],out[2],out[5] };
    cubemap_t c = cubemap6(swap, flags);
    int i;
    #pragma omp parallel for
    for( i = 0; i < 6; ++i) image_destroy(&out[i]);
    return c;
}

void cubemap_destroy(cubemap_t *c) {
    glDeleteTextures(1, &c->id);
    c->id = 0; // do not destroy SH coefficients still. they might be useful in the future.
}

static cubemap_t *last_cubemap;

cubemap_t* cubemap_get_active() {
    return last_cubemap;
}

// -----------------------------------------------------------------------------
// skyboxes

skybox_t skybox(const char *asset, int flags) {
    skybox_t sky = {0};

    // sky mesh
    vec3 vertices[] = {{+1,-1,+1},{+1,+1,+1},{+1,+1,-1},{-1,+1,-1},{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{-1,+1,+1}};
    unsigned indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };
    sky.geometry = mesh_create("p3", 0,countof(vertices),vertices, countof(indices),indices, MESH_TRIANGLE_STRIP);

    // sky program
    sky.flags = flags ? flags : !!asset; // either cubemap or rayleigh
    sky.program = shader(vs_3_3_skybox,
        sky.flags ? fs_3_4_skybox : fs_3_4_skybox_rayleigh,
        "att_position", "fragcolor");

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) {
            stbi_hdr_to_ldr_gamma(1.2f);
            image_t panorama = image( asset, IMAGE_RGBA );
            sky.cubemap = cubemap( panorama, 0 ); // RGBA required
            image_destroy(&panorama);
        } else { // is folder
            image_t images[6] = {0};
            images[0] = image( va("%s/posx", asset), IMAGE_RGB ); // cubepx
            images[1] = image( va("%s/negx", asset), IMAGE_RGB ); // cubenx
            images[2] = image( va("%s/posy", asset), IMAGE_RGB ); // cubepy
            images[3] = image( va("%s/negy", asset), IMAGE_RGB ); // cubeny
            images[4] = image( va("%s/posz", asset), IMAGE_RGB ); // cubepz
            images[5] = image( va("%s/negz", asset), IMAGE_RGB ); // cubenz
            sky.cubemap = cubemap6( images, 0 );
            for( int i = 0; i < countof(images); ++i ) image_destroy(&images[i]);
        }
    }

    return sky;
}
int skybox_push_state(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

//glClear(GL_DEPTH_BUFFER_BIT);
//glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
//glDisable(GL_CULL_FACE);
glDisable(GL_DEPTH_TEST);

    mat44 mvp; multiply44x2(mvp, proj, view);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->program);
    shader_mat44("u_mvp", mvp);
    if( sky->flags ) {
    shader_cubemap("u_cubemap", sky->cubemap.id);
    }
    return 0; // @fixme: return sortable hash here?
}
int skybox_pop_state(skybox_t *sky) {
    //glDepthMask(GL_TRUE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}
void skybox_destroy(skybox_t *sky) {
    glDeleteProgram(sky->program);
    cubemap_destroy(&sky->cubemap);
    mesh_destroy(&sky->geometry);
}

// -----------------------------------------------------------------------------
// meshes

mesh_t mesh_create(const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    mesh_t z = {0};
    mesh_upgrade(&z, format,  vertex_stride,vertex_count,vertex_data,  index_count,index_data,  flags);
    return z;
}

void mesh_upgrade(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    m->flags = flags;

    // setup
    unsigned sizeof_index = sizeof(GLuint);
    unsigned sizeof_vertex = 0;
    m->index_count = index_count;
    m->vertex_count = vertex_count;

    // iterate vertex attributes { position, normal + uv + tangent + bitangent + ... }
    struct vertex_descriptor {
        int vertex_type, num_attribute, num_components, alt_normalized;
        int stride, offset;
    } descriptor[16] = {0}, *dc = &descriptor[0];

    do switch( *format ) {
        break; case '*': dc->alt_normalized = 1;
        break; case '0': dc->num_components = 0;
        break; case '1': dc->num_components = 1;
        break; case '2': dc->num_components = 2;
        break; case '3': dc->num_components = 3;
        break; case '4': dc->num_components = 4;
        break; case 'F': dc->vertex_type = GL_FLOAT;
        break; case 'U': case 'I': dc->vertex_type = GL_UNSIGNED_INT;
        break; case 'B': if(format[-1] >= '0' && format[-1] <= '9') dc->vertex_type = GL_UNSIGNED_BYTE; //else bitangent.
        break; case ' ': while (format[1] == ' ') format++; case '\0':
            if (!dc->vertex_type) dc->vertex_type = GL_FLOAT;
            dc->offset = sizeof_vertex;
            sizeof_vertex += (dc->stride = dc->num_components * (dc->vertex_type == GL_UNSIGNED_BYTE ? 1 : 4));
            ++dc;
        break; default: if( !strchr("pntbcwai", *format) ) PANIC("unsupported vertex type '%c'", *format);
    } while (*format++);

    if(vertex_stride > 0) sizeof_vertex = vertex_stride;

    // layout
    if(!m->vao) glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // index data
    if( index_data && index_count ) {
        m->index_count = index_count;

        if(!m->ibo) glGenBuffers(1, &m->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof_index, index_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex data
    if( vertex_data && vertex_count ) {
        m->vertex_count = vertex_count;

        if(!m->vbo) glGenBuffers(1, &m->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof_vertex, vertex_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    for( int i = 0; i < 8; ++i ) {
//        glDisableVertexAttribArray(i);
    }

    // vertex setup: iterate descriptors
    for( int i = 0; i < countof(descriptor); ++i ) {
        if( descriptor[i].num_components ) {
            glDisableVertexAttribArray(i);
            glVertexAttribPointer(i,
                descriptor[i].num_components, descriptor[i].vertex_type, (descriptor[i].vertex_type == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE) ^ (descriptor[i].alt_normalized ? GL_TRUE : GL_FALSE),
                sizeof_vertex, (GLchar*)NULL + descriptor[i].offset);
            glEnableVertexAttribArray(i);
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    glBindVertexArray(0);
}

void mesh_render(mesh_t *sm) {
    glBindVertexArray(sm->vao);
    if( sm->ibo ) { // with indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
        glDrawElements(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, sm->index_count, GL_UNSIGNED_INT, (char*)0);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", sm->index_count/3);
    } else { // with vertices only
        glDrawArrays(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, sm->vertex_count /* / 3 */);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", sm->vertex_count/3);
    }
}

void mesh_destroy(mesh_t *m) {
    // @todo
}

// -----------------------------------------------------------------------------
// screenshots

void* screenshot( unsigned n ) { // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static local uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 );
#if 1
    // sync, 10 ms
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // disable any pbo, in case somebody did for us
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return pixels;
#else
    // async
    enum { NUM_PBOS = 16 };
    static local GLuint pbo[NUM_PBOS] = {0}, lastw, lasth;
    static local int frame = 0, bound = 0;

    if( lastw != w || lasth != h ) {
        lastw = w, lasth = h;
        frame = 0;
        bound = 0;

        // @fixme: delete previous pbos
        for( int i = 0; i < NUM_PBOS; ++i ) {
        glGenBuffers(1, &pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, NULL, GL_STREAM_READ); // GL_STATIC_READ);
        }
    }

    if (frame < NUM_PBOS) {
        // do setup during initial frames
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
        glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    } else {
        // read from oldest bound pbo
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
        void *ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        memcpy(pixels, ptr, w * h * abs(n));
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        // trigger next read
        glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    }

    bound = (bound + 1) % NUM_PBOS;
    frame += frame >= 0 && frame < NUM_PBOS;
    frame *= frame == NUM_PBOS ? -1 : +1;

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    return pixels;
#endif
}

// -----------------------------------------------------------------------------
// viewports

void viewport_color3(vec3 color3) {
    glClearColor(color3.x, color3.y, color3.z, 1);
}
void viewport_color(uint32_t rgba) {
    float b = ((rgba >>  0) & 255) / 255.f;
    float g = ((rgba >>  8) & 255) / 255.f;
    float r = ((rgba >> 16) & 255) / 255.f;
    glClearColor(r, g, b, 1);
}

void viewport_clear(bool color, bool depth) {
    glClearDepthf(1);
    glClearStencil(0);
    glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0));
}

void viewport_clip(vec2 from, vec2 to) {
    float x = from.x, y = from.y, w = to.x-from.x, h = to.y-from.y;

    y = window_height()-y-h;
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);
}

// -----------------------------------------------------------------------------
// fbos

unsigned fbo(unsigned color_texture_id, unsigned depth_texture_id, int flags) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if( color_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0);
    if( depth_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);
#if 0 // this is working; it's just not enabled for now
    else {
        // create a non-sampleable renderbuffer object for depth and stencil attachments
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color.width, color.height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    }
#endif

    if(flags) glDrawBuffer(GL_NONE);
    if(flags) glReadBuffer(GL_NONE);

#if 1
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( GL_FRAMEBUFFER_COMPLETE != result ) {
        PANIC("ERROR: Framebuffer not complete.");
    }
#else
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE: break;
        case GL_FRAMEBUFFER_UNDEFINED: PANIC("GL_FRAMEBUFFER_UNDEFINED");
        case GL_FRAMEBUFFER_UNSUPPORTED: PANIC("GL_FRAMEBUFFER_UNSUPPORTED");
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
//      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: PANIC("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
//      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        default: PANIC("ERROR: Framebuffer not complete. glCheckFramebufferStatus returned %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
#endif

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    return fbo;
}
void fbo_bind(unsigned id) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
void fbo_unbind() {
    fbo_bind(0);
}
void fbo_destroy(unsigned id) {
    // glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &id);
}

// -----------------------------------------------------------------------------
// post-fxs swapchain

typedef struct passfx passfx;
typedef struct postfx postfx;

void postfx_create(postfx *fx, int flags);
void postfx_destroy(postfx *fx);

bool postfx_load(postfx *fx, const char *name, const char *fragment);
bool postfx_begin(postfx *fx, int width, int height);
bool postfx_end(postfx *fx);

bool postfx_enabled(postfx *fx, int pass_number);
bool postfx_enable(postfx *fx, int pass_number, bool enabled);
// bool postfx_toggle(postfx *fx, int pass_number);
void postfx_clear(postfx *fx);

char* postfx_name(postfx *fx, int slot);

struct passfx {
    mesh_t m;
    char *name;
    unsigned program;
    int uniforms[16];
};

struct postfx {
    // renderbuffers: color & depth textures
    unsigned fb[2];
    texture_t diffuse[2], depth[2];
    // shader passes
    passfx pass[64];
    uint64_t mask;
    // global enable flag
    bool enabled;
    //
    int num_loaded;
};

enum {
    u_color,
    u_depth,
    u_time,
    u_frame,
    u_width, u_height,
    u_mousex, u_mousey,
    u_channelres0x, u_channelres0y,
    u_channelres1x, u_channelres1y,
};

void postfx_create(postfx *fx, int flags) {
    postfx z = {0};
    *fx = z;
    fx->enabled = 1;
}

void postfx_destroy( postfx *fx ) {
    for( int i = 0; i < 64; ++i ) {
        FREE(fx->pass[i].name);
    }
    texture_destroy(&fx->diffuse[0]);
    texture_destroy(&fx->diffuse[1]);
    texture_destroy(&fx->depth[0]);
    texture_destroy(&fx->depth[1]);
    fbo_destroy(fx->fb[0]);
    fbo_destroy(fx->fb[1]);
    postfx z = {0};
    *fx = z;
}

char* postfx_name(postfx *fx, int slot) {
    return fx->pass[ slot & 63 ].name;
}

bool postfx_load_from_mem( postfx *fx, const char *name, const char *fs ) {
    PRINTF("%s\n", name);
    if(!fs || !fs[0]) PANIC("!invalid fragment shader");

    int slot = fx->num_loaded++;

    passfx *p = &fx->pass[ slot & 63 ];
    p->name = STRDUP(name);

    const char *vs = vs_0_2_fullscreen_quad_B;

    // patch fragment
    char *fs2 = (char*)CALLOC(1, 128*1024);
    strcat(fs2, fs_2_4_preamble);

    if( strstr(fs, "mainImage") ) {
        strcat(fs2, fs_main_shadertoy );
    }

    strcat(fs2, fs);

    p->program = shader(vs, fs2, "vtexcoord", "fragColor" );

    FREE(fs2);

    glUseProgram(p->program); // needed?

    for( int i = 0; i < countof(p->uniforms); ++i ) p->uniforms[i] = -1;

    if( p->uniforms[u_time] == -1 )   p->uniforms[u_time] = glGetUniformLocation(p->program, "iTime");

    if( p->uniforms[u_frame] == -1 )   p->uniforms[u_frame] = glGetUniformLocation(p->program, "iFrame");

    if( p->uniforms[u_width] == -1 )  p->uniforms[u_width] = glGetUniformLocation(p->program, "iWidth");
    if( p->uniforms[u_height] == -1 ) p->uniforms[u_height] = glGetUniformLocation(p->program, "iHeight");

    if( p->uniforms[u_mousex] == -1 ) p->uniforms[u_mousex] = glGetUniformLocation(p->program, "iMousex");
    if( p->uniforms[u_mousey] == -1 ) p->uniforms[u_mousey] = glGetUniformLocation(p->program, "iMousey");

    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex0");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tColor");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tDiffuse");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "iChannel0");

    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tex1");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tDepth");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "iChannel1");

    if( p->uniforms[u_channelres0x] == -1 ) p->uniforms[u_channelres0x] = glGetUniformLocation(p->program, "iChannelRes0x");
    if( p->uniforms[u_channelres0y] == -1 ) p->uniforms[u_channelres0y] = glGetUniformLocation(p->program, "iChannelRes0y");

    if( p->uniforms[u_channelres1x] == -1 ) p->uniforms[u_channelres1x] = glGetUniformLocation(p->program, "iChannelRes1x");
    if( p->uniforms[u_channelres1y] == -1 ) p->uniforms[u_channelres1y] = glGetUniformLocation(p->program, "iChannelRes1y");

    // set quad
    glGenVertexArrays(1, &p->m.vao);
    return true;
}

uint64_t postfx_count_ones(uint64_t x) {
    // [src] https://en.wikipedia.org/wiki/Hamming_weight
    x -= (x >> 1) & 0x5555555555555555ULL;                                //put count of each 2 bits into those 2 bits
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;                           //put count of each 8 bits into those 8 bits
    return (x * 0x0101010101010101ULL) >> 56;                             //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}

bool postfx_enable(postfx *fx, int pass, bool enabled) {
    fx->mask = enabled ? fx->mask | (1ull << pass) : fx->mask & ~(1ull << pass);
    fx->enabled = !!postfx_count_ones(fx->mask);
    return fx->enabled;
}

bool postfx_enabled(postfx *fx, int pass) {
    return (!!(fx->mask & (1ull << pass)));
}

bool postfx_toggle(postfx *fx, int pass) {
    return postfx_enable(fx, pass, 1 ^ postfx_enabled(fx, pass));
}

void postfx_clear(postfx *fx) {
    fx->mask = fx->enabled = 0;
}

bool postfx_begin(postfx *fx, int width, int height) {
    width += !width;
    height += !height;

    // resize if needed
    if( fx->diffuse[0].w != width || fx->diffuse[0].h != height ) {
        texture_destroy(&fx->diffuse[0]);
        texture_destroy(&fx->diffuse[1]);
        texture_destroy(&fx->depth[0]);
        texture_destroy(&fx->depth[1]);
        fbo_destroy(fx->fb[0]);
        fbo_destroy(fx->fb[1]);

        // create texture, set texture parameters and content
        fx->diffuse[0] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[0] = texture_create(width, height, 1,  NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[0] = fbo(fx->diffuse[0].id, fx->depth[0].id, 0);

        // create texture, set texture parameters and content
        fx->diffuse[1] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[1] = texture_create(width, height, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[1] = fbo(fx->diffuse[1].id, fx->depth[1].id, 0);
    }

    uint64_t num_active_passes = postfx_count_ones(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        fbo_unbind();
        return false;
    }

    fbo_bind(fx->fb[1]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    fbo_bind(fx->fb[0]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    return true;
}

bool postfx_end(postfx *fx) {
    uint64_t num_active_passes = postfx_count_ones(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    fbo_unbind();

    // disable depth test in 2d rendering
    glDisable(GL_DEPTH_TEST);

    int frame = 0;
    float t = time_ms() / 1000.f;
    float w = fx->diffuse[0].w;
    float h = fx->diffuse[0].h;
    float mx = input(MOUSE_X);
    float my = input(MOUSE_Y);

    for(int i = 0, e = countof(fx->pass); i < e; ++i) {
        if( fx->mask & (1ull << i) ) {
            passfx *pass = &fx->pass[i];

            if( !pass->program ) { --num_active_passes; continue; }
            glUseProgram(pass->program);

            // bind texture to texture unit 0
            // shader_texture_unit(fx->diffuse[frame], 0);
 glActiveTexture(GL_TEXTURE0 + 0);            glBindTexture(GL_TEXTURE_2D, fx->diffuse[frame].id);
            glUniform1i(pass->uniforms[u_color], 0);

            glUniform1f(pass->uniforms[u_channelres0x], fx->diffuse[frame].w);
            glUniform1f(pass->uniforms[u_channelres0y], fx->diffuse[frame].h);

            // bind depth to texture unit 1
            // shader_texture_unit(fx->depth[frame], 1);
 glActiveTexture(GL_TEXTURE0 + 1);            glBindTexture(GL_TEXTURE_2D, fx->depth[frame].id);
            glUniform1i(pass->uniforms[u_depth], 1);

            // bind uniforms
            static unsigned f = 0; ++f;
            glUniform1f(pass->uniforms[u_time], t);
            glUniform1f(pass->uniforms[u_frame], f-1);
            glUniform1f(pass->uniforms[u_width], w);
            glUniform1f(pass->uniforms[u_height], h);

            glUniform1f(pass->uniforms[u_mousex], mx);
            glUniform1f(pass->uniforms[u_mousey], my);

            // bind the vao
            int bound = --num_active_passes;
            if( bound ) fbo_bind(fx->fb[frame ^= 1]);

                // fullscreen quad
                glBindVertexArray(pass->m.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                profile_incstat("drawcalls", +1);
                profile_incstat("triangles", +2);
                glBindVertexArray(0);

            if( bound ) fbo_unbind();
            else glUseProgram(0);
        }
    }

    return true;
}

static postfx fx;
void fx_load_from_mem(const char *nameid, const char *content) {
    do_once postfx_create(&fx, 0);
    postfx_load_from_mem(&fx, nameid, content);
}
void fx_load(const char *file) {
    postfx_load_from_mem(&fx, file_name(file), vfs_read(file));
}
void fx_begin() {
    postfx_begin(&fx, window_width(), window_height());
}
void fx_end() {
    postfx_end(&fx);
}
int fx_enabled(int pass) {
    return postfx_enabled(&fx, pass);
}
void fx_enable(int pass, int enabled) {
    postfx_enable(&fx, pass, enabled);
}
void fx_enable_all(int enabled) {
    for( int i = 0; i < fx.num_loaded; ++i ) fx_enable(i, enabled);
}
char *fx_name(int pass) {
    return postfx_name(&fx, pass);
}

// -----------------------------------------------------------------------------
// brdf

static texture_t brdf = {0};

static void brdf_load() {
    const char *filename;
    filename = "Skyboxes/brdf_lut1k_256x256_32F.ktx";
    filename = "Skyboxes/brdf_lut2k_512x512_32F.ktx";

    brdf = texture_compressed( filename,
        TEXTURE_CLAMP | TEXTURE_NEAREST | TEXTURE_RG | TEXTURE_FLOAT | TEXTURE_SRGB
    );
    ASSERT(brdf.id != texture_checker().id, "!Couldn't load BRDF lookup table '%s'!", filename );
}

texture_t brdf_lut() {
    do_once brdf_load();
    return brdf;
}

// -----------------------------------------------------------------------------
// materials

bool colormap( colormap_t *cm, const char *material_file, bool load_as_srgb ) {
    if( cm->texture ) {
        texture_destroy(cm->texture);
        FREE(cm->texture), cm->texture = NULL;
    }

    int srgb = load_as_srgb ? TEXTURE_SRGB : 0;
    int hdr = strendi(material_file, ".hdr") ? TEXTURE_FLOAT | TEXTURE_RGBA : 0;
    texture_t t = texture_compressed(material_file, TEXTURE_LINEAR | TEXTURE_MIPMAPS | TEXTURE_REPEAT | hdr | srgb);

    if( t.id == texture_checker().id ) {
        cm->texture = NULL;
        return false;
    }
    cm->texture = CALLOC(1, sizeof(texture_t));
    *cm->texture = t;
    return true;
}

bool pbr_material(pbr_material_t *pbr, const char *material) {
    if( !material || !pbr ) return false;

    //pbr_material_destroy(pbr);
    *pbr = (pbr_material_t){0};

    pbr->name = STRDUP(material);

    pbr->specular_shininess = 1.0f;
    /*
    if( const float *f = aiGetMaterialFloat(scn_material[i], aiMaterialTypeString(MATERIAL_SHININESS)) ) {
        pbr->specular_shininess = *f;
    }
    */

    pbr->diffuse.color = vec4(0.5,0.5,0.5,0.5);
    pbr->normals.color = vec4(0,0,0,0);
    pbr->specular.color = vec4(0,0,0,0);
    pbr->albedo.color = vec4(0.5,0.5,0.5,1.0);
    pbr->roughness.color = vec4(1,1,1,1);
    pbr->metallic.color = vec4(0,0,0,0);
    pbr->ao.color = vec4(1,1,1,1);
    pbr->ambient.color = vec4(0,0,0,1);
    pbr->emissive.color = vec4(0,0,0,0);

    array(char*) tokens = strsplit(material, "+");
    for( int j = 0, end = array_count(tokens); j < end; ++j ) {
        char *t = tokens[j];
        if( strstri(t, "_D.") || strstri(t, "Diffuse") || strstri(t, "BaseColor") )    colormap(&pbr->diffuse, t, 1);
        if( strstri(t, "_N.") || strstri(t, "Normal") )     colormap(&pbr->normals, t, 0);
        if( strstri(t, "_S.") || strstri(t, "Specular") )   colormap(&pbr->specular, t, 0);
        if( strstri(t, "_A.") || strstri(t, "Albedo") )     colormap(&pbr->albedo, t, 1); // 0?
        if( strstri(t, "_MR.")|| strstri(t, "Roughness") )  colormap(&pbr->roughness, t, 0);
        else
        if( strstri(t, "_M.") || strstri(t, "Metallic") )   colormap(&pbr->metallic, t, 0);
      //if( strstri(t, "_S.") || strstri(t, "Shininess") )  colormap(&pbr->roughness, t, 0);
      //if( strstri(t, "_A.") || strstri(t, "Ambient") )    colormap(&pbr->ambient, t, 0);
        if( strstri(t, "_E.") || strstri(t, "Emissive") )   colormap(&pbr->emissive, t, 1);
        if( strstri(t, "_AO.") || strstri(t, "AO") || strstri(t, "Occlusion") ) colormap(&pbr->ao, t, 0);
    }

    return true;
}

void pbr_material_destroy(pbr_material_t *m) {
    if( m->name )              FREE(m->name), m->name = NULL;
    if( m->diffuse.texture)    texture_destroy( m->diffuse.texture );
    if( m->normals.texture)    texture_destroy( m->normals.texture );
    if( m->specular.texture)   texture_destroy( m->specular.texture );
    if( m->albedo.texture)     texture_destroy( m->albedo.texture );
    if( m->roughness.texture)  texture_destroy( m->roughness.texture );
    if( m->metallic.texture)   texture_destroy( m->metallic.texture );
    if( m->ao.texture )        texture_destroy( m->ao.texture );
    if( m->ambient.texture )   texture_destroy( m->ambient.texture );
    *m = (pbr_material_t){0};
}

// -----------------------------------------------------------------------------
// skeletal meshes (iqm)

#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

struct iqmheader {
    char magic[16];
    unsigned version;
    unsigned filesize;
    unsigned flags;
    unsigned num_text, ofs_text;
    unsigned num_meshes, ofs_meshes;
    unsigned num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned num_triangles, ofs_triangles, ofs_adjacency;
    unsigned num_joints, ofs_joints;
    unsigned num_poses, ofs_poses;
    unsigned num_anims, ofs_anims;
    unsigned num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned num_comment, ofs_comment;
    unsigned num_extensions, ofs_extensions;
};

struct iqmmesh {
    unsigned name;
    unsigned material;
    unsigned first_vertex, num_vertexes;
    unsigned first_triangle, num_triangles;
};

enum {
    IQM_POSITION,
    IQM_TEXCOORD,
    IQM_NORMAL,
    IQM_TANGENT,
    IQM_BLENDINDEXES,
    IQM_BLENDWEIGHTS,
    IQM_COLOR,
    IQM_CUSTOM = 0x10
};

enum {
    IQM_BYTE,
    IQM_UBYTE,
    IQM_SHORT,
    IQM_USHORT,
    IQM_INT,
    IQM_UINT,
    IQM_HALF,
    IQM_FLOAT,
    IQM_DOUBLE,
};

struct iqmtriangle {
    unsigned vertex[3];
};

struct iqmadjacency {
    unsigned triangle[3];
};

struct iqmjoint {
    unsigned name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose {
    int parent;
    unsigned mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim {
    unsigned name;
    unsigned first_frame, num_frames;
    float framerate;
    unsigned flags;
};

enum {
    IQM_LOOP = 1<<0
};

struct iqmvertexarray {
    unsigned type;
    unsigned flags;
    unsigned format;
    unsigned size;
    unsigned offset;
};

struct iqmbounds {
    union {
        struct { float bbmin[3], bbmax[3]; };
        struct { vec3 min3, max3; };
        aabb box;
    };
    float xyradius, radius;
};

// -----------------------------------------------------------------------------

typedef struct iqm_vertex {
    GLfloat position[3];
    GLfloat texcoord[2];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLubyte blendindexes[4];
    GLubyte blendweights[4];
    GLubyte color[4];
} iqm_vertex;

typedef struct iqm_t {
    int nummeshes, numtris, numverts, numjoints, numframes, numanims;
    GLuint program;
    GLuint vao, ibo, vbo;
    GLuint *textures;
    uint8_t *buf, *meshdata, *animdata;
    struct iqmmesh *meshes;
    struct iqmjoint *joints;
    struct iqmpose *poses;
    struct iqmanim *anims;
    struct iqmbounds *bounds;
    mat34 *baseframe, *inversebaseframe, *outframe, *frames;
    GLint bonematsoffset;
} iqm_t;

#define program (q->program)
#define meshdata (q->meshdata)
#define animdata (q->animdata)
#define nummeshes (q->nummeshes)
#define numtris (q->numtris)
#define numverts (q->numverts)
#define numjoints (q->numjoints)
#define numframes (q->numframes)
#define numanims (q->numanims)
#define meshes (q->meshes)
#define textures (q->textures)
#define joints (q->joints)
#define poses (q->poses)
#define anims (q->anims)
#define baseframe (q->baseframe)
#define inversebaseframe (q->inversebaseframe)
#define outframe (q->outframe)
#define frames (q->frames)
#define vao (q->vao)
#define ibo (q->ibo)
#define vbo (q->vbo)
#define bonematsoffset (q->bonematsoffset)
#define buf (q->buf)
#define bounds (q->bounds)

void model_set_texture(model_t m, texture_t t) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    for( int i = 0; i < nummeshes; ++i) { // assume 1 texture per mesh
        textures[i] = t.id;
    }
}

static
void model_set_uniforms(model_t m, int shader, mat44 mv, mat44 proj, mat44 view, mat44 model) {  // @todo: cache uniform locs
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glUseProgram(shader);
    int loc;
    //if( (loc = glGetUniformLocation(shader, "M")) >= 0 ) glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, m); // RIM
    if( (loc = glGetUniformLocation(shader, "MV")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, mv);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mv")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, mv);
    }
    if( (loc = glGetUniformLocation(shader, "MVP")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, mvp);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mvp")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, mvp);
    }
#if 0
    // @todo: mat44 projview (useful?)
#endif
    if ((loc = glGetUniformLocation(shader, "M")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, model);
    }
    else
    if ((loc = glGetUniformLocation(shader, "model")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, model);
    }
    if ((loc = glGetUniformLocation(shader, "V")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, view);
    }
    else
    if ((loc = glGetUniformLocation(shader, "view")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, view);
    }
    if ((loc = glGetUniformLocation(shader, "P")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, proj);
    }
    else
    if ((loc = glGetUniformLocation(shader, "proj")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE/*GL_TRUE*/, proj);
    }
    if( (loc = glGetUniformLocation(shader, "SKINNED")) >= 0 ) glUniform1i( loc, numanims ? GL_TRUE : GL_FALSE);
    if( numanims )
    if( (loc = glGetUniformLocation(shader, "vsBoneMatrix")) >= 0 ) glUniformMatrix3x4fv( loc, numjoints, GL_FALSE, outframe[0]);

    if ((loc = glGetUniformLocation(shader, "u_matcaps")) >= 0) {
        glUniform1i(loc, m.flags & MODEL_MATCAPS ? GL_TRUE:GL_FALSE);
    }
}
static
void model_set_state(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, position) );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord) );
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, normal) );
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, tangent) );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    if(numframes > 0) {
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendindexes) );
        glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendweights) );
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
    }

    // 6 color
    // 7 bitangent? into texcoord.z?

    glBindVertexArray( 0 );
}

static
bool model_load_meshes(iqm_t *q, const struct iqmheader *hdr, model_t *m) {
    if(meshdata) return false;

    lil32p(&buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint32_t));

    meshdata = buf;
    nummeshes = hdr->num_meshes;
    numtris = hdr->num_triangles;
    numverts = hdr->num_vertexes;
    numjoints = hdr->num_joints;
    outframe = CALLOC(hdr->num_joints, sizeof(mat34));

    float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL;
    uint8_t *inblendindex8 = NULL, *inblendweight8 = NULL;
    int *inblendindexi = NULL; float *inblendweightf = NULL;
    struct iqmvertexarray *vas = (struct iqmvertexarray *)&buf[hdr->ofs_vertexarrays];
    for(int i = 0; i < (int)hdr->num_vertexarrays; i++) {
        struct iqmvertexarray *va = &vas[i];
        switch(va->type) {
        default: continue; // return PANIC("unknown iqm vertex type (%d)", va->type), false;
        break; case IQM_POSITION: if(va->format != IQM_FLOAT || va->size != 3) return PANIC("!"); false; inposition = (float *)&buf[va->offset]; lil32pf(inposition, 3*hdr->num_vertexes);
        break; case IQM_NORMAL: if(va->format != IQM_FLOAT || va->size != 3) return PANIC("!"); false; innormal = (float *)&buf[va->offset]; lil32pf(innormal, 3*hdr->num_vertexes);
        break; case IQM_TANGENT: if(va->format != IQM_FLOAT || va->size != 4) return PANIC("!"); false; intangent = (float *)&buf[va->offset]; lil32pf(intangent, 4*hdr->num_vertexes);
        break; case IQM_TEXCOORD: if(va->format != IQM_FLOAT || va->size != 2) return PANIC("!"); false; intexcoord = (float *)&buf[va->offset]; lil32pf(intexcoord, 2*hdr->num_vertexes);
        break; case IQM_BLENDINDEXES: if(va->size != 4) return PANIC("!"); false; if(va->format != IQM_UBYTE && va->format != IQM_INT) return PANIC("!"); false;
        if(va->format == IQM_UBYTE) inblendindex8 = (uint8_t *)&buf[va->offset];
        else inblendindexi = (int *)&buf[va->offset];
        break; case IQM_BLENDWEIGHTS: if(va->size != 4) return PANIC("!"); false; if(va->format != IQM_UBYTE && va->format != IQM_FLOAT) return PANIC("!"); false;
        if(va->format == IQM_UBYTE) inblendweight8 = (uint8_t *)&buf[va->offset];
        else inblendweightf = (float *)&buf[va->offset];
        }
    }

    if (hdr->ofs_bounds) lil32p(buf + hdr->ofs_bounds, hdr->num_frames * sizeof(struct iqmbounds));
    if (hdr->ofs_bounds) bounds = (struct iqmbounds *) &buf[hdr->ofs_bounds];

    meshes = (struct iqmmesh *)&buf[hdr->ofs_meshes];
    joints = (struct iqmjoint *)&buf[hdr->ofs_joints];

    baseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    inversebaseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    for(int i = 0; i < (int)hdr->num_joints; i++) {
        struct iqmjoint *j = &joints[i];
        compose34(baseframe[i], ptr3(j->translate), normq(ptrq(j->rotate)), ptr3(j->scale));
        invert34(inversebaseframe[i], baseframe[i]);
        if(j->parent >= 0) {
            multiply34x2(baseframe[i], baseframe[j->parent], baseframe[i]);
            multiply34(inversebaseframe[i], inversebaseframe[j->parent]);
        }
    }

    struct iqmtriangle *tris = (struct iqmtriangle *)&buf[hdr->ofs_triangles];

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(!ibo) glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, hdr->num_triangles*sizeof(struct iqmtriangle), tris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    iqm_vertex *verts = CALLOC(hdr->num_vertexes, sizeof(iqm_vertex));
    for(int i = 0; i < (int)hdr->num_vertexes; i++) {
        iqm_vertex *v = &verts[i];
        if(inposition) memcpy(v->position, &inposition[i*3], sizeof(v->position));
        if(innormal) memcpy(v->normal, &innormal[i*3], sizeof(v->normal));
        if(intangent) memcpy(v->tangent, &intangent[i*4], sizeof(v->tangent));
        if(intexcoord) memcpy(v->texcoord, &intexcoord[i*2], sizeof(v->texcoord));
        if(inblendindex8) memcpy(v->blendindexes, &inblendindex8[i*4], sizeof(v->blendindexes));
        if(inblendweight8) memcpy(v->blendweights, &inblendweight8[i*4], sizeof(v->blendweights));
        if(inblendindexi) {
            uint8_t conv[4] = { inblendindexi[i*4], inblendindexi[i*4+1], inblendindexi[i*4+2], inblendindexi[i*4+3] };
            memcpy(v->blendindexes, conv, sizeof(v->blendindexes));
        }
        if(inblendweightf) {
            uint8_t conv[4] = { inblendweightf[i*4] * 255, inblendweightf[i*4+1] * 255, inblendweightf[i*4+2] * 255, inblendweightf[i*4+3] * 255 };
            memcpy(v->blendweights, conv, sizeof(v->blendweights));
        }
    }

    if(!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, hdr->num_vertexes*sizeof(iqm_vertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

m->stride = sizeof(iqm_vertex);
#if 0
m->stride = 0;
if(inposition) m->stride += sizeof(verts[0].position);
if(innormal) m->stride += sizeof(verts[0].normal);
if(intangent) m->stride += sizeof(verts[0].tangent);
if(intexcoord) m->stride += sizeof(verts[0].texcoord);
if(inblendindex8) m->stride += sizeof(verts[0].blendindexes); // no index8? bug?
if(inblendweight8) m->stride += sizeof(verts[0].blendweights); // no weight8? bug?
if(inblendindexi) m->stride += sizeof(verts[0].blendindexes);
if(inblendweightf) m->stride += sizeof(verts[0].blendweights);
#endif
//for( int i = 0; i < 16; ++i ) printf("%.9g%s", ((float*)verts)[i], (i % 3) == 2 ? "\n" : ",");
//m->verts = verts; //FREE(verts);
m->verts = 0; FREE(verts);

    textures = CALLOC(hdr->num_meshes * 8, sizeof(GLuint));
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        int invalid = texture_checker().id;
        textures[i] = invalid;
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];
        PRINTF("loaded mesh: %s\n", &str[m->name]);
    }

    return true;
}

static
bool model_load_anims(iqm_t *q, const struct iqmheader *hdr) {
    if((int)hdr->num_poses != numjoints) return false;

    if(animdata) {
        if(animdata != meshdata) FREE(animdata);
        FREE(frames);
        animdata = NULL;
        anims = NULL;
        frames = 0;
        numframes = 0;
        numanims = 0;
    }

    lil32p(&buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint32_t));
    lil16p((uint16_t *)&buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);

    animdata = buf;
    numanims = hdr->num_anims;
    numframes = hdr->num_frames;

    anims = (struct iqmanim *)&buf[hdr->ofs_anims];
    poses = (struct iqmpose *)&buf[hdr->ofs_poses];
    frames = CALLOC(hdr->num_frames * hdr->num_poses, sizeof(mat34));
    uint16_t *framedata = (uint16_t *)&buf[hdr->ofs_frames];

    for(int i = 0; i < (int)hdr->num_frames; i++) {
        for(int j = 0; j < (int)hdr->num_poses; j++) {
            struct iqmpose *p = &poses[j];
            quat rotate;
            vec3 translate, scale;
            translate.x = p->channeloffset[0]; if(p->mask&0x01) translate.x += *framedata++ * p->channelscale[0];
            translate.y = p->channeloffset[1]; if(p->mask&0x02) translate.y += *framedata++ * p->channelscale[1];
            translate.z = p->channeloffset[2]; if(p->mask&0x04) translate.z += *framedata++ * p->channelscale[2];

            rotate.x = p->channeloffset[3]; if(p->mask&0x08) rotate.x += *framedata++ * p->channelscale[3];
            rotate.y = p->channeloffset[4]; if(p->mask&0x10) rotate.y += *framedata++ * p->channelscale[4];
            rotate.z = p->channeloffset[5]; if(p->mask&0x20) rotate.z += *framedata++ * p->channelscale[5];
            rotate.w = p->channeloffset[6]; if(p->mask&0x40) rotate.w += *framedata++ * p->channelscale[6];

            scale.x = p->channeloffset[7]; if(p->mask&0x80)  scale.x += *framedata++ * p->channelscale[7];
            scale.y = p->channeloffset[8]; if(p->mask&0x100) scale.y += *framedata++ * p->channelscale[8];
            scale.z = p->channeloffset[9]; if(p->mask&0x200) scale.z += *framedata++ * p->channelscale[9];

            // Concatenate each pose with the inverse base pose to avoid doing this at animation time.
            // If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
            // Thus it all negates at animation time like so:
            //   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
            //   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
            //   parentPose * childPose * childInverseBasePose

            mat34 m; compose34(m, translate, normq(rotate), scale);
            if(p->parent >= 0) multiply34x3(frames[i*hdr->num_poses + j], baseframe[p->parent], m, inversebaseframe[j]);
            else multiply34x2(frames[i*hdr->num_poses + j], m, inversebaseframe[j]);
        }
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_anims; i++) {
        struct iqmanim *a = &anims[i];
        PRINTF("loaded anim[%d]: %s\n", i, &str[a->name]);
    }

    return true;
}

static
bool model_load_textures(iqm_t *q, const struct iqmheader *hdr, model_t *model) {
    textures = textures ? textures : CALLOC(hdr->num_meshes * 8, sizeof(GLuint)); // up to 8 textures per mesh

    GLuint *out = textures;

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];

        int flags = TEXTURE_MIPMAPS|TEXTURE_REPEAT; // LINEAR, NEAREST
        int invalid = texture_checker().id;

#if 1
        char* material_name;
        // remove any material+name from materials (.fbx)
        // try left token first
        if( 1 ) {
            material_name = va("%s", &str[m->material]);
            char* plus = strrchr(material_name, '+');
            if (plus) { strcpy(plus, file_ext(material_name)); }
            *out = texture_compressed(material_name, flags).id;
        }
        // else try right token
        if (*out == invalid) {
            material_name = file_normalize( va("%s", &str[m->material]) );
            char* plus = strrchr(material_name, '+'), *slash = strrchr(material_name, '/');
            if (plus) {
                strcpy(slash ? slash + 1 : material_name, plus + 1);
                *out = texture_compressed(material_name, flags).id;
            }
        }
        // else last resort
        if (*out == invalid) {
            *out = texture_compressed(material_name, flags).id; // needed?
        }

        if( *out != invalid) {
            PRINTF("loaded material[%d]: %s\n", i, &str[m->material]);
        } else {
            PRINTF("fail: material[%d] not found: %s\n", i, &str[m->material]);
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out = texture_checker().id; // placeholder
        }

        {
            model->num_textures++;
            array_push(model->texture_names, STRDUP(&str[m->material]));

            material_t mt = {0};
            mt.name = STRDUP(&str[m->material]);
            mt.layer[mt.count++].texture = *out++;
            array_push(model->materials, mt);
        }

#else
        material_t mt = {0};
        mt.name = STRDUP(&str[m->material]);

        array(char*) tokens = strsplit(&str[m->material], "+");
        for each_array(tokens, char*, it) {
            *out = texture(it, flags).id;

            if( *out == invalid ) {
                PRINTF("fail: material[%d] not found: %s\n", i, it);
            } else {
                PRINTF("loaded material[%d]: %s\n", i, it);

                mt.layer[mt.count++].texture = *out;

                ++out;
            }
        }

        // if no materials were loaded, try to signal a checkered placeholder
        if( out == textures ) {
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out++ = invalid;
        }

        int count = (int)(intptr_t)(out - textures);
        model->num_textures += count;
        array_push(model->texture_names, STRDUP(&str[m->material]));

        array_push(model->materials, mt);
#endif

    }

    if( array_count(model->materials) == 0 ) {
        material_t mt = {0};
        mt.name = "placeholder";
        mt.count = 1;
        mt.layer[0].texture = texture_checker().id;

        array_push(model->materials, mt);
    }

    return true;
}

model_t model_from_mem(const void *mem, int len, int flags) {
    model_t m = {0};

    const char *ptr = (const char *)mem;
    static int shaderprog = -1;
    if( shaderprog < 0 ) {
        const char *symbols[] = { "{{include-shadowmap}}", fs_0_0_shadowmap_lit }; // #define RIM
        shaderprog = shader(strlerp(1,symbols,vs_32344443_332_model), strlerp(1,symbols,fs_32_4_model), //fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_indexes,att_weights,att_color,att_bitangent","fragColor");
    }

    iqm_t *q = CALLOC(1, sizeof(iqm_t));
    program = shaderprog;

    int error = 1;
    if( ptr && len ) {
        struct iqmheader hdr; memcpy(&hdr, ptr, sizeof(hdr)); ptr += sizeof(hdr);
        if( !memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic))) {
            lil32p(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint32_t));
            if(hdr.version == IQM_VERSION) {
                buf = CALLOC(hdr.filesize, sizeof(uint8_t));
                memcpy(buf + sizeof(hdr), ptr, hdr.filesize - sizeof(hdr));
                error = 0;
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_MESHES) )     error |= !model_load_meshes(q, &hdr, &m);
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_TEXTURES) )   error |= !model_load_textures(q, &hdr, &m);
                if( hdr.num_anims  > 0 && !(flags & MODEL_NO_ANIMATIONS) ) error |= !model_load_anims(q, &hdr);
                if( buf != meshdata && buf != animdata ) FREE(buf);
            }
        }
    }

    if( error ) {
        PRINTF("Error: cannot load %s", "model");
        FREE(q), q = 0;
    } else {
        #undef vao
        #undef ibo
        #undef vbo
        m.vao = q->vao;
        m.ibo = q->ibo;
        m.vbo = q->vbo;
        m.num_verts = numverts;
        #define vao (q->vao)
        #define ibo (q->ibo)
        #define vbo (q->vbo)

        // m.boxes = bounds; // <@todo
        m.num_meshes = nummeshes;
        m.num_triangles = numtris;
        m.num_joints = numjoints;
        //m.num_poses = numposes;
        m.num_anims = numanims;
        m.num_frames = numframes;
        m.iqm = q;
        m.curframe = model_animate(m, 0);
        #undef program
        m.program = (q->program);
        #define program (q->program)

        //m.num_textures = nummeshes; // assume 1 texture only per mesh
        #undef textures
        m.textures = (q->textures);
        #define textures (q->textures)

        m.flags = flags;

        id44(m.pivot);
        model_set_state(m);
    }
    return m;
}
model_t model(const char *filename, int flags) {
    int len;  // vfs_pushd(filedir(filename))
    char *ptr = vfs_load(filename, &len); // + vfs_popd
    return model_from_mem( ptr, len, flags );
}

void model_get_bone_pose(model_t m, float curframe, int joint, vec3 *pos, vec3 *from) { // bugs?
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    // mat34 *mat = &frames[(int)curframe * numjoints];
    float *a = outframe[joint];

#if 0
           mat34 m34 = {0};
           muladd34(m34, outframe[int(att_indexes.x)], att_weights.x);
           muladd34(m34, outframe[int(att_indexes.y)], att_weights.y);
           muladd34(m34, outframe[int(att_indexes.z)], att_weights.z);
           muladd34(m34, outframe[int(att_indexes.w)], att_weights.w);
           objPos = vec4(att_position, 1.0) * m34;
#endif

    *pos = vec3(a[12], a[13], a[14]);

    if (joints[joint].parent >= 0) {
        float *b = outframe[joints[joint].parent];

        /*
        @fixme: do as above
        */

        *from = vec3(b[12], b[13], b[14]);
    } else {
        *from = vec3(0, 0, 0);
    }
}

float model_animate_clip(model_t m, float curframe, int minframe, int maxframe, bool loop) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    float retframe = -1;
    if( numframes > 0 ) {
        int frame1 = (int)/*floor*/(curframe);
        int frame2 = frame1 + (curframe >= m.curframe ? 1 : -1);
        float frameoffset = curframe - frame1;

        if( loop ) {
            int distance = (maxframe - minframe);
            frame1 = frame1 >= maxframe ? minframe : frame1 < minframe ? maxframe - clampf(minframe - frame1, 0, distance) : frame1;
            frame2 = frame2 >= maxframe ? minframe : frame2 < minframe ? maxframe - clampf(minframe - frame2, 0, distance) : frame2;
            retframe = frame1 + frameoffset;
        } else {
            frame1 = clampf(frame1, minframe, maxframe);
            frame2 = clampf(frame2, minframe, maxframe);
            retframe = minf(frame1 + frameoffset, maxframe); // clamp to maxframe
        }

        mat34 *mat1 = &frames[frame1 * numjoints];
        mat34 *mat2 = &frames[frame2 * numjoints];
        // @todo: add animation blending and inter-frame blending here
        // Interpolate matrixes between the two closest frames and concatenate with
        // parent matrix if necessary. Concatenate the result with the inverse of the base pose.
        for(int i = 0; i < numjoints; i++) {
            mat34 mat; lerp34(mat, mat1[i], mat2[i], frameoffset);
            if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], mat);
            else copy34(outframe[i], mat);
        }

        // model_render_skeleton
        if(0)
        for( int i = 0; i < numjoints; i++ ) {
            vec3 pos, from;
            model_get_bone_pose(m, curframe, i, &pos, &from);
            ddraw_line(pos, from);
        }
    }

    return retframe;
}

float model_animate(model_t m, float curframe) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;
    return model_animate_clip(m, curframe, 0, numframes-1, true);
}

static
void model_draw_call(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    struct iqmtriangle *tris = NULL;
    for(int i = 0; i < nummeshes; i++) {
        struct iqmmesh *m = &meshes[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i] );
        glUniform1i(glGetUniformLocation(program, "fsDiffTex"), 0 /*<-- unit!*/ );

        glDrawElements(GL_TRIANGLES, 3*m->num_triangles, GL_UNSIGNED_INT, &tris[m->first_triangle]);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", +m->num_triangles);
    }

    glBindVertexArray( 0 );
}

void model_render(model_t m, mat44 proj, mat44 view, mat44 model, int shader) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    mat44 mv; multiply44x2(mv, view, model);
    if( m.billboard ) {
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(m.billboard & 4) mv[4*0+0] = d, mv[4*0+1] =  0, mv[4*0+2] = 0;
        if(m.billboard & 2) mv[4*1+0] = 0, mv[4*1+1] = -d, mv[4*1+2] = 0;
        if(m.billboard & 1) mv[4*2+0] = 0, mv[4*2+1] =  0, mv[4*2+2] = d;
    }

    model_set_uniforms(m, shader > 0 ? shader : program, mv, proj, view, model);
    model_draw_call(m);
}

static
aabb aabb_transform( aabb A, mat44 M) {
    // Based on "Transforming Axis-Aligned Bounding Boxes" by Jim Arvo, 1990
    aabb B = { {M[12],M[13],M[14]}, {M[12],M[13],M[14]} }; // extract translation from mat44
    for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 3; j++ ) {
        float a = M[i*4+j] * j[&A.min.x]; // use mat33 from mat44
        float b = M[i*4+j] * j[&A.max.x]; // use mat33 from mat44
        if( a < b ) {
            i[&B.min.x] += a;
            i[&B.max.x] += b;
        } else {
            i[&B.min.x] += b;
            i[&B.max.x] += a;
        }
    }
    return B;
}

aabb model_aabb(model_t m, mat44 transform) {
    iqm_t *q = m.iqm;
    if( q && bounds ) {
    int f = ( (int)m.curframe ) % (numframes + !numframes);
    vec3 bbmin = ptr3(bounds[f].bbmin);
    vec3 bbmax = ptr3(bounds[f].bbmax);
    return aabb_transform(aabb(bbmin,bbmax), transform);
    }
    return aabb(vec3(0,0,0),vec3(0,0,0));
}

void model_destroy(model_t m) {
    FREE(m.verts);
    for( int i = 0, end = array_count(m.texture_names); i < end; ++i ) {
        FREE(m.texture_names[i]);
    }
    array_free(m.texture_names);

    iqm_t *q = m.iqm;
//    if(m.mesh) mesh_destroy(m.mesh);
    FREE(outframe);
    FREE(textures);
    FREE(baseframe);
    FREE(inversebaseframe);
    if(animdata != meshdata) FREE(animdata);
    //FREE(meshdata);
    FREE(frames);
    FREE(buf);
    FREE(q);
}

#undef program
#undef meshdata
#undef animdata
#undef nummeshes
#undef numtris
#undef numverts
#undef numjoints
#undef numframes
#undef numanims
#undef meshes
#undef textures
#undef joints
#undef poses
#undef anims
#undef baseframe
#undef inversebaseframe
#undef outframe
#undef frames
#undef vao
#undef ibo
#undef vbo
#undef bonematsoffset
#undef buf
#undef bounds
#line 0
#line 1 "fwk_renderdd.c"
static const char *dd_vs =
//    "#version 130\n"
    "in vec3 att_position;\n"
    "uniform mat4 u_MVP;\n"
    "uniform vec3 u_color;\n"
    "out vec3 out_color;\n"
    "void main() {\n"
    "    gl_Position = u_MVP * vec4(att_position, 1.0);\n"
    "    gl_PointSize = 4.0; /* for GL_POINTS draw commands */\n"
    "    out_color = u_color;\n"
    "}";

static const char *dd_fs =
//    "#version 130\n"
    "precision mediump float;\n"
    "in vec3 out_color;\n"
    "out vec4 fragcolor;\n"
    "void main() {\n"
    "   fragcolor = vec4(out_color, 1.0);\n"
    "}";

#define X(x) RGBX(x,255)
const uint32_t secret_palette[32] = { // pico8 secret palette (CC0, public domain)
X(0x000000),X(0x1D2B53),X(0x7E2553),X(0x008751),X(0xAB5236),X(0x5F574F),X(0xC2C3C7),X(0xFFF1E8), /*00.07*/
X(0xFF004D),X(0xFFA300),X(0xFFEC27),X(0x00E436),X(0x29ADFF),X(0x83769C),X(0xFF77A8),X(0xFFCCAA), /*08.15*/
X(0x291814),X(0x111D35),X(0x422136),X(0x125359),X(0x742F29),X(0x49333B),X(0xA28879),X(0xF3EF7D), /*16.23*/
X(0xBE1250),X(0xFF6C24),X(0xA8E72E),X(0x00B543),X(0x065AB5),X(0x754665),X(0xFF6E59),X(0xFF9D81), /*24.31*/
};
#undef X

static uint32_t    dd_color = ~0u;
static GLuint      dd_program = -1;
static int         dd_u_color = -1;
static map(unsigned,array(vec3)) dd_lists[2][3] = {0}; // [0/1 ontop][0/1/2 thin lines/thick lines/points]
static bool        dd_use_line = 0;
static bool        dd_ontop = 0;

void ddraw_flush() {
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    mat44 mvp;
    multiply44x2(mvp, camera_get_active()->proj, camera_get_active()->view); // MVP where M=id

    glUseProgram(dd_program);
    glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);

    static GLuint vao, vbo;
    if(!vao) glGenVertexArrays(1, &vao);    glBindVertexArray(vao);
    if(!vbo) glGenBuffers(1, &vbo);         glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS
    glEnable(GL_LINE_SMOOTH); // for GL_LINES (thin)

    for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
        GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
        glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
        for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
            int count = array_count(list);
            if(!count) continue;
                // color
                vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                // config vertex data
                glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                // feed vertex data
                glDrawArrays(mode, 0, count);
                profile_incstat("drawcalls", +1);
                profile_incstat(i < 2 ? "lines" : "points", count);
            array_clear(list);
        }
    }

    if(0)
    {
        // ddraw_text2d(vec3(0,0,1), "hello world\n123");
        float mvp[16]; float zdepth_max = 1;
        ortho44(mvp, -window_width()/2, window_width()/2, -window_height()/2, window_height()/2, -1, 1);
        translate44(mvp, -window_width()/2, window_height()/2, 0);
        glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);
        ddraw_color(BLACK);
        for(int i = 0; i < 10; ++i)
        ddraw_text(vec3(window_width()/2,-(i * 12),0), 0.5, "\nhello world"); // scale 0.5 is like 12units each
        ddraw_color(WHITE);
        for(int i = 0; i < 10; ++i)
        ddraw_text(vec3(window_width()/2+1,-(i * 12)-1,0), 0.5, "\nhello world"); // scale 0.5 is like 12units each
        for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
            GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
            glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
            for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
                int count = array_count(list);
                if(!count) continue;
                    // color
                    vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                    glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                    // config vertex data
                    glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                    // feed vertex data
                    glDrawArrays(mode, 0, count);
                    profile_incstat("drawcalls", +1);
                    profile_incstat(i < 2 ? "lines" : "points", count);
                array_clear(list);
            }
        }
    }

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(0);

    ddraw_color(WHITE); // reset color for next drawcall
}

void ddraw_ontop(int enabled) {
    dd_ontop = !!enabled;
}
void ddraw_color(unsigned rgb) {
    dd_color = rgb;
}
void ddraw_point(vec3 from) {
#if 0
    array(vec3) *found = map_find(dd_lists[dd_ontop][2], dd_color);
    if(!found) found = map_insert(dd_lists[dd_ontop][2], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][2], dd_color, 0);
#endif
    array_push(*found, from);
}
void ddraw_line_thin(vec3 from, vec3 to) { // thin lines
#if 0
    array(vec3) *found = map_find(dd_lists[dd_ontop][0], dd_color);
    if(!found) found = map_insert(dd_lists[dd_ontop][0], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][0], dd_color, 0);
#endif
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line(vec3 from, vec3 to) { // thick lines
#if 0
    array(vec3) *found = map_find(dd_lists[dd_ontop][1], dd_color);
    if(!found) found = map_insert(dd_lists[dd_ontop][1], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][1], dd_color, 0);
#endif
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line_dashed(vec3 from, vec3 to) { // thick lines
    vec3 dist = sub3(to, from); vec3 unit = norm3(dist);
    for( float len = 0, mag = len3(dist) / 2; len < mag; ++len ) {
        to = add3(from, unit);
        ddraw_line(from, to);
        from = add3(to, unit);
    }
}
void ddraw_triangle(vec3 pa, vec3 pb, vec3 pc) {
    ddraw_line(pa, pb);
    ddraw_line(pa, pc);
    ddraw_line(pb, pc);
}
void ddraw_axis(float units) {
    ddraw_color(RED);   ddraw_line(vec3(0,0,0), vec3(units,0,0)); ddraw_line_dashed(vec3(0,0,0), vec3(-units,0,0));
    ddraw_color(GREEN); ddraw_line(vec3(0,0,0), vec3(0,units,0)); ddraw_line_dashed(vec3(0,0,0), vec3(0,-units,0));
    ddraw_color(BLUE);  ddraw_line(vec3(0,0,0), vec3(0,0,units)); ddraw_line_dashed(vec3(0,0,0), vec3(0,0,-units));
}
void ddraw_ground_(float scale) { // 10x10
    ddraw_color( WHITE ); // outer
    for( float i = -scale, c = 0; c <= 20; c += 20, i += c * (scale/10) ) {
        ddraw_line(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
    ddraw_color( GRAY ); // inner
    for( float i = -scale + scale/10, c = 1; c < 20; ++c, i += (scale/10) ) {
        ddraw_line_thin(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line_thin(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
}
void ddraw_ground(float scale) {
    if( scale ) {
        ddraw_ground_(scale);
    } else {
        ddraw_ground_(100);
        ddraw_ground_(10);
        ddraw_ground_(1);
        ddraw_ground_(0.1);
        ddraw_ground_(0.01);
    }
}
void ddraw_grid(float scale) {
    ddraw_ground(scale);
    ddraw_axis(scale ? scale : 100);
}


void (ddraw_text)(vec3 pos, float scale, const char *text) {
    // [ref] http://paulbourke.net/dataformats/hershey/ (PD)
    // [ref] https://sol.gfxile.net/hershey/fontprev.html (WTFPL2)
    static const signed char *hershey[] = { /* simplex font */
    "AQ","IKFVFH@@FCEBFAGBFC","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
    "M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
    "UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
    "OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
    "ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
    "C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
    "LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
    "VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
    "AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
    "EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
    "KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
    "EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
    "ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
    "MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
    "RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
    "SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
    "SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
    "EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
    "EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
    "RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
    "EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
    "IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
    "FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
    "COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
    "EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
    "LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
    "PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
    ":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
    "A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
    "NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
    "LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
    "ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
    "IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
    "FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
    "MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
    "YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
    "XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };
    vec3 src = pos, old = {0};
    for( signed char c; (c = *text++, c > 0 && c < 127); ) {
        if( c == '\n' || c == '\r' ) {
            pos.x = src.x, pos.y -= scale * (hershey['W'-32][1] - 65) * 1.25f; // spacing @1
        } else {
            char *glyph = (char*)hershey[c - 32];
            if( c > 32 ) for( int pen = 0, i = 0; i < (glyph[0] - 65); i++ ) { // verts @0
                int x = glyph[2 + i*2 + 0] - 65, y = glyph[2 + i*2 + 1] - 65;
                if( x == -1 && y == -1 ) pen = 0; else {
                    vec3 next = add3(pos, vec3(scale*x, scale*y, 0));
                    if( !pen ) pen = 1; else ddraw_line(old, next);
                    old = next;
                }
            }
            pos.x += scale * (glyph[1] - 65); // spacing @1
        }
    }
}

/*
void ddraw_prism(vec3 pos, int segments) {
    ddraw_color(vec3(1,1,1));
    float cycle = 2 * 3.14159 + 2 * 3.14159 / segments, acc = 0;
    for( int i = 0; i < segments; ++i, acc += cycle ) {
        ddraw_line(add3(pos,vec3(cos(acc),0,sin(acc))), add3(pos,vec3(cos(acc+cycle),0,sin(acc+cycle))));
    }
}
*/
void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments) {
    vec3 left = {0}, up = {0};
    ortho3(&left, &up, normal);

    vec3 point, lastPoint;
    up = scale3(up, radius);
    left = scale3(left, radius);
    lastPoint = add3(center, up);
    vec3 pivot = add3(center, scale3(normal, height));

    for (int i = 1; i <= segments; ++i) {
        const float radians = (C_PI * 2) * i / segments;

        vec3 vs = scale3(left, sinf(radians));
        vec3 vc = scale3(up,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        if( height > 0 ) ddraw_line(point, pivot);
        else if(height < 0) {
            ddraw_line(point, add3(point,scale3(normal, -height)));
        }
        lastPoint = point;
    }

    if(height < 0) ddraw_prism(add3(center, scale3(normal, -height)), radius, 0, normal, segments);
}

void ddraw_cube(vec3 center, float radius) { // draw_prism(center, 1, -1, vec3(0,1,0), 4);
    float half = radius * 0.5f;
    vec3 l = vec3(center.x-half,center.y+half,center.z-half); // left-top-far
    vec3 r = vec3(center.x+half,center.y-half,center.z+half); // right-bottom-near

    ddraw_line(l, vec3(r.x,l.y,l.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,l.y,r.z));
    ddraw_line(vec3(r.x,l.y,r.z), vec3(l.x,l.y,r.z));
    ddraw_line(vec3(l.x,l.y,r.z), l);
    ddraw_line(l, vec3(l.x,r.y,l.z));

    ddraw_line(r, vec3(l.x,r.y,r.z));
    ddraw_line(vec3(l.x,r.y,r.z), vec3(l.x,r.y,l.z));
    ddraw_line(vec3(l.x,r.y,l.z), vec3(r.x,r.y,l.z));
    ddraw_line(vec3(r.x,r.y,l.z), r);
    ddraw_line(r, vec3(r.x,l.y,r.z));

    ddraw_line(vec3(l.x,l.y,r.z), vec3(l.x,r.y,r.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,r.y,l.z));
}
void ddraw_normal(vec3 pos, vec3 n) {
    ddraw_color(YELLOW);
    ddraw_line(pos, add3(pos, norm3(n)));
}

void ddraw_circle(vec3 pos, vec3 n, float r) { ddraw_prism(pos, r, 0, n, 32); }
void ddraw_hexagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 6); }
void ddraw_pentagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 5); }
void ddraw_square(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 4); }
//void ddraw_triangle(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 3); }
void ddraw_sphere(vec3 center, float radius) {
    float lod = 15, yp = -radius, rp = 0, y, r, x, z;
    for( int j = 1; j <= lod / 2; ++j, yp = y, rp = r ) {
        y = j * 2.f / (lod / 2) - 1;
        r = cosf(y * 3.14159f / 2) * radius;
        y = sinf(y * 3.14159f / 2) * radius;

        float xp = 1, zp = 0;
        for( int i = 1; i <= lod; ++i, xp = x, zp = z ) {
            x = 3.14159f * 2 * i / lod;
            z = sinf(x);
            x = cosf(x);

            vec3 a1 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b1 = add3(center, vec3(xp * r,  y,  zp * r));
            vec3 c1 = add3(center, vec3(x  * r,  y,  z  * r));

            ddraw_line(a1,b1);
            ddraw_line(b1,c1);
            ddraw_line(c1,a1);

            vec3 a2 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b2 = add3(center, vec3(x  * r,  y,  z  * r));
            vec3 c2 = add3(center, vec3(x  * rp, yp, z  * rp));

            ddraw_line(a2,b2);
            ddraw_line(b2,c2);
            ddraw_line(c2,a2);
        }
    }
}
void ddraw_box(vec3 c, vec3 extents) {
    vec3 points[8], whd = scale3(extents, 0.5f);
    #define DD_BOX_V(v, op1, op2, op3) (v).x = c.x op1 whd.x; (v).y = c.y op2 whd.y; (v).z = c.z op3 whd.z
    DD_BOX_V(points[0], -, +, +);
    DD_BOX_V(points[1], -, +, -);
    DD_BOX_V(points[2], +, +, -);
    DD_BOX_V(points[3], +, +, +);
    DD_BOX_V(points[4], -, -, +);
    DD_BOX_V(points[5], -, -, -);
    DD_BOX_V(points[6], +, -, -);
    DD_BOX_V(points[7], +, -, +);
    #undef DD_BOX_V
    ddraw_bounds(points);
}
void ddraw_capsule(vec3 from, vec3 to, float r) {
    /* calculate axis */
    vec3 up, right, forward;
    forward = sub3(to, from);
    forward = norm3(forward);
    ortho3(&right, &up, forward);

    /* calculate first two cone verts (buttom + top) */
    vec3 lastf, lastt;
    lastf = scale3(up,r);
    lastt = add3(to,lastf);
    lastf = add3(from,lastf);

    /* step along circle outline and draw lines */
    enum { step_size = 20 };
    for (int i = step_size; i <= 360; i += step_size) {
        /* calculate current rotation */
        vec3 ax = scale3(right, sinf(i*TO_RAD));
        vec3 ay = scale3(up, cosf(i*TO_RAD));

        /* calculate current vertices on cone */
        vec3 tmp = add3(ax, ay);
        vec3 pf = scale3(tmp, r);
        vec3 pt = scale3(tmp, r);

        pf = add3(pf, from);
        pt = add3(pt, to);

        /* draw cone vertices */
        ddraw_line(lastf, pf);
        ddraw_line(lastt, pt);
        ddraw_line(pf, pt);

        lastf = pf;
        lastt = pt;

        /* calculate first top sphere vert */
        vec3 prevt = scale3(tmp, r);
        vec3 prevf = add3(prevt, from);
        prevt = add3(prevt, to);

        /* sphere (two half spheres )*/
        for (int j = 1; j < 180/step_size; j++) {
            /* angles */
            float ta = j*step_size;
            float fa = 360-(j*step_size);

            /* top half-sphere */
            ax = scale3(forward, sinf(ta*TO_RAD));
            ay = scale3(tmp, cosf(ta*TO_RAD));

            vec3 t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, to);
            ddraw_line(pf, prevt);
            prevt = pf;

            /* bottom half-sphere */
            ax = scale3(forward, sinf(fa*TO_RAD));
            ay = scale3(tmp, cosf(fa*TO_RAD));

            t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, from);
            ddraw_line(pf, prevf);
            prevf = pf;
        }
    }
}
void ddraw_pyramid(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, height, vec3(0,1,0), segments);
}
void ddraw_diamond(vec3 from, vec3 to, float size) {
    poly p = diamond(from, to, size);
    vec3 *dmd = p.verts;

    vec3 *a = dmd + 0;
    vec3 *b = dmd + 1;
    vec3 *c = dmd + 2;
    vec3 *d = dmd + 3;
    vec3 *t = dmd + 4;
    vec3 *f = dmd + 5;

    /* draw vertices */
    ddraw_line(*a, *b);
    ddraw_line(*b, *c);
    ddraw_line(*c, *d);
    ddraw_line(*d, *a);

    /* draw roof */
    ddraw_line(*a, *t);
    ddraw_line(*b, *t);
    ddraw_line(*c, *t);
    ddraw_line(*d, *t);

    /* draw floor */
    ddraw_line(*a, *f);
    ddraw_line(*b, *f);
    ddraw_line(*c, *f);
    ddraw_line(*d, *f);

    poly_free(&p);
}
void ddraw_cone(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 24);
}
void ddraw_cone_lowres(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 3);
}
void ddraw_bone(vec3 center, vec3 end) {
    vec3 diff3 = sub3(end, center);
    float len = len3(diff3), len10 = len / 10;
    ddraw_prism(center, len10, 0, vec3(1,0,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,1,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,0,1), 24);
    ddraw_line(end, add3(center, vec3(0,+len10,0)));
    ddraw_line(end, add3(center, vec3(0,-len10,0)));
}
void ddraw_bounds(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        ddraw_line(points[i], points[(i + 1) & 3]);
        ddraw_line(points[i], points[4 + i]);
        ddraw_line(points[4 + i], points[4 + ((i + 1) & 3)]);
    }
}
void ddraw_bounds_corners(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        #define ddraw_unit(a,b) ddraw_line(a,add3(a,norm3(sub3(b,a)))), ddraw_line(b,add3(b,norm3(sub3(a,b))))
        ddraw_unit(points[i], points[(i + 1) & 3]);
        ddraw_unit(points[i], points[4 + i]);
        ddraw_unit(points[4 + i], points[4 + ((i + 1) & 3)]);
        #undef ddraw_unit
    }
}
void ddraw_aabb(vec3 minbb, vec3 maxbb) {
    vec3 points[8], bb[2] = { minbb, maxbb };
    for (int i = 0; i < 8; ++i) {
        points[i].x = bb[(i ^ (i >> 1)) & 1].x;
        points[i].y = bb[     (i >> 1)  & 1].y;
        points[i].z = bb[     (i >> 2)  & 1].z;
    }
    ddraw_bounds/*_corners*/(points);
}
void ddraw_frustum(float projview[16]) {
    mat44 clipmatrix = {0}; // clip matrix
    invert44(clipmatrix, projview);

    // Start with the standard clip volume, then bring it back to world space.
    const vec3 planes[8] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1}, // near plane
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}, // far plane
    };

    vec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for( int i = 0; i < 8; ++i ) {
        // wCoords[i] = matTransformPointXYZW2(&points[i], planes[i], clipmatrix);
        vec3 *out = &points[i], in = planes[i]; const float *m = clipmatrix;
        out->x = (m[0] * in.x) + (m[4] * in.y) + (m[ 8] * in.z) + m[12]; // in.w (vec4) assumed to be 1
        out->y = (m[1] * in.x) + (m[5] * in.y) + (m[ 9] * in.z) + m[13];
        out->z = (m[2] * in.x) + (m[6] * in.y) + (m[10] * in.z) + m[14];
        wCoords[i] =  (m[3] * in.x) + (m[7] * in.y) + (m[11] * in.z) + m[15]; // rw

        // bail if any W ended up as zero.
        const float epsilon = 1e-9f;
        if (absf(wCoords[i]) < epsilon) {
            return;
        }
    }

    // Divide by the W component of each:
    for( int i = 0; i < 8; ++i ) {
        points[i].x /= wCoords[i];
        points[i].y /= wCoords[i];
        points[i].z /= wCoords[i];
    }

    // Connect the dots:
    ddraw_bounds(points);
}
void ddraw_arrow(vec3 begin, vec3 end) {
    vec3 diff = sub3(end, begin);
    float len = len3(diff);
    float stick_len = len * 2 / 3, head_radius = len / 6;

    ddraw_line(begin, end);
    ddraw_cone_lowres(add3(begin, scale3(norm3(diff), stick_len)), end, head_radius);
}

void ddraw_plane(vec3 p, vec3 n, float scale) {
    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(n, dot3(n, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(n, v1);

    // draw axis
    ddraw_line(p, add3(p,n));
    ddraw_line(p, add3(p,v1));
    ddraw_line(p, add3(p,v2));

    // get plane coords
    v1 = scale3(v1, scale);
    v2 = scale3(v2, scale);
    vec3 p1 = add3(add3(p, v1), v2);
    vec3 p2 = add3(sub3(p, v1), v2);
    vec3 p3 = sub3(sub3(p, v1), v2);
    vec3 p4 = sub3(add3(p, v1), v2);

    // draw plane
    ddraw_line(p1, p2);
    ddraw_line(p2, p3);
    ddraw_line(p3, p4);
    ddraw_line(p4, p1);
}

void ddraw_boid(vec3 position, vec3 dir) {
    dir = norm3(dir);

    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(dir, dot3(dir, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(dir, v1);
    v1 = cross3(dir, v2);

    uint32_t bak = dd_color;
    ddraw_color( position.y < 0 ? ORANGE : CYAN );

        vec3 front = add3(position, scale3(dir, 1));
        vec3 back = add3(position, scale3(dir, -0.25f));
        vec3 right = add3(back, scale3(v1, 0.5f));
        vec3 left = add3(back, scale3(v1, -0.5f));
        ddraw_line( front, left );
        ddraw_line( left, position );
        ddraw_line( position, right );
        ddraw_line( right, front );

    dd_color = bak;
}

void ddraw_circle__with_orientation(vec3 center, vec3 dir, float radius) {
    // we'll skip 3 segments out of 32. 1.5 per half circle.
    int segments = 32, skip = 3, drawn_segments = segments-skip;

//  dir = norm3(dir);
    vec3 right = cross3(dir, vec3(0,1,0));
    vec3 up = cross3(dir, right);
    right = cross3(dir, up);

    vec3 point, lastPoint;
    dir = scale3(dir, radius);
    right = scale3(right, radius);
    //lastPoint = add3(center, dir);

    {
        const float radians = (C_PI * 2) * (0+skip/2.f) / segments;
        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));
        lastPoint = add3(center, vs);
        lastPoint = add3(lastPoint, vc);
    }

    //ddraw_color(GREEN);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius*1.25)));
    //ddraw_color(YELLOW);

    for (int i = 0; i <= drawn_segments; ++i) {
        const float radians = (C_PI * 2) * (i+skip/2.f) / segments;

        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        lastPoint = point;
    }

    //ddraw_color(RED);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius*1.25)));
}

void ddraw_position_dir( vec3 position, vec3 direction, float radius ) {
    // idea from http://www.cs.caltech.edu/~keenan/m3drv.pdf and flotilla game UI
    uint32_t bak = dd_color;

    vec3 ground = vec3(position.x, 0, position.z);
    ddraw_color( position.y < 0 ? ORANGE : CYAN );
    ddraw_point( ground );
    ddraw_point( position );
    (position.y < 0 ? ddraw_line_dashed : ddraw_line)( ground, position );

    vec3 n = norm3(direction), up = vec3(0,1,0);
    for( int i = 0; i < 10 && i <= abs(position.y); ++i ) {
        if( i < 2 && len3(direction) )
        ddraw_circle__with_orientation(ground, n, radius);
        else
        ddraw_circle(ground, up, radius);
        radius *= 0.9f;
    }

    dd_color = bak;
}

void ddraw_position( vec3 position, float radius ) {
    ddraw_position_dir(position, vec3(0,0,0), radius);
}

void ddraw_init() {
    for( int i = 0; i < 2; ++i )
    for( int j = 0; j < 3; ++j ) map_init(dd_lists[i][j], less_int, hash_int);
    dd_program = shader(dd_vs,dd_fs,"att_position","fragcolor");
    dd_u_color = glGetUniformLocation(dd_program, "u_color");
    ddraw_flush(); // alloc vao & vbo, also resets color
}

void ddraw_demo() {
    // freeze current frame for (frustum) camera forensics
    static mat44 projview_copy;
    do_once {
        multiply44x2(projview_copy, camera_get_active()->proj, camera_get_active()->view);
    }
    ddraw_frustum(projview_copy);

    //ddraw_grid();

    vec3 origin = {0,0,0};
    ddraw_color(ORANGE);
    ddraw_arrow(origin, vec3(-1,1,1));
    ddraw_color(YELLOW);
    ddraw_text(vec3(-1,1,1), 0.008f, "hello 1%s2!", "world");

    const char abc[] = " !\"#$%&'()*+,-./\n"
        "0123456789:;<=>?@\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\n"
        "abcdefghijklmnopqrstuvwxyz{|}~";
    ddraw_text(vec3(2,2,2), 0.008f, abc);


    for( int i = -5; i <= 5; ++i ) {
        ddraw_pyramid(vec3(i*2,0,3),  0, i+5+2); ddraw_text(vec3(i*2,0,3), 0.008f, "%d/1", i);
        ddraw_pyramid(vec3(i*2,0,6), -2, i+5+2); ddraw_text(vec3(i*2,0,6), 0.008f, "%d/2", i);
        ddraw_pyramid(vec3(i*2,0,9), +2, i+5+2); ddraw_text(vec3(i*2,0,9), 0.008f, "%d/3", i);
    }

#if 1 // @fixme: add positions to these
    // ddraw_triangle(origin, 1);
    ddraw_square(origin, 1);
    ddraw_pentagon(origin, 1);
    ddraw_hexagon(origin, 1);
    ddraw_cube(origin, 1);
    ddraw_pyramid(origin, 2, 3);
    ddraw_pyramid(origin, 2, 16);
    ddraw_cone(origin, add3(origin, vec3(0,1,0)), 0.5f);
    ddraw_arrow(origin, vec3(0,1,0));
    ddraw_bone(vec3(0,0,0), vec3(3,3,3));
    ddraw_aabb(vec3(0,0,0), vec3(1,1,1));
#endif

    ddraw_plane(vec3(0,10,0), vec3(0,1,0), 10);
    //ddraw_boid(vec3(15,0,15), vec3(-15,0,-15) );
    ddraw_position(vec3(10,10,10), 1);
    ddraw_position(vec3(-10,-10,10), 1);

    ddraw_point(vec3(-2,0,-2));
    ddraw_color(PURPLE);
    ddraw_sphere(vec3(-3,0,-3),1);
}
#line 0
#line 1 "fwk_scene.c"
//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

camera_t camera() {
    camera_t cam = {0};
    cam.speed = 1;
    cam.position = vec3(10,10,10);
    cam.last_look = cam.last_move = vec3(0,0,0);
    cam.up = vec3(0,1,0);

    // @todo: remove this hack
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--with-camera-smooth");
    if( smoothing ) {
        for( int i = 0; i < 1000; ++i ) {
            camera_move(&cam,0,0,0);
            camera_fps(&cam,0,0);
        }
    }

    // update proj & view
    camera_lookat(&cam,vec3(-5,0,-5));

    last_camera = &cam;
    return cam;
}

camera_t *camera_get_active() {
    static camera_t defaults = {0};
    if( !last_camera ) {
        identity44(defaults.view);
        identity44(defaults.proj);
        last_camera = &defaults;
    }
    return last_camera;
}

void camera_move(camera_t *cam, float x, float y, float z) {
    // enable camera smoothing
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--with-camera-smooth");
    if( smoothing ) {
        float move_friction = 0.99f;
        cam->last_move = scale3(cam->last_move, move_friction);
        float move_filtering = 0.025f;
        x = cam->last_move.x = x * move_filtering + cam->last_move.x * (1 - move_filtering);
        y = cam->last_move.y = y * move_filtering + cam->last_move.y * (1 - move_filtering);
        z = cam->last_move.z = z * move_filtering + cam->last_move.z * (1 - move_filtering);
    }

    vec3 dir = norm3(cross3(cam->look, cam->up));
    cam->position = add3(cam->position, scale3(dir, x)); // right
    cam->position = add3(cam->position, scale3(cam->up, y)); // up
    cam->position = add3(cam->position, scale3(cam->look, z)); // front

    camera_fps(cam, 0, 0);
}

void camera_lookat(camera_t *cam, vec3 target) {
    // invert expression that cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    // look.y = sin p > y = asin(p)
    // look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
    // look.z = sin y * cos p; -> cos p = look.z / sin y /
    // so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

    vec3 look = norm3(sub3(target, cam->position));
    const float rad2deg = 1 / 0.0174532f;
    float pitch = asin(look.y) * rad2deg;
    float yaw = atan2(look.z, look.x) * rad2deg; // coords swapped. it was (look.x, look.z) before. @todo: testme

    camera_fps(cam, yaw-cam->yaw, pitch-cam->pitch);
}

void camera_enable(camera_t *cam) {
    // camera_t *other = camera_get_active(); // init default camera in case there is none
    last_camera = cam;
    // trigger a dummy update -> update matrices
    camera_fps(cam, 0, 0);
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    last_camera = cam;

    // enable camera smoothing
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--with-camera-smooth");
    if( smoothing ) {
        float look_friction = 0.999f;
        cam->last_look.x *= look_friction;
        cam->last_look.y *= look_friction;
        float look_filtering = 0.05f;
        yaw = cam->last_look.y = yaw * look_filtering + cam->last_look.y * (1 - look_filtering);
        pitch = cam->last_look.x = pitch * look_filtering + cam->last_look.x * (1 - look_filtering);
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad;
    cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));

    lookat44(cam->view, cam->position, add3(cam->position, cam->look), cam->up); // eye,center,up
    perspective44(cam->proj, 45, window_width() / ((float)window_height()+!window_height()), 0.01f, 1000.f);

#if 0 // isometric/dimetric
    #define orthogonal(proj, fov, aspect, znear, zfar) \
    ortho44((proj), -(fov) * (aspect), (fov) * (aspect), -(fov), (fov), (znear), (zfar))

    float DIMETRIC = 30.000f;
    float ISOMETRIC = 35.264f;
    float aspect = window_width() / ((float)window_height()+!!window_height());
    orthogonal(cam->proj, 45, aspect, -1000, 1000); // why -1000?
    // cam->yaw = 45;
    cam->pitch = -ISOMETRIC;
#endif
}

void camera_orbit( camera_t *cam, float yaw, float pitch, float inc_distance ) {
    vec2 inc_mouse = vec2(yaw, pitch);

    // @todo: worth moving all these members into camera_t ?
    static vec2 _mouse = {0,0};
    static vec2 _polarity = { +1,-1 };
    static vec2 _sensitivity = { 2,2 };
    static float _friction = 0.75; //99;
    static float _distance; do_once _distance = len3(cam->position);

    // update dummy state
    camera_fps(cam, 0,0);

    // add smooth input
    _mouse = mix2(_mouse, add2(_mouse, mul2(mul2(inc_mouse,_sensitivity),_polarity)), _friction);
    _distance = mixf(_distance, _distance+inc_distance, _friction);

    // look: update angles
    vec2 offset = sub2( _mouse, ptr2(&cam->last_move.x) );
    if( 1 ) { // if _enabled
        cam->yaw += offset.x;
        cam->pitch += offset.y;
        // look: limit pitch angle [-89..89]
        cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;
    }

    // compute view matrix
    float x = rad(cam->yaw), y = rad(cam->pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
    lookat44(cam->view, vec3( cx*cy*_distance, sy*_distance, sx*cy*_distance ), vec3(0,0,0), vec3(0,1,0) );

    // save for next call
    cam->last_move.x = _mouse.x;
    cam->last_move.y = _mouse.y;
}

// -----------------------------------------------------------------------------

static
void object_update(object_t *obj) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(obj->euler.x,obj->euler.y,obj->euler.z));
    compose44(obj->transform, obj->pos, mulq(e, p), obj->sca);
}

object_t object() {
    object_t obj = {0};
    identity44(obj.transform);
    //obj.rot = idq();
    obj.sca = vec3(1,1,1);
    //obj.bounds = aabb(vec3(0,0,0),vec3(1,1,1)); // defaults to small 1-unit cube
object_rotate(&obj, vec3(0,0,0));
    //array_init(obj.textures);
    return obj;
}

void object_pivot(object_t *obj, vec3 euler) {
    obj->pivot = euler;
    object_update(obj);
}

void object_rotate(object_t *obj, vec3 euler) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(euler.x,euler.y,euler.z));
    obj->rot = mulq(p,e);
    obj->euler = euler;
    object_update(obj);
}

void object_teleport(object_t *obj, vec3 pos) {
    obj->pos = pos;
    object_update(obj);
}

void object_move(object_t *obj, vec3 inc) {
    obj->pos = add3(obj->pos, inc);
    object_update(obj);
}

void object_scale(object_t *obj, vec3 sca) {
    obj->sca = vec3(sca.x, sca.y, sca.z);
    object_update(obj);
}

vec3 object_position(object_t *obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void object_model(object_t *obj, model_t model) {
    obj->model = model;
}

void object_push_diffuse(object_t *obj, texture_t tex) {
    array_push(obj->textures, tex.id);
}

void object_pop_diffuse(object_t *obj) {
    array_pop(obj->textures);
}

void object_diffuse(object_t *obj, texture_t tex) {
    array_clear(obj->textures);
    object_push_diffuse(obj, tex);
}

void object_billboard(object_t *obj, unsigned mode) {
    obj->billboard = mode;
}

// -----------------------------------------------------------------------------

array(scene_t*) scenes;
scene_t* last_scene;

scene_t* scene_get_active() {
    return last_scene;
}

scene_t* scene_push() {
    scene_t *s = REALLOC(0, sizeof(scene_t)), clear = {0}; *s = clear;
    const char *symbols[] = { "{{include-shadowmap}}", fs_0_0_shadowmap_lit };
    s->program = shader(strlerp(1, symbols, vs_332_32), strlerp(1, symbols, fs_32_4_model), "att_position,att_normal,att_texcoord", "fragcolor");
    s->skybox = skybox(NULL, 0);
    array_push(scenes, s);
    last_scene = s;
    return s;
}

void scene_pop() {
    // @fixme: fix leaks, scene_cleanup();
    scene_t clear = {0};
    *last_scene = clear;
    array_pop(scenes);
    last_scene = *array_back(scenes);
}

int scene_merge(const char *source) {
    int count = 0;
    if( data_push(source) ) {
        for(int i = 0, e = data_count("/") - 1; i <= e ; ++i) {
            const char *skybox_folder = data_string("/[%d]/skybox",i);
            if( skybox_folder[0] ) {
                PRINTF("Loading skybox folder: %s\n", skybox_folder);
                last_scene->skybox = skybox( skybox_folder, 0 );
                continue;
            }
            const char *mesh_file = data_string("/[%d]/mesh",i);
            const char *texture_file = data_string("/[%d]/texture",i);
            const char *animation_file = data_string("/[%d]/animation",i);
            vec3 position = vec3(data_float("/[%d]/position[0]",i),data_float("/[%d]/position[1]",i),data_float("/[%d]/position[2]",i));
            vec3 rotation = vec3(data_float("/[%d]/rotation[0]",i),data_float("/[%d]/rotation[1]",i),data_float("/[%d]/rotation[2]",i));
            vec3 scale = scale3(vec3(1,1,1), data_float("/[%d]/scale",i));
            bool opt_swap_zy = data_int("/[%d]/swapzy",i);
            bool opt_flip_uv = data_int("/[%d]/flipuv",i);
            PRINTF("Scene %d/%d Loading: %s\n", i, e, mesh_file);
            PRINTF("Scene %d/%d Texture: %s\n", i, e, texture_file);
            PRINTF("Scene %d/%d Animation: %s\n", i, e, animation_file);
            PRINTF("Scene %d/%d Position: (%f,%f,%f)\n", i, e, position.x, position.y, position.z);
            PRINTF("Scene %d/%d Rotation: (%f,%f,%f)\n", i, e, rotation.x, rotation.y, rotation.z);
            PRINTF("Scene %d/%d Scale: (%f,%f,%f)\n", i, e, scale.x, scale.y, scale.z);
            PRINTF("Scene %d/%d Swap_ZY: %d\n", i, e, opt_swap_zy );
            PRINTF("Scene %d/%d Flip_UV: %d\n", i, e, opt_flip_uv );
            model_t m = model_from_mem(vfs_read(mesh_file), vfs_size(mesh_file), 0/*opt_swap_zy*/);
            //char *a = archive_read(animation_file);
            object_t *o = scene_spawn();
            object_model(o, m);
            if( texture_file[0] ) object_diffuse(o, texture_from_mem(vfs_read(texture_file), vfs_size(texture_file), opt_flip_uv ? IMAGE_FLIP : 0) );
            object_scale(o, scale);
            object_teleport(o, position);
            object_pivot(o, rotation); // object_rotate(o, rotation);
            //object_name(x), scene_find(name)
// o->bounds = aabb(mul3(m.bounds.min,o->sca),mul3(m.bounds.max,o->sca));
// PRINTF("aabb={%f,%f,%f},{%f,%f,%f}\n", o->bounds.min.x, o->bounds.min.y, o->bounds.min.z, o->bounds.max.x, o->bounds.max.y, o->bounds.max.z);

/*
            if(opt_swap_zy) {
                // swap zy bounds
                vec3 min = o->bounds.min, max = o->bounds.max;
                o->bounds = aabb( vec3(min.x,min.z,min.y), vec3(max.x,max.z,max.y) );
            }
*/

            count++;
        }
        data_pop();
    }
    // PRINTF("scene loading took %5.2fs\n", secs);
    PRINTF("%d objects merged into scene\n", count);
    return count;
}

object_t* scene_spawn() {
    object_t obj = object();
    array_push(last_scene->objs, obj);

    return array_back(last_scene->objs);
}

unsigned scene_count() {
    return array_count(last_scene->objs);
}

object_t* scene_index(unsigned obj_index) {
    unsigned obj_count = scene_count(last_scene);
    ASSERT(obj_index < obj_count, "Object index %d exceeds number (%d) of spawned objects", obj_index, obj_count);
    return &last_scene->objs[obj_index];
}

void scene_render(int flags) {
    camera_t *cam = camera_get_active();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(last_scene->program);

    if(flags & SCENE_BACKGROUND) {
        if(last_scene->skybox.program) {
        skybox_push_state(&last_scene->skybox, cam->proj, cam->view);

        glDisable(GL_DEPTH_TEST);
    //  glDepthFunc(GL_LESS);
    //    glActiveTexture(GL_TEXTURE0);
    //    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    //    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );

        mesh_render(&last_scene->skybox.geometry);
        skybox_pop_state(&last_scene->skybox);
        }

        ddraw_flush();
    }

    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);
//  glUseProgram(last_scene->program);

    // @fixme: CW ok for one-sided rendering. CCW ok for FXs. we need both
    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );
    // @todo alpha mode
    // @todo texture mode

    if( flags & SCENE_FOREGROUND ) {
        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            mat44 *views = (mat44*)(&cam->view);

            // @todo: avoid heap allocs here?
            static array(handle) old_textures = 0;

            int do_retexturing = model->iqm && array_count(obj->textures) > 0;
            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    array_push(old_textures, model->iqm->textures[i]);
                    model->iqm->textures[i] = *array_back(obj->textures);
                }
            }

            model->billboard = obj->billboard;
            model_render(*model, cam->proj, cam->view, obj->transform, 0);

            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    model->iqm->textures[i] = old_textures[i];
                }
                array_resize(old_textures, 0);
            }
        }
        glBindVertexArray(0);
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}
#line 0
#line 1 "fwk_script.c"
typedef lua_State lua;

// the Lua interpreter
lua *L;

static void* script__realloc(void *userdef, void *ptr, size_t osize, size_t nsize) {
    return ptr = REALLOC( ptr, (osize+1) * nsize );
}
static int script__traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) { // try metamethod if non-string error object
        if (lua_isnoneornil(L, 1) ||
            !luaL_callmeta(L, 1, "__tostring") ||
            !lua_isstring(L, -1))
            return 1;  // return non-string error object
        lua_remove(L, 1);  // replace object with result of __tostring metamethod
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}
static void script__error(lua_State *L, int status) {
    if (status != 0) {
        const char *errormsg = lua_tostring(L, -1);
        PRINTF( "!-- %s\n", errormsg);
        lua_pop(L, 1); // remove error message
    }
}
static int script__call(lua_State *L, int narg, int clear) {
#if WITH_FASTCALL_LUA
    lua_call(L, 0, 0);
    return 0;
#else
    int base = lua_gettop(L) - narg;  // function index
    lua_pushcfunction(L, script__traceback);  // push traceback function
    lua_insert(L, base);  // put it under chunk and args
    int status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
    script__error(L, status);
    lua_remove(L, base);  // remove traceback function
    if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0); // force gc in case of errors
    return status;
#endif
}

void script_bind_function(const char *c_name, void *c_function) {
    lua_pushcfunction( L, c_function );
    lua_setglobal( L, c_name );
}

void script_call(const char *lua_function) {
    lua_getglobal( L, lua_function );
    script__call( L, 0, 1 );
}

void script_bind_class(const char *classname, int num, const char **methods, void **functions) {
    lua_newtable( L );

    for( int i = 0; i < num; ++i) {
        lua_pushcfunction( L, functions[i] );
        lua_setfield( L, 1, methods[i] );
    }

    lua_setglobal( L, classname );
}

void script_run(const char *script) {
    int ret = luaL_dostring(L, script);
    if( ret != LUA_OK ) {
      PRINTF("!Script failed to run: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1); // pop error message
    }
}

void script_runfile(const char *pathfile) {
    PRINTF( "Loading script '%s'\n", pathfile );
    int loadResult = luaL_loadfile( L, pathfile );

    /**/ if( loadResult == LUA_OK ) {
        script__call( L, 0, 1 );
    }
    else if( loadResult == LUA_ERRSYNTAX ) {
        PRINTF("!Script failed to load (LUA_ERRSYNTAX, '%s'): %s\n", lua_tostring( L, 1 ), pathfile );
//      lua_pop(L, 1); // pop error message
    }
    else if( loadResult == LUA_ERRMEM ) {
        PRINTF("!Script failed to load (LUA_ERRMEM): %s\n", pathfile);
    }
    else {
        PRINTF("!Script failed to load: %s\n", pathfile );
    }
}

// syntax sugars
/* usage:
int window_create_lua(lua *L) {
    window_create(arg_float(1), arg_int(2));
    return_void(0);
}
int window_swap_lua(lua *L) {
    int r = window_swap();
    return_int(r);
}
*/

#define arg_int(nth) lua_tointeger(L, nth)
#define arg_bool(nth) lua_toboolean(L, nth)
#define arg__Bool(nth) lua_toboolean(L, nth)
#define arg_float(nth) (float)lua_tonumber(L, nth)
#define arg_double(nth) lua_tonumber(L, nth)
#define arg_string(nth) lua_tolstring(L, nth, 0)
#define return_void(x) return ((x), 0)
#define return_bool(x) return (lua_pushboolean(L, x), 1)
#define return__Bool(x) return (lua_pushboolean(L, x), 1)
#define return_int(x) return (lua_pushinteger(L, x), 1)
#define return_float(x) return (lua_pushnumber(L, x), 1)
#define return_double(x) return (lua_pushnumber(L, x), 1)
#define return_string(x) return (lua_pushstring(L, x), 1)

#define WRAP_ALL(...)                EXPAND(WRAP_ALL, __VA_ARGS__)
#define WRAP_ALL2(rc, func)          int func##_lua(lua*L) { return_##rc(func()); }
#define WRAP_ALL3(rc, func, a1)      int func##_lua(lua*L) { return_##rc(func(arg_##a1(1))); }
#define WRAP_ALL4(rc, func, a1,a2)   int func##_lua(lua*L) { return_##rc(func(arg_##a1(1),arg_##a2(2))); }

#define BIND_ALL(...)                EXPAND(BIND_ALL, __VA_ARGS__);
#define BIND_ALL2(rc,func)           script_bind_function(#func, func##_lua)
#define BIND_ALL3(rc,func,a1)        script_bind_function(#func, func##_lua)
#define BIND_ALL4(rc,func,a1,a2)     script_bind_function(#func, func##_lua)

#define XMACRO(X) /* @fixme: add all remaining FWK functions */ \
    X(void, window_create, float, int ) \
    X(bool, window_swap ) \
    X(void, ddraw_grid, float ) \
    X(void, ddraw_flush ) \
    X(bool, ui_begin, string, int ) \
    X(void, ui_end )

XMACRO(WRAP_ALL)

void script_quit(void) {
    lua_close(L);
    L = 0;
}
void script_init() {
    if( !L ) {
        fwk_init();

        // initialize Lua
        L = lua_newstate(script__realloc, 0); // L = luaL_newstate();

        // load various Lua libraries
        luaL_openlibs(L);
        luaopen_base(L);
        luaopen_table(L);
        luaopen_io(L);
        luaopen_string(L);
        luaopen_math(L);

        XMACRO(BIND_ALL);

        atexit(script_quit);
    }
}

#undef XMACRO
#line 0
#line 1 "fwk_system.c"
#if (is(tcc) && is(linux)) || (is(gcc) && !is(mingw)) // || is(clang)
int __argc; char **__argv;
#if !is(ems)
__attribute__((constructor)) void init_argcv(int argc, char **argv) { __argc = argc; __argv = argv; }
#endif
#endif

const char *app_path() { // should return absolute path always
    static char buffer[1024] = {0};
    if( buffer[0] ) return buffer;
#if is(win32)
    unsigned length = GetModuleFileNameA(NULL, buffer, sizeof(buffer)); // @todo: use GetModuleFileNameW+wchar_t && convert to utf8 instead
    char *a = strrchr(buffer, '/');  if(!a) a = buffer + strlen(buffer);
    char *b = strrchr(buffer, '\\'); if(!b) b = buffer + strlen(buffer);
    char slash = (a < b ? *a : b < a ? *b : '/');
    snprintf(buffer, 1024, "%.*s%c", length - (int)(a < b ? b - a : a - b), buffer, slash), buffer;
    if( strendi(buffer, "art\\tools\\tcc-win\\") ) { // fix tcc -g -run case. @fixme: fix on non art-tools custom pipeline folder
        strcat(buffer, "..\\..\\..\\");
    }
#else // #elif is(linux)
    char path[21] = {0};
    sprintf(path, "/proc/%d/exe", getpid());
    readlink(path, buffer, sizeof(buffer));
#endif
    return buffer;
}

const char *app_temp() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        snprintf(buffer, 256, "%s", ifdef(win32, getenv("TEMP"), P_tmpdir));
        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
        if(buffer[strlen(buffer)-1] != '/') strcat(buffer, "/");
    }
    return buffer;
}

#ifndef APP_NAME
#define APP_NAME __argv[0]
#endif

const char *app_name() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        char s[256];
        strncpy(s, APP_NAME, 256);
        char *a = strrchr(s, '/');
        char *b = strrchr(s, '\\');
        strncpy(buffer, a > b ? a+1 : b > a ? b+1 : s, 256);
        a = (char*)strendi(buffer, ".exe"); if(a) *a = 0;
    }
    return buffer;
}

const char *app_cache() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {

        #if is(osx)
        snprintf(buffer, 256, "~/Library/Caches/%s/", app_name()); // getenv("user.home")
        #elif is(win32) // APPDATA for roaming?
        snprintf(buffer, 256, "%s\\%s\\", getenv("LOCALAPPDATA"), app_name()); // getenv("LOCALAPPDATA")
        #else // linux
        snprintf(buffer, 256, "~/.cache/%s/", app_name()); // getenv("user.home")
        #endif

        mkdir(buffer, 0777);

        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
    }

    return buffer;
}

void app_reload() {
    // @todo: save_on_exit();
    fflush(0);
    chdir(app_path());
    execv(__argv[0], __argv);
    exit(0);
}

char* os_exec_output() {
    static local char os_exec__output[4096] = {0};
    return os_exec__output;
}
int (os_exec)( const char *cmd ) {
    int rc = -1;
    char *buf = os_exec_output(); buf[0] = 0; // memset(buf, 0, 4096);
    for( FILE *fp = popen( cmd, "r" ); fp; rc = pclose(fp), fp = 0) {
        while( fgets(buf, 4096 - 1, fp) ) {
            char *r = strrchr(buf, '\r'); if(r) *r = 0;
            char *n = strrchr(buf, '\n'); if(n) *n = 0;
        }
    }
    return rc;
}
char* (os_exec_)(int *rc, const char *cmd ) {
    int x = os_exec(cmd);
    if(rc) *rc = x;
    return os_exec_output();
}

#if is(osx)
#include <execinfo.h> // backtrace, backtrace_symbols
#include <dlfcn.h>    // dladdr, Dl_info
#elif is(gcc) && !is(ems) && !is(mingw)
#include <execinfo.h>  // backtrace, backtrace_symbols
#elif is(win32) // && !defined __TINYC__
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) {
    static HANDLE process = 0;
    if( !process ) process = GetCurrentProcess();
    if( !process ) exit( puts( "error: no current process" ) );
    static int init = 0;
    if( !init ) init = SymSetOptions(SYMOPT_UNDNAME), SymInitialize( process, NULL, TRUE );
    if( !init ) exit( puts( "error: cannot initialize DbgHelp.lib" ) );

    typedef USHORT (WINAPI *pFN)(); // TINYC
    //typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
    static pFN rtlCaptureStackBackTrace = 0;
    if( !rtlCaptureStackBackTrace ) {
        rtlCaptureStackBackTrace = (pFN)GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace");
    }
    if( !rtlCaptureStackBackTrace ) {
        return 0;
    }
    return rtlCaptureStackBackTrace(1, maxtraces, (PVOID *)addr, (DWORD *) 0);
}
static char **backtrace_symbols(void *const *array,int size) {
    HANDLE process = GetCurrentProcess();
    enum { MAXSYMBOLNAME = 512 - sizeof(IMAGEHLP_SYMBOL64) };
    char symbol64_buf     [ 512 ];
    char symbol64_bufblank[ 512 ] = {0};
    IMAGEHLP_SYMBOL64 *symbol64       = (IMAGEHLP_SYMBOL64*)symbol64_buf;
    IMAGEHLP_SYMBOL64 *symbol64_blank = (IMAGEHLP_SYMBOL64*)symbol64_bufblank;
    symbol64_blank->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    symbol64_blank->MaxNameLength = (MAXSYMBOLNAME-1) / 2; //wchar?

    int symlen = size * sizeof(char *);
    char **symbols = (char **)SYS_REALLOC(0, symlen);

    if( symbols ) {
        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = NULL;
        }

        char begin[1024];
        for( int i = 0; i < size; ++i ) {
            char **symbuf, *buffer = begin;

            DWORD64 dw1 = 0, dw2 = 0;
            *symbol64 = *symbol64_blank;
            if( SymGetSymFromAddr64( process, (DWORD64)array[i], &dw1, symbol64 ) ) {
                IMAGEHLP_LINE64 l64 = {0};
                l64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                if( SymGetLineFromAddr64( process, (DWORD64)array[i], (DWORD*)&dw2, &l64 ) ) {
                    // int lenbase( const char *str );
                    int base = 0; // lenbase( l64.FileName );
                    buffer += sprintf(buffer,"%s (%s:%d)%c", symbol64->Name, &l64.FileName[ base ], l64.LineNumber, 0);
                } else {
                    buffer += sprintf(buffer,"%s (?""?)%c", symbol64->Name, 0);
                }
            } else  buffer += sprintf(buffer,"(?""?)%c", 0);

            size_t buflen = buffer - begin + 1;

            symbuf = (char **)SYS_REALLOC( symbols, symlen + buflen );
            if( symbuf ) {
                memcpy( (char *)symbuf + symlen, begin, buflen );
                symbuf[ i ] = (char *)(size_t)symlen;

                symbols = symbuf;
                symlen += buflen;
            } else break;
        }

        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = (char *)symbols + (size_t)symbols[ i ];
        }
    }

    return symbols;
}
#else
static int backtrace(void **heap, int num) { return 0; }
static char **backtrace_symbols(void *const *sym,int num) { return 0; }
#endif

void trace_cb( int traces, int (*yield)(const char *)) {
    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 128 };

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return;
    if( traces > maxtraces ) traces = maxtraces;

    void *stack[ maxtraces ];
    traces = backtrace( stack, traces );
    char **symbols = backtrace_symbols( stack, traces );

    char demangled[1024] = "??", buf[1024];
    int L = 0, B = inc>0 ? skip - 1 : traces, E = inc>0 ? traces : skip - 1;
    for( int i = B; ( i += inc ) != E; ) {
#if is(linux)
        char *address = strstr( symbols[i], "[" ) + 1; address[strlen(address) - 1] = '\0';
        char *binary = symbols[i]; strstr( symbols[i], "(" )[0] = '\0';
        char command[1024]; sprintf(command, "addr2line -e %s %s", binary, address);
        for( FILE *fp = popen( command, "r" ); fp ; pclose(fp), fp = 0 ) {
            fgets(demangled, sizeof(demangled), fp);
            int len = strlen(demangled); while( len > 0 && demangled[len-1] < 32 ) demangled[--len] = 0;
        }
        symbols[i] = demangled;
#elif is(osx)
        /*struct*/ Dl_info info;
        if( dladdr(stack[i], &info) && info.dli_sname ) {
            char *dmgbuf = info.dli_sname[0] != '_' ? NULL :
                 ifdef(cpp, __cxa_demangle(info.dli_sname, NULL, 0, NULL), info.dli_sname);
            strcpy( demangled, dmgbuf ? dmgbuf : info.dli_sname );
            symbols[i] = demangled;
            FREE( dmgbuf );
        }
#endif
        sprintf(buf, "%03d: %#016p %s", ++L, stack[i], symbols[i]);
        //sprintf(buf, "%03d: %s", ++L, symbols[i]);
        if( yield(buf) < 0 ) break;
    }

     symbols = SYS_REALLOC(symbols, 0);
}

static local char *trace_strbuf[128] = {0};
static local int trace_counter = 0, trace_len = 0;
int trace_(const char *line) {
    int len = strlen(line);
    trace_len += len + 1;
    trace_strbuf[trace_counter] = (char*)SYS_REALLOC(trace_strbuf[trace_counter], (len * 1.5));
    strcpy(trace_strbuf[trace_counter], line );
    trace_counter = (trace_counter +1) % 128;
    return 1;
}
char *callstack( int traces ) {
#if is(linux)
    return ""; // @fixme: not really working as expected
#else
    //if( trace_ ) trace_str_ = SYS_REALLOC(trace_str_, trace_lenbuf_ = 0);
    trace_counter = trace_len = 0;
    trace_cb( traces, trace_ );
    static local char *buf = 0; // @fixme: 1 leak per invoking thread
    buf = SYS_REALLOC(buf, 0);
    buf = (char*)SYS_REALLOC( 0, trace_len + 1 ); buf[0] = 0;
    for( int i = 0; i < trace_counter; ++i ) {
        strcat(buf, trace_strbuf[i] ); // <-- optimize
        strcat(buf, "\n");
    }
    return buf ? buf : ""; // @fixme: should return NULL if no callstack is retrieved?
#endif
}
int callstackf( FILE *fp, int traces ) {
    char *buf = callstack(traces);
    fputs(buf, fp);
    return 0;
}

// -----------------------------------------------------------------------------
// endian

#if is(msc)
#include <stdlib.h>
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif is(gcc)
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#else
uint16_t swap16( uint16_t x ) { return (x << 8) | (x >> 8); }
uint32_t swap32( uint32_t x ) { x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); return (x << 16) | (x >> 16); }
uint64_t swap64( uint64_t x ) { x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL); x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL); return (x << 32) | (x >> 32); }
#endif

float    swap32f(float n)  { union { float  t; uint32_t i; } conv; conv.t = n; conv.i = swap32(conv.i); return conv.t; }
double   swap64f(double n) { union { double t; uint64_t i; } conv; conv.t = n; conv.i = swap64(conv.i); return conv.t; }

#define is_big()    ((*(uint16_t *)"\0\1") == 1)
#define is_little() ((*(uint16_t *)"\0\1") != 1)

uint16_t  lil16(uint16_t n) { return is_big()    ? swap16(n) : n; }
uint32_t  lil32(uint32_t n) { return is_big()    ? swap32(n) : n; }
uint64_t  lil64(uint64_t n) { return is_big()    ? swap64(n) : n; }
uint16_t  big16(uint16_t n) { return is_little() ? swap16(n) : n; }
uint32_t  big32(uint32_t n) { return is_little() ? swap32(n) : n; }
uint64_t  big64(uint64_t n) { return is_little() ? swap64(n) : n; }

float     lil32f(float n)  { return is_big()    ? swap32f(n) : n; }
double    lil64f(double n) { return is_big()    ? swap64f(n) : n; }
float     big32f(float n)  { return is_little() ? swap32f(n) : n; }
double    big64f(double n) { return is_little() ? swap64f(n) : n; }

uint16_t* lil16p(void *p, int sz)  { if(is_big()   ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint16_t* big16p(void *p, int sz)  { if(is_little()) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint32_t* lil32p(void *p, int sz)  { if(is_big()   ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint32_t* big32p(void *p, int sz)  { if(is_little()) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint64_t* lil64p(void *p, int sz)  { if(is_big()   ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }
uint64_t* big64p(void *p, int sz)  { if(is_little()) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }

float   * lil32pf(void *p, int sz) { if(is_big()   ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
float   * big32pf(void *p, int sz) { if(is_little()) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
double  * lil64pf(void *p, int sz) { if(is_big()   ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }
double  * big64pf(void *p, int sz) { if(is_little()) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }

// -----------------------------------------------------------------------------
// cpu

int cpu_cores() {
#if is(win32)
    DWORD_PTR pm, sm;
    if( GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) ) if( pm ) {
        int count = 0;
        while( pm ) {
            ++count;
            pm &= pm - 1;
        }
        return count;
    }
    { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
#else // unix
    int count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? count : 1;
#endif
#if 0
#elif is(linux)
    cpu_set_t prevmask, testmask;
    CPU_ZERO(&prevmask);
    CPU_ZERO(&testmask);
    sched_getaffinity(0, sizeof(prevmask), &prevmask);     //Get current mask
    sched_setaffinity(0, sizeof(testmask), &testmask);     //Set zero mask
    sched_getaffinity(0, sizeof(testmask), &testmask);     //Get mask for all CPUs
    sched_setaffinity(0, sizeof(prevmask), &prevmask);     //Reset current mask
    int num = CPU_COUNT(&testmask);
    return (num > 1 ? num : 1);
#elif is(cpp)
    return (int)std::thread::hardware_concurrency();
#elif defined(_OPENMP)
    // omp
    int cores = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        ++cores;
    }
    return cores;
#endif
}

// -----------------------------------------------------------------------------
// Battery API. Based on code by Rabia Alhaffar (UNLICENSE)
// - rlyeh, public domain.

#ifdef _WIN32
#include <winsock2.h>

int battery() {
    SYSTEM_POWER_STATUS ibstatus;

    if (GetSystemPowerStatus(&ibstatus) == FALSE) {
        return 0;
    }

    int level = (ibstatus.BatteryLifePercent != 255) ? ibstatus.BatteryLifePercent : 0;
    int charging = (ibstatus.BatteryFlag & 8 > 0);
    return charging ? +level : -level;
}

#elif defined __linux__ // is(linux)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int battery() {
    static int battery_status_handle;
    static int battery_capacity_handle;

    do_once {
        battery_status_handle = open("/sys/class/power_supply/BAT0/status", O_RDONLY);
        battery_capacity_handle = open("/sys/class/power_supply/BAT0/capacity", O_RDONLY);
    }

    if (battery_status_handle == -1 || battery_capacity_handle == -1) {
        return 0;
    }

    char buffer[513];

    // level
    lseek(battery_capacity_handle, 0, SEEK_SET);
    int readlen = read(battery_capacity_handle, buffer, 512); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int level = atoi(buffer);

    // charging
    lseek(battery_status_handle, 0, SEEK_SET);
    readlen = read(battery_status_handle, buffer, 512); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int charging = strstr(buffer, "Discharging") ? 0 : 1;
    return charging ? +level : -level;
}

#else // if is(osx)
#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

int battery() {
    static CFDictionaryRef psrc;

    do_once {
        CFTypeRef blob = IOPSCopyPowerSourcesInfo();
        CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
        int sourcesCount = CFArrayGetCount(sources);

        if (sourcesCount > 0) {
            psrc = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, 0));
        }
    }

    if(psrc == NULL) return 0;

    int cur_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSCurrentCapacityKey)), kCFNumberSInt32Type, &cur_cap);

    int max_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSMaxCapacityKey)), kCFNumberSInt32Type, &max_cap);

    int level = (int)(cur_cap * 100.f / max_cap);
    int charging = CFDictionaryGetValue(psrc, CFSTR(kIOPSIsChargingKey)) == kCFBooleanTrue;
    return charging ? +level : -level;
}

#endif

// ----------------------------------------------------------------------------
// time

#if 0
uint64_t time_gpu() {
    GLint64 t = 123456789;
    glGetInteger64v(GL_TIMESTAMP, &t);
    return (uint64_t)t;
}
#endif
uint64_t date() {
    time_t epoch = time(0);
    struct tm *ti = localtime(&epoch);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
uint64_t date_epoch() {
    time_t epoch = time(0);
    return epoch;
}
#if 0
double time_ss() {
    return glfwGetTime();
}
double time_ms() {
    return glfwGetTime() * 1000.0;
}
uint64_t time_us() {
    return (uint64_t)(glfwGetTime() * 1000000.0); // @fixme: use a high resolution timer instead, or time_gpu below
}
uint64_t sleep_us(uint64_t us) { // @fixme: use a high resolution sleeper instead
    return sleep_ms( us / 1000.0 );
}
double sleep_ms(double ms) {
    double now = time_ms();
    if( ms <= 0 ) {
#if is(win32)
        Sleep(0); // yield
#else
        usleep(0);
#endif
    } else {
#if is(win32)
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }
    return time_ms() - now;
}
double sleep_ss(double ss) {
    return sleep_ms( ss * 1000 ) / 1000.0;
}
#endif

// high-perf functions

#define TIMER_E3 1000ULL
#define TIMER_E6 1000000ULL
#define TIMER_E9 1000000000ULL

#ifdef CLOCK_MONOTONIC_RAW
#define TIME_MONOTONIC CLOCK_MONOTONIC_RAW
#elif defined CLOCK_MONOTONIC
#define TIME_MONOTONIC CLOCK_MONOTONIC
#else
// #define TIME_MONOTONIC CLOCK_REALTIME // untested
#endif

static uint64_t nanotimer(uint64_t *out_freq) {
    if( out_freq ) {
#ifdef _WIN32
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        *out_freq = li.QuadPart;
//#elif is(ANDROID)
//      *out_freq = CLOCKS_PER_SEC;
#elif defined TIME_MONOTONIC
        *out_freq = TIMER_E9;
#else
        *out_freq = TIMER_E6;
#endif
    }
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (uint64_t)li.QuadPart;
//#elif is(ANDROID)
//    return (uint64_t)clock();
#elif defined TIME_MONOTONIC
    struct timespec ts;
    clock_gettime(TIME_MONOTONIC, &ts);
    return (TIMER_E9 * (uint64_t)ts.tv_sec) + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (TIMER_E6 * (uint64_t)tv.tv_sec) + tv.tv_usec;
#endif
}

uint64_t time_ns() {
    static uint64_t epoch = 0;
    static uint64_t freq = 0;
    if( !freq ) {
        epoch = nanotimer(&freq);
    }

    uint64_t a = nanotimer(NULL) - epoch;
    uint64_t b = TIMER_E9;
    uint64_t c = freq;

    // Computes (a*b)/c without overflow, as long as both (a*b) and the overall result fit into 64-bits.
    // [ref] https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
    uint64_t q = a / c;
    uint64_t r = a % c;
    return q * b + r * b / c;
}
uint64_t time_us() {
    return time_ns() / TIMER_E3;
}
uint64_t time_ms() {
    return time_ns() / TIMER_E6;
}
double time_ss() {
    return time_ns() / 1e9; // TIMER_E9;
}
double time_mm() {
    return time_ss() / 60;
}
double time_hh() {
    return time_mm() / 60;
}

void sleep_ns( double ns ) {
#ifdef _WIN32
    if( ns >= 100 ) {
        LARGE_INTEGER li;      // Windows sleep in 100ns units
        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        li.QuadPart = (LONGLONG)(__int64)(-ns/100); // Negative for relative time
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#else
    if( ns > 0 ) {
        struct timespec wait = {0};
        wait.tv_sec = ns / 1e9;
        wait.tv_nsec = ns - wait.tv_sec * 1e9;
        nanosleep(&wait, NULL);
#endif
    } else {
#ifdef _WIN32
        Sleep(0); // yield, Sleep(0), SwitchToThread
#else
        usleep(0);
#endif
    }
}
void sleep_us( double us ) {
    sleep_ns(us * 1e3);
}
void sleep_ms( double ms ) {
    sleep_ns(ms * 1e6);
}
void sleep_ss( double ss ) {
    sleep_ns(ss * 1e9);
}


// ----------------------------------------------------------------------------
// argc/v

int argc() { return __argc; }
char* argv(int arg) { return __argv[arg]; }

// ----------------------------------------------------------------------------
// options

int flag(const char *commalist) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( !strcmpi( arg, token ) ) {  // --arg
                return 1;
            }
        }

        commalist = end + !!end[0];
    }
    return 0;
}

const char *option(const char *commalist, const char *defaults) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128], tokeneq[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);
        snprintf(tokeneq, 128, "%.*s=", (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( strbegi( arg, tokeneq ) ) { // --arg=value
                return argv(i) + strlen(tokeneq);
            }
            if( !strcmpi( arg, token ) ) {  // --arg value
                if( (i+1) < argc() ) {
                    return argv(i+1);
                }
            }
        }

        commalist = end + !!end[0];
    }
    return defaults;
}

int optioni(const char *commalist, int defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atoi(rc) : defaults;
}
float optionf(const char *commalist, float defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atof(rc) : defaults;
}

// ----------------------------------------------------------------------------
// tty

void tty_color(unsigned color) {
    #if is(win32)
    do_once {
        DWORD mode = 0; SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4));
    }
    #endif
    if( color ) {
        // if( color == RED ) breakpoint("break on RED"); // debug
        unsigned r = (color >> 16) & 255;
        unsigned g = (color >>  8) & 255;
        unsigned b = (color >>  0) & 255;
        // 24-bit console ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
        // 256-color console ESC[38;5;<fgcode>m
        // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
        // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
        // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
        // 0xE8-0xFF:  grayscale from black to white in 24 steps
        r /= 51, g /= 51, b /= 51; // [0..5]
        printf("\033[38;5;%dm", r*36+g*6+b+16); // "\033[0;3%sm", color_code);
    } else {
        printf("%s", "\x1B[39;49m"); // reset
    }
}
void tty_puts(unsigned color, const char *text) {
    tty_color(color); puts(text);
}
void tty_init() {
    tty_color(0);
}
int tty_cols() {
#if is(win32)
    CONSOLE_SCREEN_BUFFER_INFO c;
    if( GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c) ) {
        int w = c.srWindow.Right-c.srWindow.Left-c.dwCursorPosition.X;
        return w > 2 ? w - 1 : w; // w-1 to allow window resizing to a larger dimension (already printed text would break otherwise)
    }
#endif
#ifdef TIOCGWINSZ
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col - 1;
#endif
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols - 1;
#endif
    return 80;
}

// -----------------------------------------------------------------------------
// debugger

#include <stdio.h>
void hexdumpf( FILE *fp, const void *ptr, unsigned len, int width ) {
    unsigned char *data = (unsigned char*)ptr;
    for( unsigned jt = 0; jt < len; jt += width ) {
        fprintf( fp, "; %05d ", jt );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, "%02x %s", (unsigned char)data[it], &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
        fprintf( fp, "; %05d ", jt );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, " %c %s", (signed char)data[it] >= 32 ? (signed char)data[it] : (signed char)'.', &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
    }
}
void hexdump( const void *ptr, unsigned len ) {
    hexdumpf( stdout, ptr, len, 16 );
}

#if 0 // is(msc) only
static void debugbreak(void) {
    do { \
        __try { DebugBreak(); } \
        __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ? \
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {} \
    } while(0);
}
#endif

#if is(win32)
static void debugbreak(void) { if(IsDebuggerPresent()) DebugBreak(); }
#else // is(unix)
static int is_debugger_present = -1;
static void _sigtrap_handler(int signum) {
    is_debugger_present = 0;
    signal(SIGTRAP, SIG_DFL);
}
static void debugbreak(void) { // break if debugger present
    // __builtin_trap(); //
    //raise(SIGABRT); // SIGTRAP);
    //__asm__ volatile("int $0x03");
    if( is_debugger_present < 0 ) {
        is_debugger_present = 1;
        signal(SIGTRAP, _sigtrap_handler);
        raise(SIGTRAP);
    }
}
#endif

void alert(const char *message) { // @todo: move to app_, besides die()
#if is(win32)
    MessageBoxA(0, message, 0,0);
#elif is(linux)
    for(FILE *fp = fopen("/tmp/fwk.warning","wb");fp;fp=0)
    fputs(message,fp), fclose(fp), system("xmessage -center -file /tmp/fwk.warning");
#elif is(osx)
    system(va("osascript -e 'display alert \"Alert\" message \"%s\"'", message));
#endif
}

void breakpoint(const char *reason) {
    window_visible(false);
    const char *fulltext = reason[0] == '!' ? va("%s\n%s", reason+1, callstack(+48)) : reason;
    PRINTF("%s", fulltext);

    (alert)(fulltext);
    debugbreak();

    window_visible(true);
}

bool has_debugger() {
#if is(win32)
    return IsDebuggerPresent(); // SetLastError(123); OutputDebugStringA("\1"); enabled = GetLastError() != 123;
#else
    return false;
#endif
}

void die(const char *message) {
   fprintf(stderr, "%s\n", message);
   fflush(stderr);
   alert(message);
   exit(-1);
}

// ----------------------------------------------------------------------------
// logger

unsigned determine_color_from_text(const char *text) {
    /**/ if( strstri(text, "fail") || strstri(text, "error") ) return RED;
    else if( strstri(text, "warn") || strstri(text, "not found") ) return YELLOW;
    return 0;
}

int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function) {
    double secs = time_ss();
    uint32_t color = /*errno ? RED :*/ determine_color_from_text(text); // errno = 0;
    #if is(msc)
    char *slash = strrchr(file, '\\'); if(slash) file = slash + 1;
    #endif
    char *location = va("|%s|%s:%d", /*errno?strerror(errno):*/function, file, line);
    int cols = tty_cols() + 1 - (int)strlen(location);

static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
thread_mutex_lock( &lock );

    tty_color(color);
    printf("\r%*.s%s", cols, "", location);
    printf("\r%07.3fs|%s%s", secs, text, stack);
    tty_color(0);

thread_mutex_unlock( &lock );
    return 1;
}

// ----------------------------------------------------------------------------
// panic

static void *panic_oom_reserve; // for out-of-memory recovery
int (PANIC)(const char *error, const char *file, int line) {
    panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 0);

    tty_color(RED);

    error += error[0] == '!';
    fprintf(stderr, "Error: %s (%s:%d) (errno:%s)\n", error, file, line, strerror(errno));
    fprintf(stderr, "%s", callstack(+16)); // no \n
    fflush(0); // fflush(stderr);

    tty_color(0);

    breakpoint(error);
    exit(-line);
    return 1;
}
#line 0
#line 1 "fwk_ui.c"
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static struct nk_context *ui_ctx;
static struct nk_glfw ui_glfw = {0};

void ui_destroy(void) {
    if(ui_ctx) {
        nk_glfw3_shutdown(&ui_glfw); // nk_sdl_shutdown();
        ui_ctx = 0;
    }
}

void ui_create() {
    if(!ui_ctx) {
        ui_ctx = nk_glfw3_init(&ui_glfw, window_handle(), NK_GLFW3_INSTALL_CALLBACKS); // nk_sdl_init(window_handle());
        atexit(ui_destroy);

        /* Load Fonts: if none of these are loaded a default font will be used  */
        /* Load Cursor: if you uncomment cursor loading please hide the cursor */
        //{struct nk_font_atlas *atlas;

#define ICON_FONTNAME "fontawesome-webfont.ttf"
#define ICON_FONTSIZE 15
#define ICON_MIN 0xF000
#define ICON_MAX 0xF2E0
#define ICON_FILE_O "\xef\x80\x96" // U+f016
#define ICON_BARS "\xef\x83\x89" // U+f0c9
#define ICON_TEST_GLYPH ICON_FILE_O

        {struct nk_font_atlas *atlas;
        struct nk_font_config cfg = nk_font_config(ICON_FONTSIZE+1);
        static const nk_rune icon_range[] = {ICON_MIN, ICON_MAX, 0};
        cfg.range = icon_range;
        cfg.merge_mode = 1;

cfg.spacing.x += 8.f;
//    cfg.spacing.y += 0.f;
//    cfg.font->ascent ++;
//    cfg.font->height += 4;

        struct nk_font *last = NULL;
        nk_glfw3_font_stash_begin(&ui_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);
#if 0 // is(win32)
        struct nk_font *arial = nk_font_atlas_add_from_file(atlas, va("%s/fonts/arial.ttf",getenv("windir")), 15.f, 0); last = arial ? arial : last;
#else
        struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, vfs_find("Carlito-Regular.ttf"), 14.5f, 0); last = roboto ? roboto : last;
#endif
        //struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", 14, 0); last = droid ? droid : last;
        struct nk_font *icons = nk_font_atlas_add_from_file(atlas, vfs_find(ICON_FONTNAME), ICON_FONTSIZE, &cfg);
        nk_glfw3_font_stash_end(&ui_glfw); // nk_sdl_font_stash_end();
        /* nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); */
        if(last) nk_style_set_font(ui_ctx, &last->handle);}

        nk_style_default(ui_ctx);
        struct nk_color blue_gray = nk_rgba(53, 88, 156, 255);
        struct nk_color washed_cyan = nk_rgb_f(0.35,0.6,0.8);
        struct nk_color vivid_cyan = nk_rgb_f(0.0,0.75,1.0);
        struct nk_color main_color = blue_gray;
        struct nk_color hover_color = washed_cyan;
        struct nk_color lit_color = vivid_cyan;
        struct nk_color table[NK_COLOR_COUNT] = {0};
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(42, 42, 42, 215);
        table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = main_color;
        table[NK_COLOR_BUTTON_HOVER] = hover_color;
        table[NK_COLOR_BUTTON_ACTIVE] = lit_color;
        // ok
        table[NK_COLOR_TOGGLE] = nk_rgba(45*1.2, 53*1.2, 56*1.2, 255); // table[NK_COLOR_WINDOW]; // nk_rgba(45/1.2, 53/1.2, 56/1.2, 255);
        table[NK_COLOR_TOGGLE_HOVER] = vivid_cyan;
        table[NK_COLOR_TOGGLE_CURSOR] = main_color; // vivid_blue;
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = main_color;
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = hover_color;
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = lit_color;
        table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SLIDER_CURSOR] = main_color;
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = hover_color;
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = lit_color;
        table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);

        // table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);

        // table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
        // table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        // table[NK_COLOR_CHART_COLOR] = main_color;
        // table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        // table[NK_COLOR_TAB_HEADER] = main_color;
        // table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
        // table[NK_COLOR_SELECT_ACTIVE] = main_color;

        nk_style_from_table(ui_ctx, table);

        struct nk_style *s = &ui_ctx->style;
        s->window.spacing = nk_vec2(4,0);
        s->window.combo_border = 0.f;
        s->window.scrollbar_size = nk_vec2(5,5);
        s->property.rounding = 0;
        s->combo.border = 0;
        s->button.border = 1;
        s->edit.border = 0;

        // enum { MAX_MEMORY = 2 * 1024 * 1024 };
        // nk_init_fixed(&ui_ctx, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);
    }
}

// -----------------------------------------------------------------------------
// ui menu

static array(char*) ui_items; // queued menu names. to be evaluated during next frame
static vec2 ui_results = {0}; // clicked menu items from last frame

int ui_item() {
    return ui_items ? (ui_results.x == array_count(ui_items) ? ui_results.y : 0) : 0;
}

int ui_menu(const char *items) { // semicolon- or comma-separated items
    array_push(ui_items, STRDUP(items));
    return ui_item();
}

static
int ui_menu_active() {
    return !!ui_items; // array_count(ui_items) > 0;
}

static
void ui_separator_line() {
    struct nk_rect space; nk_layout_peek(&space, ui_ctx); // bounds.w *= 0.95f;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y+0,space.x+space.w,space.y+0, 3.0, nk_rgb(128,128,128));
}

static
void ui_menu_render() {
    // clean up from past frame
    ui_results = vec2(0,0);
    if( !ui_items ) return;

    // process menus
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), 32), NK_WINDOW_NO_SCROLLBAR/*|NK_WINDOW_BACKGROUND*/)) {
        nk_menubar_begin(ui_ctx);
        nk_layout_row_begin(ui_ctx, NK_STATIC, 25, array_count(ui_items));

        for( int i = 0, end = array_count(ui_items); i < end; ++i ) {
            char id[64]; sscanf(ui_items[i], "%[^,;|]", id);

            nk_layout_row_push(ui_ctx, 45);
            if (nk_menu_begin_label(ui_ctx, id, NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(ui_ctx, 32, 1);

                int j = 0;
                for each_substring(ui_items[i] + strlen(id) + 1, ",;|", item) {
                    if( *item == '-' ) {
                        while(*item == '-') ++item;
                        //nk_menu_item_label(ui_ctx, "---", NK_TEXT_LEFT);
                        ui_separator_line();
                    }

                    if( nk_menu_item_label(ui_ctx, item, NK_TEXT_LEFT) ) {
                        ui_results = vec2(i+1, j+1);
                    }
                    ++j;
                }
                nk_menu_end(ui_ctx);
            }
        }

        nk_layout_row_end(ui_ctx);
        nk_menubar_end(ui_ctx);
    }
    nk_end(ui_ctx);

    // clean up for next frame
    for( int i = 0; i < array_count(ui_items); ++i ) {
        FREE(ui_items[i]);
    }
    array_resize(ui_items, 0);
}

// -----------------------------------------------------------------------------

static int ui_dirty = 1;
static int ui_popups_active = 0;
static float ui_hue = 0; // hue
static int ui_hovering = 0;
static int ui_actived = 0;

int ui_hover() {
    return ui_hovering;
}
int ui_active() {
    return ui_actived; //window_has_cursor() && nk_window_is_any_hovered(ui_ctx) && nk_item_is_any_active(ui_ctx);
}

void ui_render() {
    // draw queued menus
    ui_menu_render();

    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    nk_glfw3_render(&ui_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    ui_dirty = 1;
    ui_hue = 0;

    ui_hovering = nk_window_is_any_hovered(ui_ctx) && window_has_cursor();

    if(input_down(MOUSE_L))
        ui_actived = (ui_hovering && nk_item_is_any_active(ui_ctx));
    if(input_up(MOUSE_L))
        ui_actived = 0;
}


// ----------------------------------------------------------------------------

int ui_begin(const char *title, int flags) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;

    ui_create();
    if(ui_dirty) {
        nk_glfw3_new_frame(&ui_glfw);
        ui_dirty = 0;
    }

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; title[i]; ++i) hash = (hash ^ title[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

#if 0
    static int rows = 0;
    static hashmap(const char*,int) map, *init = 0;
    if( !init ) { hashmap_create(init = &map, 128 /*maxwindows*/, str64); }
    int *found = hashmap_find(&map, title);
    if( !found ) hashmap_insert(&map, title, ++rows);
    int row = found ? *found : rows;
#else // fixme
    static const char *list[128] = {0};
    static int         posx[128] = {0};
    int row = -1, find = 0; while( find < 128 && list[find] ) {
        if( !strcmp(title, list[find]) ) { row = find; break; } else ++find;
    }
    if( row < 0 ) list[row = find] = STRDUP(title);
    if( ui_menu_active() ) ++row; // add 1 to skip menu
#endif

    int window_flags = nk_window_is_active(ui_ctx, title) ? 0 : NK_WINDOW_MINIMIZED; //NK_MINIMIZED;
    window_flags |= NK_WINDOW_BORDER;
    window_flags |= NK_WINDOW_SCALABLE;
    window_flags |= NK_WINDOW_MOVABLE;
    //window_flags |= NK_WINDOW_NO_SCROLLBAR;
    //window_flags |= NK_WINDOW_SCALE_LEFT;
    window_flags |= NK_WINDOW_MINIMIZABLE;

//  struct nk_style *s = &ui_ctx->style;
//  nk_style_push_color(ui_ctx, &s->window.header.normal.data.color, nk_hsv_f(ui_hue,0.6,0.8));
if( posx[row] > 0 ) window_flags &= ~NK_WINDOW_MINIMIZED;

    int ui_open;
    vec2 offset = vec2(0, 32*row);
    float w = window_width() / 3.33, h = window_height() - offset.y * 2;
    if( nk_begin(ui_ctx, title, nk_rect(offset.x, offset.y, offset.x+w, offset.y+h), window_flags) ) {

posx[row] = nk_window_get_position(ui_ctx).x;

        ui_open = 1;
    } else {
        ui_end();
        ui_open = 0;
    }

    return ui_open;
}

void ui_end() {
    nk_end(ui_ctx);
//  nk_style_pop_color(ui_ctx);
}


int ui_button(const char *s) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    const char *split = strchr(s, '@'), *tooltip = split + 1;
    int ret = split ? nk_button_text(ui_ctx, s, (int)(split - s)) : nk_button_label(ui_ctx, s);

    const struct nk_input *in = &ui_ctx->input;
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_popups_active) {
        nk_tooltip(ui_ctx, tooltip);
    }
    return ret;
}

int ui_label_(const char *label, int alignment) {
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    const struct nk_input *in = &ui_ctx->input;
    int is_hovering = nk_input_is_mouse_hovering_rect(in, bounds);

    int indent = 8;
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout;
    layout->at_x += indent;
    layout->bounds.w -= indent;
        bounds.w = is_hovering ? indent*3/4 : indent/2-1;
        bounds.h -= 1;
        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        struct nk_input *input = &ui_ctx->input;
        nk_fill_rect(canvas, bounds, 0, nk_hsv_f(ui_hue, 0.6f, 0.8f) );

        const char *split = strchr(label, '@'), *tooltip = split + 1;
            char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer);
            nk_label(ui_ctx, label, alignment);
            if (split && is_hovering && !ui_popups_active) {
                nk_tooltip(ui_ctx, split ? tooltip : label);
            }

    layout->at_x -= indent;
    layout->bounds.w += indent;

    return is_hovering ? nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, layout->bounds, nk_true) : 0;
}

int ui_label(const char *label) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(label, NK_TEXT_LEFT);
}
int ui_label2(const char *label, const char *caption) {
    char buffer[2048]; snprintf(buffer, 2048, "%s@%s", label, caption);
    return ui_label(buffer);
}
int ui_const_bool(const char *text, const double value) {
    bool b = !!value;
    return ui_bool(text, &b), 0;
}
int ui_const_float(const char *text, const double value) {
    float f = (float)value;
    return ui_float(text, &f), 0;
}
int ui_const_string(const char *label, const char *text) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);
    return nk_label(ui_ctx, text, NK_TEXT_LEFT), 0;
}



int ui_toggle(const char *label, bool *value) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_style_button button = ui_ctx->style.button; if(!*value) {
    ui_ctx->style.button.normal =
    ui_ctx->style.button.hover =
    ui_ctx->style.button.active = nk_style_item_color(nk_rgba(50, 58, 61, 225)); // same as table[NK_COLOR_EDIT]
    ui_ctx->style.button.border_color =
    ui_ctx->style.button.text_background =
    ui_ctx->style.button.text_normal =
    ui_ctx->style.button.text_hover =
    ui_ctx->style.button.text_active = nk_rgb(120,120,120);
    }

    int rc = nk_button_label(ui_ctx, *value ? "on":"off");
    ui_ctx->style.button = button;
    return rc ? (*value ^= 1), rc : rc;
}

int ui_color4f(const char *label, float *color4) {
    float c[4] = { color4[0]*255, color4[1]*255, color4[2]*255, color4[3]*255 };
    int ret = ui_color4(label, c);
    for( int i = 0; i < 4; ++i ) color4[i] = c[i] / 255.0f;
    return ret;
}

int ui_color4(const char *label, float *color4) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color4[0]/255, color4[1]/255, color4[2]/255, color4[3]/255 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        enum color_mode {COL_RGB, COL_HSV};
        static int col_mode = COL_RGB;
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGBA);

        nk_layout_row_dynamic(ui_ctx, 25, 2);
        col_mode = nk_option_label(ui_ctx, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
        col_mode = nk_option_label(ui_ctx, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;

        nk_layout_row_dynamic(ui_ctx, 25, 1);
        if (col_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", 0, after.r, 1.0f, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", 0, after.g, 1.0f, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", 0, after.b, 1.0f, 0.01f,0.005f);
            after.a = nk_propertyf(ui_ctx, "#A:", 0, after.a, 1.0f, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", 0, hsva[0], 1.0f, 0.01f,0.05f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", 0, hsva[1], 1.0f, 0.01f,0.05f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", 0, hsva[2], 1.0f, 0.01f,0.05f);
            hsva[3] = nk_propertyf(ui_ctx, "#A:", 0, hsva[3], 1.0f, 0.01f,0.05f);
            after = nk_hsva_colorfv(hsva);
        }

        color4[0] = after.r * 255;
        color4[1] = after.g * 255;
        color4[2] = after.b * 255;
        color4[3] = after.a * 255;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_color3f(const char *label, float *color3) {
    float c[3] = { color3[0]*255, color3[1]*255, color3[2]*255 };
    int ret = ui_color3(label, c);
    for( int i = 0; i < 3; ++i ) color3[i] = c[i] / 255.0f;
    return ret;
}

int ui_color3(const char *label, float *color3) {
    float color4[] = { color3[0], color3[1], color3[2], 255 };
    int ret = ui_color4(label, color4);
    color3[0] = color4[0];
    color3[1] = color4[1];
    color3[2] = color4[2];
    return ret;
}

int ui_list(const char *label, const char **items, int num_items, int *selector) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = nk_combo(ui_ctx, items, num_items, *selector, 25, nk_vec2(200,200));
    int chg = val != *selector;
    *selector = val;
    return chg;
}

int ui_slider(const char *label, float *slider) {
    // return ui_slider2(label, slider, va("%.2f ", *slider));
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    nk_size val = *slider * 1000;
    int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
    *slider = val / 1000.f;
    return chg;
}
int ui_slider2(const char *label, float *slider, const char *caption) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_window *win = ui_ctx->current;
    const struct nk_style *style = &ui_ctx->style;
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;
    struct nk_vec2 item_padding = style->text.padding;
    struct nk_text text;
    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = nk_rgb(255,255,255);

        nk_size val = *slider * 1000;
        int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
        *slider = val / 1000.f;

    nk_widget_text(&win->buffer, bounds, caption, strlen(caption), &text, NK_TEXT_RIGHT, style->font);
    return chg;
}

int ui_bool(const char *label, bool *enabled ) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = *enabled;
    int chg = nk_checkbox_label(ui_ctx, "", &val);
    *enabled = !!val;
    return chg;
}

int ui_int(const char *label, int *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int prev = *v;
    *v = nk_propertyi(ui_ctx, "#", INT_MIN, *v, INT_MAX, 1,1);
    return prev != *v;
}

int ui_short(const char *label, short *v) {
    int i = *v, ret = ui_int( label, &i );
    return *v = (short)i, ret;
}

int ui_float(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", -FLT_MAX, v[0], FLT_MAX, 0.01f,0.005f);
    return prev != v[0];
}

int ui_clampf(const char *label, float *v, float minf, float maxf) {
    if( minf > maxf ) return ui_clampf(label, v, maxf, minf);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", minf, v[0], maxf, 0.1f,0.05f);
    return prev != v[0];
}

int ui_float2(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f", v[0], v[1]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1];
    }
    return 0;
}

int ui_float3(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f", v[0], v[1], v[2]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2];
    }
    return 0;
}

int ui_float4(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f, %.2f", v[0], v[1], v[2], v[3]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        float prev3 = v[3]; nk_property_float(ui_ctx, "#W:", -FLT_MAX, &v[3], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2] || prev3 != v[3];
    }
    return 0;
}

int ui_string(const char *label, char *buffer, int buflen) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, buffer, buflen, nk_filter_default);
    return !!(active & NK_EDIT_COMMITED);
}

int ui_separator() {
    nk_layout_row_dynamic(ui_ctx, 10, 1);

    // cycle color (phi ratio)
    //ui_hue = (ui_hue+0.61803f)*1.61803f; while(ui_hue > 1) ui_hue -= 1;
    ui_hue *= 1.61803f / 1.85f; while(ui_hue > 1) ui_hue -= 1;

    struct nk_command_buffer *canvas;
    struct nk_input *input = &ui_ctx->input;
    canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect space;
    enum nk_widget_layout_states state;
    state = nk_widget(&space, ui_ctx);
    if (state) nk_fill_rect(canvas, space, 0, ui_ctx->style.window.header.normal.data.color );

    return 1;
}

int ui_image(const char *label, handle id, unsigned w, unsigned h) {
    //nk_layout_row_dynamic(ui_ctx, 1, 1);
    //struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    nk_layout_row_dynamic(ui_ctx, h < 30 || id == texture_checker().id ? 0 : h, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int ret = nk_button_image(ui_ctx, nk_image_id((int)id));
    return !!ret;
}

int ui_colormap( const char *map_name, colormap_t *cm ) {
    int ret = 0;
    if( !cm->texture ) {
        const char *title = va("%s (no image)", map_name);
        if( ui_image( title, texture_checker().id, 0,0 ) ) {
            ret = 2;
        }
    } else {
        unsigned w = cm->texture->w, h = cm->texture->h;
        ui_label(va("%s (%s)", map_name, cm->texture->filename) ); // @fixme: labelf would crash?

        const char *fmt[] = { "", "R", "RG", "RGB", "RGBA" };
        const char *title = va("%s %dx%d %s", map_name, w, h, fmt[cm->texture->n]);
        if( ui_image( title, cm->texture->id, 0, 128 ) ) {
            ret = 2;
        }
    }

    if( ui_color4f( va("%s Color", map_name), (float *) &cm->color ) ) {
        ret = 1;
    }
    return ret;
}

int ui_radio(const char *label, const char **items, int num_items, int *selector) {
    int ret = 0;
    if( label && label[0] ) ui_label(label);
    for( int i = 0; i < num_items; i++ ) {
        bool enabled = *selector == i;
        if( ui_bool( va("%s%s", label && label[0] ? "  ":"", items[i]), &enabled ) ) {
            *selector = i;
            ret |= 1;
        }
    }
    return ret;
}

int ui_dialog(const char *title, const char *text, int choices, bool *show) { // @fixme: return
    ui_popups_active = 0;
    if(*show) {
        static struct nk_rect s = {0, 0, 300, 190};
        if (nk_popup_begin(ui_ctx, NK_POPUP_STATIC, title, NK_WINDOW_BORDER|NK_WINDOW_CLOSABLE, s)) {
            nk_layout_row_dynamic(ui_ctx, 20, 1);
            for( char label[1024]; *text && sscanf(text, "%[^\r\n]", label); ) {
                nk_label(ui_ctx, label, NK_TEXT_LEFT);
                text += strlen(label); while(*text && *text < 32) text++;
            }

            if( choices ) {
                nk_layout_row_dynamic(ui_ctx, 25, choices > 1 ? 2 : 1);
                if (nk_button_label(ui_ctx, "OK")) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
                if (choices > 1 && nk_button_label(ui_ctx, "Cancel")) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
            }

            nk_popup_end(ui_ctx);
        } else {
            *show = nk_false;
        }
        ui_popups_active = *show;
    }
    return *show;
}

#define ui_bits_template(X) \
int ui_bits##X(const char *label, uint##X##_t *enabled) { \
    /* @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive */ \
    nk_layout_row_dynamic(ui_ctx, 1, 1); \
    struct nk_rect bounds = nk_widget_bounds(ui_ctx); \
\
    /* actual widget: label + X checkboxes */ \
    nk_layout_row_begin(ui_ctx, NK_STATIC, 0, 1+X); \
\
        int offset = bounds.w > (15*X) ? bounds.w - (15*X) : 0; /* bits widget below needs at least 118px wide */ \
        nk_layout_row_push(ui_ctx, offset); \
        ui_label_(label, NK_TEXT_LEFT); \
\
        uint8_t copy = *enabled; \
        for( int i = 0; i < X; ++i ) { \
            nk_layout_row_push(ui_ctx, 10); \
            /* bit */ \
            int val = (*enabled >> i) & 1; \
            int chg = nk_checkbox_label(ui_ctx, "", &val); \
            *enabled = (*enabled & ~(1 << i)) | ((!!val) << i); \
            /* tooltip */ \
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, 30 }; /* 10:padding,14:width,30:height */ \
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_popups_active) { \
                nk_tooltipf(ui_ctx, "Bit %d", i); \
            } \
        } \
\
    nk_layout_row_end(ui_ctx); \
    return copy ^ *enabled; \
}

ui_bits_template(8);
ui_bits_template(16);
//ui_bits_template(32);

// ----------------------------------------------------------------------------

void ui_demo() {
    static int integer = 42;
    static bool toggle = true;
    static bool boolean = true;
    static float floating = 3.14159;
    static float float2[2] = {1,2};
    static float float3[3] = {1,2,3};
    static float float4[4] = {1,2,3,4};
    static float rgb[3] = {0.84,0.67,0.17};
    static float rgba[4] = {0.67,0.90,0.12,1};
    static float slider = 0.5f;
    static float slider2 = 0.5f;
    static char string[64] = "hello world 123";
    static int item = 0; const char *list[] = {"one","two","three"};
    static bool show_dialog = false;
    static uint8_t bitmask = 0x55;

    // if( ui_menu("File;Open;Save;Quit") ) printf("menu option %d\n", ui_item());
    // if( ui_menu("Edit;Cut;Copy;Paste") ) printf("menu option %d\n", ui_item());

    if( ui_begin("UI", 0)) {
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}
        if( ui_separator() ) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_string("my string", string, 64) ) puts("string changed");
        if( ui_separator() ) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
        if( ui_separator() ) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");
        if( ui_separator() ) {}
        if( ui_color3f("my color3", rgb) ) puts("color3 changed");
        if( ui_color4f("my color4@this is a tooltip", rgba) ) puts("color4 changed");
        if( ui_separator() ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");
        if( ui_bits8("my bitmask", &bitmask) ) puts("bitmask changed");
        if( ui_separator() ) {}
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_separator() ) {}
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        if(0) { // plot
            nk_layout_row_dynamic(ui_ctx, 100, 1);
            static float values[1000] = {0}; static int offset = 0;
            values[offset=(offset+1)%1000] = window_fps();
            nk_plot(ui_ctx, NK_CHART_LINES, values,1000, 0);
        }
        if(0) { // logger/console
            static char box_buffer[512] = {0};
            static int box_len;

            nk_layout_row_dynamic(ui_ctx, 100, 1); // height(100), horiz_slots(1)
            nk_edit_string(ui_ctx, NK_EDIT_BOX/*_EDITOR*/|NK_EDIT_GOTO_END_ON_ACTIVATE/*|NK_EDIT_READ_ONLY*/, box_buffer, &box_len, 512, nk_filter_default);

            ui_separator();

            static char text[64] = "demo";
            static int text_len = 4;

            nk_layout_row_dynamic(ui_ctx, 0, 1); // height(min), horiz_slots(2)
            int active = nk_edit_string(ui_ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_NO_CURSOR, text, &text_len, 64, nk_filter_ascii);
            if (active & NK_EDIT_COMMITED) {
                text[text_len++] = '\n';
                memcpy(&box_buffer[box_len], &text, text_len);
                box_len += text_len;
                text_len = 0;
            }

            if(text_len > 0 && text[0] != '>') { snprintf(text, 64, va(">%s", text)); text_len++; }
        }

        ui_end();
    }
}
#line 0
#line 1 "fwk_video.c"
struct video_t {
    // mpeg player
    plm_t *plm;
    double previous_time;
    bool paused;
    bool has_ycbcr;
    bool has_audio;
    // yCbCr
    union {
        struct {
            texture_t textureY;
            texture_t textureCb;
            texture_t textureCr;
        };
        texture_t textures[3];
    };
    // rgb
    void *surface;
    texture_t texture;
};

static void mpeg_update_texture(GLuint unit, GLuint texture, plm_plane_t *plane) {
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RED, plane->width, plane->height, 0,
        GL_RED, GL_UNSIGNED_BYTE, plane->data
    );
}
static void mpeg_video_callback( plm_t* plm, plm_frame_t* frame, void* user ) {
    video_t *v = (video_t*)user;
    if(v->paused) return;

    if(v->has_ycbcr) {
        mpeg_update_texture(GL_TEXTURE0, v->textureY.id, &frame->y);
        mpeg_update_texture(GL_TEXTURE1, v->textureCb.id, &frame->cb);
        mpeg_update_texture(GL_TEXTURE2, v->textureCr.id, &frame->cr);
    } else {
        plm_frame_to_rgb( frame, v->surface, v->texture.w * 3 );
        texture_update( &v->texture, v->texture.w, v->texture.h, v->texture.n, v->surface, v->texture.flags );
    }
}
static void mpeg_audio_callback(plm_t *plm, plm_samples_t *samples, void *user) {
    video_t *v = (video_t*)user;
    audio_queue(v->paused ? NULL : samples->interleaved, samples->count, AUDIO_FLOAT | AUDIO_2CH | AUDIO_44KHZ );
}

video_t* video( const char *filename, int flags ) {
    plm_t* plm = plm_create_with_filename( vfs_find(filename) );
    if ( !plm ) {
        PANIC( "!Cannot open '%s' file for reading\n", filename );
        return 0;
    }

    int w = plm_get_width( plm );
    int h = plm_get_height( plm );
    float fps = plm_get_framerate( plm );
    float rate = plm_get_samplerate( plm );

    video_t *v = MALLOC(sizeof(video_t)), zero = {0};
    *v = zero;

    v->has_ycbcr = flags & VIDEO_RGB ? 0 : 1;

    if( v->has_ycbcr ) {
        v->textureY = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCb = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCr = texture_create( w, h, 1, NULL, TEXTURE_R );
    } else {
        int w16 = (w+15) & ~15;
        int h16 = (h+15) & ~15;
        v->texture = texture_create( w16, h16, 3, NULL, TEXTURE_SRGB );
        v->surface = REALLOC( v->surface,  w16 * h16 * 3 );
    }

    v->plm = plm;

    v->has_audio = flags & VIDEO_NO_AUDIO ? 0 : 1;

    plm_set_loop(plm, false);
    plm_set_video_decode_callback(plm, mpeg_video_callback, v);
    if( v->has_audio ) {
        plm_set_audio_enabled(plm, true);
        plm_set_audio_stream(plm, 0);
        plm_set_audio_decode_callback(plm, mpeg_audio_callback, v);
    }

    PRINTF( "Video texture: %s (%dx%dx%d %.0ffps %.1fKHz)\n", file_name(filename), w, h, v->has_ycbcr ? 3:1, fps, rate / 1000 );
    return v;
}

texture_t* video_decode( video_t *v ) { // decodes next frame, returns associated texture(s)
    double current_time = time_ss();
    double elapsed_time = current_time - v->previous_time;
    if (elapsed_time > 1.0 / 30.0) {
        elapsed_time = 1.0 / 30.0;
    }
    v->previous_time = current_time;

    if(!v->paused)
    plm_decode(v->plm, elapsed_time);

    return v->has_ycbcr ? &v->textureY : &v->texture;
}

void video_destroy(video_t *v) {
    plm_destroy( v->plm );

    if( v->has_ycbcr ) {
        texture_destroy(&v->textureY);
        texture_destroy(&v->textureCr);
        texture_destroy(&v->textureCb);
    } else {
        texture_destroy(&v->texture);
        v->surface = REALLOC(v->surface, 0);
    }

    video_t zero = {0};
    *v = zero;
    FREE(v);
}

int video_has_finished(video_t *v) {
    return !!plm_has_ended(v->plm);
}
double video_duration(video_t *v) {
    return plm_get_duration(v->plm);
}
int video_seek(video_t *v, double seek_to) {
    plm_seek(v->plm, clampf(seek_to, 0, video_duration(v)), FALSE);
    if( v->has_audio ) audio_queue_clear();
    return 1;
}
double video_position(video_t *v) {
    return plm_get_time(v->plm);
}
void video_pause(video_t *v, bool paused) {
    v->paused = paused;
}
bool video_is_paused(video_t *v) {
    return v->paused;
}
texture_t* video_textures( video_t *v ) {
    return v->has_ycbcr ? &v->textureY : &v->texture;
}

#line 0
#line 1 "fwk_window.c"

//-----------------------------------------------------------------------------
// fps locking

static volatile float framerate = 0;
static volatile unsigned fps_active, timer_counter, loop_counter;
static
int fps__timing_thread(void *arg) {
    int64_t ns_excess = 0;
    while( fps_active ) {
        if( framerate <= 0 ) {
            loop_counter = timer_counter = 0;
            sleep_ms(250);
        } else {
            timer_counter++;
            int64_t tt = (int64_t)(1e9/(float)framerate) - ns_excess;
            uint64_t took = -time_ns();
        #ifdef _WIN32
            timeBeginPeriod(1); Sleep( tt > 0 ? tt/1e6 : 0 );
        #else
            sleep_ns( (float)tt );
        #endif
            took += time_ns();
            ns_excess = took - tt;
            if( ns_excess < 0 ) ns_excess = 0;
            //puts( strf("%lld", ns_excess) );
        }
    }
    fps_active = 1;

    return thread_exit(0), 0;
}
static
void fps_locker( int on ) {
    if( on ) {
        // private threaded timer
        fps_active = 1, timer_counter = loop_counter = 0;
        thread_init( fps__timing_thread, 0, "fps__timing_thread()", 0 );
    } else {
        fps_active = 0;
    }
}
// function that locks render to desired `framerate` framerate (in FPS).
// - assumes fps_locker() was enabled beforehand.
// - returns true if must render, else 0.
static
int fps_wait() {
    if( framerate <= 0 ) return 1;
    if( !fps_active ) return 1;

    // if we throttled too much, cpu idle wait
    while( fps_active && (loop_counter > timer_counter) ) {
        //thread_yield();
        sleep_ns(100);
    }

    // max auto frameskip is 10: ie, even if speed is low paint at least one frame every 10
    enum { maxframeskip = 10 };
    if( timer_counter > loop_counter + maxframeskip ) {
        loop_counter = timer_counter;
    }
    loop_counter++;

    // only draw if we are fast enough, otherwise skip the frame
    return loop_counter >= timer_counter;
}
static
void window_vsync(float hz) {
    if( hz <= 0 ) return;
    do_once fps_locker(1);
    framerate = hz;
    fps_wait();
}

//-----------------------------------------------------------------------------

static void (*hooks[64])() = {0};
static void *userdatas[64] = {0};

bool window_hook(void (*func)(), void* user) {
    window_unhook( func );
    for( int i = 0; i < 64; ++i ) {
        if( !hooks[i] ) {
            hooks[i] = func;
            userdatas[i] = user;
            return true;
        }
    }
    return false;
}
void window_unhook(void (*func)()) {
    for( int i = 0; i < 64; ++i ) {
        if(hooks[i] == func) {
            hooks[i] = 0;
            userdatas[i] = 0;
        }
    }
}

static GLFWwindow *window;
static int w, h, xpos, ypos, paused;
static int fullscreen, xprev, yprev, wprev, hprev;
static uint64_t frame_count;
static double t, dt, fps, hz = 0.00;
static char title[128] = {0};
static char screenshot_file[512];
static char videorec_file[512];
static int locked_aspect_ratio = 0;

void window_drop_callback(GLFWwindow* window, int count, const char** paths) {
    // @fixme: win: convert from utf8 to window16 before processing
    // @fixme: remove USERNAME for nonwin32
    // @fixme: wait until any active import (launch) is done

    char pathdir[512]; snprintf(pathdir, 512, "%s/import/%llu_%s/", ART, date(), ifdef(linux, getlogin(), getenv("USERNAME")));
    mkdir( pathdir, 0777 );

    int errors = 0;
    for( int i = 0; i < count; ++i ) {
        const char *src = paths[i];
        const char *dst = va("%s%s", pathdir, file_name(src));
        errors += file_copy(src, dst) ? 0 : 1;
    }

    if( errors ) PANIC("%d errors found during file dropping", errors);
    else  app_reload();
}

static // @todo
void window_create_from_handle(void *handle, int flags) {
    fwk_init();
    if(!t) t = glfwGetTime();

window = handle;
glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
glDebugEnable();

    // 0:disable vsync, 1:enable vsync, <0:adaptive (allow vsync when framerate is higher than syncrate and disable vsync when framerate drops below syncrate)
    flags |= optioni("--with-vsync", 1) || flag("--with-vsync") ? WINDOW_VSYNC : WINDOW_VSYNC_DISABLED;
    flags |= optioni("--with-vsync-adaptive", 0) || flag("--with-vsync-adaptive") ? WINDOW_VSYNC_ADAPTIVE : 0;
    int has_adaptive_vsync = glfwExtensionSupported("WGL_EXT_swap_control_tear") || glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("EXT_swap_control_tear");
    int wants_adaptive_vsync = (flags & WINDOW_VSYNC_ADAPTIVE);
    int interval = has_adaptive_vsync && wants_adaptive_vsync ? -1 : (flags & WINDOW_VSYNC_DISABLED ? 0 : 1);
    glfwSwapInterval(interval);

    GLFWmonitor* monitor = glfwGetWindowMonitor(window); if(!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    PRINTF("Monitor: %s (%dHz, vsync=%d)\n", glfwGetMonitorName(monitor), mode->refreshRate, interval);
    PRINTF("GPU device: %s\n", glGetString(GL_RENDERER));
    PRINTF("GPU driver: %s\n", glGetString(GL_VERSION));

    int lock_aspect_ratio = !!( flags & (WINDOW_SQUARE | WINDOW_PORTRAIT | WINDOW_LANDSCAPE | WINDOW_FIXED));
    if (lock_aspect_ratio) window_lock_aspect(w, h);

    // window_cursor(flags & WINDOW_NO_MOUSE ? false : true);

    glfwSetDropCallback(window, window_drop_callback);

    glfwPollEvents();

    fwk_pre_init_subsystems();

    // display a progress bar meanwhile cooker is working in the background
    // Sleep(500);
    if( file_directory(TOOLS) && cooker_jobs() )
    while( cooker_progress() < 100 ) {
        for( int frames = 0; frames < 10 && window_swap(); frames += cooker_progress() >= 100 ) {
            window_title(va("Cooking assets %.2d%%", cooker_progress()));

            glfwGetFramebufferSize(window, &w, &h);
            glNewFrame();

            static float previous[100] = {0};

            #define draw_cooker_progress_bar(JOB_ID, JOB_MAX, PERCENT) do { \
               /* NDC coordinates (2d): bottom-left(-1,-1), center(0,0), top-right(+1,+1) */ \
               float progress = (PERCENT+1) / 100.f; if(progress > 1) progress = 1; \
               float speed = progress < 1 ? 0.2f : 0.5f; \
               float smooth = previous[JOB_ID] = progress * speed + previous[JOB_ID] * (1-speed); \
               \
               float pixel = 2.f / window_height(), dist = smooth*2-1, y = pixel*3*JOB_ID; \
               if(JOB_ID==0)ddraw_line(vec3(-1,y-pixel*2,0), vec3(1,   y-pixel*2,0)); /* full line */ \
               ddraw_line(vec3(-1,y-pixel  ,0), vec3(dist,y-pixel  ,0)); /* progress line */ \
               ddraw_line(vec3(-1,y+0      ,0), vec3(dist,y+0      ,0)); /* progress line */ \
               ddraw_line(vec3(-1,y+pixel  ,0), vec3(dist,y+pixel  ,0)); /* progress line */ \
               if(JOB_ID==JOB_MAX-1)ddraw_line(vec3(-1,y+pixel*2,0), vec3(1,   y+pixel*2,0)); /* full line */ \
            } while(0)

            for(int i = 0; i < cooker_jobs(); ++i) draw_cooker_progress_bar(i, cooker_jobs(), jobs[i].progress);
            // draw_cooker_progress_bar(0, 1, cooker_progress());

            ddraw_flush();

            do_once window_visible(1);
        }
        // set black screen
        glfwGetFramebufferSize(window, &w, &h);
        glNewFrame();
        window_swap();
        window_title("");
    }

    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    fwk_post_init_subsystems();

    hz = mode->refreshRate;

    //    t = glfwGetTime();
}

void window_create(float zoom, int flags) {
    fwk_init();
    if(!t) t = glfwGetTime();

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    int area_width, area_height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &area_width, &area_height);
    float width = area_width, height = area_height, ratio = width / (height + !!height);

#ifdef __APPLE__
//glfwInitHint( GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );
#endif

    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // osx
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // osx, 2:#version150,3:330
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //osx
    glfwWindowHint(GLFW_STENCIL_BITS, 8); //osx
glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

//glfwWindowHint( GLFW_RED_BITS, 8 );
//glfwWindowHint( GLFW_GREEN_BITS, 8 );
//glfwWindowHint( GLFW_BLUE_BITS, 8 );
//glfwWindowHint( GLFW_ALPHA_BITS, 8 );
//glfwWindowHint( GLFW_DEPTH_BITS, 24 );

//glfwWindowHint(GLFW_AUX_BUFFERS, Nth);
//glfwWindowHint(GLFW_STEREO, GL_TRUE);
glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

// Prevent fullscreen window minimize on focus loss
glfwWindowHint( GLFW_AUTO_ICONIFY, GL_FALSE );

    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // makes it non-resizable
    if(flags & WINDOW_MSAA2) glfwWindowHint(GLFW_SAMPLES, 2); // x2 AA
    if(flags & WINDOW_MSAA4) glfwWindowHint(GLFW_SAMPLES, 4); // x4 AA
    if(flags & WINDOW_MSAA8) glfwWindowHint(GLFW_SAMPLES, 8); // x8 AA

    zoom = (zoom < 1 ? zoom * 100 : zoom > 100 ? 100 : zoom);
    if( zoom >= 100 ) {
        // full screen
        window = glfwCreateWindow(width = mode->width, height = mode->height, "", monitor, NULL);
    }
    else if( zoom >= 99 ) {
        // full screen windowed
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(width = mode->width, height = mode->height, "", monitor, NULL);
    } else {
        // windowed
        if( flags & WINDOW_SQUARE )   width = height = width > height ? height : width;
        if( flags & WINDOW_LANDSCAPE ) if( width < height ) height = width * ratio;
        if( flags & WINDOW_PORTRAIT )  if( width > height ) width = height * (1.f / ratio);

        window = glfwCreateWindow(width * zoom / 100, height * zoom / 100, "", NULL, NULL);
        glfwSetWindowPos(window, xpos = xpos + (area_width - width * zoom / 100) / 2, ypos = ypos + (area_height - height * zoom / 100) / 2);
    }
    wprev = w, hprev = h;
    xprev = xpos, yprev = ypos;

    window_create_from_handle(window, flags);
}

static double boot_time = 0;

static
char* window_stats() {
    static double num_frames = 0, begin = FLT_MAX, prev_frame = 0;

    double now = time_ss();
    if( boot_time < 0 ) boot_time = now;

    if( begin > now ) {
        begin = now;
        num_frames = 0;
    }
    if( (now - begin) >= 0.25f ) {
        fps = num_frames * (1.f / (now - begin));
    }
    if( (now - begin) > 1 ) {
        begin = now + ((now - begin) - 1);
        num_frames = 0;
    }

    // @todo: print %used/%avail kib mem, %used/%avail objs as well
    static char buf[192];
    snprintf(buf, 192, "%s | boot %.2fs | %5.2ffps (%.2fms)", title, !boot_time ? now : boot_time, fps, (now - prev_frame) * 1000.f);

    prev_frame = now;
    ++num_frames;

    return buf + 3 * (buf[0] == ' ');
}

int window_needs_flush = 1;
void window_flush() {
    // flush systems that are batched and need to be rendered before frame swapping. order matters.
    if( window_needs_flush ) {
        window_needs_flush = 0;

        fwk_pre_swap_subsystems();
    }
}
void window_lock_fps(float fps) {
    hz = fps;
}
void window_unlock_fps() {
    hz = 0;
}
int window_swap() {
    int ready = !glfwWindowShouldClose(window);
    if( !ready ) {
        #if OFLAG < 3
        #if WITH_SELFIES

//            static int frame = 100;
            bool do_it = (cooker_progress() >= 100); // && ( frame > 0 && !--frame ); // || input_down(KEY_F12)
            if(do_it) {
               snprintf(screenshot_file, 512, "%s.png", app_name());

               int n = 3;
               void *rgb = screenshot(n);
               stbi_flip_vertically_on_write(true);
               if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
                   PANIC("!could not write screenshot file `%s`\n", screenshot_file);
               }
               screenshot_file[0] = 0;
            }

        #endif
        #endif
    }
    else {
        static int first = 1;

        window_flush();

        glfwPollEvents();

        // input_update(); // already hooked!

        double now = paused ? t : glfwGetTime();
        dt = now - t;
        t = now;

#if 0
        static unsigned frames = 0;
        static double t = 0;
        t += window_delta();
        if( t >= 1.0 ) {
            fps = frames / t;
            glfwSetWindowTitle(window, va("%s | %5.2f fps (%.2fms)", title, fps, (t*1000.0) / fps));
            frames = 0;
            t = 0;
        }
        ++frames;
#else
        char *st = window_stats();
        static double t = 0;
        t += window_delta();
        if( t >= 0.25 ) {
            glfwSetWindowTitle(window, st);
            t = 0;
        }
#endif

        // save screenshot if queued
        if( screenshot_file[0] ) {
            int n = 3;
            void *rgb = screenshot(n);
            stbi_flip_vertically_on_write(true);
            if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
                PANIC("!could not write screenshot file `%s`\n", screenshot_file);
            }
            screenshot_file[0] = 0;
        }
        if( videorec_file[0] ) {
            for( FILE *fp = fopen(videorec_file, "a+b"); fp; fclose(fp), fp = 0) {
                void* rgb = screenshot(-3); // 3 RGB, -3 BGR
                jo_write_mpeg(fp, rgb, window_width(), window_height(), 24);  // 24fps
            }
            // videorec_file[0] = 0;
        }

        if( !first ) {
            // glFinish();
            window_vsync(hz);
            glfwSwapBuffers(window);
            ++frame_count;
        }

        glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);
        glNewFrame();
        window_needs_flush = 1;

        // @todo: deprecate me, this is only useful for apps that plan to use ddraw without any camera setup
        // ddraw_flush();

        // run user-defined hooks
        for(int i = 0; i < 64; ++i) {
            if( hooks[i] ) hooks[i]( userdatas[i] );
        }

        first = 0;
    }
    return ready;
}

int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_aspect() {
    return (double)w / (h + !!h);
}
double window_time() {
    return t;
}
double window_delta() {
    return dt;
}
double window_fps() {
    return fps;
}
uint64_t window_frame() {
    return frame_count;
}
void window_title(const char *title_) {
    snprintf(title, 128, "%s", title_);
    if( !title[0] ) glfwSetWindowTitle(window, title);
}
void window_icon(const char *file_icon) {
    void *data = vfs_read(file_icon); data = data ? data : file_read(file_icon);
    if( data ) {
        unsigned len = vfs_size(file_icon); len = len ? len : file_size(file_icon);
        if( len ) {
            image_t img = image_from_mem(data, len, IMAGE_RGBA);
            if( img.w && img.h && img.pixels ) {
                GLFWimage images[1];
                images[0].width = img.w;
                images[0].height = img.h;
                images[0].pixels = img.pixels;
                glfwSetWindowIcon(window, 1, images);
                return;
            }
        }
    }
#ifdef _WIN32
    HANDLE hIcon = LoadImageA(0, file_icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if( hIcon ) {
        HWND hWnd = glfwGetWin32Window(window);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        return;
    }
#endif
}
void* window_handle() {
    return window;
}
static
GLFWmonitor *window_find_monitor(int wx, int wy) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    // find best monitor given current window coordinates. @todo: select by ocuppied window area inside each monitor instead.
    int num_monitors = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&num_monitors);
    for( int i = 0; i < num_monitors; ++i) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
        monitor = wx >= mx && wx <= (mx+mw) && wy >= my && wy <= (my+mh) ? monitors[i] : monitor;
    }

    return monitor;
}
void window_fullscreen(int enabled) {
    fullscreen = !!enabled;
    if( fullscreen ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
}
int window_has_fullscreen() {
    return fullscreen;
}
void window_pause(int enabled) {
    paused = enabled;
}
int window_has_pause() {
    return paused;
}
void window_focus() {
    glfwFocusWindow(window);
}
int window_has_focus() {
    return !!glfwGetWindowAttrib(window, GLFW_FOCUSED);
}
void window_cursor(int visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
int window_has_cursor() {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}
void window_visible(int visible) {
    if(!window) return;
    //if(window) (visible ? glfwRestoreWindow : glfwIconifyWindow)(window);
    (visible ? glfwShowWindow : glfwHideWindow)(window);
    // call glfwpollevents in linux to flush visiblity changes that would happen in next frame otherwise
    #if is(linux) || is(osx)
    glfwPollEvents();
    #endif
}
int window_has_visible() {
    return glfwGetWindowAttrib(window, GLFW_VISIBLE);
}

void window_screenshot(const char* filename_png) {
    snprintf(screenshot_file, 512, "%s", filename_png ? filename_png : "");
}

void window_videorec(const char* filename_mpg) {
    snprintf(videorec_file, 512, "%s", filename_mpg ? filename_mpg : "");
}
int window_has_videorec() {
    return !!videorec_file[0];
}

void window_lock_aspect(unsigned numer, unsigned denom) {
    if(!window) return;
    if( numer * denom ) {
        glfwSetWindowAspectRatio(window, numer, denom);
    } else {
        glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
}
void window_unlock_aspect() {
    if(!window) return;
    window_lock_aspect(0, 0);
}
#line 0
#line 1 "fwk_editor.c"
// editing:
// nope > functions: add/rem property

static int editor_mode = 1;
static int editor_selected = -1; // object in scene selected
static vec2 editor_mouse; // 2d coord for ray/picking
static int editor_hz = 60;

vec3 editor_pick(float mouse_x, float mouse_y) {
    // unproject 2d coord as 3d coord
    camera_t *camera = camera_get_active();
    vec3 out, xyd = vec3(mouse_x,window_height()-mouse_y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
    bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);
    return out;
}

static int gizmo__mode;
static int gizmo__active;
bool gizmo_active() {
    return gizmo__active;
}
int gizmo(vec3 *pos, vec3 *rot, vec3 *sca) {
    uint32_t bak = dd_color;

#if 0
    ddraw_flush();
    mat44 copy; copy44(copy, camera_get_active()->view);
    if( 1 ) {
        float *mv = camera_get_active()->view;
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(4) mv[4*0+0] = d, mv[4*0+1] = 0, mv[4*0+2] = 0;
        if(2) mv[4*1+0] = 0, mv[4*1+1] = d, mv[4*1+2] = 0;
        if(1) mv[4*2+0] = 0, mv[4*2+1] = 0, mv[4*2+2] = d;
    }
#endif

    ddraw_ontop(1);

    vec3 mouse = ui_active() ? vec3(0,0,0) : vec3(input(MOUSE_X),input(MOUSE_Y),input_down(MOUSE_L)); // x,y,l
    vec3 from = camera_get_active()->position;
    vec3 to = editor_pick(mouse.x, mouse.y);
    ray r = ray(from, to);

    static vec3 src3, hit3, off3; static vec2 src2;
    #define on_gizmo_dragged(X,Y,Z,COLOR,DRAWCMD, ...) do { \
        vec3 dir = vec3(X,Y,Z); \
        line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
        plane ground = { vec3(0,0,0), vec3(Y?1:0,Y?0:1,0) }; \
        vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
        aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
        hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
        ddraw_color( hit_arrow || gizmo__active == (X*4+Y*2+Z) ? YELLOW : COLOR ); \
        DRAWCMD; \
        if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), src3 = *pos, hit3 = hit_ground->p, off3 = mul3(sub3(src3,hit3),vec3(X,Y,Z)), gizmo__active = X*4+Y*2+Z; \
        if( (gizmo_active && gizmo__active==(X*4+Y*2+Z)) || (!gizmo__active && hit_arrow) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line : ddraw_line_dashed)(axis.a, axis.b); } \
        if( gizmo__active == (X*4+Y*2+Z) && hit_ground ) {{ __VA_ARGS__ }; modified = 1; gizmo__active *= !!input(MOUSE_L); } \
    } while(0)
    #define gizmo_translate(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, ddraw_arrow(*pos,add3(*pos,vec3(X,Y,Z))), { \
            *pos = add3(line_closest_point(axis, hit_ground->p), off3); \
        } )
    #define gizmo_scale(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, (ddraw_line(*pos,add3(*pos,vec3(X,Y,Z))),ddraw_sphere(add3(*pos,vec3(X-0.1*X,Y-0.1*Y,Z-0.1*Z)),0.1)), { /*ddraw_aabb(arrow.min,arrow.max)*/ \
            int component = (X*1+Y*2+Z*3)-1; \
            float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
            float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
            float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
            float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
            sca->v[component] -= sgn * mag * 0.01; \
            src2 = vec2(mouse.x, mouse.y); \
        } )
    #define gizmo_rotate(X,Y,Z,COLOR) do { \
            vec3 dir = vec3(X,Y,Z); \
            line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
            plane ground = { vec3(0,0,0), vec3(0,1,0) }; \
                vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
                aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
                hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
                int hover = hit_arrow ? (X*4+Y*2+Z) : 0; \
            if( gizmo__active == (X*4+Y*2+Z) ) { ddraw_color(gizmo__active ? YELLOW : WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active && hover == (X*4+Y*2+Z) ) { ddraw_color(COLOR); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active ) { ddraw_color(WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), gizmo__active = hover; \
            if( (!gizmo__active && hover == (X*4+Y*2+Z)) || gizmo__active == (X*4+Y*2+Z) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line_thin : ddraw_line_dashed)(axis.a, axis.b); } \
            if( gizmo__active && gizmo__active == (X*4+Y*2+Z) && hit_ground && !ui_hover() ) { \
                int component = (Y*1+X*2+Z*3)-1; /*pitch,yaw,roll*/ \
                float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
                float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
                float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
                float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
                rot->v[component] += sgn * mag; \
                /*rot->v[component] = clampf(rot->v[component], -360, +360);*/ \
                src2 = vec2(mouse.x, mouse.y); \
                \
            } \
            gizmo__active *= !!input(MOUSE_L); \
        } while(0)

    int modified = 0;
    if(input_down(KEY_SPACE)) gizmo__active = 0, gizmo__mode = (gizmo__mode + 1) % 3;
    if(gizmo__mode == 0) gizmo_translate(1,0,0, RED);
    if(gizmo__mode == 0) gizmo_translate(0,1,0, GREEN);
    if(gizmo__mode == 0) gizmo_translate(0,0,1, BLUE);
    if(gizmo__mode == 1) gizmo_scale(1,0,0, RED);
    if(gizmo__mode == 1) gizmo_scale(0,1,0, GREEN);
    if(gizmo__mode == 1) gizmo_scale(0,0,1, BLUE);
    if(gizmo__mode == 2) gizmo_rotate(1,0,0, RED);
    if(gizmo__mode == 2) gizmo_rotate(0,1,0, GREEN);
    if(gizmo__mode == 2) gizmo_rotate(0,0,1, BLUE);

#if 0
    ddraw_flush();
    copy44(camera_get_active()->view, copy);
#endif

    ddraw_ontop(0);
    ddraw_color(bak);

    return modified;
}

void editor_update() {
    scene_t *scene = scene_get_active();
    camera_t *camera = camera_get_active();

    // input: mouse
    bool any_active = (ui_hover() || ui_active()) || gizmo_active();
    if( !any_active && input_down(MOUSE_L) ) {
        editor_mouse = vec2(input(MOUSE_X), input(MOUSE_Y));
    }
    if( !any_active && input_click(MOUSE_L, 500) ) { // pick entity
        // unproject 2d coord as 3d coord
        vec3 out = editor_pick(editor_mouse.x, editor_mouse.y);
        vec3 from = camera_get_active()->position, to = out;
        ray r = ray(from, to);
        //ddraw_line(from, to); // visualize ray

        int found = -1, count = scene_count();
        for( int i = 0; i < count; ++i ) {
            object_t *obj = scene_index(i);
            // bring aabb box to object position
            aabb box = model_aabb(obj->model, obj->transform); //add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
            // test ray hit
            if( ray_hit_aabb(r, box) ) {
                editor_selected = i;
                break;
            }
        }
    }

    object_t *obj = 0;
    if( editor_selected >= 0 ) {
        obj = scene_index(editor_selected);
        // bring aabb box to object position
        aabb box = model_aabb(obj->model, obj->transform); // aabb box = aabb(add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
        ddraw_color(YELLOW);
        ddraw_aabb(box.min, box.max);
        // draw gizmo
        if( gizmo(&obj->pos, &obj->euler, &obj->sca) ) {
            object_update(obj);
        }
    }

    if( ui_begin("Editor", 0) ) {
        bool x;
        ui_float2("mouse (2d pick)", &editor_mouse.x);
        if( ui_bool("breakpoint", (x = 0, &x)) ) breakpoint("editor breakpoint");
        if( ui_bool("debugger", (x = has_debugger(), &x))) {}
        if( ui_bool("fullscreen", (x = window_has_fullscreen(), &x)) ) window_fullscreen( x );
        if( ui_bool("pause", (x = window_has_pause(), &x)) ) window_pause( x );
        if( ui_int(va("target fps (%.2f)", fps), &editor_hz) ) if(editor_hz < 0) editor_hz = 0; if(editor_hz > 0 && editor_hz < 5) editor_hz = 5;
        ui_separator();
        if( editor_selected >= 0 ) {
            ui_label(va("[%p]", obj));
            if(ui_float3("Position", &obj->pos.x))   object_teleport(obj, obj->pos), gizmo__mode = 0;
            if(ui_float3("Rotation", &obj->euler.x)) object_rotate(obj, obj->euler), gizmo__mode = 2;
            if(ui_float3("Scale", &obj->sca.x))      object_scale(obj, obj->sca),    gizmo__mode = 1;
        }
        ui_end();
    }

    window_lock_fps( editor_hz );
}
bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? 1 : 0;
}

void editor() {
    // input: keyboard.
#if 0
    if( input_up(KEY_TAB) ) editor_mode ^= 1; // cycle editor mode
    if( input_up(KEY_F1) )  window_pause( window_has_pause() ^ 1 );
    if( input_up(KEY_F5) )  app_reload();
    if( input_up(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1);
    if( input_up(KEY_F12) ) screenshot("_screenshot.png", 3, true);
#endif
    // @fixme: send keys to game
    // if( input_repeat(KEY_TAB, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    if( ui_menu(ICON_BARS "File;New;" ICON_TEST_GLYPH "Open;-Save;-Quit") ) printf("Selected File.%d\n", ui_item());
    if( ui_menu("Edit;Cut;Copy;Paste") ) printf("Selected Edit.%d\n", ui_item());

    if( editor_mode ) editor_update();
}

char* dialog_load() {
    const char *windowTitle = NULL; 
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };
    int allowMultipleSelections = 0;

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_openFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints, allowMultipleSelections );
}
char* dialog_save() {
    const char *windowTitle = NULL; 
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_saveFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints );
}


#line 0
// editor is last in place, so it can use all internals from above headers

#line 1 "fwk_main.c"
// ----------------------------------------------------------------------------

//static int threadlocal _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

static void fwk_pre_init_subsystems() {
    profile_init();
    ddraw_init();
    sprite_init();

    // window_swap();

    script_init();
    audio_init(0);
}
static int fwk_unlock_pre_swap_events = 0;
static void fwk_post_init_subsystems() {
    // cooker cleanup
    cooker_stop();

    // mount virtual filesystems later (lower priority)
    bool mounted = 0;
    for( int i = 0; i < 16; ++i) {
        mounted |= !!vfs_mount(va(".art[%d].zip", i));
    }
    // mount physical filesystems first (higher priority)
    if(!mounted) vfs_mount(ART);

    // unlock preswap events
    fwk_unlock_pre_swap_events = 1;

    // init more subsystems
    scene_push();           // create an empty scene by default
    input_init();
    network_init();
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;
}
static void fwk_pre_swap_subsystems() {
    if( fwk_unlock_pre_swap_events ) {
        // flush all batched sprites before swap
        sprite_flush();

        // queue ui drawcalls for profiler
        // hack: skip first frame, because of conflicts with ui_menu & ui_begin auto-row order
        static int once = 0; if(once) profile_render(); once = 1;

        // flush all debugdraw calls before swap
        dd_ontop = 0;
        ddraw_flush();
        glClear(GL_DEPTH_BUFFER_BIT);
        dd_ontop = 1;
        ddraw_flush();

        font_goto(0,0);

        // flush all batched ui before swap (creates single dummy if no batches are found)
        ui_create();
        ui_render();
    }
}

static
void fwk_error_callback(int error, const char *description) {
    if( is(osx) && error == 65544 ) return; // whitelisted
    PANIC("%s (error %x)", description, error);
}

#if 0
#include <signal.h>
void fwk_on_signal(int sn) {
    PRINTF("!Signal caught: %d (SIGFPE=%d SIGBUS=%d SIGSEGV=%d SIGILL=%d SIGABRT=%d)\n", sn, SIGFPE, SIGBUS, SIGSEGV, SIGILL, SIGABRT );
//    signal(sn, fwk_on_signal); // continue
}
void fwk_install_signal_handler() {
    signal(SIGFPE, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGBUS, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGSEGV, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGILL, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGABRT, fwk_on_signal); // fwk_on_signal or SIG_IGN
}
/*
void fwk_on_signal_error(int signum, siginfo_t *siginf, void *puc) {
    // [...]
}
void fwk_install_signal_handler() {
    struct sigaction sigact;
    sigact.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigact.sa_sigaction = fwk_on_signal_error;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGFPE, &sigact, NULL);
    sigaction(SIGILL, &sigact, NULL);
    sigaction(SIGSEGV, &sigact, NULL);
    sigaction(SIGBUS, &sigact, NULL);
    sigaction(SIGABRT, &sigact, NULL);
}
*/
#endif

void fwk_init() {
    do_once {
        // printf("-O%d | %s %s\n", O_FLAG, __DATE__, __TIME__); // display optimization flag

        // signal handler
        // fwk_install_signal_handler();

        // init glfw
        glfwSetErrorCallback(fwk_error_callback);
        glfwInit();
        atexit(glfwTerminate);

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        chdir(app_path());
        // skip tcc argvs (if invoked as tcc file.c fwk.c -g -run)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            // __argc = 1; __argv[0] = __argv[1]; char *found = strstr(__argv[1], ".c"); if(found) *found = 0;
            // __argc = 1;
        }

        // create or update cook.zip file
        cooker_config(NULL, NULL, NULL);
        if( file_directory(TOOLS) && cooker_jobs() ) {
            cooker_start( "**", 0|COOKER_ASYNC );
        }
    }
}

// Enable more performant GPUs on laptops. Does this work into a dll?
// int NvOptimusEnablement = 1;
// int AmdPowerXpressRequestHighPerformance = 1;

#line 0
