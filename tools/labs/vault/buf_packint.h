// int packing -----------------------------------------------------------------
// - rlyeh, public domain

#pragma once
#include <string.h>

// pack16i() -- store a 16-bit int into a char buffer (like htons())
// pack32i() -- store a 32-bit int into a char buffer (like htonl())
// pack64i() -- store a 64-bit int into a char buffer (like htonl())

static void pack16i(uint8_t *buf, uint16_t i, int swap) {
    if( swap ) i = bswap16(i);
    memcpy( buf, &i, sizeof(i) );
}

static void pack32i(uint8_t *buf, uint32_t i, int swap) {
    if( swap ) i = bswap32(i);
    memcpy( buf, &i, sizeof(i) );
}

static void pack64i(uint8_t *buf, uint64_t i, int swap) {
    if( swap ) i = bswap64(i);
    memcpy( buf, &i, sizeof(i) );
}

// unpack16i() -- unpack a 16-bit int from a char buffer (like ntohs())
// unpack32i() -- unpack a 32-bit int from a char buffer (like ntohl())
// unpack64i() -- unpack a 64-bit int from a char buffer (like ntohl())
// changes unsigned numbers to signed if needed.

static int16_t unpack16i(const uint8_t *buf, int swap) {
    uint16_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = bswap16(i);
    return i <= 0x7fffu ? (int16_t)i : -1 -(uint16_t)(0xffffu - i);
}

static int32_t unpack32i(const uint8_t *buf, int swap) {
    uint32_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = bswap32(i);
    return i <= 0x7fffffffu ? (int32_t)i : -1 -(int32_t)(0xffffffffu - i);
}

static int64_t unpack64i(const uint8_t *buf, int swap) {
    uint64_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = bswap64(i);
    return i <= 0x7fffffffffffffffull ? (int64_t)i : -1 -(int64_t)(0xffffffffffffffffull - i);
}

