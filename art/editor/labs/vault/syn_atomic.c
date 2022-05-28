// # atomics ##################################################################
// - rlyeh, public domain
//
// looking for better coverage? check pfx_compiler.h by @mmozeiko (public domain)

#ifndef ATOMIC_H
#define ATOMIC_H
#include <stdint.h>

int64_t atomic_set( int64_t *ptr, const int value );
int64_t atomic_get( int64_t *ptr );
int64_t atomic_inc( int64_t *ptr );
int64_t atomic_dec( int64_t *ptr );

#endif

#ifdef ATOMIC_C
#pragma once
#  if defined _MSC_VER
#   include <intrin.h>
#   define __sync_add_and_fetch(p,x)               (_InterlockedExchangeAdd64((__int64 volatile *)(p), (x)) + (x))
#   define __sync_bool_compare_and_swap(p, c, s)   (_InterlockedCompareExchange64((__int64 volatile *)(p), (__int64)(s), (__int64)(c)) == (__int64)(c))
#   define __sync_lock_test_and_set(p,v)           (_InterlockedExchange64( (__int64 volatile *)(p), (__int64)(v) ))
#elif defined __TINYC__
#   include <winnt.h>
#   define __sync_add_and_fetch(p,x)               (__int64)(InterlockedExchangeAdd((LONG volatile *)(p), (x)) + (x))
#   define __sync_bool_compare_and_swap(p, c, s)   (__int64)(InterlockedCompareExchange((LONG volatile *)(p), (__int32)(s), (__int32)(c)) == (__int32)(c))
#   define __sync_lock_test_and_set(p,v)           (__int64)(InterlockedExchange( (LONG volatile *)(p), (__int32)(v) ))
#   pragma message("TODO: Warning! 32-bit atomics untested")
#else
#endif

int64_t atomic_set( int64_t *ptr, const int value ) {
    return __sync_lock_test_and_set( ptr, value );
}
int64_t atomic_get( int64_t *ptr ) {
    return __sync_add_and_fetch( ptr,  0 );
}
int64_t atomic_inc( int64_t *ptr ) {
    return __sync_add_and_fetch( ptr, +1 );
}
int64_t atomic_dec( int64_t *ptr ) {
    return __sync_add_and_fetch( ptr, -1 );
}

#ifdef ATOMIC_DEMO
#include <assert.h>
#include <stdio.h>
int main() {
    int64_t k = 0;
    assert( atomic_set(&k, 123) ==   0 );
    assert( atomic_set(&k, 123) == 123 );
    assert( atomic_get(&k) == 123 );
    atomic_dec(&k);
    assert( atomic_get(&k) == 122 );
    atomic_inc(&k);
    assert( atomic_get(&k) == 123 );
    assert( puts("ok") >= 0 );
}
#define main main__
#endif // ATOMIC_DEMO
#endif // ATOMIC_C


