// hash ------------------------------------------------------------------------
// - rlyeh, public domain

uint64_t hash_int(int key);
uint64_t hash_flt(double dbl);
uint64_t hash_u64(uint64_t key);
uint64_t hash_str(const char* str);
uint64_t hash_ptr(const void* ptr);
uint64_t hash_vec2(const float pt[2]);
uint64_t hash_vec3(const float pt[3]);
uint64_t hash_vec4(const float pt[4]);

// compile time string hash (pure C) -------------------------------------------
// Based on code by http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
// This macro is not always inlined. Depends on compilers and optimization flags.
// For example g++ requires -O3 and msvc requires /O2 (verify output assembly with /Fa).
// About the hash function, it is using my own and free hashing algorithm:
// a few more collisions than FNV1A64 (faster though).
// - rlyeh, public domain.
#ifndef HASH_STR
#define HASH_STR(str)                  HASH_STR64(0,131ull,str,0)
#define HASH_STR64(hsh,mul,str,idx)    HASH_STR16(HASH_STR16(HASH_STR16(HASH_STR16(hsh,mul,str,idx+48),mul,str,idx+32),mul,str,idx+16),mul,str,idx)
#define HASH_STR16(hsh,mul,str,idx)    HASH_STR04(HASH_STR04(HASH_STR04(HASH_STR04(hsh,mul,str,idx+12),mul,str,idx+ 8),mul,str,idx+ 4),mul,str,idx)
#define HASH_STR04(hsh,mul,str,idx)    HASH_STR01(HASH_STR01(HASH_STR01(HASH_STR01(hsh,mul,str,idx+ 3),mul,str,idx+ 2),mul,str,idx+ 1),mul,str,idx)
#define HASH_STR01(hsh,mul,str,idx)    ((HASH_STRCHR(str,idx) ^ hsh) * mul)
#define HASH_STRCHR(str,idx)           ((unsigned)str[(idx)<HASH_STRLEN(str)?HASH_STRLEN(str)-1-(idx):HASH_STRLEN(str)])
#define HASH_STRLEN(str)               (sizeof(str)-1)
#endif



#ifdef HASH_C
#pragma once

// { Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!

uint32_t hash32(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t unhash32(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}

uint64_t hash64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}
/*
uint32_t hash64(uint64_t v) {
    uint32_t hi = v >> 32;
    uint32_t lo = v & 0xFFFFFFFF; // (uint32_t)-1;
    return triple32(hi) ^ triple32(lo);
}
*/

uint64_t unhash64(uint64_t x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return x;
}

// } Thomas Mueller

uint64_t hash_triple32(uint32_t x) { 
    // triple32 hashing (unlicensed) https://github.com/skeeto/hash-prospector
    // exact bias: 0.020888578919738908
    x ^= x >> 17;
    x *= UINT32_C(0xed5ad4bb);
    x ^= x >> 11;
    x *= UINT32_C(0xac4c1b51);
    x ^= x >> 15;
    x *= UINT32_C(0x31848bab);
    x ^= x >> 14;
    return x;
}
uint64_t hash_untriple32(uint32_t x) {
    x ^= x >> 14 ^ x >> 28;
    x *= UINT32_C(0x32b21703);
    x ^= x >> 15 ^ x >> 30;
    x *= UINT32_C(0x469e0db1);
    x ^= x >> 11 ^ x >> 22;
    x *= UINT32_C(0x79a85073);
    x ^= x >> 17;
    return x;
}

uint64_t hash_mix64(uint64_t key) {
    // Thomas Wang's 64bit Mix Function (public domain) http://www.cris.com/~Ttwang/tech/inthash.htm
    key += ~(key << 32);
    key ^=  (key >> 22);
    key += ~(key << 13);
    key ^=  (key >>  8);
    key +=  (key <<  3);
    key ^=  (key >> 15);
    key += ~(key << 27);
    key ^=  (key >> 31);
    return key;
}

uint64_t hash_int(int key) {
    // return hash64((uint64_t)key);
    // return hash_mix64((uint64_t)key);
    // return hash_triple32((uint32_t)key);
    return hash32((uint32_t)key);
}

uint64_t hash_u64(uint64_t key) {
    // return hash_mix64(key);
    return hash64(key);
}

// ---

uint64_t hash_ptr(const void *ptr) {
    uint64_t key = (uint64_t)(uintptr_t)ptr;
    return hash_u64(key >> 3); // >> 4? needed?
}

uint64_t hash_flt(double dbl) {
    union { uint64_t i; double d; } u; u.d = dbl;
    return hash_u64( u.i );
}

uint64_t hash_vec2(const float v2[2]) {
    return hash_flt(v2[0]) ^ hash_flt(v2[1]);
}

uint64_t hash_vec3(const float v3[3]) {
    return hash_flt(v3[0]) ^ hash_flt(v3[1]) ^ hash_flt(v3[2]);
}

uint64_t hash_vec4(const float v4[4]) {
    return hash_flt(v4[0]) ^ hash_flt(v4[1]) ^ hash_flt(v4[2]) ^ hash_flt(v4[3]);
}

uint64_t hash_str(const char* str) {
    // faster than fnv1a, a few more collisions though.
    uint64_t hash = 0; // fnv1a: 14695981039346656037ULL;
    while( *str ) {
        hash = ( (unsigned char)*str++ ^ hash ) * 131; // fnv1a: 0x100000001b3ULL;
    }
    return hash;
}

#ifdef HASH_DEMO
#include <stdio.h>
int main() {
    uint64_t literal = (uint64_t)0xf0e28e9bdecd6646ull;
    printf("%016llx\n", literal);

    uint64_t precomp = (uint64_t)STRHASH("/hello/world.txt");
    printf("%016llx\n", precomp);

    uint64_t runtime = (uint64_t)strhash("/hello/world.txt");
    printf("%016llx\n", runtime);
}
#define main __main
#endif // HASH_DEMO
#endif // HASH_C
