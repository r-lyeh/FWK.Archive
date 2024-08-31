// ARC4 de/cryptor. Based on code by Mike Shaffer.
// - rlyeh, public domain.

void *arc4( void *buffer, unsigned buflen, const void *pass, unsigned passlen );

#ifdef ARC4_C
#pragma once
#include <assert.h>

void *arc4( void *buf_, unsigned buflen, const void *pass_, unsigned passlen ) {
    // [ref] http://www.4guysfromrolla.com/webtech/code/rc4.inc.html
    assert(passlen);
    int sbox[256], key[256];
    char *buf = (char*)buf_;
    const char *pass = (const char*)pass_;
    for( unsigned a = 0; a < 256; a++ ) {
        key[a] = pass[a % passlen];
        sbox[a] = a;
    }
    for( unsigned a = 0, b = 0; a < 256; a++ ) {
        b = (b + sbox[a] + key[a]) % 256;
        int swap = sbox[a]; sbox[a] = sbox[b]; sbox[b] = swap;
    }
    for( unsigned a = 0, b = 0, i = 0; i < buflen; ++i ) {
        a = (a + 1) % 256;
        b = (b + sbox[a]) % 256;
        int swap = sbox[a]; sbox[a] = sbox[b]; sbox[b] = swap;
        buf[i] ^= sbox[(sbox[a] + sbox[b]) % 256];
    }
    return buf_;
}

#ifdef ARC4_DEMO
#include <stdio.h>
#include <string.h>
int main( int argc, char **argv ) {
    char buffer[] = "Hello world."; int buflen = strlen(buffer);
    char *password = argc > 1 ? argv[1] : "abc123"; int passlen = strlen(password);

    printf("Original: %s\n", buffer);
    printf("Password: %s\n", password);

    char *encrypted = arc4( buffer, buflen, password, passlen );
    printf("ARC4 Encrypted text: '%s'\n", encrypted);

    char *decrypted = arc4( buffer, buflen, password, passlen );
    printf("ARC4 Decrypted text: '%s'\n", decrypted);
}
#endif // ARC4_DEMO
#endif // ARC4_C
