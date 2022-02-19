// -----------------------------------------------------------------------------
// memory framework
// - rlyeh, public domain

// memory leaks detector
#if WITH_LEAK_DETECTOR
#define WATCH(ptr,sz) watch((ptr), (sz))
#define FORGET(ptr)   forget(ptr)
#else
#define WATCH(ptr,sz) (ptr)
#define FORGET(ptr)   (ptr)
#endif

// default allocator (aborts on out-of-mem)
API void*  xrealloc(void* p, size_t sz);
API size_t xsize(void* p);

// stack based allocator (negative bytes does rewind stack, like when entering new frame)
API void*  stack(int bytes);

// memory leaks
API void*  watch( void *ptr, int sz );
API void*  forget( void *ptr );

// memory api
#define ALLOCSIZE(p)   xsize(p)
#define REALLOC(p,n)   (len1_ = (n), (len1_ ? WATCH(xrealloc((p),len1_),len1_) : xrealloc(FORGET(p),0)))
#define MALLOC(n)      REALLOC(0,(n))
#define FREE(p)        REALLOC(FORGET(p), 0)
#define CALLOC(m,n)    (len2_ = (m)*(n), memset(REALLOC(0,len2_),0,len2_))
#define STRDUP(s)      (len3_ = strlen(s)+1, ((char*)memcpy(REALLOC(0,len3_), (s), len3_)))
static __thread size_t len1_, len2_, len3_;

#if 0 // ifndef REALLOC
#define REALLOC            realloc
#define MALLOC(n)          REALLOC(0, n)
#define FREE(p)            REALLOC(p, 0)
#define CALLOC(c, n)       memset(MALLOC((c)*(n)), 0, (c)*(n))
#define STRDUP(s)          strcpy(MALLOC(strlen(s)+1), (s))
#endif
