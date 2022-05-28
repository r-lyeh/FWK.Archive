// - rlyeh, public domain

void os_entropy( void *buf, unsigned n );


#ifdef ENTROPY_C
#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32")

void os_entropy_( void *buf, unsigned n ) {
    HCRYPTPROV provider;
    if( CryptAcquireContext( &provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) == 0 ) {
        assert(!"CryptAcquireContext failed");
    }

    int rc = CryptGenRandom( provider, n, (BYTE *)buf );
    assert( rc != 0 );
    CryptReleaseContext( provider, 0 );
}

#elif 1
#include <stdio.h>

void os_entropy_( void *buf, unsigned n ) {
    FILE *fp = fopen( "/dev/urandom", "r" );
    if( !fp ) assert(!"/dev/urandom open failed");

    size_t read = fread( buf, 1, n, fp );
    assert( read == n && "/dev/urandom read failed" );
    fclose( fp );
}

#elif 1 // unused for now

// pseudo random number generator with 128 bit internal state... probably not suited for cryptographical usage.
// [src] http://github.com/kokke (UNLICENSE)
// [ref] http://burtleburtle.net/bob/rand/smallprng.html

#include <time.h>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

static uint32_t prng_next(void) {
    #define prng_rotate(x,k) (x << k) | (x >> (32 - k))
    #define prng_shuffle() do { \
    uint32_t e = ctx[0] - prng_rotate(ctx[1], 27); \
    ctx[0] = ctx[1] ^ prng_rotate(ctx[2], 17); \
    ctx[1] = ctx[2] + ctx[3]; \
    ctx[2] = ctx[3] + e; \
    ctx[3] = e + ctx[0]; } while(0)
    static __thread uint32_t ctx[4], *once = 0; if( !once ) {
        uint32_t seed = (uint32_t)( getpid() + time(0) + ((uintptr_t)once) );
        ctx[0] = 0xf1ea5eed;
        ctx[1] = ctx[2] = ctx[3] = seed;
        for (int i = 0; i < 31; ++i) {
            prng_shuffle();
        }
        once = ctx;
    }
    prng_shuffle();
    return ctx[3];
}

void os_entropy_( void *buf, unsigned n ) {
    for( ; n >= 4 ; n -= 4 ) {
        uint32_t a = prng_next();
        memcpy(buf, &a, 4);
        buf = ((char*)buf) + 4;
    }
    if( n > 0 ) {
        uint32_t a = prng_next();
        memcpy(buf, &a, n);
    }
}

#endif

void os_entropy(void *buf, unsigned n) {
    assert(n <= 2048);

#if 0
    os_entropy_(buf, n);
#else
    // acquire twice, ensure 1st and 2nd parts are different
    // would this prevent some kind of dummy hijacking?
    memset(buf, 0, n);
    os_entropy_(buf, n/2);
    os_entropy_((char*)buf + n/2, n/2 + n&1);
    assert(0 != memcmp(buf, (char*)buf + n/2, n/2));
#endif
}



#ifdef ENTROPY_DEMO
int main() {
    unsigned char buf[128];
    os_entropy(buf, 128);
    for( int i = 0; i < 128; ++i ) {
        printf("%02x", buf[i]);
    }
    puts("");
}
#define main __main
#endif // ENTROPY_DEMO
#endif // ENTROPY_C
