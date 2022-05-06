// Based on code by Brian "Beej Jorgensen" Hall (public domain) [1].
// Based on code by Ginger Bill's half<->float (public domain) [2].
// - rlyeh, public domain.
//
// pack.c  -- perl/python-ish pack/unpack functions
// like printf and scanf, but for binary data.
//
// format flags:
//  (<) little endian       (>) big endian (! also)     (=) native endian
//  (c) 8-bit  char         (b) 8-bit  byte
//  (h) 16-bit half         (w) 16-bit word
//  (i) 32-bit integer      (u) 32-bit unsigned         (f) 32-bit float
//  (l) 64-bit long         (q) 64-bit quad             (d) 64-bit double
//  (v) varint
//  (s) string   (64-bit varint length prepended)
//  (S) string   (32-bit fixed  length prepended)
//  (m) memblock (64-bit varint length prepended)
//  (M) memblock (32-bit fixed  length prepended)
//  (z) memblock (zeroed)
//  (#) number of arguments processed (only when unpacking)
//
// @todo:
// - (x) document & test flag
// @totest:
// - (s) string   (64-bit variable length automatically prepended)
// - (S) string   (32-bit fixed    length automatically prepended)
// - (m) memblock (64-bit variable length automatically prepended)
// - (M) memblock (32-bit fixed    length automatically prepended)
// - (z) memblock (zeroed)
// - (#) number of arguments processed (only when unpacking)
//
// @refs:
// [1] http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization (Modified to encode NaN and Infinity as well.)
// [2] https://github.com/gingerBill/gb
// [3] http://www.mrob.com/pub/math/floatformats.html#minifloat
// [4] microfloat: [0.002 to 240] range.
// [5] half float: can approximate any 16-bit unsigned integer or its reciprocal to 3 decimal places.

#ifndef PACK_H
#define PACK_H
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

// - save data dictated by the format string from the buffer. return: number of bytes written, or 0 if error.
//   if first argument is zero, returns number of bytes required for packing.

int savef(FILE *file, const char *format, ...);
int saveb(unsigned char *buf, const char *format, ...);

// - load data dictated by the format string into the buffer. return: number of bytes read, or 0 if error.
//   if first argument is zero, returns number of bytes required for unpacking.

int loadf(FILE *file, const char *format, ...);
int loadb(const unsigned char *buf, const char *format, ...);

#endif


#ifdef PACK_C
#pragma once

// #include "packendian.c" // endianness
// #include "pack754.c" // float754 packing --------------------------------------
// #include "packint.c" // integer packing ---------------------------------------
// #include "packvli.c" // variable int packing ----------------------------------

// b/f packing -----------------------------------------------------------------

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int loadb_(const uint8_t *buf, const char *fmt, va_list ap) {
    uint64_t args = 0;
    const uint8_t *buf0 = buf;
    char tmp[16+1];
    //uint64_t size = 0, len;
    int32_t len, count, maxstrlen=0;
    int le = 0;

    if(!buf) // buffer estimation
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default: if (!isdigit(*fmt)) return 0;
        break; case '!': case '>': case '<': case '=': case ' ':                                                      //  0-bit endianness
        break; case 'c': case 'b': { int8_t c = (int8_t)va_arg(ap, int); buf += 1; }                                  //  8-bit promoted
        break; case 'h': case 'w': { int16_t h = (int16_t)va_arg(ap, int); buf += 2; }                                // 16-bit promoted
        break; case 'i': case 'u': { int32_t l = va_arg(ap, int32_t); buf += 4; }                                     // 32-bit
        break; case 'l': case 'q': { int64_t L = va_arg(ap, int64_t); buf += 8; }                                     // 64-bit
        break; case 'f': { float f = (float)va_arg(ap, double); buf += 4; }                                           // 32-bit float promoted
        break; case 'd': { double F = (double)va_arg(ap, double); buf += 8; }                                         // 64-bit float (double)
        break; case 'v': { int64_t L = va_arg(ap, int64_t); buf += pack64iv(tmp, L); }                                // varint (8,16,32,64 ...)
        break; case 's': { char* s = va_arg(ap, char*); len = strlen(s); buf += pack64iv(tmp, len) + len; }           // string, 64-bit variable length prepended
        break; case 'S': { char* s = va_arg(ap, char*); len = strlen(s); buf += 4 + len; }                            // string, 32-bit fixed    length prepended
        break; case 'm': { int len = va_arg(ap, int); char *s = va_arg(ap, char*); buf += pack64iv(tmp, len) + len; } // memblock, 64-bit variable length prepended
        break; case 'M': { int len = va_arg(ap, int); char *s = va_arg(ap, char*); buf += 4 + len; }                  // memblock, 32-bit fixed    length prepended
        break; case 'z': { int len = va_arg(ap, int); buf += len; }                                                   // memblock (zeroed)
        }
    }

    if(buf) // buffer unpacking
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default:
            if (isdigit(*fmt)) { // track max str len
                maxstrlen = maxstrlen * 10 + (*fmt-'0');
            } else {
                return 0;
            }
        break; case ' ':
        break; case '!': le = 0;
        break; case '>': le = 0;
        break; case '<': le = 1;
        break; case '=': le = is_little() ? 1 : 0;
        break; case 'c': case 'b': ++args; { // 8-bit
            int8_t *v = va_arg(ap, int8_t*);
            *v = *buf <= 0x7f ? (int8_t)*buf : -1 -(uint8_t)(0xffu - *buf);
            buf += 1;
        }
        break; case 'h': case 'w': ++args; { // 16-bit
            int16_t *v = va_arg(ap, int16_t*);
            *v = unpack16i(buf, le);
            buf += 2;
        }
        break; case 'i': case 'u': ++args; { // 32-bit
            int32_t *v = va_arg(ap, int32_t*);
            *v = unpack32i(buf, le);
            buf += 4;
        }
        break; case 'l': case 'q': ++args; { // 64-bit
            int64_t *v = va_arg(ap, int64_t*);
            *v = unpack64i(buf, le);
            buf += 8;
        }
        break; case 'v': ++args; { // varint (8,16,32,64 ...)
            int64_t *L = va_arg(ap, int64_t*);
            buf += unpack64iv(buf, L);
        }
        break; case 'f': ++args; { // 32-bit float
            float *v = va_arg(ap, float*);
            int32_t i = unpack32i(buf, le);
            *v = unpack754_32(i);
            buf += 4;
        }
        break; case 'd': ++args; { // 64-bit float (double)
            double *v = va_arg(ap, double*);
            int64_t i = unpack64i(buf, le);
            *v = unpack754_64(i);
            buf += 8;
        }
        break; case 'S': ++args; { // string, 32-bit fixed length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen = unpack32i(buf, le), read = 4;
            count = (maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            s[count] = '\0';
            buf += read + vlen;
        }
        break; case 's': ++args; { // string, 64-bit variable length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen, read = unpack64iv(buf, &vlen);
            count = (maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'M': ++args; { // memblock, 32-bit fixed length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen = unpack64iv(buf, &vlen), read = 4;
            count = vlen; //(maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            //s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'm': ++args; { // memblock, 64-bit variable length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen, read = unpack64iv(buf, &vlen);
            count = vlen; //(maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            //s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'z': ++args; { // zero-init mem block
            int *l = va_arg(ap, int*);
            const uint8_t *prev = buf;
            while( *buf == 0 ) ++buf;
            *l = buf - prev;
        }
        break; case '#': {
            int *l = va_arg(ap, int*);
            *l = args;
        }
        }

        if (!isdigit(*fmt)) {
            maxstrlen = 0;
        }
    }

    return (int)( buf - buf0 );
}

int saveb_(uint8_t *buf, const char *fmt, va_list ap) {
    uint64_t size = 0, len;
    int le = 0;

    // buffer estimation
    if( !buf ) {
        return loadb_(buf, fmt, ap); // + strlen(buf) * 17; // worse (v)arint estimation for 128-bit ints (17 bytes each)
    }

    // buffer packing
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default: size = 0; // error
        break; case '!': le = 0; 
        break; case '>': le = 0; 
        break; case '<': le = 1; 
        break; case ' ': le = le; 
        break; case '=': le = is_little() ? 1 : 0;
        break; case 'c': case 'b': { // 8-bit
            int v = (int8_t)va_arg(ap, int /*promoted*/ );
            *buf++ = (v>>0)&0xff;
            size += 1;
        }
        break; case 'h': case 'w': { // 16-bit
            int v = (int16_t)va_arg(ap, int /*promoted*/ );
            pack16i(buf, v, le);
            buf += 2;
            size += 2;
        }
        break; case 'i': case 'u': { // 32-bit
            int32_t v = va_arg(ap, int32_t);
            pack32i(buf, v, le);
            buf += 4;
            size += 4;
        }
        break; case 'l': case 'q': { // 64-bit
            int64_t v = va_arg(ap, int64_t);
            pack64i(buf, v, le);
            buf += 8;
            size += 8;
        }
        break; case 'v': { // varint (8,16,32,64 ...)
            int64_t v = va_arg(ap, int64_t);
            int64_t L = pack64iv(buf, v);
            buf += L;
            size += L;
        }
        break; case 'f': { // 32-bit float
            double v = (float)va_arg(ap, double /*promoted*/ );
            int32_t i = pack754_32(v); // convert to IEEE 754
            pack32i(buf, i, le);
            buf += 4;
            size += 4;
        }
        break; case 'd': { // 64-bit float (double)
            double v = (double)va_arg(ap, double);
            int64_t i = pack754_64(v); // convert to IEEE 754
            pack64i(buf, i, le);
            buf += 8;
            size += 8;
        }
        break; case 'S': { // string, 32-bit fixed length prepended
            char* s = va_arg(ap, char*);
            int len = strlen(s);
            pack32i(buf, len, le);
            memcpy(buf + 4, s, len);
            buf += 4 + len;
            size += 4 + len;
        }
        break; case 's': { // string, 64-bit variable length prepended
            char* s = va_arg(ap, char*);
            int len = strlen(s);
            int64_t L = pack64iv(buf, len);
            memcpy(buf + L, s, len);
            buf += L + len;
            size += L + len;
        }
        break; case 'M': { // memblock, 32-bit fixed length prepended
            int len = va_arg(ap, int);
            char* s = va_arg(ap, char*);
            pack32i(buf, len, le);
            memcpy(buf + 4, s, len);
            buf += 4 + len;
            size += 4 + len;
        }
        break; case 'm': { // memblock, 64-bit variable length prepended
            int len = va_arg(ap, int);
            char* s = va_arg(ap, char*);
            int64_t L = pack64iv(buf, len);
            memcpy(buf + L, s, len);
            buf += L + len;
            size += L + len;
        }
        break; case 'z': { // memblock (zeroed)
            int len = va_arg(ap, int);
            memset(buf, 0, len);
            buf += len;
            size += len;
        }
        }
    }

    return (int)size;
}


int saveb(uint8_t *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = saveb_( buf, fmt, ap);
    va_end(ap);
    return rc;
}
int loadb(const uint8_t *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = loadb_( buf, fmt, ap);
    va_end(ap);
    return rc;
}

int savef(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // estimate bytes
    int req = saveb_( 0, fmt, ap);

    char stack[4096];
    char *buf = req < 4096 ? stack : (char*)calloc(1, req + 1 );
        int rc = saveb_(buf, fmt, ap);
        fwrite(buf, req,1, fp);
    if( !(req < 4096) ) free(buf);

    va_end(ap);
    return rc;
}
int loadf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // estimate bytes
    int req = loadb_( 0, fmt, ap) * 2; // *2 in case it is underestimated

    char stack[4096];
    char *buf = req < 4096 ? stack : (char*)calloc(1, req + 1 );
        fread(buf, req,1, fp);
        int rc = loadb_(buf, fmt, ap);
    if( !(req < 4096) ) free(buf);

    va_end(ap);
    return rc;
}
#endif

#ifdef PACK_DEMO
typedef struct bootsector {
    uint8_t jump_instruction[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t max_dirs;
    uint16_t sector_count;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_head;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t sector_countz;
} bootsector;
int main() {
    const char *dna = "3b8bhbhbbhhbhhhuu"; // "1c1h212122122233"; "i3c8chchchhchhhdd"

    bootsector fat = { {0,1,2},{3,4,5,6,7,8,9,10},11,12,13,14,15,16,17,18,19,20,21,22 };
    hexdump(&fat, sizeof(bootsector));

    FILE *fp = fopen("test.mbr", "wb");
    savef(fp, dna, &fat); // 
    fclose(fp);

    memset(&fat, 0, sizeof(bootsector));

    fp = fopen("test.mbr", "rb");
    loadf(fp, dna, &fat);
    fclose(fp);

    hexdump(&fat, sizeof(bootsector));
}
#endif
