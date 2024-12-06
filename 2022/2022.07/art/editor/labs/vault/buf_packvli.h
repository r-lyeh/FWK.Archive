// vli varint spec (licensed under public domain, unlicense, CC0 and MIT-0: pick one).
// - rlyeh.
// 7 [0 xxxx xxx]                        7-bit value in  1 byte  (0-                127)
// 7 [10 xxx xxx] [yyyyyyyy]            14-bit value in  2 bytes (0-             16,383)
// 6 [110 xx xxx] [yyyyyyyy] [zzzzzzzz] 21-bit value in  3 bytes (0-          2,097,151)
// 8 [111 00 xxx] [ 3 bytes]            27-bit value in  4 bytes (0-        134,217,727)
// 8 [111 01 xxx] [ 4 bytes]            35-bit value in  5 bytes (0-     34,359,738,367)
// 5 [111 10 xxx] [ 5 bytes]            43-bit value in  6 bytes (0-  8,796,093,022,207)
// 8 [111 11 000] [ 6 bytes]            48-bit value in  7 bytes (0-281,474,976,710,655)
// 8 [111 11 001] [ 7 bytes]            56-bit value in  8 bytes (...)
// 8 [111 11 010] [ 8 bytes]            64-bit value in  9 bytes
// 8 [111 11 011] [ 9 bytes]            72-bit value in 10 bytes
// 8 [111 11 100] [10 bytes]            80-bit value in 11 bytes
// A [111 11 101] [12 bytes]            96-bit value in 13 bytes
// A [111 11 110] [14 bytes]           112-bit value in 15 bytes
// A [111 11 111] [16 bytes]           128-bit value in 17 bytes
// 1 1 2 3 = 7

#pragma once

#if 0 // try
// 7 [0 xxxx xxx]                        7-bit value in   1 byte  (0-                  127)
// 7 [10 xxx xxx] [yyyyyyyy]            14-bit value in   2 bytes (0-               16,383)
// 6 [110 xx xxx] [yyyyyyyy] [zzzzzzzz] 21-bit value in   3 bytes (0-            2,097,151)
// 8 [111 00 xxx] [ 3 bytes]            27-bit value in   4 bytes (0-          134,217,727)
// 7 [111 01 xxx] [ 4 bytes]            35-bit value in   5 bytes (0-       34,359,738,367)
// 8 [111 10 0xx] [ 5 bytes]            42-bit value in   6 bytes (0-    4,398,046,511,103)
// 6 [111 10 1xx] [ 6 bytes]            50-bit value in   7 bytes (0-1,125,899,906,842,623)
// 8 [111 11 000] [ 7 bytes]            56-bit value in   8 bytes (...)
// 8 [111 11 001] [ 8 bytes]            64-bit value in   9 bytes
// 8 [111 11 010] [ 9 bytes]            72-bit value in  10 bytes
// 8 [111 11 011] [10 bytes]            80-bit value in  11 bytes
// A [111 11 100] [12 bytes]            96-bit value in  13 bytes
// A [111 11 101] [14 bytes]           112-bit value in  15 bytes
// A [111 11 110] [16 bytes]           128-bit value in  17 bytes
// N [111 11 111] [nn bytes]           custom bit value in N+1 bytes
// 1 1 2 1 2 = 7
#endif


#include <stdio.h>
#include <stdint.h>

static uint64_t pack64uv( uint8_t *buffer, uint64_t value ) {
    #define ADD(bits) *buffer++ = (value >>= bits)
    if( value < (1ull<< 7) ) return *buffer = value, 1;
    if( value < (1ull<<14) ) return *buffer++ = 0x80|(value&0x3f), ADD(6), 2;
    if( value < (1ull<<21) ) return *buffer++ = 0xc0|(value&0x1f), ADD(5), ADD(8), 3;
    if( value < (1ull<<27) ) return *buffer++ = 0xe0|(value&0x07), ADD(3), ADD(8), ADD(8), 4;
    if( value < (1ull<<35) ) return *buffer++ = 0xe8|(value&0x07), ADD(3), ADD(8), ADD(8), ADD(8), 5;
    if( value < (1ull<<43) ) return *buffer++ = 0xf0|(value&0x07), ADD(3), ADD(8), ADD(8), ADD(8), ADD(8), 6;
    if( value < (1ull<<48) ) return *buffer++ = 0xf8|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 7;
    if( value < (1ull<<56) ) return *buffer++ = 0xf9|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 8;
  /*if( value < (1ull<<64))*/return *buffer++ = 0xfa|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 9;
  /*...*/
    #undef ADD
}
static uint64_t unpack64uv( const uint8_t *buffer, uint64_t *value ) {
    uint64_t bytes, out = 0, shift = 0;
    const int table[] = { 6,7,8,9,10,12,14,16 };
    /**/ if( *buffer >= 0xf8 ) bytes = table[*buffer - 0xf8];
    else if( *buffer >= 0xe0 ) bytes = 3 + ((*buffer>>3) & 3);
    else if( *buffer >= 0xc0 ) bytes = 2;
    else                       bytes = *buffer >= 0x80;

    #define POP(bits) out = out | (uint64_t)*buffer++ << (shift += bits);
    switch( bytes ) {
        default:
        break; case 0: out = *buffer++;
        break; case 1: out = *buffer++ & 0x3f; POP(6);
        break; case 2: out = *buffer++ & 0x1f; POP(5); POP(8);
        break; case 3: out = *buffer++ & 0x07; POP(3); POP(8); POP(8);
        break; case 4: out = *buffer++ & 0x07; POP(3); POP(8); POP(8); POP(8);
        break; case 5: out = *buffer++ & 0x07; POP(3); POP(8); POP(8); POP(8); POP(8);
        break; case 6: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
        break; case 7: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
        break; case 8: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
    }
    #undef POP

    return *value = out, bytes+1;
}

static uint64_t pack64iv( uint8_t *buffer, int64_t value_ ) {
    uint64_t value = (uint64_t)((value_ >> 63) ^ (value_ << 1));
    return pack64uv(buffer, value); /* convert sign|magnitude to magnitude|sign */
}

static uint64_t unpack64iv( const uint8_t *buffer, int64_t *value ) {
    uint64_t out = 0, ret = unpack64uv( buffer, &out );
    *value = ((out >> 1) ^ -(out & 1)); /* convert magnitude|sign to sign|magnitude */
    return ret;
}



#if 0

// variable int packing --------------------------------------------------------
// vbyte, varint (signed)

static uint64_t pack_LEB128( uint8_t *buffer, int64_t value_ ) {
    /* convert sign|magnitude to magnitude|sign */
    // uint64_t value = (uint64_t)value_; value = value & (1ull << 63) ? ~(value << 1) : (value << 1);
    uint64_t value = (uint64_t)((value_ >> 63) ^ (value_ << 1)); // (branchless)
    /* encode unsigned : 7-bit pack. MSB terminates stream */
    const uint8_t *buffer0 = buffer;
    while( value > 127 ) {
        *buffer++ = value | 0x80; // (uint8_t)(( value & 0xFF ) | 0x80 );
        value >>= 7;
    }
    *buffer++ = value;

    return buffer - buffer0;
}
static uint64_t unpack_LEB128( const uint8_t *buffer, int64_t *value ) {
    /* decode unsigned : 7-bit unpack. MSB terminates stream */
    const uint8_t *buffer0 = buffer;
    uint64_t out = 0, j = -7;
    do {
        out |= ( ((uint64_t)*buffer) & 0x7f) << (j += 7);
    } while( (*buffer++) & 0x80 );
    /* convert magnitude|sign to sign|magnitude */
    // *value = out & (1) ? ~(out >> 1) : (out >> 1);
    *value = ((out >> 1) ^ -(out & 1)); // (branchless)
    return buffer - buffer0;
}

#endif


#ifdef VLI_DEMO

int main() {
    int tests = 0, passes = 0;

    #define testi(v) do { \
        int64_t val = v; \
        char out[16]; \
        int len = pack64iv(out, val); \
        int64_t in = ~val; \
        unpack64iv(out, &in); \
        int ok = val == in; ++tests; passes += ok; \
        printf("%c %02d/%02d (-) %#llx (%llu) <-> %d bytes <-> %#llx (%llu)\n", "NY"[ok], passes, tests, val, val, len, in, in); \
    } while(0)

    #define testu(v) do { \
        uint64_t val = (v); \
        char out[16]; \
        int len = pack64uv(out, val); \
        uint64_t in = ~val; \
        unpack64uv(out, &in); \
        int ok = val == in; ++tests; passes += ok; \
        printf("%c %02d/%02d (+) %#llx (%llu) <-> %d bytes <-> %#llx (%llu)\n", "NY"[ok], passes, tests, val, val, len, in, in); \
    } while(0)

    #define test(v) do { testi(v); testu(v); } while(0)

    test(0);
    test((1ull<<7)-1);
    test( 1ull<<7);
    test((1ull<<14)-1);
    test( 1ull<<14);
    test((1ull<<21)-1);
    test( 1ull<<21);
    test((1ull<<27)-1);
    test( 1ull<<27);
    test((1ull<<35)-1);
    test( 1ull<<35);
    test((1ull<<48)-1);
    test( 1ull<<48);
    test(~0ull-1);
    test(~0ull);

    printf("%d tests, %d errors\n", tests, tests - passes);
}

#endif
