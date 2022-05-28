// base64 de/encoder. Based on code by Jon Mayo - November 13, 2003 (PUBLIC DOMAIN).
// - rlyeh, public domain
//
// note: functions return 0 on error
// note: base64("")==""

#ifndef BASE64_H
#define BASE64_H

unsigned base64_bounds(unsigned inlen);
unsigned base64_encode(const void *in, unsigned inlen, void *out, unsigned outlen);
unsigned base64_decode(const void *in, unsigned inlen, void *out, unsigned outlen);

#endif

#ifdef BASE64_C
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

unsigned base64_bounds(unsigned size) {
    return 4 * ((size + 2) / 3);
}

unsigned base64_encode(const void *in_, unsigned inlen, void *out_, unsigned outlen) {
    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)in_;
    const uint8_t base64enc_tab[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    for(io = 0, ii = 0, v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        ch = in[ii];
        v = (v << 8) | ch;
        rem += 8;
        while (rem >= 6) {
            rem -= 6;
            if (io >= outlen)
                return 0; /* truncation is failure */
            out[io ++] = base64enc_tab[(v >> rem) & 63];
        }
    }
    if (rem) {
        v <<= (6 - rem);
        if (io >= outlen)
            return 0; /* truncation is failure */
        out[io ++] = base64enc_tab[v & 63];
    }
    if (io < outlen)
        out[io] = 0;

    return io;
}

unsigned base64_decode(const void *in_, unsigned inlen, void *out_, unsigned outlen) {
    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)in_;
    const uint8_t base64dec_tab[256]= {
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255, 62,255,255,
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
        255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    };

    for (io = 0, ii = 0,v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        if (isspace(in[ii]))
            continue;
        if ((in[ii]=='=') || (!in[ii]))
            break; /* stop at = or null character*/
        ch = base64dec_tab[(unsigned char)in[ii]];
        if (ch == 255)
            break; /* stop at a parse error */
        v = (v<<6) | ch;
        rem += 6;
        if (rem >= 8) {
            rem -= 8;
            if (io >= outlen)
                return 0; /* truncation is failure */
            out[io ++] = (v >> rem) & 255;
        }
    }
    if (rem >= 8) {
        rem -= 8;
        if (io >= outlen)
            return 0; /* truncation is failure */
        out[io ++] = (v >> rem) & 255;
    }
    return io;
}

#endif // BASE64_C
