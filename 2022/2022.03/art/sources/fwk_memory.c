size_t dlmalloc_usable_size(void*); // __ANDROID_API__

#if is(bsd) || is(osx) // bsd or osx
#  include <malloc/malloc.h>
#else
#  include <malloc.h>
#endif

#ifndef SYS_REALLOC
#define SYS_REALLOC realloc
#define SYS_MSIZE /* bsd/osx, then win32, then ems/__ANDROID_API__, then __GLIBC__ */ \
    ifdef(osx, malloc_size, ifdef(bsd, malloc_size, \
        ifdef(win32, _msize, \
            ifdef(ems, dlmalloc_usable_size, malloc_usable_size))))
#endif

// xrealloc --------------------------------------------------------------------

void* xrealloc(void* oldptr, size_t size) {
    void *ptr = SYS_REALLOC(oldptr, size);
    if( !ptr && size ) {
        PANIC("Not memory enough (trying to allocate %u bytes)", (unsigned)size);
    }
#if WITH_XREALLOC_POISON
    if( !oldptr && size ) {
        memset(ptr, 0xCD, size);
    }
#endif
    return ptr;
}
size_t xsize(void* p) {
    if( p ) return SYS_MSIZE(p);
    return 0;
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) { // use negative bytes to rewind stack
    static threadlocal uint8_t *stack_mem = 0;
    static threadlocal uint64_t stack_ptr = 0;
    static threadlocal uint64_t stack_max = 0; // watch this var, in case you want to fine tune 4 MiB value below
    if( bytes < 0 ) {
        if( stack_ptr > stack_max ) stack_max = stack_ptr;
        return (stack_ptr = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xsize(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_ptr += bytes) - bytes ];
}

// leaks ----------------------------------------------------------------------

void* watch( void *ptr, int sz ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        for( FILE *fp = fopen(buf, "a+"); fp; fclose(fp), fp = 0 ) {
            fseek(fp, 0L, SEEK_END);
            char *callstack(int);
            const char *cs = callstack( +48 ); // NULL; // =
            fprintf(fp, "Built %s %s\n", __DATE__, __TIME__); // today() instead?
            fprintf(fp, "Memleak address: [%p], size: %d\n%s\n", ptr, sz, cs ? cs : "No callstack.");
        }
    }
    return ptr;
}
void* forget( void *ptr ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        unlink(buf);
    }
    return ptr;
}
