// Based on code by Jacques Fortier.
// "Redistribution and use in source and binary forms are permitted, with or without modification."
//
// Consistent Overhead Byte Stuffing is an encoding that removes all 0 bytes from arbitrary binary data.
// The encoded data consists only of bytes with values from 0x01 to 0xFF. This is useful for preparing data for
// transmission over a serial link (RS-232 or RS-485 for example), as the 0 byte can be used to unambiguously indicate
// packet boundaries. COBS also has the advantage of adding very little overhead (at least 1 byte, plus up to an
// additional byte per 254 bytes of data). For messages smaller than 254 bytes, the overhead is constant.
// 
// This implementation is designed to be both efficient and robust. 
// The decoder is designed to detect malformed input data and report an error upon detection.
//

#ifndef COBS_H
#define COBS_H

unsigned cobs_bounds(unsigned len);
unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen);
unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen);

#endif

#ifdef COBS_C
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>

unsigned cobs_bounds( unsigned len ) {
    return ceil(len / 254.0) + 1;
}
unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code = 1;
    size_t read_index = 0, write_index = 1, code_index = 0;

    while( read_index < srclen ) {
        if( src[ read_index ] == 0) {
            dst[ code_index ] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        } else {
            dst[ write_index++ ] = src[ read_index++ ];
            code++;
            if( code == 0xFF ) {
                dst[ code_index ] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    dst[ code_index ] = code;
    return write_index;
}
unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code, i;
    size_t read_index = 0, write_index = 0;

    while( read_index < srclen ) {
        code = src[ read_index ];

        if( ((read_index + code) > srclen) && (code != 1) ) {
            return 0;
        }

        read_index++;

        for( i = 1; i < code; i++ ) {
            dst[ write_index++ ] = src[ read_index++ ];
        }
        if( (code != 0xFF) && (read_index != srclen) ) {
            dst[ write_index++ ] = '\0';
        }
    }

    return write_index;
}

#ifdef COBS_DEMO
#include <stdio.h>
#include <string.h>
#include <assert.h>
void test( const char *buffer, int buflen ) {
    char enc[4096];
    int enclen = cobs_encode( buffer, buflen, enc, 4096 );

    char dec[4096];
    int declen = cobs_decode( enc, enclen, dec, 4096 );

    assert( enclen >= buflen );
    assert( declen == buflen );
    assert( memcmp(dec, buffer, buflen) == 0 );

    printf("%d->%d->%d (+%d extra bytes)\n", declen, enclen, declen, enclen - declen);
}
int main() {
    const char *null = 0;
    test( null, 0 );

    const char empty[] = "";
    test( empty, sizeof(empty) );

    const char text[] = "hello world\n";
    test( text, sizeof(text) );

    const char bintext[] = "hello\0\0\0world\n";
    test( bintext, sizeof(bintext) );

    const char blank[512] = {0};
    test( blank, sizeof(blank) );

    char longbintext[1024];
    for( int i = 0; i < 1024; ++i ) longbintext[i] = (unsigned char)i;
    test( longbintext, sizeof(longbintext) );

    assert(~puts("Ok"));
}
#define main main__
#endif // COBS_DEMO
#endif // COBS_C
