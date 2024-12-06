// stream: read/write/sync, stats, sim
// implementations: mem/list(str), file/dir, tcp/udp, stdin/stdout, sql/kissdb
// - rlyeh, public domain

#ifndef STREAM_MEM_ENLARGE_FACTOR
#define STREAM_MEM_ENLARGE_FACTOR 1.5 // 2 // 1.5
#endif

// hacky solution to an old problem: hook stdio.h to new tech, so old code gets free upgrades.
// this is only enabled if STDIO2 is defined
#ifdef STDIO2

#define _FILE_OFFSET_BITS 64
#ifdef __cplusplus
#include <cstdio>
#endif
#include <stdio.h>

#ifndef STDIO2_H
#define STDIO2_H
typedef FILE STREAM_FILE;
#define FILE                 STREAM
#define fopen(uri,...)       stream_open((uri),__VA_ARGS__)
#define fread(ptr,c,sz,fp)   (stream_read((fp),(ptr),(c)*(sz)) == (c)*(sz) ? (sz) : 0)
#define fwrite(ptr,c,sz,fp)  (stream_puts((fp),(ptr),(c)*(sz)) == (c)*(sz) ? (sz) : 0)
#define fclose(fp)           stream_shut((fp))
#define ftell(fp)            stream_tell((fp))
#define fputs(s,fp)          stream_puts((fp),(s),strlen(s)) // @todo: optimize
#define fprintf              #error:not_supported // stream_puts((fp),va(__VA_ARGS__),strlen(va(__VA_ARGS__))) // @todo: optimize
#define fseek(fp,p,m)        stream_seek((fp),(p),(m))
#define fflush(fp)           stream_sync((fp))
#define fgets                #error:not_supported
#define fgetc                #error:not_supported
#define fputc                #error:not_supported
#define fscanf               #error:not_supported
#define feof                 #error:not_supported
#define ferror               #error:not_supported
#define fgetpos              #error:not_supported
#define fsetpos              #error:not_supported
#define freopen              #error:not_supported
#define funopen              #error:not_supported
#define fopen_s(pfp,n,m)     ((*pfp) = fopen((n),(m)), -!(*pfp))
#define fseeko               fseek
#define _fseeki64            fseek
#define ftello               ftell
#define _ftelli64            ftell
#endif
#endif


#ifndef STREAM_H
#define STREAM_H
#include <stdarg.h>

// stream is a virtual interface that reads and writes bytes, with seeking support.
// could have used funopen() instead, but it is not available everywhere.
// ---
// [x] transports: null://, file://, mem://.
// [ ] extra transports: udp://, tcp://, http://, ftp://, ipc://, rpc://
// [x] pipes: chain of transforms to apply after reading/before writing.
// [ ] transactions: txbegin(id) + content + txend(id)
// [x] linked streams -[hub]< (multicast write)
// [ ] linked streams: >[sink]- >[multi]<) (multicast read, multicast read/write)

typedef struct STREAM STREAM;

// basic usage
//int   stream_stat(const char *spec); // check for presence
STREAM* stream_open(const char *spec, ...);
int     stream_read(STREAM *s, void *ptr, int sz);
int     stream_puts(STREAM *s, const void *ptr, int sz);
int     stream_seek(STREAM *s, int offs, int mode);
int     stream_tell(STREAM *s);
int     stream_sync(STREAM *s);
int     stream_shut(STREAM *s);
int     stream_link(STREAM *s, STREAM *other);
int     stream_pipe(STREAM *s, int(*pipe)(void *ptr, int sz) );

char   *stream_info(STREAM *s);

// create and register new stream protocol
int     stream_make(
        const char *spec,
        int (*open)(STREAM *self, const char *spec, va_list vl),
        int (*read)(STREAM *self, void *ptr, int sz),
        int (*puts)(STREAM *self, const void *ptr, int sz),
        int (*seek)(STREAM *self, int offs, int mode),
        int (*tell)(STREAM *self),
        int (*sync)(STREAM *self),
        int (*shut)(STREAM *self)
);

#endif

// -----------------------------------------------------------------------------

#ifdef STREAM_C
#pragma once

// null:// interface ----------------------------------------------------------

static int nil_open(STREAM *s, const char *spec, va_list vl) { return 0; }
static int nil_read(STREAM *s, void *ptr, int sz) { return sz; }
static int nil_write(STREAM *s, const void *ptr, int sz) { return sz; }
static int nil_seek(STREAM *s, int offs, int mode) { return 0; }
static int nil_tell(STREAM *s) { return 0; }
static int nil_sync(STREAM *s) { return 0; }
static int nil_close(STREAM *s) { return 0; }

// file:// interface ----------------------------------------------------------

typedef struct file {
#ifdef STDIO2
#undef FILE
#endif
    FILE *fp;
#ifdef STDIO2
#define FILE STREAM
#endif
} file;

static int file_open(STREAM *s, const char *spec, va_list vl) {
    return (((file*)s)->fp = (fopen)(spec, va_arg(vl, const char *))) ? 0 : -1;
}

static int file_read_(STREAM *s, void *ptr, int sz) {
    return (fread)(ptr, 1, sz, ((file*)s)->fp);
}

static int file_write_(STREAM *s, const void *ptr, int sz) {
    return (fwrite)(ptr, 1, sz, ((file*)s)->fp);
}

static int file_seek(STREAM *s, int offs, int mode) {
    return (fseek)(((file*)s)->fp, offs, mode);
}

static int file_tell(STREAM *s) {
    return (int)(ftell)(((file*)s)->fp);
}

static int file_sync(STREAM *s) {
    return (fflush)(((file*)s)->fp), 0;
}

static int file_close(STREAM *s) {
    return (fclose)(((file*)s)->fp), 0;
}
#if 1 // not used
static int file_access( const char *filename ) {
#ifdef _WIN
    return _access( filename, 0 ) != -1 ? 0 : -1;
#else
    return access( filename, F_OK ) != -1 ? 0 : -1;
#endif
}
#endif

// mem:// interface -----------------------------------------------------------

// @works: wb, rb
// @todo: w+b, r+b, a+b, ab

typedef struct membuffer {
    char *begin, *cur, *end, *bottom;
    int owned; // true if using own allocated mem; false if using external mem
    char *filename;
    int mode;
} membuffer;

typedef struct ramdisk {
    char *filename;
    char *begin, *end;
    struct ramdisk *next;
} ramdisk;
ramdisk *top = 0;
ramdisk *ramnew(void) {
    if( !top ) return top = CALLOC(1,sizeof(ramdisk));
    while(top->next) top = top->next;
    return top->next = CALLOC(1,sizeof(ramdisk));
}
ramdisk *ramfind(const char *filename) {
    for(ramdisk *t = top; t; t = t->next) {
        if(!strcmp(t->filename, filename)) return t;
    }
    return 0;
}

static int mem_open(STREAM *s, const char *spec, va_list vl) {
#if 0
    int external = 1;
    char* begin = (char*)va_arg(vl, void*);
    char* end = (char*)va_arg(vl, void*);
#else
    int external = 0;
    char* begin = MALLOC(1), *end = begin+1;
    const char *mode = va_arg(vl, const char *);

    if( mode[0] == 'r' ) {
        ramdisk *found = ramfind(spec);
        if( found ) {
            FREE(begin);
            begin = found->begin;
            end = found->end;
        }
    }
#endif

    membuffer *mb = (membuffer*)s;
    mb->owned = !external; // !begin;
    mb->begin = begin;
    mb->cur = mb->begin;
    mb->end = end;
    mb->bottom = mb->end;
    mb->filename = STRDUP(spec);
    mb->mode = mode[0];

    return 0;
}

static int mem_read(STREAM *s, void *ptr, int sz) {
    membuffer *mb = (membuffer*)s;
    int avail = mb->end - mb->cur;
    if( avail < sz ) {
        sz = avail;
    }
    memcpy(ptr, mb->cur, sz);
    mb->cur += sz;
    return sz;
}

static unsigned round_next_pow2(unsigned v) {
    // compute the next highest power of 2 of 32-bit v
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static int mem_write(STREAM *s, const void *ptr, int sz) {
    membuffer *mb = (membuffer*)s;
    int avail = mb->bottom - mb->cur;
    if( mb->owned && sz >= avail ) { /* used to be > before adding extra zero comment below */ 
        int pointer = mb->cur - mb->begin;
        int oldlen = (mb->end - mb->begin);
        int oldcap = (mb->bottom - mb->begin);
        int newcap = (oldcap + sz) * STREAM_MEM_ENLARGE_FACTOR;
        // newcap = round_next_pow2(newcap);
        mb->begin = REALLOC(mb->begin, newcap);
        mb->cur = mb->begin + pointer;
        mb->end = mb->begin + oldcap;
        mb->bottom = mb->begin + newcap;
    }
    /* <-- check below removed because it does not work with streams of unknown size (mb->avail == 0)
    if( mb->avail < sz ) {
        sz = mb->avail;
    }
    */
    memcpy(mb->cur, ptr, sz);
    mb->cur += sz;
    if(mb->cur > mb->end) mb->end = mb->cur + 1;
    return sz;
}

static int mem_seek(STREAM *s, int offs, int mode) {
    membuffer *mb = (membuffer*)s;
    char *dst;
    /**/ if( mode == SEEK_SET ) dst = mb->begin + offs;
    else if( mode == SEEK_CUR ) dst = mb->cur + offs;
    else if( mode == SEEK_END ) dst = mb->end - 1 + offs;
    else return -1;
    if( dst >= mb->begin && dst < mb->end) {
        mb->cur = dst;
        return 0;
    }
    return -1;
}

static int mem_tell(STREAM *s) {
    membuffer *mb = (membuffer*)s;
    return mb->cur - mb->begin;
}

static int mem_sync(STREAM *s) {
    membuffer *mb = (membuffer*)s;
    return 0;
}

static int mem_close(STREAM *s) {
    membuffer *mb = (membuffer*)s;
#if 0
    if( mb->owned ) REALLOC( mb->begin, 0 );
#else
    if( mb->mode == 'w' ) {
        ramdisk *found = ramfind(mb->filename);
        if( !found ) found = ramnew();
        if( found ) {
            FREE(found->begin);
            FREE(found->filename);
            found->filename = STRDUP(mb->filename);
            found->begin = mb->begin;
            found->end = mb->end;
        }
    }
#endif
    return 0;
}

// stream ---------------------------------------------------------------------

typedef struct STREAM {
    // opaque data always first member in struct.
    // implementations can cast directly to their types as long as their sizes are less than sizeof(dummy).
    char dummy[ 256 - sizeof(const char *) - sizeof(int(*)()) * (7+8) - sizeof(STREAM*) - 8*4 ];
    // members
    const char *spec;
    int (*open)(void *self, const char *spec, ...);
    int (*read)(void *self, void *ptr, int sz);
    int (*puts)(void *self, const void *ptr, int sz);
    int (*seek)(void *self, int offs, int mode);
    int (*tell)(void *self);
    int (*sync)(void *self);
    int (*shut)(void *self);
    int (*pipe[8])(void *ptr, int sz);
    STREAM *next;
    uint64_t rd, wr, rdb, wrb; // rd/wr hits, rd/wr bytes
} STREAM;

typedef int sizeof_stream[ sizeof(STREAM) == 256 ];

static STREAM sprotocols[32] = {0};
static int sprotocols_count = 0;

int stream_make(
    const char *spec,
    int (*open)(STREAM *self, const char *spec, va_list),
    int (*read)(STREAM *self, void *ptr, int sz),
    int (*puts)(STREAM *self, const void *ptr, int sz),
    int (*seek)(STREAM *self, int offs, int mode),
    int (*tell)(STREAM *self),
    int (*sync)(STREAM *self),
    int (*shut)(STREAM *self)
) {
    STREAM protocol = { {0}, spec, open, read, puts, seek, tell, sync, shut };
    sprotocols[sprotocols_count++] = protocol;

    // default to file:// (slot #0) if no explicit protocol is provided
    int special_case = !strcmp(protocol.spec, "file://");
    if( special_case ) {
        STREAM swapped = sprotocols[ 0 ];
        sprotocols[ 0 ] = sprotocols[ sprotocols_count - 1 ];
        sprotocols[ sprotocols_count - 1 ] = swapped;
    }

    return 0;
}

STREAM *stream_open(const char *spec, ...) {
    static int registered = 0;
    if( !registered ) { // auto-register provided interfaces
        registered = 1;
        stream_make( "file://", file_open, file_read_, file_write_, file_seek, file_tell, file_sync, file_close );
        stream_make( "mem://", mem_open, mem_read, mem_write, mem_seek, mem_tell, mem_sync, mem_close );
        stream_make( "null://", nil_open, nil_read, nil_write, nil_seek, nil_tell, nil_sync, nil_close );
    }
    STREAM *self = (STREAM*)REALLOC(0, sizeof(STREAM));
    *self = sprotocols[0]; // default file:// interface
    if( strstr(spec, "://") ) {
        for( int i = 0; i < sprotocols_count; ++i ) {
            if( strstr(spec, sprotocols[i].spec) ) {
                *self = sprotocols[i];
                spec += strlen(sprotocols[i].spec);
                break;
            }
        }
    }

    va_list vl;
    va_start(vl, spec);
    if( 0 != self->open( self, spec, vl ) ) {
        REALLOC(self, 0);
        self = 0;
    }
    va_end(vl);

    return self;
}

int stream_pipe(STREAM *self, int(*pipe)(void*,int)) {
    int it = 0; 
    for(;it <= 8 && self->pipe[it]; ++it);
    return it == 8 ? -1 : (self->pipe[it] = pipe, 0);
}

static
int read_loop(STREAM *self, void *buffer, int count) {
    int offset = 0;
    while( count > 0 ) {
        int block = self->read(self, (char *)buffer + offset, count);

        if( block <= 0 ) { // < 0 for blocking sockets!
            return block;
        }
        if( block ) {
            offset += block;
            count -= block;

            ++self->rd;
            self->rdb += block;
        }
    }
    return offset;
}

static
int write_loop(STREAM *self, const void *buffer, int count) {
    int offset = 0;
    while( count > 0 ) {
        int block = self->puts(self, (const char *)buffer + offset, count);

        if (block <= 0) { // < 0 for blocking sockets!
            return block;
        }
        if( block ) {
            offset += block;
            count -= block;

            ++self->wr;
            self->wrb += block;
        }
    }
    return offset;
}

int stream_read(STREAM *self, void *ptr, int sz) {
    int rc = 0;
    while( self ) {
        rc = read_loop(self, ptr, sz);
        for(int it=0;it<8 && rc>=0;++it) rc = self->pipe[it] ? self->pipe[it]((void*)ptr, sz) : rc;
        if( rc < 0 ) break;

        self = self->next;
    }
    return rc;
}

int stream_puts(STREAM *self, const void *ptr, int sz) {
    int rc = 0;
    while( self ) {
        for(int it=0;it<8 && rc>=0;++it) rc = self->pipe[it] ? self->pipe[it]((void*)ptr, sz) : rc;
        rc = write_loop(self, ptr, sz);
        if( rc < 0 ) break;

        self = self->next;
    }
    return rc;
}

int stream_seek(STREAM *self, int offs, int mode) {
    int errors = 0;
    while( self ) {
        STREAM *next = self->next;
        errors |= !!self->seek(self, offs, mode);
        self = next;
    }
    return -errors;
}

int stream_tell(STREAM *self) {
    int minimum = INT_MAX;
    while( self ) {
        STREAM *next = self->next;
        int at = self->tell(self);
        if( at < minimum ) minimum = at;
        self = next;
    }
    return minimum;
}

int stream_sync(STREAM *self) {
    while( self ) {
        STREAM *next = self->next;
        self->sync(self);
        self = next;
    }
    return 0;
}

int stream_shut(STREAM *self) {
    while( self ) {
        STREAM *next = self->next;
        self->sync(self);
        self->shut(self);
        free(self);
        self = next;
    }
    return 0;
}

int stream_link(STREAM *self, STREAM *other) {
    if( !self->next ) return !!(self->next = other);
    return stream_link(self->next, other);
}

char* stream_info(STREAM *self) {
    static __declspec(thread) char buf[128];
    snprintf(buf, 128, "[STREAM:%p]: puts/pull %llu/%llu bytes (%llu/%llu ops)", self, self->wrb, self->rdb, self->wr, self->rd);
    return buf;
}

#endif
