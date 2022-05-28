// fragment data
// - rlyeh, public domain
//
// ## fragment format
// [fragnum:16][fraglen:16][crc:32][protocol-id(*):32][fragment-data:XX][eof:8]
//   fragment-number: 000,001,002...
//   fragment-length: end of stream if length < 65535
//   crc: crc32 made of protocol-id+fragment-data
//   protocol-id: used to calc crc of fragment-data only; never written or sent (*)
//   fragment-data: user data; likely crypted
//   end-of-fragment: least byte (lsb) from crc32. must match.

#ifndef FRAGMENT_H
#define FRAGMENT_H

int   fragment_num(int msglen); // number of total fragments required for a full split
char* fragment_split(int fragment_seq, const void *msg, int msglen); // split one frag
int   fragment_sizeof(int fragment_seq, int msglen); // size of specific fragment

int   fragment_verify(array(char*) fragments);
char* fragment_join(array(char*) fragments);

#endif

#ifdef FRAGMENT_C
#pragma once

#define FRAGMENT_VERSION 0x595c7b10 // crc32("0.0.1")
#define FRAGMENT_MAXSIZE 65536      // 4 or 8 for testing
//#define FRAGMENT_PREALLOCATE         // @todo

#pragma pack(push, 1)
typedef struct fragment_header {
    uint16_t fragment_seq;
    uint16_t fragment_len;
    uint32_t crc;
    char     fragment_bin[];
} fragment_header;
#pragma pack(pop)

int fragment_num(int msglen) {
    return 1 + (msglen / FRAGMENT_MAXSIZE);
}
int fragment_sizeof(int fragment_seq, int msglen) {
    int chunklen = fragment_seq == (fragment_num(msglen)-1) ? (msglen % FRAGMENT_MAXSIZE) : FRAGMENT_MAXSIZE;
    return 8 + chunklen + 1;
}
char* fragment_split(int fragment_seq, const void *msg, int msglen) {
    uint16_t fragment_len = (uint16_t)(fragment_sizeof(fragment_seq, msglen) - 8 - 1);

    // relocate pointer in msg
    msg = ((const char*)msg) + fragment_seq * FRAGMENT_MAXSIZE;
    uint32_t crc = crc32(FRAGMENT_VERSION, msg, fragment_len);

    // prepare header
    fragment_header header = { (uint16_t)fragment_seq, fragment_len, crc };

    // store header + msg
    char *fragment_buf = va("%*.s", 8 + fragment_len + 1, ""); // FRAGMENT_PREALLOCATE
    memcpy(fragment_buf, &header, sizeof(struct fragment_header));
    memcpy(fragment_buf + 8, msg, fragment_len);
    fragment_buf[8 + fragment_len] = header.crc & 0xFF;

    return fragment_buf;
}

// returns +N if fragment Nth has checksum error (@todo)
// returns  0 if ok
// returns -N if fragment Nth is missing (@todo)
int fragment_verify(array(char*) fragments) {
    // 1. check that all fragments are present, and no dupes are found.
    // 2. check that fragments are not truncated
    // 3. check that fragments pass crc
    int errcode = 0, quotient = -1;
    int num_fragments = array_count(fragments);
    char *present = CALLOC(1, num_fragments);
    for( int i = 0; i < num_fragments; ++i ) {
        fragment_header *ph = (fragment_header*)fragments[i];
        if( ph->fragment_seq >= num_fragments ) { FREE(present); return -1; }; // incomplete
        if( present[ph->fragment_seq] ) { FREE(present); return -1; }; // dupe
        if( ph->fragment_bin[-4] != ph->fragment_bin[ph->fragment_len] ) { FREE(present); return -1; }; // truncated. @todo: endian (-1)
        if( ph->crc != crc32(FRAGMENT_VERSION, ph->fragment_bin, ph->fragment_len) ) { FREE(present); return -1; }; // crc mismatch
        if( ph->fragment_len < FRAGMENT_MAXSIZE ) if( quotient < 0 ) quotient = ph->fragment_len; else { FREE(present); return -1; } // dupe eof

        present[ph->fragment_seq] = 1; // pass
    }
    FREE(present);
    return 0;
}

char* fragment_join(array(char*) fragments) {
    if( fragment_verify(fragments) != 0 ) return 0;

    static __thread char *ptr[16] = {0};
    static __thread int slot = 0;
    slot = (slot + 1) % 16;
    int num_fragments = array_count(fragments);
    ptr[slot] = REALLOC(ptr[slot], num_fragments * FRAGMENT_MAXSIZE); // worst case
    for( int i = 0; i < num_fragments; ++i ) {
        fragment_header *ph = (fragment_header*)fragments[i];
        size_t offset = ph->fragment_seq * FRAGMENT_MAXSIZE;
        memcpy( ptr[slot] + offset, ph->fragment_bin, ph->fragment_len );
    }
    return ptr[slot];
}

#ifdef FRAGMENT_DEMO
int main() {
    const char *msg = "hello world";
    int msglen = 12;

    array(char*) fragments = 0;
    for( int i = 0; i < fragment_num(msglen); ++i ) {
        char *fragment = fragment_split(i, msg, msglen);
        int fraglen = fragment_sizeof(i, msglen);
        hexdump(fragment, fraglen);

        array_push(fragments, fragment);
    }

    assert(fragment_verify(fragments) == 0);
    puts(fragment_join(fragments));
    assert(~puts("Ok"));
}
#endif // FRAGMENT_DEMO
#endif // FRAGMENT_C
