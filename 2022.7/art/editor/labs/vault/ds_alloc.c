// [x] memory: realloc, vrealloc, stack
// [ ] @todo: gc, pool, game mark/rewind+game stack |level|game|>---<stack|
// - rlyeh, public domain

#ifndef ALLOC_H
#define ALLOC_H

#define align_up(addr,N)   ((addr + (N - 1)) & -N)  // Round up to N-byte boundary
#define align_down(addr,N) (addr & -N)  // Round down to a N-byte boundary

// default allocator (aborts on out-of-mem)
void*  xrealloc(void* p, size_t sz);
size_t xlen(void* p);

// vector based allocator (x1.75 enlarge factor)
void*  vrealloc(void* p, size_t sz);
size_t vlen(void* p);

// stack based allocator (negative bytes will rewind stack, like when entering new frame)
void*  stack(int bytes);

#endif

// vector ----------------------------------------------------------------------

#ifdef ALLOC_C
#pragma once

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            REALLOC( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret = (size_t*)p - 2;
        if( !p ) {
            ret = (size_t*)REALLOC( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)REALLOC( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

// realloc ---------------------------------------------------------------------

void* xrealloc(void* ptr, size_t size) {
    static void *oom_reserve = 0; if(!oom_reserve) oom_reserve = REALLOC(0, 4 * 1024 * 1024);
    ptr = REALLOC(ptr, size);
    if( !ptr && size ) {
        REALLOC(oom_reserve, 0), oom_reserve = 0;
        abort(); // exit( ENOMEM );
    }
#ifdef XREALLOC_POISON
    if( ptr && size ) {
        memset(ptr, 0xCD, size);
    }
#endif
    return ptr;
}
size_t xlen(void* p) {
    if( p ) return MSIZE(p);
    return 0;
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) { // use negative bytes to rewind stack
    static __thread uint8_t *stack_mem = 0;
    static __thread uint64_t stack_ptr = 0;
    static __thread uint64_t stack_max = 0; // watch this var, in case you want to fine tune 4 MiB value below
    if( bytes < 0 ) {
        if( stack_ptr > stack_max ) stack_max = stack_ptr;
        return (stack_ptr = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xlen(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_ptr += bytes) - bytes ];
}

#endif
