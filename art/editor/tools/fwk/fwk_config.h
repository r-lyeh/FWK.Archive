// -----------------------------------------------------------------------------
// if/n/def hell

#define ifdef(macro, yes, /*no*/...)   ifdef_##macro(yes, __VA_ARGS__)
#define ifndef(macro, yes, /*no*/...)  ifdef_##macro(__VA_ARGS__, yes)
#define is(macro)                      ifdef_##macro(1,0)
#define isnt(macro)                    ifdef_##macro(0,1)
#define ifdef_true(yes, /*no*/...)     yes
#define ifdef_false(yes, /*no*/...)    __VA_ARGS__

#ifdef _MSC_VER
#define ifdef_gcc                      ifdef_false
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_true
#elif defined __TINYC__
#define ifdef_gcc                      ifdef_false
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_true
#define ifdef_cl                       ifdef_false
#elif defined __MINGW64__ // __MINGW__ ?
#define ifdef_gcc                      ifdef_true
#define ifdef_mingw                    ifdef_true
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_false
#else // also __clang__
#define ifdef_gcc                      ifdef_true
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_false
#endif

#ifdef __cplusplus
#define ifdef_cpp                      ifdef_true
#define ifdef_c                        ifdef_false
#else
#define ifdef_c                        ifdef_true
#define ifdef_cpp                      ifdef_false
#endif

#if defined _WIN32
#define ifdef_win32                    ifdef_true
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __linux__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_true
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __APPLE__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_true
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __EMSCRIPTEN__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_true
#else // __FreeBSD__ || @todo: __ANDROID_API__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_true
#define ifdef_ems                      ifdef_false
#endif

// -----------------------------------------------------------------------------
// new C keywords
// @todo: autorun (needed?)

#define countof(x)       (int)(sizeof (x) / sizeof 0[x])

#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b

#define macro(name)      concat(name, __LINE__)
#define defer(begin,end) for(int macro(i) = ((begin), 0); !macro(i); macro(i) = ((end), 1))
#define scope(end)       defer((void)0, end)
#define benchmark        for(double macro(t) = -time_ss(); macro(t) < 0; printf("%.2fs (" FILELINE ")\n", macro(t)+=time_ss()))
#define do_once          static int macro(once) = 0; for(;!macro(once);macro(once)=1)

#ifdef _MSC_VER
#define __thread         __declspec(thread)
#elif defined __TINYC__ && defined _WIN32
#define __thread         __declspec(thread) // compiles fine, but does not work apparently
#elif defined __TINYC__
#define __thread
#endif
#define threadlocal      __thread

// usage: bool static(audio_is_init) = audio_init();
//#define static(var)    static var; do_once var

//-----------------------------------------------------------------------------
// new C macros

#define ASSERT(expr, ...)          do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; static int seen = 0; if(!seen) seen = 1, breakpoint(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)); } } while(0)

#define FILELINE                   __FILE__ ":" STRINGIZE(__LINE__)
#define STRINGIZE(x)               STRINGIZ3(x)
#define STRINGIZ3(x)               #x

#define EXPAND(name, ...)          EXPAND_QUOTE(EXPAND_JOIN(name, EXPAND_COUNT_ARGS(__VA_ARGS__)), (__VA_ARGS__))
#define EXPAND_QUOTE(x, y)         x y
#define EXPAND_JOIN(name, count)   EXPAND_J0IN(name, count)
#define EXPAND_J0IN(name, count)   EXPAND_J01N(name, count)
#define EXPAND_J01N(name, count)   name##count
#define EXPAND_COUNT_ARGS(...)     EXPAND_ARGS((__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define EXPAND_ARGS(args)          EXPAND_RETURN_COUNT args
#define EXPAND_RETURN_COUNT(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, count, ...) count

//#define STATIC_ASSERT(EXPR)      typedef char UNIQUE_NAME(_static_assert_on_line)[(EXPR)?1:-1]
#define STATIC_ASSERT(EXPR)        STATIC_ASSER7(EXPR, __LINE__)
#define STATIC_ASSER7(EXPR, LINE)  STATIC_ASS3R7(EXPR, LINE)
#define STATIC_ASS3R7(EXPR, LINE)  typedef struct { unsigned static_assert_on_line_##LINE : !!(EXPR); } static_assert_on_line_##LINE // typedef int static_assert_on_line_##LINE[ !!(EXPR) ]

#if defined(_MSC_VER) && !defined(__cplusplus)
#define INLINE __inline
#else
#define INLINE inline
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE INLINE
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
#define FINITE _finite
#else
#define FINITE isfinite
#endif

// usage: #define vec2(...) C_CAST(vec2, __VA_ARGS__)
// typedef union vec2 { float X,Y; }; vec2 a = {0,1}, b = vec2(0,1);
#ifdef __cplusplus
#define C_CAST(type, ...)  ( type { __VA_ARGS__ } )
#else
#define C_CAST(type, ...)  ((type){ __VA_ARGS__ } )
#endif

// -----------------------------------------------------------------------------
// visibility

#if is(win32)
#define WITH_DLL 1
#define IMPORT ifdef(gcc, __attribute__ ((dllimport)), __declspec(dllimport))
#define EXPORT ifdef(gcc, __attribute__ ((dllexport)), __declspec(dllexport))
#define STATIC
#else
#define WITH_DLL 0
#define IMPORT    
#define EXPORT    
#define STATIC    
#endif

// win32 users would need to -DAPI=IMPORT/EXPORT as needed when using/building FWK as DLL.
#ifndef API
#define API STATIC
#endif

// -----------------------------------------------------------------------------
// config directives
// @todo: debug (+_debug) /O0 /D3 > debugopt (+_debug) /O1 /D2 > releasedbg (+ndebug) /O2 /D1 > release (+ndebug) /O3 /D0 ... debug: /D0 -> -g0 instead?

#if defined NDEBUG || (!defined _DEBUG && !defined O0)
#define RELEASE 1
#else
#define RELEASE 0
#endif

#if defined __OPTIMIZE__ || defined O3 || defined O2 || defined O1 // __OPTIMIZE__ is gcc+clang exclusive. O3/O2/O1 is FWK convention. vc+tcc would need explicitly to set up any.
#define OPTIMIZE 1
#else
#define OPTIMIZE 0
#endif

#ifndef O_FLAG
#define O_FLAG (2 * RELEASE + OPTIMIZE)
#endif

#ifndef WITH_FASTCALL_LUA
#define WITH_FASTCALL_LUA    1 ///+
#endif

#ifndef WITH_LEAK_DETECTOR
#define WITH_LEAK_DETECTOR   0 ///+
#endif

#ifndef WITH_PROFILE
#define WITH_PROFILE         1 ///+
#endif

#ifndef WITH_SELFIES
#define WITH_SELFIES         0 ///+
#endif

#ifndef WITH_XREALLOC_POISON
#define WITH_XREALLOC_POISON 1 ///+
#endif

#ifndef WITH_LINUX_CALLSTACKS
#define WITH_LINUX_CALLSTACKS 0 ///+
#endif

// -----------------------------------------------------------------------------
// system headers

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   // for linux
#endif

#if defined(_MSC_VER) && defined(_WIN32) // for VC IDE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0502 // GetInfoAddrW/FreeAddrInfoW for X86
#endif

#ifdef _MSC_VER
#include <omp.h>      // compile with /openmp to speed up some computations
#endif
#include <assert.h>
//#include <float.h>
//#include <limits.h>
#include <math.h>     // NAN
#include <stdarg.h>   // va_*(), ...
#include <stdbool.h>  // bool,true,false
#include <stdint.h>   // u/int8/16/32/64_t
#include <stdio.h>    // FILE,NULL
#include <stdlib.h>   // malloc,free,exit,
#include <string.h>   // strlen,memset,memcpy,

// -----------------------------------------------------------------------------
// pragma libs

#if defined _WIN32 && (defined _MSC_VER || defined __TINYC__)
#pragma comment(lib, "advapi32")
#pragma comment(lib, "comdlg32")
#pragma comment(lib, "dbghelp")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "user32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "ws2_32")
#endif

#if defined __linux__ && defined __TINYC__
#pragma comment(lib, "dl")
#pragma comment(lib, "m")
#pragma comment(lib, "pthread")
#endif
