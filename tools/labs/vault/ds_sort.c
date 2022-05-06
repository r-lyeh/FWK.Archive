// compare (<0:less,0:equal,>0:greater) ----------------------------------------
// - rlyeh, public domain

// typed
int sort_int(int a, int b);
int sort_u64(uint64_t a, uint64_t b);
int sort_str(const char *a, const char *b);
int sort_ptr(void *a, void *b);

// untyped (generic, qsort style). single indirection.
int sort_int_qs(const void *a, const void *b);
int sort_uns_qs(const void *a, const void *b);
int sort_i16_qs(const void *a, const void *b);
int sort_u16_qs(const void *a, const void *b);
int sort_i32_qs(const void *a, const void *b);
int sort_u32_qs(const void *a, const void *b);
int sort_f32_qs(const void *a, const void *b);
int sort_i64_qs(const void *a, const void *b);
int sort_u64_qs(const void *a, const void *b);
int sort_f64_qs(const void *a, const void *b);
int sort_ptr_qs(const void *a, const void *b);
int sort_str_qs(const void *a, const void *b);
int sort_stri_qs(const void *a, const void *b);
int sort_stra_qs(const void *a, const void *b);

// untyped (generic, qsort style). double indirection.
int sort_int_qs2(const void *a, const void *b);
int sort_uns_qs2(const void *a, const void *b);
int sort_i16_qs2(const void *a, const void *b);
int sort_u16_qs2(const void *a, const void *b);
int sort_i32_qs2(const void *a, const void *b);
int sort_u32_qs2(const void *a, const void *b);
int sort_f32_qs2(const void *a, const void *b);
int sort_i64_qs2(const void *a, const void *b);
int sort_u64_qs2(const void *a, const void *b);
int sort_f64_qs2(const void *a, const void *b);
int sort_ptr_qs2(const void *a, const void *b);
int sort_str_qs2(const void *a, const void *b);
int sort_stri_qs2(const void *a, const void *b);
int sort_stra_qs2(const void *a, const void *b);

// -----------------------------------------------------------------------------

#ifdef SORT_C
#pragma once

// typed. no indirection.

int sort_int(int a, int b) {
    return a - b;
}
int sort_u64(uint64_t a, uint64_t b) {
    return a > b ? +1 : -!!(a - b);
}
int sort_ptr(void *a, void *b) {
    return (uintptr_t)a > (uintptr_t)b ? +1 : -!!((uintptr_t)a - (uintptr_t)b);
}
int sort_str(const char *a, const char *b) {
#if 0 // useful?
    int sa = strlen((const char*)a);
    int sb = strlen((const char*)b);
    return sa>sb ? +1 : sa<sb ? -1 : strncmp((const char*)a, (const char*)b, sa);
#else
    return strcmp((const char *)a, (const char *)b);
#endif
}

// untyped (generic, qsort style). double indirection. decay & pass.

int sort_int_qs2(const void *a, const void *b) { return  sort_int_qs(*(const int**)a, *(const int**)b); }
int sort_uns_qs2(const void *a, const void *b) { return  sort_uns_qs(*(const unsigned**)a, *(const unsigned**)b); }
int sort_i16_qs2(const void *a, const void *b) { return  sort_i16_qs(*(const int16_t**)a, *(const int16_t**)b); }
int sort_u16_qs2(const void *a, const void *b) { return  sort_u16_qs(*(const uint16_t**)a, *(const uint16_t**)b); }
int sort_i32_qs2(const void *a, const void *b) { return  sort_i32_qs(*(const int32_t**)a, *(const int32_t**)b); }
int sort_u32_qs2(const void *a, const void *b) { return  sort_u32_qs(*(const uint32_t**)a, *(const uint32_t**)b); }
int sort_f32_qs2(const void *a, const void *b) { return  sort_f32_qs(*(const float**)a, *(const float**)b); }
int sort_i64_qs2(const void *a, const void *b) { return  sort_i64_qs(*(const int64_t**)a, *(const int64_t**)b); }
int sort_u64_qs2(const void *a, const void *b) { return  sort_u64_qs(*(const uint64_t**)a, *(const uint64_t**)b); }
int sort_f64_qs2(const void *a, const void *b) { return  sort_f64_qs(*(const double**)a, *(const double**)b); }
int sort_ptr_qs2(const void *a, const void *b) { return  sort_ptr_qs(*(const uintptr_t**)a, *(const uintptr_t**)b); }
int sort_str_qs2(const void *a, const void *b) { return  sort_str_qs(*(const char***)a, *(const char***)b); }
int sort_stri_qs2(const void *a,const void *b) { return sort_stri_qs(*(const char***)a, *(const char***)b); }
int sort_stra_qs2(const void *a,const void *b) { return sort_stra_qs(*(const char***)a, *(const char***)b); }

// untyped (generic, qsort style). single indirection.

int sort_int_qs(const void *a, const void *b) { return ( *((      int*)a) - *((      int*)b) ); }
int sort_uns_qs(const void *a, const void *b) { return ( *(( unsigned*)a) - *(( unsigned*)b) ); }
int sort_i16_qs(const void *a, const void *b) { return ( *((  int16_t*)a) - *((  int16_t*)b) ); }
int sort_u16_qs(const void *a, const void *b) { return ( *(( uint16_t*)a) - *(( uint16_t*)b) ); }
int sort_i32_qs(const void *a, const void *b) { return ( *((  int32_t*)a) - *((  int32_t*)b) ); }
int sort_u32_qs(const void *a, const void *b) { return ( *(( uint32_t*)a) - *(( uint32_t*)b) ); }
int sort_f32_qs(const void *p, const void *q) { const     float*a = (const     float*)p; const     float*b = (const     float*)q; return *a > *b ? +1 : -!!(*a-*b); }
int sort_i64_qs(const void *p, const void *q) { const   int64_t*a = (const   int64_t*)p; const   int64_t*b = (const   int64_t*)q; return *a > *b ? +1 : -!!(*a-*b); }
int sort_u64_qs(const void *p, const void *q) { const  uint64_t*a = (const  uint64_t*)p; const  uint64_t*b = (const  uint64_t*)q; return *a > *b ? +1 : -!!(*a-*b); }
int sort_f64_qs(const void *p, const void *q) { const    double*a = (const    double*)p; const    double*b = (const    double*)q; return *a > *b ? +1 : -!!(*a-*b); }
int sort_ptr_qs(const void *p, const void *q) { const uintptr_t*a = (const uintptr_t*)p; const uintptr_t*b = (const uintptr_t*)q; return *a > *b ? +1 : -!!(*a-*b); }

int sort_str_qs(const void *a, const void *b) { // str
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);
}
int sort_stri_qs(const void *a, const void *b) { // str
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    for( ; *sa && *sb ; ++sa, ++sb ) {
        if( ((*sa)|32) != ((*sb)|32)) {
            break;
        }
    }
    return (*sa) - (*sb);
}
int sort_stra_qs(const void *a, const void *b) { // str
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    int sort__alphanum(const char *l, const char *r);
    return sort__alphanum(sa, sb);
}

#include <ctype.h>
int sort__alphanum(const char *l, const char *r) {
    /* The Alphanum Algorithm is an improved sorting algorithm for strings
    containing numbers.  Instead of sorting numbers in ASCII order like a
    standard sort, this algorithm sorts numbers in numeric order.
    The Alphanum Algorithm is discussed at http://www.DaveKoelle.com

    This implementation is Copyright (c) 2008 Dirk Jagdmann <doj@cubic.org>.
    It is a cleanroom implementation of the algorithm and not derived by
    other's works. In contrast to the versions written by Dave Koelle this
    source code is distributed with the libpng/zlib license.
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you
           must not claim that you wrote the original software. If you use
           this software in a product, an acknowledgment in the product
           documentation would be appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and
           must not be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
           distribution. */
    int is_string = 1;
    while(*l && *r) {
        if(is_string) {
            char l_char, r_char;
            while((l_char=*l) && (r_char=*r)) {
                // check if this are digit characters
                const bool l_digit=isdigit(l_char), r_digit=isdigit(r_char);
                // if both characters are digits, we continue in NUMBER mode
                if(l_digit && r_digit) {
                    is_string = 0;
                    break;
                }
                // if only the left character is a digit, we have a result
                if(l_digit) return -1;
                // if only the right character is a digit, we have a result
                if(r_digit) return +1;
                // compute the difference of both characters
                const int diff=l_char - r_char;
                // if they differ we have a result
                if(diff != 0) return diff;
                // otherwise process the next characters
                ++l;
                ++r;
            }
        } else { // mode==NUMBER
            // get the left number
            unsigned long l_int=0;
            while(*l && isdigit(*l)) { // TODO: this can overflow
                l_int=l_int*10 + *l-'0';
                ++l;
            }

            // get the right number
            unsigned long r_int=0;
            while(*r && isdigit(*r)) { // TODO: this can overflow
                r_int=r_int*10 + *r-'0';
                ++r;
            }

            // if the difference is not equal to zero, we have a comparison result
            const long diff=l_int-r_int;
            if(diff != 0)
                return diff;

            // otherwise we process the next substring in STRING mode
            is_string = 1;
        }
    }
    if(*r) return -1;
    if(*l) return +1;
    return 0;
}

#ifdef SORT_DEMO
#include <assert.h>
#include <stdio.h>
int main() {
    uint64_t a64 = 0, b64 = 0;
    assert( 0 == sort_u64(a64, b64));
    const char *astr = "hello", *bstr = "Hello";
    assert( 0  < sort_str(astr, bstr));
    assert( 0 == sort_stri_qs(&astr, &bstr));
    assert( 0  < sort_stra_qs(&astr, &bstr));
    assert(~puts("Ok"));
}
#define main main__
#endif // SORT_DEMO
#endif // SORT_C
