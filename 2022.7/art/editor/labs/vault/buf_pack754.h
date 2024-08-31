// float754.c packing ----------------------------------------------------------
// [1] http://www.mrob.com/pub/math/floatformats.html#minifloat
// [2] microfloat: [0.002 to 240] range.
// [3] half float: can approximate any 16-bit unsigned integer or its reciprocal to 3 decimal places.
// - rlyeh, public domain

#pragma once
#include <math.h>
#include <stdint.h>

#define pack754_8(f)    (  pack754((f),  8,  4)) //@r-lyeh, add microfloat [1][3]
#define pack754_16(f)   (  pack754((f), 16,  5)) //@r-lyeh, add half [1][4]
#define pack754_32(f)   (  pack754((f), 32,  8))
#define pack754_64(f)   (  pack754((f), 64, 11))
#define unpack754_8(u)  (unpack754((u),  8,  4)) //@r-lyeh, add microfloat
#define unpack754_16(u) (unpack754((u), 16,  5)) //@r-lyeh, add half 
#define unpack754_32(u) (unpack754((u), 32,  8))
#define unpack754_64(u) (unpack754((u), 64, 11))

#if 1
// [src] http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization
// Modified to encode NaN and Infinity as well.

#pragma once
#include <stdint.h>
#include <math.h>

static
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way
//@r{ beware! works for 32/64 only
    else if (f ==  INFINITY) return 0x7f800000ULL << (bits - 32); // 0111 1111 1000
    else if (f == -INFINITY) return 0xff800000ULL << (bits - 32);
    else if (f != f)         return 0x7fc00000ULL << (bits - 32); // 0111 1111 1100 NaN
//@r}

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

static
long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;
//@r{ beware! works for 32 only
    else if (i == 0x7fc00000ULL) return  NAN;      //  NaN
    else if (i == 0x7f800000ULL) return  INFINITY; // +Inf
    else if (i == 0xff800000ULL) return -INFINITY; // -Inf
//@r}

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
#else
static uint64_t pack754(long double f, unsigned bits, unsigned expbits) {
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

//@r-lyeh{
    #define FLOAT_NINF (((0xFFFFffffFFFFffffULL) >> significandbits) << significandbits)
    #define FLOAT_PINF (((0x7FFFffffFFFFffffULL) >> significandbits) << significandbits)
    #define FLOAT_NANF (FLOAT_PINF | (1ULL << (significandbits-1)))
    #define FLOAT_NEG0 (1ULL << (bits-1))

    /**/ if(f != f)     return FLOAT_NANF;
    else if(f == 0.0f)  return signbit(f) ? FLOAT_NEG0 : 0;
    else if(f/f != f/f) return f > 0 ? FLOAT_PINF : FLOAT_NINF;
//@r-lyeh}

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

static long double unpack754(uint64_t i, unsigned bits, unsigned expbits) {
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

//@r-lyeh{
    /**/ if(i == 0)          return 0.0;
    else if(i == FLOAT_NEG0) return -0.0;
    else if(i == FLOAT_PINF) return INFINITY;
    else if(i == FLOAT_NINF) return -INFINITY;
    else if(i == FLOAT_NANF) return NAN;
//@r-lyeh}

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
#endif

typedef int static_assert_flt[ sizeof(float) == 4 ];
typedef int static_assert_dbl[ sizeof(double) == 8 ];

