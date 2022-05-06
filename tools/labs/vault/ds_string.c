// string library
// - rlyeh, public domain

#ifndef STRING_H
#define STRING_H

// temporary strings api (stack)
#define strtmp(fmt, ...)  va(fmt, __VA_ARGS__)

// allocated strings api (heap)
#define strnew(fmt, ...)  STRDUP(va(fmt,__VA_ARGS__))
#define strdel(s)         ((is_va(s) ? (void)0 : REALLOC((s), 0)), (s)=0)
#define stradd(s,fmt,...) stradd((s), va(fmt, __VA_ARGS__))

// string utils (beware! destructive operations mostly)
char*           strlower(char *s);
char*           strupper(char *s);
char*           strcamel(char *s);
char*           strtrim(char *s);
char*           strswap(char *s, const char *src, const char *dst);
int             strmatch(const char *s, const char *wildcard);
uint64_t        strhash(const char *s);
array(uint32_t) strutf8(const char *utf8);
const char*     strrstr(const char *s1, const char *s2);
char*           strmap(char *inout, const char *src, const char *dst);
int             strhead( const char *string, const char *substr );
int             strtail( const char *s, const char *e );
char*           strflip(char *s);
const char*     strsub( const char *str, int pos );
int             streq( const char *string, const char *substr );
int             streqi( const char *string, const char *substr );
const char*     strskip(const char *s, const char *chars);
const char*     strfind(const char *s, const char *chars);
char*           (stradd)(char **s, const char *s2);

// tokenizer utils
char**          strsplit(const char *string, const char *delimiters); // adds additional eos
char*           strjoin(int num, char **list, const char *separator); // if num<=0, check for eos
int             strchop(const char *string, const char *substr, char **left, char **right);

// tokenizer utils (array version; without trailing null string)
array(char*)    strsplit2(const char *string, const char *delimiters);
char*           strjoin2(array(char*) list, const char *separator);

// aliases
typedef char*   string;

#endif

// -----------------------------------------------------------------------------

#ifdef STRING_C
#pragma once

#if defined _MSC_VER && !defined __thread
#define __thread __declspec(thread)
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// tokenizer -------------------------------------------------------------------

array(char*) strsplit(const char *text_, const char *delimiters) {
    static __thread char *mems[16] = {0};
    static __thread array(char*) list[16] = {0};
    static __thread int list_count = 0;

    int len = strlen(text_);

    int slot = (list_count = (list_count+1) % 16);
    mems[slot] = REALLOC(mems[slot], len+1);

    char *text = mems[slot];
    memcpy(text, text_, len+1);

    array(char *) *out = &list[slot];
    array_clear(*out);

    int found[256] = {1,0}, i = 0;
    while( *delimiters ) found[(unsigned char)*delimiters++] = 1;
    while( text[i] ) {
        int begin = i; while(text[i] && !found[(unsigned char)text[i]]) ++i;
        int end = i;   while(text[i] &&  found[(unsigned char)text[i]]) ++i;
        if (end > begin) {
            array_push(*out, (text + begin));
            (text + begin)[ end - begin ] = 0;
        }
    }
    array_push(*out, 0); //(end of array)
    return *out;
}
char* strjoin(int num_list, char **list, const char *separator) {
#if 1
    static __thread char* mems[16] = {0};
    static __thread int num = 0;
    int slot = (num = (num+1) % 16);

    int len = 0, inc = 0, seplen = strlen(separator);
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        len += strlen(list[i]) + inc;
        inc = seplen;
    }

    mems[slot] = REALLOC(mems[slot], len+1);
    char *p = mems[slot]; *p = 0;
    const char *sep = "";
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        p += sprintf(p, "%s%s", sep, list[i]);
        sep = separator;
    }

    return mems[slot];
#else
    char *x = 0, *sep = "";
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        stradd(&x, "%s%s", sep, list[i]);
        sep = separator;
    }
    char *ret = va("%s", x);
    FREE(x);
    return ret;
#endif
}

#include <string.h>
array(char*) strsplit2(const char *str, const char *separator) { // @todo: replaces strsplit?
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    array_resize(list[slot], 0);
    buf[slot] = REALLOC(buf[slot], strlen(str)+1);

    for(char *dst = buf[slot]; str && *str; ) {
        // find && skip separator
       const char *sep = strpbrk(str, separator);
       const char *src = str; int slen = (int)(sep - src);
       str = sep + (sep ? strspn(sep, separator) : 0);

       // append & update content
       array_push(list[slot], dst);
       memcpy((dst += slen) - slen, src, slen);
       *dst++ = '\0';
    }

    return list[slot];
}
char *strjoin2(array(char*) list, const char *separator) {
    return strjoin( array_count(list), list, separator );
}

// string utils ----------------------------------------------------------------

const char *strsub( const char *str, int pos ) {
    int size = strlen(str);
    pos = pos && size ? (pos > 0 ? pos % size : size-1 + ((pos+1) % size)) : 0;
    return str + pos;
}
int streq( const char *string, const char *substr ) {
    return strcmp( string, substr );
}
int streqi( const char *string, const char *substr ) {
    while( *string && *substr ) {
        int eqi = (*string++ | 32) - (*substr++ | 32);
        if( eqi ) return eqi;
    }
    return *string - *substr;
}
char *strflip(char *str) { // Based on code by Bob Stout (public domain).
    if(str && *str)
    for( char *p1 = str, *p2 = p1 + strlen(p1) - 1; p2 > p1; ++p1, --p2 ) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}
// skip all occurences of 'chars' in s
const char *strskip(const char *s, const char *chars) {
    return s + strspn(s, chars);
}
// find any of 'chars' in s. null if no matches are found.
const char *strfind(const char *s, const char *chars) {
    return strpbrk(s, chars);
}
// returns true if text starts with substring
int strhead( const char *str, const char *substr ) {
    return strncmp(str, substr, strlen(substr)) == 0;
}
// returns true if text ends with substring
int strtail( const char *s, const char *e ) {
    int ls = strlen(s);
    int le = strlen(e);
    if( ls < le ) return 0;
    return 0 == memcmp( s + ls - le, e, le );
}
static char *strmap_(char *inout, const char *src, const char *dst) {
    char *find = strpbrk(inout, src);
    if(!find) return inout;
    char *which = strchr(src, find[0]);
    int distance = (int)( which - src );
    int len = strlen(dst);
    if( distance < len ) {
        *find = dst[distance];
        return strmap_(find+1, src, dst); // continue mapping
    } else {
        char *ptr = find;
        do ptr[0] = ptr[1]; while( *++ptr ); // memmove one
        return strmap_(find, src, dst);
    }
}
char *strmap(char *inout, const char *src, const char *dst) {
    // assert(strlen(src) >= strlen(dst));
    if(strlen(src) == strlen(dst)) {
        uint8_t map[256] =
        "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
        "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
        "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
        "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
        "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"
        "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f"
        "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
        "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f"
        "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
        "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
        "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
        "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
        "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
        "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
        "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
        "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
        for( int i = 0; src[i]; ++i) map[(uint8_t)src[i]] = dst[i];
        for( int i = 0; inout[i]; ++i ) inout[i] = map[(uint8_t)inout[i]];
        return inout;
    }
    return strmap_(inout, src, dst), inout;
}
const char *strrstr(const char *s1, const char *s2) {
    int s1len = strlen(s1);
    char *find = strstr(s1, s2), *find1;
    if( !find ) return s1 + s1len;
    while( (find1 = strstr(find+s1len,s2)) != 0 ) find = find1;
    return find;
}
char *strlower( char *copy ) {
    for( char *s = copy; *s; ++s ) *s = tolower(*s); // &= ~32
    return copy;
}
char *strupper( char *copy ) {
    for( char *s = copy; *s; ++s ) *s = toupper(*s); // |= 32
    return copy;
}
char *strcamel( char *copy ) {
    for( char *s = copy; *s; ++s ) *s = tolower(*s); // &= ~32
    char **list = strsplit(copy, " ");
    for( int i = 0; list[i]; ++i ) list[i][0] = toupper(list[i][0]);
    return strcpy(copy, strjoin(0, list, ""));
}
char* strtrim(char *str) {
    // trims leading, trailing and excess whitespaces.
    char *ibuf, *obuf;
    if( str ) {
        for( ibuf = obuf = str; *ibuf; ) {
            while( *ibuf && isspace(*ibuf)  )  (ibuf++);
            if(    *ibuf && obuf != str     ) *(obuf++) = ' ';
            while( *ibuf && !isspace(*ibuf) ) *(obuf++) = *(ibuf++);
        }
        *obuf = 0;
    }
    return str;
}
char *strswap( char *copy, const char *target, const char *replacement ) {
    // replaced only if new text is shorter than old one
    int rlen = strlen(replacement), diff = strlen(target) - rlen;
    if( diff >= 0 ) {
        for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
            if( rlen ) s = (char*)memcpy( s, replacement, rlen ) + rlen;
            if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
        }
    }
    return copy;
}
int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard=='\0' ) return !*s;
    if( *wildcard=='*' )  return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard=='?' )  return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
uint64_t strhash( const char *s ) { // for convenience. wrapper over original hash_str() function
    return hash_str(s);
//  uint64_t hash = 0; // fnv1a: 14695981039346656037ULL;
//  while( *s++ ) hash = ( s[-1] ^ hash ) * 131; // fnv1a: 0x100000001b3ULL; }
//  return hash;
}
array(uint32_t) strutf8( const char *utf8 ) {
    // Based on code by @ddiakopoulos (unlicensed).
    // Based on code by @nothings (public domain).
    array(uint32_t) out = 0; //array_reserve(out, strlen(utf8) + 1);
    while( *utf8 ) {
        const char **p = &utf8;
        uint32_t unicode = 0;
        /**/ if( (**p & 0x80) == 0x00 ) {
            int a = *((*p)++);
            unicode = a;
        }
        else if( (**p & 0xe0) == 0xc0 ) {
            int a = *((*p)++) & 0x1f;
            int b = *((*p)++) & 0x3f;
            unicode = (a << 6) | b;
        }
        else if( (**p & 0xf0) == 0xe0 ) {
            int a = *((*p)++) & 0x0f;
            int b = *((*p)++) & 0x3f;
            int c = *((*p)++) & 0x3f;
            unicode = (a << 12) | (b << 6) | c;
        }
        else if( (**p & 0xf8) == 0xf0 ) {
            int a = *((*p)++) & 0x07;
            int b = *((*p)++) & 0x3f;
            int c = *((*p)++) & 0x3f;
            int d = *((*p)++) & 0x3f;
            unicode = (a << 18) | (b << 12) | (c << 8) | d;
        }
        array_push(out, unicode);
    }
    return out;
}

int strchop(const char *src, const char *substr, char **left, char **right) {
    char *find = strstr(src, substr);
    if( !find ) return *left = va(""), *right = va(""), 0;
    *left = va("%.*s", (int)(find - src), src);
    *right = va("%s", find + strlen(substr));
    return 1;
}

char *(stradd)(char **x, const char *buf) {
    char *z = x && *x ? *x : 0;
    int bl = strlen(buf);
    int zl = (z ? strlen(z) : 0);
    z = REALLOC(z, zl + bl + 1 );
    memcpy(z + zl, buf, bl + 1 );
    if( x && *x ) *x = z;
    return z;
}

// -----------------------------------------------------------------------------

#ifdef STRING_DEMO
#pragma once
int main() {
    // test creation & destruction
    char *x = strtmp("hello %d", 123); puts(x); strdel(x); assert(x == 0); // optional free, since x is a temporary string (strtmp)
    char *y = strnew("hello %d", 123); puts(y); strdel(y); assert(y == 0); // required free, since y was explicitly allocated (with strnew)

    // test concat
    char *z = strtmp("%d",6); z = strtmp("%s%s%s",z,z,z); assert( 0 == strcmp(z,"666") );

    // test memory is never exhausted
    for(int i = 0; i < 10000; ++i) assert(strtmp("hello %d",123));

    // test utils 1/3
    char buf[128];
    puts( strswap( strcpy(buf, "abracadabra"), "bra", "BRA") ); // same len
    puts( strswap( strcpy(buf, "abracadabra"), "bra", "BR") );  // smaller len
    puts( strswap( strcpy(buf, "abracadabra"), "bra", "B") );   // smaller len
    puts( strswap( strcpy(buf, "abracadabra"), "bra", "") );    // erase
    puts( strswap( strcpy(buf, "abracadabra"), "boo", "foo") ); // no matches

    // test utils 2/3
    puts( strcamel("Camel case TEST") );

    // test utils 3/3
    array(char*) tokens = strsplit("JAN,;,FEB,MAR,,APR", ",;");
    puts(strjoin( tokens, "/" ));

    // strrstr
    puts( strrstr("banana",  "an") );
    puts( strrstr("banana",  "ban") );
    puts( strrstr("banana",  "xxx") );

    // strmap
    char banana[] = "BANANA";
    assert(!strcmp("banana", strmap(banana, "BNACXYZ ", "bnac")));
    char stairway[] = "There's a lady who's sure ... ";
    assert(!strcmp("There'saladywho'ssure...", strmap(stairway, " ", "")));
    char remap[] = "h3110 w0r1d";
    assert( 0 == strcmp("hello world", strmap(remap, "310", "elo")));

    // 
    char *left, *right;
    strchop("abracadabra", "ca", &left, &right);  assert(!strcmp("abra",left)); assert(!strcmp("dabra",right));
    strchop("abracadabra", "xxx", &left, &right); assert(!strcmp("",left)); assert(!strcmp("",right));

    char hello2[] = "hello cruel world";
    assert( 0 == strcmp("hello world", strswap(hello2, "cruel ", "")) );
    assert( 0 == strcmp("dlrow olleh", strflip(hello2)) );

    //
    assert( strhead("banana", "ban") );
    assert(!strhead("banana", "xxx") );
    assert( strtail("banana", "ana") );
    assert(!strtail("banana", "xxx") );
    assert( strhead("hello", "hell") );
    assert(!strhead("hell", "hello") );
    assert( strtail("hello", "llo") );

    // 
    assert(!streq("hello", "hello") );
    assert( streq("HELLO", "hello") );
    assert(!streqi("hello", "hello") );
    assert(!streqi("HELLO", "hello") );

    assert(!strcmp(strsub("hello world",6),strsub("hello world", -5))); // world
    assert(!strcmp("hello world", strskip("   \t\rhello world", " \t\r")));
    assert(!strcmp("(world)", strfind("hello(world)", "()")));

    // test asserts are enabled
    assert(~puts("Ok"));
}
#define main main__
#endif // STRING_DEMO
#endif // STRING_C
