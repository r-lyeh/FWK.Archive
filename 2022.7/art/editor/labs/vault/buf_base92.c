// THE BEERWARE LICENSE (Revision 42):
// <thenoviceoof> wrote this file. As long as you retain this notice you
// can do whatever you want with this stuff. If we meet some day, and you
// think this stuff is worth it, you can buy me a beer in return
// - Nathan Hwang (thenoviceoof)

#ifndef BASE92_H
#define BASE92_H

unsigned base92_encode(const void *in, unsigned inlen, void* out, unsigned outlen);
unsigned base92_decode(const void *in, unsigned inlen, void* out, unsigned outlen);
unsigned base92_bounds(unsigned inlen);

#endif

#ifdef BASE92_C
#pragma once
#include <stdlib.h>
#include <string.h>

unsigned base92_bounds(unsigned inlen) {
    unsigned size = (inlen * 8) % 13, extra_null = 1;
    if(size == 0) return 2 * ((inlen * 8) / 13) + extra_null;
    if(size  < 7) return 2 * ((inlen * 8) / 13) + extra_null + 1;
                  return 2 * ((inlen * 8) / 13) + extra_null + 2;
}

unsigned base92_encode(const void* in, unsigned inlen, void *out, unsigned size) {
    char *res = (char *)out;
    const unsigned char *str = (const unsigned char *)in;
    unsigned int j = 0;          // j for encoded
    unsigned long workspace = 0; // bits holding bin
    unsigned short wssize = 0;   // number of good bits in workspace
    unsigned char c;
    const unsigned char ENCODE_MAPPING[256] = {
        33, 35, 36, 37, 38, 39, 40, 41, 42, 43,
        44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
        74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
        84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
        94, 95, 97, 98, 99, 100, 101, 102, 103, 104,
        105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
        115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
        125, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    if (inlen) {
        for (unsigned i = 0; i < inlen; i++) {
            workspace = workspace << 8 | str[i];
            wssize += 8;
            if (wssize >= 13) {
                int tmp = (workspace >> (wssize - 13)) & 8191;
                c = ENCODE_MAPPING[(tmp / 91)];
                if (c == 0) return 0; // illegal char
                res[j++] = c;
                c = ENCODE_MAPPING[(tmp % 91)];
                if (c == 0) return 0; // illegal char
                res[j++] = c;
                wssize -= 13;
            }
        }
        // encode a last byte
        if (0 < wssize && wssize < 7) {
            int tmp = (workspace << (6 - wssize)) & 63;  // pad the right side
            c = ENCODE_MAPPING[(tmp)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
        } else if (7 <= wssize) {
            int tmp = (workspace << (13 - wssize)) & 8191; // pad the right side
            c = ENCODE_MAPPING[(tmp / 91)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
            c = ENCODE_MAPPING[(tmp % 91)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
        }
    } else {
        res[j++] = '~';
    }
    // add null byte
    res[j] = 0;
    return j;
}

// this guy expects a null-terminated string
// gives back a non-null terminated string, and properly filled len
unsigned base92_decode(const void* in, unsigned size, void *out, unsigned outlen_unused) {
    const char* str = (const char*)in;
    unsigned char *res = (unsigned char *)out;
    int i, j = 0, b1, b2;
    unsigned long workspace = 0;
    unsigned short wssize = 0;
    const unsigned char DECODE_MAPPING[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 0, 255, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
        46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 255, 62, 63, 64,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
        85, 86, 87, 88, 89, 90, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255
    };

    // handle small cases first
    if (size == 0 || (str[0] == '~' && str[1] == '\0')) {
        res[0] = 0;
        return 1;
    }
    // calculate size
    int len = ((size/2 * 13) + (size%2 * 6)) / 8;
    // handle pairs of chars
    for (i = 0; i + 1 < size; i += 2) {
        b1 = DECODE_MAPPING[(str[i])];
        b2 = DECODE_MAPPING[(str[i+1])];
        workspace = (workspace << 13) | (b1 * 91 + b2);
        wssize += 13;
        while (wssize >= 8) {
            res[j++] = (workspace >> (wssize - 8)) & 255;
            wssize -= 8;
        }
    }
    // handle single char
    if (size % 2 == 1) {
        workspace = (workspace << 6) | DECODE_MAPPING[(str[size - 1])];
        wssize += 6;
        while (wssize >= 8) {
            res[j++] = (workspace >> (wssize - 8)) & 255;
            wssize -= 8;
        }
    }
    //assert(j == len);
    return j;
}

#endif
