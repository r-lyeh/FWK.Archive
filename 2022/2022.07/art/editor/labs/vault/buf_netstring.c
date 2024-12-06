// netstring en/decoder
// - rlyeh, public domain.

unsigned netstring_bounds(unsigned inlen);
unsigned netstring_encode(const char *in, unsigned inlen, char *out, unsigned outlen);
unsigned netstring_decode(const char *in, unsigned inlen, char *out, unsigned outlen);

#ifdef NETSTRING_C
#pragma once
#include <stdio.h>
#include <string.h>

unsigned netstring_bounds(unsigned inlen) {
    return 5 + inlen + 3; // 3 for ;,\0 + 5 if inlen < 100k ; else (unsigned)ceil(log10(inlen + 1))
}
unsigned netstring_encode(const char *in, unsigned inlen, char *out, unsigned outlen) {
//  if(outlen < netstring_bounds(inlen)) return 0;
    sprintf(out, "%u:%.*s,", inlen, inlen, in);
    return strlen(out);
}
unsigned netstring_decode(const char *in, unsigned inlen, char *out, unsigned outlen) {
//  if(outlen < inlen) return 0;
    const char *bak = in;
    sscanf(in, "%u", &outlen);
    while( *++in != ':' );
    memcpy(out, in+1, outlen), out[outlen-1] = 0;
    // return outlen; // number of written bytes
    return (outlen + (in+2 - bak)); // number of read bytes
}

#ifdef NETSTRING_DEMO
#include <stdlib.h>
int main() {
    // encode
    const char text1[] = "hello world!", text2[] = "abc123";
    unsigned buflen = netstring_bounds(strlen(text1) + strlen(text2));
    char *buf = malloc(buflen), *ptr = buf;
    ptr += netstring_encode(text1, strlen(text1), ptr, buflen -= (ptr - buf));
    ptr += netstring_encode(text2, strlen(text2), ptr, buflen -= (ptr - buf));
    printf("%s -> ", buf);

    // decode
    char out[12];
    unsigned plen = strlen(ptr = buf);
    while(plen > 0) {
        int written = netstring_decode(ptr, plen, out, 12);
        ptr += written;
        plen -= written;
        printf("'%s'(%s)(%d), ", out, ptr, plen );
    }
    puts("");
}
#define main main__
#endif // NETSTRING_DEMO
#endif // NETSTRING_C
