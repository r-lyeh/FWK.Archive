
#if is(cl)
#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)
#define STATIC
#else
#define IMPORT
#define EXPORT
#define STATIC
#endif

// msvc users may need to -DAPI=IMPORT/EXPORT as needed when loading/building FWK as DLL.
#ifndef API
#define API
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

#ifdef _MSC_VER
#include <omp.h>      // compile with /openmp to speed up some computations
#endif
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// stl, forward includes

#define array(t) t*
#define threadlocal __thread

#ifdef _MSC_VER
#define __thread         __declspec(thread)
#elif defined __TINYC__ && defined _WIN32
#define __thread         __declspec(thread) // compiles fine, but does not work apparently
#elif defined __TINYC__
#define __thread
#endif

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
