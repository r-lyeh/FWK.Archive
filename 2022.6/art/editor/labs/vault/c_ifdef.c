// - rlyeh, public domain

#ifndef IFDEF_H
#define IFDEF_H

#define IF(x,T,...)           IF_(x)(T,__VA_ARGS__/*F*/)
#define IFN(x,T,...)          IF_(x)(__VA_ARGS__/*F*/,T)
#define IF_(x)                IF_DETECT(x)
#define IF_DETECT(...)        IF_DETECT_##__VA_ARGS__
#define IF_DETECT_0(t,...)    __VA_ARGS__
#define IF_DETECT_1(t,...)    t

#define IFDEF IF
#define IFNDEF IFN

// -----------------------------------------------------------------------------

// build type
#define DEBUG          0
#define DEBUGOPT       0
#define RELEASE        0

// endianness symbols
#define BIG            0
#define LITTLE         0

// arch wordsize and cacheline symbols
#define X32            0
#define X64            0
#define L64            0
#define L128           0

// architecture symbols
#define ARM            0
#define PPC            0
#define INTEL          0
#define JIT            0

// language symbols
#define C89            0
#define C99            0
#define C11            0
#define CPP            0
#define CPP03          0
#define CPP11          0
#define EXCEPTIONS     0

// language features
#define COMPUTED_GOTO  0
#define FLEXIBLE_ARRAY 0

// compiler symbols
#define CLANG          0
#define GCC            0
#define ICC            0
#define MINGW          0
#define MSC            0
#define SNC            0
#define GCC_COMPAT     0

// platform symbols
#define ANDROID        0
#define BSD            0
#define HTML5          0
#define IOS            0
#define LINUX          0
#define OSX            0
#define PS4            0
#define RASPBERRYPI    0
#define STEAMLINK      0
#define UNIX           0
#define WINDOWS        0
#define XBOX1          0

// detect build type (@todo: check _DEBUG compatibility in non-msvc compilers)
// we assume that we are on shipping build if asserts are removed (ie, NDEBUG is defined)
#if defined NDEBUG && defined _DEBUG
#   undef  DEBUGOPT
#   define DEBUGOPT    1
#elif defined NDEBUG
#   undef  RELEASE
#   define RELEASE     1
#else
#   undef  DEBUG
#   define DEBUG       1
#endif

// detect compiler
// also compiler arm __CC_ARM, ibm __xlc__ || __xlC__, sun __SUNPRO_C || __SUNPRO_CC
#if   defined __ICC // __ICL, __INTEL_COMPILER
#   undef  ICC
#   define ICC         1
#elif defined __SNC__
#   undef  SNC
#   define SNC         1
#elif defined __clang__ // before gcc
#   undef  CLANG
#   define CLANG       1
#elif defined __GNUC__
#   undef  GCC
#   define GCC         1
#elif defined _MSC_VER
#   undef  MSC
#   define MSC         1
#elif defined __MINGW32__
#   undef  MINGW
#   define MINGW       1
#endif

// detect compilers in gcc compatibility mode (clang, icc, snc)
#if defined __GNUC__ && !defined (__GNUC_PATCHLEVEL__)
#   undef  GCC
#   define GCC         1
#endif

// detect C++ language and version
#ifdef __cplusplus
#   undef  CPP
#   define CPP         1
#   if (__cplusplus < 201103L && !defined _MSC_VER) || (defined _MSC_VER && _MSC_VER < 1700)
#   undef  CPP03
#   define CPP03       1
#   else
#   undef  CPP11
#   define CPP11       1
#   endif
#else // detect C language and version
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
#   undef  C11
#   define C11         1
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   undef  C99
#   define C99         1
#else
#   undef  C89
#   define C89         1
#endif
#endif

// detect language features
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L // C99
#   undef  FLEXIBLE_ARRAY
#   define FLEXIBLE_ARRAY // char data[FLEXIBLE_ARRAY]; -> char data[];
#endif

#ifdef __GNUC__
#   undef  COMPUTED_GOTO
#   define COMPUTED_GOTO 1
#endif

// detect c++ exceptions
#if defined __cplusplus && ( \
    (defined _HAS_EXCEPTIONS && _HAS_EXCEPTIONS) || \
    (defined _STLP_USE_EXCEPTIONS && _STLP_USE_EXCEPTIONS) || \
    (defined HAVE_EXCEPTIONS) || \
    (defined __EXCEPTIONS) || \
    (defined _CPPUNWIND) )
#   undef  EXCEPTIONS
#   define EXCEPTIONS  1
#endif

#ifdef __has_feature
#if __has_feature(cxx_exceptions)
#   undef  EXCEPTIONS
#   define EXCEPTIONS  1
#endif
#endif

// detect endianness
// also BE if (*(uint16_t *)"\0\xff" < 0x100)
#if defined __BIG_ENDIAN__ || defined __ARMEB__ || defined __THUMBEB__ || defined __MIPSEB__ || (defined __BYTE_ORDER__ && __BYTE_ORDER__==__BIG_ENDIAN)
#   undef  BIG
#   define BIG         1
#else
#   undef  LITTLE
#   define LITTLE      1
#endif


// detect architecture
// also __mips__ __mips (L64), __ia64, __sparc, __alpha, __hppa, __avr32__, __sh__, __itanium__|_M_IA64
#if   defined _M_ARM // __aarch64__ __arm__
#   undef  ARM
#   define ARM         1
#elif defined _M_PPC // __ppc64__ __ppc__ __powerpc__ __powerpc64__ __powerpc
#   undef  PPC
#   define PPC         1
#elif defined _M_IX86 || defined _M_X64
#   undef  INTEL
#   define INTEL       1
#else // likely VM/JIT
#   undef  JIT
#   define JIT         1
#endif

// detect wordsize
#include <stdint.h>
#  if defined INTPTR_MAX && INTPTR_MAX == INT32_MAX
#   undef  X32
#   define X32      1
#elif defined INTPTR_MAX && INTPTR_MAX == INT64_MAX // __x86_64__ _M_X64 __aarch64__ __powerpc64__ __ppc64__ __itanium__ _M_IA64
#   undef  X64
#   define X64      1
#endif

// detect cache line size
#if PPC
#   undef  L128
#   define L128    1
#else
#   undef  L64
#   define L64     1
#endif

// detect platform
// also CYG __CYGWIN__, QNX __QNXNTO__, SOL __sun, __hpux, _AIX, __WINDOWS__ (watcom), _WINDOWS
#  if defined __EMSCRIPTEN__
#   undef  HTML5
#   define HTML5       1
#elif defined _XBOX_ONE || defined _DURANGO
#   undef  XBOX1
#   define XBOX1       1 // before windows
#elif defined _WIN32 || defined __MINGW32__
#   undef  WINDOWS
#   define WINDOWS     1
#elif defined __ANDROID__
#   undef  ANDROID
#   define ANDROID     1 // before linux
#elif defined __ORBIS__
#   undef  PS4
#   define PS4         1 // before linux
#elif defined __STEAMLINK__
#   undef  STEAMLINK
#   define STEAMLINK   1 // before linux
#elif defined __VCCOREVER__
#   undef  RASPBERRYPI
#   define RASPBERRYPI 1 // before linux
#elif defined __linux__
#   undef  LINUX
#   define LINUX       1 // before unix
#elif defined __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ || defined __ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__ // TARGET_OS_IPHONE == 1 
#   undef  IOS
#   define IOS         1 // before bsd
#elif defined __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ || defined __APPLE__ // TARGET_OS_MAC == 1
#   undef  OSX
#   define OSX         1 // before bsd
#elif defined __NetBSD__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __DragonFly__
#   undef  BSD
#   define BSD         1 // before unix
#elif defined __unix__
#   undef  UNIX
#   define UNIX        1
#endif

#ifdef IFDEF_DEMO
#pragma once
#include <stdio.h>
int main() {
    // compile-time if
    int debug = 0; IF(DEBUG, debug = 1);
    // compile-time if-else
    const char *arch = IF(X64, "64", "32") "-bit arch";
    // compile-time if-else-if
    const char *cpu = "CPU-" IF(INTEL, "Intel", IF(ARM, "ARM", "Unknown"));

    // symbols are boolean preprocessor directives as well
    #if DEBUG
    debug = 1;
    #endif

    // debug all symbols
    puts( 1 + 
        IF(X32, "+X32") IF(X64, "+X64")
        IF(L64, "+L64") IF(L128, "+L128")

        IF(BIG, "+BIG") IF(LITTLE, "+LITTLE")
        IF(ARM, "+ARM") IF(PPC, "+PPC") IF(INTEL, "+INTEL") IF(JIT, "+JIT")

        IF(DEBUG, "+DEBUG") IF(DEBUGOPT, "+DEBUGOPT") IF(RELEASE, "+RELEASE")

        IF(C89, "+C89") IF(C99, "+C99") IF(C11, "+C11")
        IF(CPP, "+CPP") IF(CPP03, "+CPP03") IF(CPP11, "+CPP11") IF(EXCEPTIONS, "+EXCEPT")

        IF(MSC, "+MSC") IF(SNC, "+SNC") IF(ICC, "+ICC") IF(GCC, "+GCC")
        IF(CLANG, "+CLANG") IF(MINGW, "+MINGW") IF(GCC_COMPAT, "+GCC_COMPAT")
        
        IF(HTML5, "+HTML5") 
        IF(BSD, "+BSD") IF(UNIX, "+UNIX")
        IF(IOS, "+IOS") IF(ANDROID, "+ANDROID")
        IF(XBOX1, "+XBOX1") IF(PS4, "+PS4")
        IF(WINDOWS, "+WINDOWS") IF(LINUX, "+LINUX") IF(OSX, "+OSX")
        IF(RASPBERRYPI, "+RASPBERRYPI") IF(STEAMLINK, "+STEAMLINK")
    );
}
#define main __main
#endif // IFDEF_DEMO
#endif // IFDEF_H
