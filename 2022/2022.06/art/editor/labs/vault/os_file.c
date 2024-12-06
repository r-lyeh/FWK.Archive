// file functions
// - rlyeh, public domain.
//
// @todo: file_lock, file_unlock

#ifndef FILE_H
#define FILE_H
#include <stdio.h>
#include <stdbool.h>

char*       file_read( const char *pathfile ); // returns temporary data
bool        file_write( const char *pathfile, const void *data, int len );
bool        file_append( const char *pathfile, const void *data, int len );

char*       file_map( const char *pathfile, size_t offset, size_t len );
void        file_unmap( char *buf, size_t len );
char*       file_chunk( const char *pathfile, size_t offset, size_t len ); // returns temporary data

int         file_size( const char *pathfile ); // 4gib limit
int         file_stamp( const char *pathfile ); // Y2038 limit

bool        file_exact( const char *pathfile1, const char *pathfile2 );
bool        file_exist( const char *pathfile );
bool        file_isdir( const char *pathfile );
bool        file_isfile( const char *pathfile );
bool        file_islink( const char *pathfile );

bool        file_copy( const char *pathsrc, const char *pathdst );
bool        file_touch( const char *pathfile, size_t modtime ); // can be 0
bool        file_delete( const char *path );

const char* file_base( const char *file );
char*       file_norm( const char *pathfile ); // returns temporary normalized name

// uint64_t file_checksum( const char *pathfile );

#endif

#ifdef FILE_C
#pragma once
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>  // stat, lstat
#include <sys/types.h> // mode_t
#ifdef _WIN32 // _MSC_VER
#include <sys/utime.h> // utimbuf, also in <utime.h>
#else
#include <utime.h>
#endif
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#include <fcntl.h> // O_RDONLY(00)
// mmap() replacement for Windows. Placed into the public domain (Mike Frysinger)
enum {  PROT_READ = 0x1, PROT_WRITE = 0x2, PROT_EXEC = 0x4,
        MAP_SHARED = 0x01, MAP_PRIVATE = 0x02, MAP_ANON = 0x20, MAP_ANONYMOUS = MAP_ANON };
#define MAP_FAILED    ((void *) -1)
static void* mmap(void* start, size_t length, int prot, int flags, int fd, size_t offset) {
    // Offsets must be a multiple of the system's allocation granularity.  We
    // guarantee this by making our view size equal to the allocation granularity.
    static int32_t page_size = -1; if (page_size < 0) {
        SYSTEM_INFO sysinfo = { 0 };
        GetSystemInfo(&sysinfo);
        page_size = sysinfo.dwAllocationGranularity;
    }

    DWORD flProtect;
    size_t end = // length + offset; // 0;
        length & ~(page_size - 1) + page_size
        + 
        offset & ~(page_size - 1);

    if( !(prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC)) ) {
        if( ( fd == -1 &&  (flags & MAP_ANON) && !offset ) ||
            ( fd != -1 && !(flags & MAP_ANON)            )) {
                 if( prot & PROT_EXEC  ) flProtect = prot & PROT_READ ? PAGE_EXECUTE_READ : PAGE_EXECUTE_READWRITE;
            else if( prot & PROT_WRITE ) flProtect = PAGE_READWRITE;
            else                         flProtect = PAGE_READONLY;
            HANDLE h = CreateFileMapping( fd == -1 ? INVALID_HANDLE_VALUE : (HANDLE)_get_osfhandle(fd),
                NULL, flProtect, (end >> 31) >> 1, (uint32_t)end, NULL);
            if( h != NULL ) {
                DWORD dwDesiredAccess = 0;
                dwDesiredAccess |= prot & PROT_WRITE ? FILE_MAP_WRITE : FILE_MAP_READ;
                dwDesiredAccess |= prot & PROT_EXEC ? FILE_MAP_EXECUTE : 0;
                dwDesiredAccess |= flags & MAP_PRIVATE ? FILE_MAP_COPY : 0;
#if 0
                void *ret = MapViewOfFile(h, dwDesiredAccess, (offset >> 31) >> 1, (uint32_t)offset, length);
#else
                // hack: avoid ERROR_MAPPED_ALIGNMENT 1132 (0x46C) error
                // see https://stackoverflow.com/questions/8583449/memory-map-file-offset-low
                // see https://stackoverflow.com/questions/9889557/mapping-large-files-using-mapviewoffile
           
                // alignment
                size_t offset_page = offset & ~(page_size - 1);
                size_t read_size = length + (offset - offset_page);

                void *ret = MapViewOfFile(h, dwDesiredAccess, (offset_page >> 31) >> 1, (uint32_t) offset_page, read_size); //ASSERT(ret, "win32 error %d", GetLastError());
                ret = (char*)ret + (offset - offset_page);
#endif
                CloseHandle(h); // close the Windows Handle here (we handle the file ourselves with fd)
                return ret == NULL ? MAP_FAILED : ret;
            }
        }
    }
    return MAP_FAILED;
}
static void munmap(void* addr, size_t length) {
    UnmapViewOfFile(addr);
}
#endif

#ifdef _WIN32
#ifdef __TINYC__
    #define CP_UTF8 65001
    int WINAPI MultiByteToWideChar();
#else
    #include <winsock2.h>
#endif
wchar_t *file_widen(const char *utf8) { // wide strings (windows only)
    static __thread wchar_t bufs[4][260];
    static __thread int index = 0;

    int sz = (int)sizeof(bufs[0]);
    wchar_t *buf = bufs[(++index) % 4];

    int needed = 2 * MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    buf[0] = 0;
    if( needed < sz ) {
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (void*)buf, needed);
    }
    return (wchar_t *)buf;
}
#endif

#define open8(path,mode)  IFDEF(WINDOWS, _wopen(file_widen(path))               ,  open(path, mode) )
#define fopen8(path,mode) IFDEF(WINDOWS, _wfopen(file_widen(path),file_widen(mode)), fopen(path,mode) )
#define remove8(path)     IFDEF(WINDOWS, _wremove(file_widen(path))             ,  remove(path)     )
#define rename8(path)     IFDEF(WINDOWS, _wrename(file_widen(path))             ,  rename(path)     )
#define stat8(path,st)    IFDEF(WINDOWS, _wstat(file_widen(path),st)            ,  stat(path,st)    ) // _stati64()
#define stat_             IFDEF(WINDOWS, _stat,                                 ,  stat_t           ) // struct _stati64



char* file_map( const char *pathfile, size_t offset, size_t len ) {
    int file = open(pathfile, O_RDONLY);
    if( file < 0 ) {
        return 0;
    }
    void *ptr = mmap(0, len, PROT_READ, MAP_SHARED/*MAP_PRIVATE*/, file, offset);
    if( ptr == MAP_FAILED ) {
        ptr = 0;
    }
    close(file); // close file. mapping held until unmapped
    return (char *)ptr;
}
void file_unmap( char *buf, size_t len ) {
    munmap( buf, len );
}
char* file_chunk(const char *pathfile, size_t offset, size_t len) {
    static __thread int map = 0;
    static __thread struct maplen { char *map; int len; } maps[16] = {0};
    char *mem = file_map( pathfile, offset, len );
    if( mem ) {
        struct maplen *bl = &maps[ map = ++map & (16-1) ];
        if( bl->map ) file_unmap( bl->map, bl->len );
        bl->map = mem;
        bl->len = len;
    }
    return mem;
}

int file_size( const char *pathfile ) {
#if 0
    FILE *fp = fopen(fname, "rb");
    if(!fp) return 0;
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    fclose(fp);
    return (int)sz;
#else
    struct stat_ st;
    uint64_t s = stat8(pathfile, &st) < 0 ? 0ULL : (uint64_t)st.st_size;
    return (int)s;
#endif
}

int file_stamp( const char *pathfile ) {
    struct stat_ st;
    uint64_t t = stat8(pathfile, &st) < 0 ? 0ULL : (uint64_t)st.st_mtime;
    return (int)t;
}

bool file_exist( const char *pathfile ) {
    struct stat_ st;
    return stat8(pathfile, &st) < 0 ? 0 : 1;
    // _WIN: return _access( filename, 0 ) != -1;
    // else: return access( filename, F_OK ) != -1;
}
bool file_isdir( const char *pathfile ) {
    struct stat_ st;
    return stat8(pathfile, &st) < 0 ? 0 : S_IFDIR == ( st.st_mode & S_IFMT );
}
bool file_isfile( const char *pathfile ) {
    struct stat_ st;
    return stat8(pathfile, &st) < 0 ? 0 : S_IFREG == ( st.st_mode & S_IFMT );
}
bool file_islink( const char *pathfile ) {
#ifdef S_IFLNK
    struct stat_ st;
    return stat8(pathfile, &st) < 0 ? 0 : S_IFLNK == ( st.st_mode & S_IFMT );
#else
    return 0;
#endif
}


bool file_exact(const char *fname1, const char *fname2) {
    bool errors = false;
    for(FILE *in1 = fopen(fname1, "rb"); in1; fclose(in1), in1 = 0)
    for(FILE *in2 = fopen(fname2, "rb"); in2; fclose(in2), in2 = 0) {
        char buffer[64*1024+1] = {0};
        while( !errors && fread(buffer,1,32*1024,in1)>0 && fread(buffer+32*1024,1,32*1024,in2)>0 ) {
            errors |= !!memcmp(buffer, buffer+32*1024, 32*1024);
        }
        errors |= !!memcmp(buffer, buffer+32*1024, 32*1024);
    }
    return !errors;
}

char* file_read(const char *pathfile) {
#if 0
    size_t sz = file_size(fname);
    if( !sz ) return 0;
    FILE *fp = fopen(fname, "rb");
    if(!fp) return 0;
    char *data = malloc(sz+1); data[sz] = 0;
    if( fread(data, 1,sz, fp) != sz ) { free(data); fclose(fp); return 0; }
    fclose(fp);
    return data;
#else
    static __thread char *bufs[16] = {0};
    static __thread int buf = 0; buf = ++buf & (16-1);

    char **data = &bufs[ buf ];
    size_t len = file_size(pathfile);
    FILE *fp = fopen8(pathfile, "rb");

    if( fp && len ) {
        *data = REALLOC(*data, len + 1);
        len = fread(*data, 1, len, fp); len[*data] = 0;
        fclose(fp);
    } else {
        *data = REALLOC(*data, 8); 0[*data] = 0;
    }

    return *data;
#endif
}


bool file_write(const char *pathfile, const void *data, int len) {
    bool ok = 0;
    for( FILE *fp = fopen8(pathfile, "wb"); fp; fclose(fp), fp = 0) {
        ok = len == fwrite(data, 1,len, fp);
    }
    return ok;
}

bool file_append(const char *pathfile, const void *data, int len) {
    bool ok = 0;
    for( FILE *fp = fopen8(pathfile, "a+b"); fp; fclose(fp), fp = 0) {
        ok = len == fwrite(data, 1,len, fp);
    }
    return ok;
}

bool file_touch( const char *pathfile, size_t modtime ) {
    struct stat_ st;
    if( stat8( pathfile, &st ) >= 0 ) {
        time_t date = modtime ? (time_t)modtime : time(0);
        struct utimbuf ut;
        ut.actime = st.st_atime;  /* keep atime unchanged */
        ut.modtime = date;        /* set mtime to given time */
        return utime( pathfile, &ut ) == 0;
    }
    return false;
}

bool file_copy( const char *pathsrc, const char *pathdst ) {
#  ifdef _WIN32 //  _WIN
    return CopyFileW( file_widen(pathsrc), file_widen(pathdst), FALSE );
#elif defined __APPLE__ // _OSX
    return copyfile( pathsrc, pathdst, 0, COPYFILE_DATA )>=0;
#else
    bool ok = 0;
    FILE *in = fopen8( pathsrc, "rb" );
    if( in ) {
        FILE *out = fopen8( pathdst, "wb" );
        if( out ) {
            char buf[1024];
            while( int n = fread( buf, 1, 1024, in ) ){
                ok = fwrite( buf, 1, n, out ) == n;
                if( !ok ) break;
            }
            fclose( out );
        }
        fclose( in );
    }
    return ok;
#endif
}

bool file_delete( const char *path ) {
    remove8( path );
    return !file_exist(path);
}

char *file_norm( const char *name ) {
    static __thread char bufs[4][260]; // 260: PATH_MAX
    static __thread int index = 0; index = ++index & 3;
    char *out = bufs[index], *buf = out, c;

    // skip lowercases+digits+underscores+slashes only. anything else is truncated.
    // remove dupe slashes
    // remove dupe underlines
    if( name ) do {
        /**/ if( *name >= 'a' && *name <= 'z' ) { *out++ = *name; }
        else if( *name >= 'A' && *name <= 'Z' ) { *out++ = *name - 'A' + 'a'; }
        else if( *name >= '0' && *name <= '9' ) { *out++ = *name; }
        else if( *name == '_'                 ) { *out++ = *name; }
        else if( *name == '/' || *name == '\\') { *out++ = '/'; name += strspn(name, "/\\"); --name; }
        else if( *name <= ' ' || *name == '.' ) { *out++ = '_'; name += strspn(name, ". "); --name; }
    } while( *++name );

    return buf;
}

const char *file_base( const char *file ) {
    const char *end = strlen(file)+file;
    const char *a = strrchr(file, '/');
    const char *b = strrchr(file, '\\');
    if( a > b ) return a+1;
    if( b > a ) return b+1;
    return file; // end;
}

#if 0
uint64_t file_checksum( const char *pathfile ) {
    void *x = file_read(pathfile); // from scratch memory
    uint64_t l = file_size(pathfile);
    uint64_t crc = crc64( 0, x, l );
    return crc;
}

bool file_stat( const char *uri, struct stat *out_info ) {
#ifdef _WIN32
    // make win32-friendly stat() call (no trailing slashes)
    char s[4096] = {0};
    strcpy(s, uri); // strncpy(s, 4096, uri);
    for( int l = (int)strlen(s); --l >= 0; ) {
        if( s[l] == '\\' || s[l] == '/' ) s[l] = 0;
    }
    uri = s;
#endif
    return stat( uri, out_info ) == 0;
}
#endif

#ifdef FILE_DEMO
#pragma once
int main() {
    char *readbuf = file_chunk(__FILE__, 0, file_size(__FILE__));
    printf( "read: %s\n", readbuf );

    char *file1 = file_norm("There's\\\\a///Lady/who's  sure   all that \"glitters\"/is (gold)/file.audio");
    char *file2 = file_norm(file1); // renormalize file should be safe
    puts(file1);
    puts(file2);
    assert( !strcmp(file1, file2) );
    assert( ~puts("ok") );
}
#define main main__
#endif // FILE_DEMO
#endif // FILE_C
