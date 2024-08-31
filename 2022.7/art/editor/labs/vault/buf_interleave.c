// array de/interleaving
// - rlyeh, public domain.
//
// results:
// R0G0B0   R1G1B1   R2G2B2...   -> R0R1R2... B0B1B2... G0G1G2...
// R0G0B0A0 R1G1B1A1 R2G2B2A2... -> R0R1R2... A0A1A2... B0B1B2... G0G1G2...

#ifndef INTERLEAVE_H
#define INTERLEAVE_H

void *interleave( void *out, const void *list, int list_count, int sizeof_item, unsigned columns );

#endif

// -----------------------------------------------------------------------------

#ifdef INTERLEAVE_C
#pragma once
#include <assert.h>
#include <string.h>

void *interleave( void *out, const void *list, int list_count, int sizeof_item, unsigned columns ) {
    void *bak = out;
    assert( columns < list_count ); // required
    int row_count = list_count / columns;
    for( int offset = 0; offset < columns; offset++ ) {
        for( int row = 0; row < row_count; row++ ) {
            memcpy( out, &((char*)list)[ (offset + row * columns) * sizeof_item ], sizeof_item );
            out = ((char*)out) + sizeof_item;
        }
    }
    return bak;
}

#ifdef INTERLEAVE_DEMO
#include <stdio.h>
void test( const char *name, int interleaving, int deinterleaving, const char *original ) {
    char interleaved[128] = {0};
    interleave( interleaved, original, strlen(original)/2, 2, interleaving );
    char deinterleaved[128] = {0};
    interleave( deinterleaved, interleaved, strlen(original)/2, 2, deinterleaving );

    printf( "\n%s\n", name );
    printf( "original:\t%s\n", original );
    printf( "interleaved:\t%s\n", interleaved );
    printf( "deinterleaved:\t%s\n", deinterleaved );

    assert( 0 == strcmp(original, deinterleaved) );
}
int main() {
    test("audio 2ch", 2, 3,
        "L0R0"
        "L1R1"
        "L2R2"
    );
    test("image 3ch", 3, 3,
        "R0G0B0"
        "R1G1B1"
        "R2G2B2"
    );
    test("image 4ch", 4, 3,
        "R0G0B0A0"
        "R1G1B1A1"
        "R2G2B2A2"
    );
    test("audio 5ch", 5, 3,
        "A0B0C0L0R0"
        "A1B1C1L1R1"
        "A2B2C2L2R2"
    );
    test("audio 5.1ch", 6, 3,
        "A0B0C0L0R0S0"
        "A1B1C1L1R1S1"
        "A2B2C2L2R2S2"
    );
    test("opengl material 9ch", 9, 3,
        "X0Y0Z0q0w0e0r0u0v0"
        "X1Y1Z1q1w1e1r1u1v1"
        "X2Y2Z2q2w2e2r2u2v2"
    );
    test("opengl material 10ch", 10, 3,
        "X0Y0Z0q0w0e0r0s0u0v0"
        "X1Y1Z1q1w1e1r1s1u1v1"
        "X2Y2Z2q2w2e2r2s2u2v2"
    );
    assert(~puts("Ok"));
}
#endif // INTERLEAVE_DEMO
#endif // INTERLEAVE_C
