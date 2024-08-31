// binary endianness ----------------------------------------------------------
// - rlyeh, public domain

#pragma once
#include <stdint.h>

static uint16_t bswap16( uint16_t x ) {
    return (x << 8) | (x >> 8);
}
static uint32_t bswap32( uint32_t x ) { 
    x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
    return (x << 16) | (x >> 16);
}
static uint64_t bswap64( uint64_t x ) { 
    x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL);
    x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL);
    return (x << 32) | (x >> 32);
}
static int is_big() {
    return (*(uint16_t *)"\0\1") == 1;
}
static int is_little() {
    return (*(uint16_t *)"\0\1") != 1;
}

#if defined(_MSC_VER)
#include <stdlib.h>
#define bswap16 _byteswap_ushort
#define bswap32 _byteswap_ulong
#define bswap64 _byteswap_uint64
#elif defined __GNUC__
#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32
#define bswap64 __builtin_bswap64
#endif
