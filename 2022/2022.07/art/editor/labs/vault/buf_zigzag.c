// zigzag de/encoder
// - rlyeh, public domain

#ifndef ZIGZAG_H
#define ZIGZAG_H
#include <stdint.h>

uint64_t zig64( int64_t value ); // convert sign|magnitude to magnitude|sign
int64_t zag64( uint64_t value ); // convert magnitude|sign to sign|magnitude

#endif

// -----------------------------------------------------------------------------

#ifdef ZIGZAG_C
#pragma once

uint64_t zig64( int64_t value ) { // convert sign|magnitude to magnitude|sign
    return (value >> 63) ^ (value << 1);
}
int64_t zag64( uint64_t value ) { // convert magnitude|sign to sign|magnitude
    return (value >> 1) ^ -(value & 1); 
}

#ifdef ZIGZAG_DEMO
#include <stdio.h>
int main() {
    int16_t x = -1000;
    printf("%d -> %llu %llx -> %lld\n", x, zig64(x), zig64(x), zag64(zig64(x)));
}
#define main main__
#endif // ZIGZAG_DEMO
#endif // ZIGZAG_C
