/* game framework.
*  - rlyeh, public domain
*
* ## FWK License
*
* This software is available under 3 licenses. Choose whichever you prefer.
* ------------------------------------------------------------------------------
* ALTERNATIVE A - Public Domain (https://unlicense.org/)
* ------------------------------------------------------------------------------
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
* software, either in source code form or as a compiled binary, for any purpose,
* commercial or non-commercial, and by any means.
*
* In jurisdictions that recognize copyright laws, the author or authors of this
* software dedicate any and all copyright interest in the software to the public
* domain. We make this dedication for the benefit of the public at large and to
* the detriment of our heirs and successors. We intend this dedication to be an
* overt act of relinquishment in perpetuity of all present and future rights to
* this software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE B - MIT-0 (No Attribution clause)
* ------------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the "Software"), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE C - Zero BSD License (https://opensource.org/licenses/FPL-1.0.0)
* ------------------------------------------------------------------------------
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* ## License: Contributed Code ------------------------------------------------
*
* Dear Contributor,
*
* In order to ensure this project remains completely free and unencumbered by
* anyone's copyright monopoly, it is advisable that you dedicate your code-base
* contributions to the three licensing terms above. This removes any possible
* ambiguity as to what terms somebody might have thought they were contributing
* under, in case of a future dispute. These concerns are not unique to public
* domain software. Most large, established open-source projects have a
* Contributor License Agreement (CLA) process, of varying degrees of formality.
*
* Please add yourself to the list below before contributing.
* Thanks.
*
* --
*
* "I dedicate any and all copyright interest in this software to the three
* license terms listed above. I make this dedication for the benefit of the
* public at large and to the detriment of my heirs and successors. I intend
* this dedication to be an overt act of relinquishment in perpetuity of all
* present and future rights to this software under copyright law."
*
* Author (name)              I agree (YES/NO)    Files/Features (optional)
* ------------------------------------------------------------------------------
* @r-lyeh                    YES                 Initial codebase
* ------------------------------------------------------------------------------
*/

#ifndef FWK_H
#define FWK_H

#ifdef __cplusplus
extern "C" {
#endif

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

#define macro(name)      concat(name, __LINE__)
#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b

#define benchmark        for(double macro(t) = -time_ss(); macro(t) < 0; printf("%.2fs\n", macro(t)+=time_ss()))
#define do_once          static int macro(once) = 0; for(;!macro(once);macro(once)=1)
#define defer(begin,end) for(int macro(i) = ((begin), 0); !macro(i); macro(i) = ((end), 1))
#define scope(end)       defer((void)0, end)

//-----------------------------------------------------------------------------
// new C macros

#define ASSERT(expr, ...)   do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; breakpoint(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)); } } while(0)
#define PRINTF(...)         PRINTF(va(__VA_ARGS__), 1[#__VA_ARGS__] == '!' ? callstack(+48) : "", __FILE__, __LINE__, __FUNCTION__)

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

//-----------------------------------------------------------------------------
// Headers

#line 1 "fwk_config.h"

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
#line 0

#line 1 "fwk_math.h"
// -----------------------------------------------------------------------------
// math framework: rand, ease, vec2, vec3, vec4, quat, mat2, mat33, mat34, mat4
// - rlyeh, public domain
//
// Credits: @ands+@krig+@vurtun (PD), @datenwolf (WTFPL2), @evanw+@barerose (CC0), @sgorsten (Unlicense).

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>

#define C_EPSILON  (1e-6)
#define C_PI       (3.141592654f) // (3.14159265358979323846f)
#define TO_RAD     (C_PI/180.f)
#define TO_DEG     (180.f/C_PI)

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
#define m_finite _finite
#else
#define m_finite isfinite
#endif

#ifdef __cplusplus
#define M_CAST(type, ...)  ( type { __VA_ARGS__ } )
#else
#define M_CAST(type, ...)  ((type){ __VA_ARGS__ } )
#endif

// ----------------------------------------------------------------------------

//#define ptr(type)         0[&(type).x]
#define vec2(x, y      )  M_CAST(vec2, (float)(x), (float)(y)                        )
#define vec3(x, y, z   )  M_CAST(vec3, (float)(x), (float)(y), (float)(z),           )
#define vec4(x, y, z, w)  M_CAST(vec4, (float)(x), (float)(y), (float)(z), (float)(w))
#define quat(x, y, z, w)  M_CAST(quat, (float)(x), (float)(y), (float)(z), (float)(w))
#define axis(x, y, z)     M_CAST(axis, (float)(x), (float)(y), (float)(z))
#define mat33(...)        M_CAST(mat33, __VA_ARGS__ )
#define mat34(...)        M_CAST(mat34, __VA_ARGS__ )
#define mat44(...)        M_CAST(mat44, __VA_ARGS__ )
//#define coord_system(...) M_CAST(coord_system, __VA_ARGS__)

typedef union vec2 { struct { float X,Y; };     struct { float x,y; }; struct { float r,g; }; struct { float w,h; }; struct { float min,max; }; float v2[2]; float array[1]; } vec2;
typedef union vec3 { struct { float X,Y,Z; };   struct { float x,y,z; }; struct { float r,g,b; }; struct { float w,h,d; }; vec2 xy; vec2 rg; vec2 wh; float v3[3]; float array[1]; } vec3;
typedef union vec4 { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; struct { float r,g,b,a; }; vec2 xy; vec3 xyz; vec2 rg; vec3 rgb; vec2 wh; vec3 whd; float v4[4]; float array[1]; } vec4;
typedef union quat { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; vec3 xyz; vec4 xyzw; float v4[4]; float array[1]; } quat;
typedef float mat33[9];
typedef float mat34[12];
typedef float mat44[16];

// ----------------------------------------------------------------------------

API void     randset(uint64_t state);
API uint64_t rand64(void);
API double   randf(void); // [0, 1) interval
API int      randi(int mini, int maxi); // [mini, maxi) interval
API double   rng(void); // [0..1) Lehmer RNG "minimal standard"

// ----------------------------------------------------------------------------

API float simplex1( float x );
API float simplex2( vec2 xy );
API float simplex3( vec3 xyz );
API float simplex4( vec4 xyzw );

// ----------------------------------------------------------------------------

API float ease_linear(float t);

API float ease_out_sine(float t);
API float ease_out_quad(float t);
API float ease_out_cubic(float t);
API float ease_out_quart(float t);
API float ease_out_quint(float t);
API float ease_out_expo(float t);
API float ease_out_circ(float t);
API float ease_out_back(float t);
API float ease_out_elastic(float t);
API float ease_out_bounce(float t);

API float ease_in_sine(float t);
API float ease_in_quad(float t);
API float ease_in_cubic(float t);
API float ease_in_quart(float t);
API float ease_in_quint(float t);
API float ease_in_expo(float t);
API float ease_in_circ(float t);
API float ease_in_back(float t);
API float ease_in_elastic(float t);
API float ease_in_bounce(float t);

API float ease_inout_sine(float t);
API float ease_inout_quad(float t);
API float ease_inout_cubic(float t);
API float ease_inout_quart(float t);
API float ease_inout_quint(float t);
API float ease_inout_expo(float t);
API float ease_inout_circ(float t);
API float ease_inout_back(float t);
API float ease_inout_elastic(float t);
API float ease_inout_bounce(float t);

API float ease_inout_perlin(float t);

API float ease_ping_pong(float t, float(*fn1)(float), float(*fn2)(float));
API float ease_pong_ping(float t, float(*fn1)(float), float(*fn2)(float));

// ----------------------------------------------------------------------------

API float deg      (float radians);
API float rad      (float degrees);

API int   mini     (int    a, int    b);
API int   maxi     (int    a, int    b);
API int   absi     (int    a          );
API int   clampi   (int v,int a,int b);

API float minf     (float  a, float  b);
API float maxf     (float  a, float  b);
API float absf     (float  a          );
API float pmodf    (float  a, float  b);
API float signf    (float  a)           ;
API float clampf   (float v,float a,float b);
API float mixf     (float a,float b,float t);

// ----------------------------------------------------------------------------

API vec2  ptr2     (const float *a    );
//
API vec2  neg2     (vec2   a          );
API vec2  add2     (vec2   a, vec2   b);
API vec2  sub2     (vec2   a, vec2   b);
API vec2  mul2     (vec2   a, vec2   b);
API vec2  inc2     (vec2   a, float  b);
API vec2  dec2     (vec2   a, float  b);
API vec2  scale2   (vec2   a, float  b);
API vec2  div2     (vec2   a, float  b);
API vec2  pmod2    (vec2   a, float  b);
API vec2  min2     (vec2   a, vec2   b);
API vec2  max2     (vec2   a, vec2   b);
API vec2  abs2     (vec2   a          );
API vec2  floor2   (vec2   a          );
API vec2  fract2   (vec2   a          );
API vec2  ceil2    (vec2   a          );
API float dot2     (vec2   a, vec2   b);
API vec2  refl2    (vec2   a, vec2   b);
API float cross2   (vec2   a, vec2   b);
API float len2sq   (vec2   a          );
API float len2     (vec2   a          );
API vec2  norm2    (vec2   a          );
API int   finite2  (vec2   a          );
API vec2  mix2  (vec2 a,vec2 b,float t);
API vec2  clamp2(vec2 v,float a,float b);
// ----------------------------------------------------------------------------

API vec3  ptr3     (const float *a    );
API vec3  vec23    (vec2   a, float z );
//
API vec3  neg3     (vec3   a          );
API vec3  add3     (vec3   a, vec3   b);
API vec3  sub3     (vec3   a, vec3   b);
API vec3  mul3     (vec3   a, vec3   b);
API vec3  inc3     (vec3   a, float  b);
API vec3  dec3     (vec3   a, float  b);
API vec3  scale3   (vec3   a, float  b);
API vec3  div3     (vec3   a, float  b);
API vec3  pmod3    (vec3   a, float  b);
API vec3  min3     (vec3   a, vec3   b);
API vec3  max3     (vec3   a, vec3   b);
API vec3  abs3     (vec3   a          );
API vec3  floor3   (vec3   a          );
API vec3  fract3   (vec3   a          );
API vec3  ceil3    (vec3   a          );
API vec3  cross3   (vec3   a, vec3   b);
API float dot3     (vec3   a, vec3   b);
API vec3  refl3    (vec3   a, vec3   b);
API float len3sq   (vec3   a          );
API float len3     (vec3   a          );
API vec3  norm3    (vec3   a          );
API vec3  norm3sq  (vec3   a          );
API int   finite3  (vec3   a          );
API vec3  mix3  (vec3 a,vec3 b,float t);
API vec3  clamp3(vec3 v,float a,float b);
//vec3 tricross3 (vec3 a, vec3 b, vec3 c);
API void  ortho3   (vec3 *left, vec3 *up, vec3 v);

API vec3  rotatex3 (vec3 dir, float degrees);
API vec3  rotatey3 (vec3 dir, float degrees);
API vec3  rotatez3 (vec3 dir, float degrees);

// ----------------------------------------------------------------------------

API vec4  ptr4     (const float *a    );
API vec4  vec34    (vec3   a, float w );
//
API vec4  neg4     (vec4   a          );
API vec4  add4     (vec4   a, vec4   b);
API vec4  sub4     (vec4   a, vec4   b);
API vec4  mul4     (vec4   a, vec4   b);
API vec4  inc4     (vec4   a, float  b);
API vec4  dec4     (vec4   a, float  b);
API vec4  scale4   (vec4   a, float  b);
API vec4  div4     (vec4   a, float  b);
API vec4  pmod4    (vec4   a, float  b);
API vec4  min4     (vec4   a, vec4   b);
API vec4  max4     (vec4   a, vec4   b);
API vec4  abs4     (vec4   a          );
API vec4  floor4   (vec4   a          );
API vec4  fract4   (vec4   a          );
API vec4  ceil4    (vec4   a          );
API float dot4     (vec4   a, vec4   b);
API vec4  refl4    (vec4   a, vec4   b);
API float len4sq   (vec4   a          );
API float len4     (vec4   a          );
API vec4  norm4    (vec4   a          );
API vec4  norm4sq  (vec4   a          );
API int   finite4  (vec4   a          );
API vec4  mix4  (vec4 a,vec4 b,float t);
API vec4  clamp4(vec4 v,float a,float b);
// vec4 cross4(vec4 v0, vec4 v1);

// ----------------------------------------------------------------------------

API quat  idq      (                  );
API quat  ptrq     (const float *a    );
API quat  vec3q    (vec3   a, float w );
API quat  vec4q    (vec4   a          );
//
API quat  negq     (quat   a          );
API quat  conjq    (quat   a          );
API quat  addq     (quat   a, quat   b);
API quat  subq     (quat   a, quat   b);
API quat  mulq     (quat   p, quat   q);
API quat  scaleq   (quat   a, float  s);
API quat  normq    (quat   a          );
API float dotq     (quat   a, quat   b);
API quat  mixq(quat a, quat b, float t);
/* quat lerpq(quat a, quat b, float s);
    return norm(quat((1-s)*a.x + s*b.x, (1-s)*a.y + s*b.y, (1-s)*a.z + s*b.z, (1-s)*a.w + s*b.w));
}*/
API quat slerpq(quat a, quat b, float s);

API quat  rotationq(float deg,float x,float y,float z);
API quat  mat44q   (mat44 M);

API vec3 rotate3q_2(vec3 v, quat q);
API vec3 rotate3q(vec3 v, quat r);

// euler <-> quat
API vec3  euler    (quat q);
API quat  eulerq   (vec3 pyr_degrees);

// ----------------------------------------------------------------------------

API void scaling33(mat33 m, float x, float y, float z);
API void scale33(mat33 m, float x, float y, float z);
API void id33(mat33 m);
API void extract33(mat33 m, const mat44 m4);
API void copy33(mat33 m, const mat33 a);//
API vec3 mulv33(mat33 m, vec3 v);
API void multiply33x2(mat33 m, const mat33 a, const mat33 b);

API void rotation33(mat33 m, float degrees, float x,float y,float z);
API void rotationq33(mat33 m, quat q);
API void rotate33(mat33 r, float degrees, float x,float y,float z);
API void compose33(mat33 m, quat r, vec3 s);

// ----------------------------------------------------------------------------

API void id34(mat34 m);
API void copy34(mat34 m, const mat34 a);
API void scale34(mat34 m, float s);
API void add34(mat34 m, mat34 n);
API void muladd34(mat34 m, mat34 n, float s);
API void add34x2(mat34 m, mat34 n, mat34 o);
API void lerp34(mat34 m, mat34 n, mat34 o, float alpha); // mix34?
API void multiply34x2(mat34 m, const mat34 m0, const mat34 m1);
API void multiply34(mat34 m, const mat34 a);
API void multiply34x3(mat34 m, const mat34 a, const mat34 b, const mat34 c);
API void compose34(mat34 m, vec3 t, quat q, vec3 s);
API void invert34(mat34 m, const mat34 o);

// ----------------------------------------------------------------------------

API void scaling44(mat44 m, float x, float y, float z);
API void id44(mat44 m);
API void identity44(mat44 m);
API void copy44(mat44 m, const mat44 a);
API void multiply44x2(mat44 m, const mat44 a, const mat44 b);
API void multiply44x3(mat44 m, const mat44 a, const mat44 b, const mat44 c);
API void multiply44(mat44 m, const mat44 a);
// ---
API void ortho44(mat44 m, float l, float r, float b, float t, float n, float f);
API void frustum44(mat44 m, float l, float r, float b, float t, float n, float f);
API void perspective44(mat44 m, float fovy_degrees, float aspect, float nearp, float farp);
API void lookat44(mat44 m, vec3 eye, vec3 center, vec3 up);
// ---
API void translation44(mat44 m, float x, float y, float z);
API void translate44(mat44 m, float x, float y, float z);
API void relocate44(mat44 m, float x, float y, float z);
API void rotationq44(mat44 m, quat q);
API void rotation44(mat44 m, float degrees, float x, float y, float z);
API void rotate44(mat44 m, float degrees, float x, float y, float z);
API void scaling44(mat44 m, float x, float y, float z);
API void scale44(mat44 m, float x, float y, float z);
// ---
API void transpose44(mat44 m, const mat44 a);
API float det44(const mat44 M);
API bool invert44(mat44 T, const mat44 M);

API vec4 transform444(const mat44, const vec4);
API bool unproject44(vec3 *out, vec3 xyd, vec4 viewport, mat44 mvp);

API void compose44(mat44 m, vec3 t, quat q, vec3 s);

// ----------------------------------------------------------------------------

API vec3 transform33(const mat33 m, vec3 p);

API vec4 transform444(const mat44 m, const vec4 p);

API vec3 transform344(const mat44 m, const vec3 p);

API vec3 transformq(const quat q, const vec3 v);

#if 0
// A value type representing an abstract direction vector in 3D space, independent of any coordinate system.
// A concrete 3D coordinate system with defined x, y, and z axes.
typedef enum AXIS_ENUMS { axis_front, axis_back, axis_left, axis_right, axis_up, axis_down } AXIS_ENUMS;
typedef union coord_system { struct { AXIS_ENUMS x,y,z; }; } coord_system;

API vec3 transform_axis(const coord_system, const AXIS_ENUMS);

API void rebase44(mat44 m, const coord_system src_basis, const coord_system dst_basis);

//API vec3 transform_axis(const coord_system basis, const AXIS_ENUMS to);

// A vector is the difference between two points in 3D space, possessing both direction and magnitude
API vec3 transform_vector  (const mat44 m, const vec3 vector);

// A point is a specific location within a 3D space
API vec3 transform_point   (const mat44 m, const vec3 p);

// A tangent is a unit-length vector which is parallel to a piece of geometry, such as a surface or a curve
API vec3 transform_tangent (const mat44 m, const vec3 tangent);

// A normal is a unit-length bivector which is perpendicular to a piece of geometry, such as a surface or a curve
API vec3 transform_normal  (const mat44 m, const vec3 normal);

// A quaternion can describe both a rotation and a uniform scaling in 3D space
API quat transform_quat    (const mat44 m, const quat q);

// A matrix can describe a general transformation of homogeneous coordinates in projective space
API float* transform_matrix(mat44 out, const mat44 m, const mat44 matrix);

// Scaling factors are not a vector, they are a compact representation of a scaling matrix
API vec3 transform_scaling (const mat44 m, const vec3 scaling);
#endif

// ----------------------------------------------------------------------------
// !!! for debugging

API void print2( vec2 v );
API void print3( vec3 v );
API void print4( vec4 v );
API void printq( quat q );
API void print33( float *m );
API void print34( float *m );
API void print44( float *m );
#line 0



#line 1 "fwk_audio.h"
// -----------------------------------------------------------------------------
// audio framework
// - rlyeh, public domain
//
// fixme: leaks, audio_delete
// fixme: sfxr (hack)
// @todo: audio_volume_fx, audio_volume_bgm, audio_volume_master instead?
// @todo: destroystream()    if( ss->type == WAV ) drwav_uninit(&ss->wav);
// @todo: destroystream()    if( ss->type == MOD ) jar_mod_unload(&ss->mod);
// @todo: destroystream()    if( ss->type == XM && ss->xm ) jar_xm_free_context(ss->xm);

typedef struct audio_handle* audio_t;

API audio_t audio_clip( const char *pathfile );
API audio_t audio_stream( const char *pathfile );
API int     audio_play( audio_t s, int flags );
API int     audio_play_gain( audio_t a, int flags, float gain/*0*/ );
API int     audio_play_gain_pitch( audio_t a, int flags, float gain, float pitch/*1*/ );
API int     audio_play_gain_pitch_pan( audio_t a, int flags, float gain, float pitch, float pan/*0*/ );

API float   audio_volume_clip(float gain);   // set     fx volume if gain is in [0..1] range. return current     fx volume in any case
API float   audio_volume_stream(float gain); // set    bgm volume if gain is in [0..1] range. return current    bgm volume in any case
API float   audio_volume_master(float gain); // set master volume if gain is in [0..1] range. return current master volume in any case

enum AUDIO_FLAGS {
    AUDIO_1CH = 0, // default
    AUDIO_2CH = 1,

    AUDIO_8 = 2,
    AUDIO_16 = 0, // default
    AUDIO_32 = 4,
    AUDIO_FLOAT = 8,

    AUDIO_22KHZ = 0, // default
    AUDIO_44KHZ = 16,

    AUDIO_MIXER_GAIN = 0, // default
    AUDIO_IGNORE_MIXER_GAIN = 32,
};

API int audio_queue( const void *samples, int num_samples, int flags );
#line 0

#line 1 "fwk_collide.h"
// -----------------------------------------------------------------------------
// original code by @vurtun (PD) and @barerose (CC0).
// [src] https://gist.github.com/vurtun/95f088e4889da2474ad1ce82d7911fee
// - rlyeh, public domain.

#ifndef GJK_H
#define GJK_H

#define GJK_MAX_ITERATIONS 20

typedef struct gjk_support {
    int aid, bid;
    vec3 a;
    vec3 b;
} gjk_support;
typedef struct gjk_vertex {
    vec3 a;
    vec3 b;
    vec3 p;
    int aid, bid;
} gjk_vertex;
typedef struct gjk_simplex {
    int max_iter, iter;
    int hit, cnt;
    gjk_vertex v[4];
    float bc[4], D;
} gjk_simplex;
typedef struct gjk_result {
    int hit;
    vec3 p0;
    vec3 p1;
    float distance_squared;
    int iterations;
} gjk_result;

int gjk(gjk_simplex *s, const gjk_support *sup, vec3 *dv);
gjk_result gjk_analyze(const gjk_simplex *s);
gjk_result gjk_quad(float a_radius, float b_radius);

#endif

//typedef struct gjk_result gjk_result;

typedef struct line     { vec3 a, b;                                                  } line;
typedef struct sphere   { vec3 c; float r;                                            } sphere;
typedef struct aabb     { vec3 min, max;                                              } aabb;
typedef struct plane    { vec3 p, n;                                                  } plane;
typedef struct capsule  { vec3 a, b; float r;                                         } capsule;
typedef struct ray      { vec3 p, d;                                                  } ray;
typedef struct triangle { vec3 p0,p1,p2;                                              } triangle;
typedef struct poly     { vec3* verts; int cnt;                                       } poly;
typedef union  frustum  { struct { vec4 l, r, t, b, n, f; }; vec4 pl[6]; float v[24]; } frustum;

#define line(...)       M_CAST(line, __VA_ARGS__)
#define sphere(...)     M_CAST(sphere, __VA_ARGS__)
#define aabb(...)       M_CAST(aabb, __VA_ARGS__)
#define plane(...)      M_CAST(plane, __VA_ARGS__)
#define capsule(...)    M_CAST(capsule, __VA_ARGS__)
#define ray(p,normdir)  M_CAST(ray, p, normdir)
#define triangle(...)   M_CAST(triangle, __VA_ARGS__)
#define poly(...)       M_CAST(poly, __VA_ARGS__)
#define frustum(...)    M_CAST(frustum, __VA_ARGS__)

// ----------------------------------------------------------------------------

typedef struct hit {
    union {
        // general case
        float depth;
        // rays only: penetration (t0) and extraction (t1) points along ray line
        struct { float t0, t1; };
        // gjk only
        struct { int hits; vec3 p0, p1; float distance2; int iterations; };
    };
    union { vec3 p; vec3 contact_point; };
    union { vec3 n; vec3 normal; };
} hit;

#define hit(...)        M_CAST(hit, __VA_ARGS__)

// ----------------------------------------------------------------------------

/* line/segment */
API float   line_distance2_point(line l, vec3 p);
API vec3    line_closest_point(line l, vec3 p);
/* ray */
API float   ray_test_plane(ray r, vec4 p4);
API float   ray_test_triangle(ray r, triangle t);
API int     ray_test_sphere(float *t0, float *t1, ray r, sphere s);
API int     ray_test_aabb(float *t0, float *t1, ray r, aabb a);
API hit*    ray_hit_plane(ray r, plane p);
API hit*    ray_hit_triangle(ray r, triangle t);
API hit*    ray_hit_sphere(ray r, sphere s);
API hit*    ray_hit_aabb(ray r, aabb a);
/* sphere */
API vec3    sphere_closest_point(sphere s, vec3 p);
API hit*    sphere_hit_aabb(sphere s, aabb a);
API hit*    sphere_hit_capsule(sphere s, capsule c);
API hit*    sphere_hit_sphere(sphere a, sphere b);
API int     sphere_test_aabb(sphere s, aabb a);
API int     sphere_test_capsule(sphere s, capsule c);
API int     sphere_test_poly(sphere s, poly p);
API int     sphere_test_sphere(sphere a, sphere b);
/* aabb */
API vec3    aabb_closest_point(aabb a, vec3 p);
API float   aabb_distance2_point(aabb a, vec3 p);
API int     aabb_contains_point(aabb a, vec3 p);
API hit*    aabb_hit_aabb(aabb a, aabb b);
API hit*    aabb_hit_capsule(aabb a, capsule c);
API hit*    aabb_hit_sphere(aabb a, sphere s);
API int     aabb_test_aabb(aabb a, aabb b);
API int     aabb_test_capsule(aabb a, capsule c);
API int     aabb_test_poly(aabb a, poly p);
API int     aabb_test_sphere(aabb a, sphere s);
/* capsule */
API float   capsule_distance2_point(capsule c, vec3 p);
API vec3    capsule_closest_point(capsule c, vec3 p);
API hit*    capsule_hit_aabb(capsule c, aabb a);
API hit*    capsule_hit_capsule(capsule a, capsule b);
API hit*    capsule_hit_sphere(capsule c, sphere s);
API int     capsule_test_aabb(capsule c, aabb a);
API int     capsule_test_capsule(capsule a, capsule b);
API int     capsule_test_poly(capsule c, poly p);
API int     capsule_test_sphere(capsule c, sphere s);
/* poly: query */
API int     poly_test_sphere(poly p, sphere s);
API int     poly_test_aabb(poly p, aabb a);
API int     poly_test_capsule(poly p, capsule c);
API int     poly_test_poly(poly a, poly b);
/* poly: query transformed */
API int     poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a);
API int     poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33, poly b, vec3 bpos3, mat33 brot33);
/* poly: gjk result */
API int     poly_hit_sphere(struct gjk_result *res, poly p, sphere s);
API int     poly_hit_aabb(struct gjk_result *res, poly p, aabb a);
API int     poly_hit_capsule(struct gjk_result *res, poly p, capsule c);
API int     poly_hit_poly(struct gjk_result *res, poly a, poly b);
/* poly: gjk result transformed */
API int     poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a);
API int     poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_hit_poly_transform(struct gjk_result *res, poly a, vec3 at3, mat33 ar33, poly b, vec3 bt3, mat33 br33);

API vec4    plane4(vec3 p, vec3 n);

API frustum frustum_build(mat44 projview);
API int     frustum_test_sphere(frustum f, sphere s);
API int     frustum_test_aabb(frustum f, aabb a);

API poly    poly_alloc(int cnt);
API void    poly_free(poly *p);

API poly    pyramid(vec3 from, vec3 to, float size); // poly_free() required
API poly    diamond(vec3 from, vec3 to, float size); // poly_free() required
#line 0

#line 1 "fwk_cooker.h"
// -----------------------------------------------------------------------------
// asset pipeline framework
// - rlyeh, public domain.
//
// all cooked assets are stored inside .cook.zip file at root folder, which acts as an asset database.
// during game boot, the database gets rebuilt as follows: (note: step 0 is an optional optimization)
// 0. for N given cores, split list of infiles into N zipfiles. then, parallelize cookers.
// 1. compare local disk files against file in zip database. for each mismatch do:
// 2. - invalidate its entry in database, if local file was removed from disk.
// 3. - write its *cooked* contents into database, if local file was created or modified from disk.
// 4. mount any existing zipfile(s) after cooking.
//
// notes: meta-datas from every raw asset are stored into comment field, inside .cook.zip archive.
// @todo: fix leaks
// @todo: symlink exact files
// @todo: idle threads should steal jobs from busy threads (maybe use jobs/coroutines for this?)
// @todo: ... and should compress them in the original cook[N] bucket

enum COOKER_FLAGS {
    COOKER_SYNC = 0,
    COOKER_ASYNC = 1,
};

API void cooker_config( const char *art_path, const char *tools_path, const char *fwk_ini ); // "art/", "art/tools/", "fwk.ini"
API bool cooker_start( const char *masks, int flags ); // "**"
API void cooker_stop();
API int  cooker_progress(); // [0..100]
API int  cooker_jobs();     // [1..N]
#line 0

#line 1 "fwk_data.h"
// -----------------------------------------------------------------------------
// data framework (json5) @todo:xml,kvdb
// - rlyeh, public domain
//
// @todo: vec2,vec3,vec4

typedef union data_t {
    char* s;
    double f;
    int64_t i;
    uintptr_t p;
    array(union data_t) arr;
} data_t;

// data api

API bool    data_push(const char *source);
API data_t*     data_find(const char *type_keypath); // @todo, array(data_t) data_array();
API data_t      data_get(const char *type_keypath); // @todo, array(data_t) data_array();
API int         data_count(const char *keypath);
#define         data_int(...)    (data_get(va("i" __VA_ARGS__)).i)
#define         data_float(...)  (data_get(va("f" __VA_ARGS__)).f)
#define         data_string(...) (data_get(va("s" __VA_ARGS__)).s)
#define         data_count(...)   data_count(va(__VA_ARGS__))
API bool    data_pop();
#line 0

#line 1 "fwk_dll.h"
// dll utils
// - rlyeh, public domain

/// !!! `filename` must contain extension
/// load dynamic library `file` and search for `symbol`
/// return: NULL if not found, found symbol otherwise.
/// filename: path to dynamic library file. must contain extension.
/// symbol: symbol name. must not be NULL.
/// see: dlopen^, dlclose^
/// > bool (*plugin_init)(void) = dll("plugin.dll", "init");
/// > assert(plugin_init());
API void* dll(const char *filename, const char *symbol);
#line 0

#line 1 "fwk_ds.h"
// data structures and utils: array, set, map, hash, sort.
// - rlyeh, public domain

// -----------------------------------------------------------------------------
// sort

API int sort_64(const void *a, const void *b);

// -----------------------------------------------------------------------------
// less

API int less_64(uint64_t a, uint64_t b);
API int less_int(int a, int b);
API int less_ptr(void *a, void *b);
API int less_str(char *a, char *b);

// -----------------------------------------------------------------------------
// un/hash

API uint32_t unhash_32(uint32_t x);
API uint32_t hash_32(uint32_t x);
API uint64_t hash_64(uint64_t x);
API uint64_t hash_flt(double x);
API uint64_t hash_str(const char* str);
API uint64_t hash_int(int key);
API uint64_t hash_ptr(const void* ptr);

// -----------------------------------------------------------------------------
// vector based allocator (x1.75 enlarge factor)

API void* vrealloc( void* p, size_t sz );
API size_t vlen( void* p );

// -----------------------------------------------------------------------------
// arrays

#ifdef __cplusplus
#define array_cast(x) (decltype x)
#else
#define array_cast(x) (void *)
#endif

#define array(t) t*
#define array_init(t) ( (t) = 0 )
#define array_resize(t, n) ( array_c_ = array_count(t), array_realloc_((t),(n)), ((n)>array_c_? memset(array_c_+(t),0,((n)-array_c_)*sizeof(0[t])) : (void*)0), (t) )
#define array_push(t, ...) ( array_realloc_((t),array_count(t)+1), (t)[ array_count(t) - 1 ] = (__VA_ARGS__) )
#define array_pop(t) ( array_realloc_((t), array_count(t)-1) )
#define array_back(t) ( &(t)[ array_count(t)-1 ] ) // ( (t) ? &(t)[ array_count(t)-1 ] : NULL )
#define array_data(t) (t)
#define array_at(t,i) (t[i])
#define array_count(t) (int)( (t) ? array_vlen_(t) / sizeof(0[t]) : 0u )
#define array_bytes(t) (int)( (t) ? array_vlen_(t) : 0u )
#define array_sort(t, cmpfunc) qsort( t, array_count(t), sizeof(0[t]), (uintptr_t)cmpfunc == (uintptr_t)strcmp ? strcmp_qsort : cmpfunc )
#define array_empty(t) ( !array_count(t) )
static __thread unsigned array_c_;

#if 0 // original: no reserve support
#define array_reserve(t, n) ((void)0) // not implemented
#define array_clear(t) ( array_realloc_((t), 0), (t) = 0 )
#define array_vlen_(t)  ( vlen(t) - 0 )
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+0) * sizeof(0[t])) )
#define array_free(t) array_clear(t)
#else // new: with reserve support (buggy still?)
#define array_reserve(t, n) ( array_realloc_((t),(n)), array_clear(t) )
#define array_clear(t) ( array_realloc_((t),0) ) // -1
#define array_vlen_(t)  ( vlen(t) - sizeof(0[t]) ) // -1
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+1) * sizeof(0[t])) ) // +1
#define array_free(t) ( array_realloc_((t), -1), (t) = 0 ) // -1
#endif

#define array_reverse(t) \
    do if( array_count(t) ) { \
        for(int l = array_count(t), e = l-1, i = (array_push(t, 0[t]), 0); i <= e/2; ++i ) \
            { l[t] = i[t]; i[t] = (e-i)[t]; (e-i)[t] = l[t]; } \
        array_pop(t); \
    } while(0)

//#define array_foreach2(t,val_t,v) \
//    for( val_t *v = &0[t]; v < (&0[t] + array_count(t)); ++v )

//#define array_foreach(t, it) \
//    for( void *end__ = (it = &0[t]) + array_count(t); it != end__; ++it )

#define array_foreach(t,val_t,v) for each_array(t,val_t,v)
#define each_array(t,val_t,v) \
    ( int __it = 0, __end = array_count(t); __it < __end; ++__it ) \
        for( val_t v = __it[t], *on__ = &v; on__; on__ = 0 )

#define array_foreach_ptr(t,val_t,v) for each_array_ptr(t,val_t,v)
#define each_array_ptr(t,val_t,v) \
    ( int __it = 0, __end = array_count(t); __it < __end; ++__it ) \
        for( val_t *v = &__it[t]; v; v = 0 )

#define array_search(t, key, cmpfn) /* requires sorted array beforehand */ \
    bsearch(&key, t, array_count(t), sizeof(t[0]), cmpfn )

#define array_insert(t, i, n) do { \
    int ac = array_count(t); \
    if( i >= ac ) { \
        array_push(t, n); \
    } else { \
        array_push(t, array_back(t)); \
        memmove( &(t)[(i)+1], &(t)[i], (ac - (i)) * sizeof(t[0]) ); \
        (t)[ i ] = (n); \
    } \
} while(0)

#define array_copy(t, src) do { /*todo: review old vrealloc call!*/ \
    array_free(t); \
    (t) = vrealloc( (t), array_count(src) * sizeof(0[t])); \
    memcpy( (t), src, array_count(src) * sizeof(0[t])); \
} while(0)

#define array_erase(t, i) do { \
    memcpy( &(t)[i], &(t)[array_count(t) - 1], sizeof(0[t])); \
    array_pop(t); \
} while(0)

#define array_unique(t, cmpfunc) do { /*@todo: optimize me*/ \
    int cnt = array_count(t), cnt_bak = cnt; \
     if( cnt > 1 ) { \
        array_sort(t, cmpfunc); \
        for( int i = 1; i < cnt; ++i ) { \
            while( i < cnt && !cmpfunc(&(t)[i-1], &(t)[i]) ) { \
                memmove(&(t)[i-1], &(t)[i], (cnt - 1 - i) * sizeof((t)[0]) ) ; \
                --cnt; \
            } \
         } \
        if( cnt_bak != cnt ) array_resize((t), cnt); \
     } \
 } while(0)

#if 0 // snippet below does work
#define array_unique(t, cmpfunc) \
    array_sort(t, cmpfunc); \
    for( int i = 0, end = array_count(t) - 1; i < end; ) { \
        if( !strcmp(t[i], t[i+1]) ) { \
            /* array_erase(t, i+1); */ \
            memmove(&(t)[i+1], &(t)[i+2], (end - 1 - i) * sizeof((t)[0]) ); \
            array_pop(t); \
            --end; \
        } else { \
            ++i; \
        } \
    }
#endif

// -----------------------------------------------------------------------------
// set<K>
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/

// config
#ifndef SET_HASHSIZE
#define SET_HASHSIZE (4096 << 4)
#endif
#ifndef SET_DONT_ERASE
#define SET_DONT_ERASE 1
#endif

// public api
#define set(K) \
    struct { set base; struct { set_item p; K key; } tmp, *ptr; K *tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define set_init(m, cmpfn, hashfn) ( \
    (m) = set_cast(m) REALLOC(0, sizeof(*m)), \
    set_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = hashfn ) \
    )

#define set_free(m) ( \
    set_clear(m), \
    set_free(&(m)->base), \
    (m) = set_cast(m) REALLOC((m), 0), \
    (m) = 0 \
    )

#define set_insert(m, k) ( \
    (m)->ptr = set_cast((m)->ptr) REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->key \
    )

#define set_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = set_cast((m)->ptr) set_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->key : 0 \
    )

#define set_find_or_add(m, k) ( \
    (m)->tmp.key = (k), \
    (m)->tmpval = set_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : set_insert((m), ((m)->tmp.key)) \
    )

#define set_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define set_foreach(m,key_t,k) for each_set(m,key_t,k)
#define each_set(m,key_t,k) \
    ( int i_ = (m)->base.count ? 0 : SET_HASHSIZE; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; on_ = 0 )

#define set_foreach_ptr(m,key_t,k) for each_set_ptr(m,key_t,k)
#define each_set_ptr(m,key_t,k) \
    ( int i_ = (m)->base.count ? 0 : SET_HASHSIZE; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; on_ = 0 )

#define set_clear(m) ( \
    set_clear(&(m)->base) \
    )

#define set_count(m)        set_count(&(m)->base)
#define set_gc(m)           set_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define set_cast(t) (decltype(t))
#else
#define set_cast(t) (void *)
#endif

typedef struct set_item {
    struct set_item *next;

    uint64_t keyhash;
    void *key;
    void *super;
} set_item;

typedef struct set {
    array(set_item*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count;
} set;

API void  (set_init)(set *m);
API void  (set_free)(set *m);

API void  (set_insert)(set *m, set_item *p, void *key, uint64_t keyhash, void *super);
API void  (set_erase)(set *m, void *key, uint64_t keyhash);
API void* (set_find)(const set *m, void *key, uint64_t keyhash);
API int   (set_count)(const set *m);
API void  (set_gc)(set *m); // only if using SET_DONT_ERASE

// -----------------------------------------------------------------------------
// map<K,V>
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/

// config
#ifndef MAP_HASHSIZE
#define MAP_HASHSIZE (4096 << 4)
#endif
#ifndef MAP_DONT_ERASE
#define MAP_DONT_ERASE 1
#endif

// public api
#define map(K,V) \
    struct { map base; struct { pair p; K key; V val; } tmp, *ptr; V* tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define map_init(m, cmpfn, hashfn) ( \
    (m) = map_cast(m) REALLOC(0, sizeof(*(m))), \
    map_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = map_cast((m)->typed_cmp) cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = map_cast((m)->typed_hash) hashfn ) \
    )

#define map_free(m) ( \
    map_free(&(m)->base), \
    map_cast(m) REALLOC((m), sizeof(*(m))), (m) = 0 \
    )

#define map_insert(m, k, v) ( \
    (m)->ptr = map_cast((m)->ptr) REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->val = (v), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, &(m)->ptr->val, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->val \
    )

#define map_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = map_cast((m)->ptr) map_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->val : 0 \
    )

#define map_find_or_add(m, k, v) ( \
    (m)->tmp.key = (k), (m)->tmp.val = (v), \
    (m)->tmpval = map_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : map_insert((m), ((m)->tmp.key), ((m)->tmp.val)) \
    )

#define map_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define map_foreach(m,key_t,k,val_t,v) for each_map(m,key_t,k,val_t,v)
#define each_map(m,key_t,k,val_t,v) \
    ( int i_ = (m)->base.count ? 0 : MAP_HASHSIZE; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; ) \
                for( val_t v = *(val_t *)cur_->value; on_; on_ = 0 )

#define map_foreach_ptr(m,key_t,k,val_t,v) for each_map_ptr(m,key_t,k,val_t,v)
#define each_map_ptr(m,key_t,k,val_t,v) \
    ( int i_ = (m)->base.count ? 0 : MAP_HASHSIZE; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; ) \
                for( val_t *v = (val_t *)cur_->value; on_; on_ = 0 )

#define map_foreach_sorted_ptr(m,key_t,k,val_t,v) for each_map_sorted_ptr(m,key_t,k,val_t,v)
#define each_map_sorted_ptr(m,key_t,k,val_t,v) \
    ( int i_ = (map_sort(&(m)->base), 0); i_ < array_count((m)->base.sorted); ++i_) \
        for( pair *cur_ = (m)->base.sorted[i_]; cur_; ) \
            for( key_t *k = (key_t *)cur_->key; cur_; ) \
                for( val_t *v = (val_t *)cur_->value; cur_; cur_ = 0 )

#define map_clear(m) ( \
    map_clear(&(m)->base) \
    )

#define map_count(m)        map_count(&(m)->base)
#define map_gc(m)           map_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define map_cast(t) (decltype(t))
#else
#define map_cast(t) (void *)
#endif

typedef struct pair {
    struct pair *next;

    uint64_t keyhash;
    void *key;
    void *value;
    void *super;
} pair;

typedef struct map {
    array(pair*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count:31;

    int is_sorted:1;
    array(pair*) sorted;
} map;

API void  (map_init)(map *m);
API void  (map_free)(map *m);

API void  (map_insert)(map *m, pair *p, void *key, void *value, uint64_t keyhash, void *super);
API void  (map_erase)(map *m, void *key, uint64_t keyhash);
API void* (map_find)(map *m, void *key, uint64_t keyhash);
API int   (map_count)(map *m);
API void  (map_gc)(map *m); // only if using MAP_DONT_ERASE
API bool  (map_sort)(map* m);
#line 0

#line 1 "fwk_editor.h"
// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.
//
// when editor is on:
// - [ ] ESC, toggles editor on/off
// - [ ] ESC (long press), sends ESC key to game
// - [ ] TAB, toggles console/log and debugdraw on/off
// - [ ] TAB (long press), sends TAB key to game
// - [ ] F1, isolated view of selected entity on/off.
// - [ ] F1 (long press), sends F1 key to game
// - [ ] F5, plays/restarts the game
// - [ ] F5 (long press), sends F5 key to game
// - [ ] F12, toggles fullscreen on/off
// - [ ] F12 (long press), sends F12 key to game
// - [ ] PAUSE, toggles game pause on/off
// - [ ] PAUSE (long press), sends PAUSE key to game
// - [ ] LMB, select item, display hierarchy @ left sidebar, status @ right sidebar. right click to unselect
// - [ ] RMB, on selected item, display contextual menu: edit, edit script...
// - [ ] RMB, camera move if no gameobj is selected + WASDEC + wheel
// - [ ] SPC, on selected item, cycle transform: locate, rotate, scale
// - [ ] CTRL+S save, CTRL+L load restore point
// - [ ] IMPR save capture bug/info
//
// roadmap
// - [x] ray/object picking
// - [x] basic gizmos (@todo: fixed screen size, snapping)
// - [ ] introspect properties on selected object
// - [ ] add/rem entities, add/rem components, add/rem/pause/resume systems
// - [ ] widgets for (bool,str,flt,int,vec2,vec3,vec4,range,mesh,texture,audio,any other asset,combo of anything)
// - [ ] osc server for properties and editor behavior
// - [ ] multiple selections/select all
// - [ ] undo/redo
// - [ ] cut/copy/paste (ctrl-c to serialize)
// - [ ] menu: open, save, save as, save all, reload

API void  editor();
API bool  editor_active();
API vec3  editor_pick(float mouse_x, float mouse_y);

API char* dialog_load();
API char* dialog_save();

API int   gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool  gizmo_active();
#line 0

#line 1 "fwk_file.h"
// -----------------------------------------------------------------------------
// files, cache and virtual filesystem (registered directories and/or compressed zip archives).
// - rlyeh, public domain.
//
// - note: vfs_mount() order matters (last mounts have higher priority).
// - note: directory/with/trailing/slash/ as mount_point, or zip/tar/pak archive otherwise.
//
// @todo: file_mmap

// physical filesystem. files

API const char** file_list(const char *path, const char *masks); // **.png;*.c
API char *       file_read(const char *filename);
API char *       file_load(const char *filename, int *len);
API uint64_t     file_size(const char *pathfile);
API bool         file_directory(const char *pathfile);

API char *       file_pathabs(const char *pathfile); // ../dir/./file.ext -> c:/prj/dir/file.ext
API char *       file_path(const char *pathfile); // c:/prj/dir/file.ext -> c:/prj/dir/
API char *       file_name(const char *pathfile); // c:/prj/dir/file.ext -> file.ext
API char *       file_ext(const char *pathfile); // c:/prj/dir/file.ext -> .ext
API char *       file_id(const char *pathfile); // c:/prj/dir/file.ext -> file/dir/prj (name then alphabetical)
API char *       file_normalize(const char *pathfile); // c:/prj/dir/file.ext -> c/prj/dir/file_ext
//API char *     file_normalize_with_folder(const char *pathfile); // c:/prj/dir/file.ext -> dir/file_ext

API uint64_t     file_stamp(const char *pathfile); // 20210319113316 (datetime in base10)
API uint64_t     file_stamp_epoch(const char *pathfile); // 1616153596 (seconds since unix epoch)

API bool         file_exist(const char *pathfile);
API bool         file_delete(const char *pathfile);
API bool         file_copy(const char *src, const char *dst);
API bool         file_move(const char *src, const char *dst);

API FILE*        file_temp();
API char*        file_tempname();

// @todo file_find() from first file_scan()


// virtual filesystem

API bool         vfs_mount(const char *mount_point);
API const char** vfs_list(const char *masks); // **.png;*.c

API char *       vfs_read(const char *pathfile);
API char *       vfs_load(const char *pathfile, int *size);
API int          vfs_size(const char *pathfile);

API const char * vfs_resolve(const char *fuzzyname); // guess best match. @todo: fuzzy path
API const char * vfs_find(const char *pathfile); // returns filename to extracted temporary file, so it can be read by foreign/3rd party libs
API FILE*        vfs_handle(const char *pathfile); // same as above, but returns file handle instead. preferred way, will clean descriptors at exit

// cache

API void *       cache_insert(const char *key, void *value, int size);
API void *       cache_lookup(const char *key, int *size);
#line 0

#line 1 "fwk_font.h"
// -----------------------------------------------------------------------------
// font framework
// - rlyeh, public domain

// font size tags
#define FONT_H1 "\1" // largest
#define FONT_H2 "\2"
#define FONT_H3 "\3"
#define FONT_H4 "\4"
#define FONT_H5 "\5"
#define FONT_H6 "\6" // smallest

// font color tags
#define FONT_COLOR1  "\x10"
#define FONT_COLOR2  "\x11"
#define FONT_COLOR3  "\x12"
#define FONT_COLOR4  "\x13"
#define FONT_COLOR5  "\x14"
#define FONT_COLOR6  "\x15"
#define FONT_COLOR7  "\x16"
#define FONT_COLOR8  "\x17"
#define FONT_COLOR9  "\x18"
#define FONT_COLOR10 "\x19"

// font face tags
#define FONT_FACE1   "\x1a"
#define FONT_FACE2   "\x1b"
#define FONT_FACE3   "\x1c"
#define FONT_FACE4   "\x1d"
#define FONT_FACE5   "\x1e"
#define FONT_FACE6   "\x1f"

// font align tags
#define FONT_LEFT     "\\<"
#define FONT_CENTER   "\\|"
#define FONT_RIGHT    "\\>"
#define FONT_TOP      "\\^"
#define FONT_MIDDLE   "\\-"
#define FONT_BASELINE "\\_"
#define FONT_BOTTOM   "\\v"

// font flags
enum FONT_FLAGS {
    // font atlas size
    FONT_512 = 0x0,
    FONT_1024 = 0x1,
    FONT_2048 = 0x2,
    FONT_4096 = 0x4,

    // font oversampling
    FONT_NO_OVERSAMPLE = 0x0,
    FONT_OVERSAMPLE_X = 0x08,
    FONT_OVERSAMPLE_Y = 0x10,

    // unicode ranges
    FONT_ASCII = 0x800, // Compatible charset
    FONT_AR = 0x001000, // Arabic and Arabic-Indic digits
    FONT_ZH = 0x002000, // Chinese Simplified (@todo: add ZH_FULL)
    FONT_EL = 0x004000, // Greek, Coptic, modern Georgian, Svan, Mingrelian, Ancient Greek
    FONT_EM = 0x008000, // Emoji
    FONT_EU = 0x010000, // Eastern/western Europe, IPA, Latin ext A/B
    FONT_HE = 0x020000, // Hebrew, Yiddish, Ladino, and other diaspora languages
    FONT_JP = 0x040000, // Hiragana, Katakana, Punctuations, Half-width chars
    FONT_KR = 0x080000, // Korean, Hangul
    FONT_RU = 0x100000, // Cyrillic + ext A/B
    FONT_TH = 0x200000, // Thai
    FONT_VI = 0x400000, // Vietnamese
    FONT_CJK = FONT_ZH|FONT_JP|FONT_KR,

    // FONT_DEFAULTS = FONT_512 | FONT_NO_OVERSAMPLE | FONT_ASCII,
};

// configures
API void  font_face(const char *face_tag, const char *filename_ttf, float font_size, unsigned flags);
API void  font_face_from_mem(const char *tag, const void *ttf_buffer, unsigned ttf_len, float font_size, unsigned flags);
API void  font_scales(const char *face_tag, float h1, float h2, float h3, float h4, float h5, float h6);
API void  font_color(const char *color_tag, uint32_t color);

// commands
API vec2  font_xy();
API void  font_goto(float x, float y);
API vec2  font_print(const char *text);
API vec2  font_rect(const char *text);
//  void  font_clip(vec2 topleft, vec2 bottomright);
//  void  font_wrap(vec2 topleft, vec2 bottomright);

// syntax highlighting
API void* font_colorize(const char *text, const char *comma_types, const char *comma_keywords); // comma separated tokens. expensive, please cache result.
API vec2  font_highlight(const char *text, const void *colors);
#line 0

#line 1 "fwk_input.h"
// -----------------------------------------------------------------------------
// input framework
// - rlyeh, public domain
//
// @todo: touch, window
// @todo: for extra savings (168->72 bytes), promote bits to real bits (/8 %8) & normalized floats [-1,+1] to shorts or chars
// @todo: GAMEPAD_A|2, MOUSE_L|1, KEY_C|3
// @todo: load/save
// @todo: send virtual presses & outputs (rumble, light, led, text, etc)
// @todo: fix if logger !60 hz
// @tofo: fix click2/repeat edge cases

API int         input_use( int controller_id ); // [0..3]

// -- basic polling api (read input at current frame)

API float       input( int vk );
API vec2        input2( int vk );
API float       input_diff( int vk );
API vec2        input_diff2( int vk );

// -- extended polling api (read input at Nth frame ago)

API float       input_frame( int vk, int frame );
API vec2        input_frame2( int vk, int frame );
API const char* input_frames( int vk, int frame );

// -- events api

API int         input_up( int vk ); // ON -> OFF (release)
API int         input_down( int vk ); // OFF -> ON (trigger)
API int         input_held( int vk ); // ON -> ON (pressed)
API int         input_idle( int vk ); // OFF -> OFF

API int         input_click( int vk, int ms ); // OFF -> ON -> OFF
API int         input_click2( int vk, int ms ); // OFF -> ON -> OFF -> ON -> OFF
API int         input_repeat( int vk, int ms ); // [...] ON -> ON -> ON

API int         input_chord2( int vk1, int vk2 ); // all vk1 && vk2 are ON
API int         input_chord3( int vk1, int vk2, int vk3 ); // all vk1 && vk2 && vk3 are ON
API int         input_chord4( int vk1, int vk2, int vk3, int vk4 ); // all vk1 && vk2 && vk3 && vk4 are ON

// -- 1d/2d filters

API float       input_filter_positive( float v ); // [-1..1] -> [0..1]
API vec2        input_filter_positive2( vec2 v ); // [-1..1] -> [0..1]
API vec2        input_filter_deadzone( vec2 v, float deadzone );
API vec2        input_filter_deadzone_4way( vec2 v, float deadzone );

// -- utils

API void        input_demo();
API void        input_send( int vk ); // @todo
API void*       input_save_state( int id, int *size); // @todo
API bool        input_load_state( int id, void *ptr, int size); // @todo


// --

enum INPUT_ENUMS {
    // -- bits: x104 keyboard, x3 mouse, x15 gamepad, x7 window
    // keyboard gaming keys (53-bit): first-class keys for gaming
    KEY_ESC,
    KEY_TICK, KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,  KEY_BS,
    KEY_TAB,   KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,
    KEY_CAPS,     KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L, KEY_ENTER,
    KEY_LSHIFT,       KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,        KEY_RSHIFT,            KEY_UP,
    KEY_LCTRL,KEY_LALT,               KEY_SPACE,                KEY_RALT,KEY_RCTRL,  KEY_LEFT,KEY_DOWN,KEY_RIGHT,

    // for completeness, secondary keys below (52-bit). beware!
    KEY_INS,KEY_HOME,KEY_PGUP,KEY_DEL,KEY_END,KEY_PGDN, // beware: different behavior win/osx (also, osx: no home/end).
    KEY_LMETA,KEY_RMETA,KEY_MENU,KEY_PRINT,KEY_PAUSE,KEY_SCROLL,KEY_NUMLOCK, // beware: may trigger unexpected OS behavior. (@todo: add RSHIFT here for win?)
    KEY_MINUS,KEY_EQUAL,KEY_LSQUARE,KEY_RSQUARE,KEY_SEMICOLON,KEY_QUOTE,KEY_HASH,KEY_BAR,KEY_COMMA,KEY_DOT,KEY_SLASH, // beware: non-us keyboard layouts
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12, // beware: complicated on laptops/osx
    KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,KEY_PAD8,KEY_PAD9,KEY_PAD0, // beware: complicated on laptops
    KEY_PADADD,KEY_PADSUB,KEY_PADMUL,KEY_PADDIV,KEY_PADDOT,KEY_PADENTER, // beware: complicated on laptops

    MOUSE_L, MOUSE_M, MOUSE_R,
    GAMEPAD_CONNECTED, GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
    GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_MENU, GAMEPAD_START,
    GAMEPAD_LB, GAMEPAD_RB, GAMEPAD_LTHUMB, GAMEPAD_RTHUMB,
    WINDOW_BLUR, WINDOW_FOCUS, WINDOW_CLOSE, WINDOW_MINIMIZE, WINDOW_MAXIMIZE, WINDOW_FULLSCREEN, WINDOW_WINDOWED,

    // -- floats: x7 gamepad, x3 mouse, x4 touch, x4 window
    GAMEPAD_LPAD, GAMEPAD_LPADX = GAMEPAD_LPAD, GAMEPAD_LPADY,
    GAMEPAD_RPAD, GAMEPAD_RPADX = GAMEPAD_RPAD, GAMEPAD_RPADY,
    GAMEPAD_LTRIGGER, GAMEPAD_LT = GAMEPAD_LTRIGGER, GAMEPAD_RTRIGGER, GAMEPAD_RT = GAMEPAD_RTRIGGER, GAMEPAD_BATTERY,
    MOUSE, MOUSE_X = MOUSE, MOUSE_Y, MOUSE_W,
    TOUCH_X1, TOUCH_Y1, TOUCH_X2, TOUCH_Y2,
    WINDOW_RESIZE, WINDOW_RESIZEX = WINDOW_RESIZE, WINDOW_RESIZEY, WINDOW_ORIENTATION, WINDOW_BATTERY,

    // -- strings: x2 gamepad
    GAMEPAD_GUID, GAMEPAD_NAME,
};
#line 0

#line 1 "fwk_memory.h"
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
#define MALLOC(n)      REALLOC(0,(n))
#define FREE(p)        REALLOC(FORGET(p), 0)
#define REALLOC(p,n)   REALLOC_((p),(n))
#define CALLOC(m,n)    CALLOC_((m),(n))
#define STRDUP(s)      STRDUP_(s)

static INLINE void *(REALLOC_)(void *p, size_t n) { return n ? WATCH(xrealloc(p,n),n) : xrealloc(FORGET(p),0); } ///-
static INLINE void *(CALLOC_)(size_t m, size_t n) { return n *= m, memset(REALLOC(0,n),0,n); } ///-
static INLINE char *(STRDUP_)(const char *s) { size_t n = strlen(s)+1; return ((char*)memcpy(REALLOC(0,n), s, n)); } ///-
#line 0

#line 1 "fwk_network.h"
// -----------------------------------------------------------------------------
// network framework
// - rlyeh, public domain

API int download( FILE *out, const char *url );
API int portname( const char *service_name, unsigned retries );

// -----------------------------------------------------------------------------
// udp wrapper
// - rlyeh, public domain.

// server
API int   udp_bind(const char *address, const char *port);

// client
API int   udp_open(const char *address, const char *port);

// common
API int   udp_send(int, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_sendto(int, const char *ip, const char *port, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_recv(int, void *buf, int len ); // <0 error, 0 orderly shutdown, >0 received bytes
API int   udp_peek(int); // <0 error, 0 timeout, >0 data

// -----------------------------------------------------------------------------
// tcp wrapper
// - rlyeh, public domain

// client
API int   tcp_open(const char *address, const char *port);

// server
API int   tcp_bind(const char *interface_, const char *port, int queue);
API int   tcp_peek(int, int(*callback)(int));

// common
API int   tcp_send(int, const void* buf, int len);
API int   tcp_recv(int, void* buf, int len);
API char* tcp_host(int); // info
API char* tcp_port(int); // info

API int   tcp_close(int);

// extras
API int   tcp_debug(int); // toggle traffic monitoring on/off for given socket
//API int   tcp_printf(int, const char *fmt, ...); // printf message in remote end
//API int   tcp_crypt(int,uint64_t);               // set shared secret
#line 0

#line 1 "fwk_obj.h"
// -----------------------------------------------------------------------------
// C object framework (constructors/destructors, methods, rtti, refcounting)
// - rlyeh, public domain.
//
// ## object api (low level)
//
// - [ ] make object from reflected type (factory)
// - [x] make object (if debug, include callstack as well)
// - [x] ctor method (optional, ref to constructor)
// - [x] dtor method (optional, ref to deleter)
// - [x] zero mem object
// - [x] object logger
// - [ ] iterate members in a struct
//
// - [x] clone/copy/mutate classes
// - [x] load/save objects from/to memory/disk
// - [ ] diff/patch objects
// - [ ] experimental: support for AoSoA layout (using objcnt, 3bits)
//
// ## object decomposition
//
//                             <---------|--------->
//            OBJ-SHADOW (64-bits)       |   OBJ CONTENT (N bytes)
// +-----+-----+-------------+-----------+-----+-----+-----+-----+--
// |TYPE |REFS.|   OBJ NAME  |  obj cnt  | ... | ... | ... | ... | .
// +-----+-----+-------------+-----------+-----+-----+-----+-----+--
// \-16-bits--/\---45-bits--/\--3-bits--/\-------N-bytes-----------
//
// OBJ TYPE+NAME format:
// - [type] custom tags at 0x0
// - [1..N] name
// - [\n]   blank separator
// - [comments, logger, infos, etc] << obj_printf();
//
// ## object limitations
// - 256 classes max
// - 256 references max
// - 8-byte overhead per object
// - 2 total allocs per object (could be flattened into 1 with some more work)
//
// @todo: obj_extend( "class_src", "class_dst" ); call[super(obj)]()
// @todo: preferred load/save format: [ver:1,user:2,type:1] ([eof|size:7/15/23/31][blob:N])+ [crc:1/2/3/4]
// @todo: more serious loading/saving spec

// object api (heap+rtti)

API void*       obj_malloc( int sz, ... );
API void*       obj_calloc( int sz, ... );
API void        obj_free( void *obj );

API bool        obj_typeeq( const void *obj1, const void *obj2 );
API const char* obj_typeof( const void *obj );
API unsigned    obj_typeid( const void *obj );
API unsigned    obj_typeid_from_name( const char *name );

// object api (ctor/dtor, refcounting, oop)

API void        obj_new( const char *type, ... );
API void        obj_del( void *obj );

API void*       obj_ref( void *obj );
API void*       obj_unref( void *obj );

API void        obj_extend( const char *dstclass, const char *srcclass );
API void        obj_override( const char *objclass, void (**vtable)(), void(*fn)() );

// object: serialize

API unsigned    obj_load(void *obj, const array(char) buffer);
API unsigned    obj_load_file(void *obj, FILE *fp);
API unsigned    obj_load_inplace(void *obj, const void *src, unsigned srclen);

API array(char) obj_save(const void *obj); // empty if error. must array_free() after use
API unsigned    obj_save_file(FILE *fp, const void *obj);
API unsigned    obj_save_inplace(void *dst, unsigned cap, const void *obj);

// object: utils

API unsigned    obj_instances( const void *obj );

API void        obj_zero( void *obj );
API unsigned    obj_sizeof( const void *obj );

API void        obj_hexdump( const void *obj );
API void        obj_hexdumpf( FILE *out, const void *obj );

API void        obj_printf( void *obj, const char *text );
API const char* obj_output( const void *obj );

API void *      obj_clone(const void *obj);
API void *      obj_copy(void **dst, const void *src);
API void *      obj_mutate(void **dst, const void *src);

// object: method dispatch tables

#define ctor(obj) obj_method0(obj, ctor) // ctor[obj_typeid(obj)](obj)
#define dtor(obj) obj_method0(obj, dtor) // dtor[obj_typeid(obj)](obj)

API extern void (*ctor[256])(); ///-
API extern void (*dtor[256])(); ///-

// object: syntax sugars

#define     obj_malloc(sz, ...) obj_initialize((void**)MALLOC(  sizeof(void*)+sz), stringf("\1untyped\n%s\n", "" #__VA_ARGS__))
#define     obj_calloc(sz, ...) obj_initialize((void**)CALLOC(1,sizeof(void*)+sz), stringf("\1untyped\n%s\n", "" #__VA_ARGS__))

#define     obj_new0(type) obj_new(type, 0)
#define     obj_new(type, ...) ( \
                obj_tmpalloc = obj_initialize((void**)CALLOC(1, sizeof(void*)+sizeof(type)), stringf("%c" #type "\n", (char)obj_typeid_from_name(#type))), \
                (*(type*)obj_tmpalloc = (type){ __VA_ARGS__ }), \
                ctor(obj_tmpalloc), \
                (type*)obj_tmpalloc )

#define     obj_override(class, method)    obj_override(#class, (void(**)())method, (void(*)())class##_##method)
#define     obj_method0(obj, method)       method[obj_typeid(obj)]((obj))
#define     obj_method(obj, method, ...)   method[obj_typeid(obj)]((obj), __VA_ARGS__)

#define     obj_printf(obj, ...)           obj_printf(obj, va(__VA_ARGS__))

#define     obj_extend(dstclass, srcclass) obj_extend(#dstclass, #srcclass)

// object: implementation details

// https://stackoverflow.com/questions/16198700/using-the-extra-16-bits-in-64-bit-pointers (note: using 19-bits here)
#define OBJBOX(ptr, payload16) (void*)(((long long unsigned)(payload16) << 48) | (long long unsigned)(ptr))
#define OBJUNBOX(ptr)          (void*)((long long unsigned)(ptr) & 0x0000FFFFFFFFFFFFull)
#define OBJPAYLOAD16(ptr)      (((long long unsigned)(ptr)) >> 48)
#define OBJPAYLOAD3(ptr)       (((long long unsigned)(ptr)) & 7)

API void* obj_initialize( void **ptr, char *type_and_info );
static threadlocal void *obj_tmpalloc;

#line 0

#line 1 "fwk_profile.h"
// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if WITH_PROFILE
#   define profile_init() do { map_init(profiler, less_str, hash_str); } while(0)
#   define profile_enable(on) do { profiler_enabled = !!(on); } while(0)
#   define profile(section) for( \
        struct profile_t *found = map_find_or_add(profiler, section "@" FILELINE, (struct profile_t){NAN} ), *dummy = (\
        found->cost = -time_us(), found); found->cost < 0; found->cost += time_us(), found->avg = found->cost * 0.25 + found->avg * 0.75)  ///+
#   define profile_incstat(name, accum) do { if(profiler) { \
        struct profile_t *found = map_find(profiler, name); \
        if(!found) found = map_insert(profiler, name, (struct profile_t){0}); \
        found->stat += accum; \
        } } while(0) ///+
#   define profile_render() if(profiler && profiler_enabled) do { \
        for(float _i = ui_begin("Profiler",0), _r; _i ; ui_end(), _i = 0) { \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( !isnan(val->stat) ) ui_slider2(va("Stat: %s", *key), (_r = val->stat, &_r), va("%.2f", val->stat)), val->stat = 0; \
            ui_separator(); \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( isnan(val->stat) ) ui_slider2(*key, (_r = val->avg/1000.0, &_r), va("%.2f ms", val->avg/1000.0)); \
        } } while(0)
struct profile_t { double stat; int32_t cost, avg; }; ///-
typedef map(char *, struct profile_t) profiler_t; ///-
extern API profiler_t profiler; ///-
extern API int profiler_enabled; ///-
#else
#   define profile_init()               do {} while(0)
#   define profile_enable(x)            do {} while(0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile(...)                 if(1) // for(int _p = 1; _p; _p = 0)
#   define profile_render()
#endif
#line 0

#line 1 "fwk_render.h"
// -----------------------------------------------------------------------------
// naive rendering framework
// - rlyeh, public domain
//
// IQM skeletal meshes by @lsalzman (public domain) - https://bit.ly/2OQh0Me
// SH code by @ands (public domain) - https://github.com/ands/spherical_harmonics_playground
// SHM code by @jarikomppa (unlicensed) - https://github.com/jarikomppa/shadertoolkit

typedef unsigned handle; // GLuint

// -----------------------------------------------------------------------------
// colors

API unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
API unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a );
API unsigned rgbaf( float r, float g, float b, float a );
API unsigned bgraf( float b, float g, float r, float a );
API float    alpha( unsigned rgba );

#define RGBX(rgb,x)   ( ((rgb)&0xFFFFFF) | (((unsigned)(x))<<24) )
#define RGB3(r,g,b)   ( ((r)<<16) | ((g)<<8) | (b) )
#define RGB4(r,g,b,a) RGBX(RGB3(r,g,b),a)

#define BLACK   RGBX(0x000000,255)
#define RED     RGBX(0xFF004D,255)
#define GREEN   RGBX(0x00B543,255)
#define BLUE    RGBX(0x065AB5,255)
#define ORANGE  RGBX(0xFF6C24,255)
#define CYAN    RGBX(0x29ADFF,255)
#define PURPLE  RGBX(0x7E2553,255)
#define YELLOW  RGBX(0xFFEC27,255)
#define WHITE   RGBX(0xFFF1E8,255)
#define GRAY    RGBX(0x725158,255)

// -----------------------------------------------------------------------------
// images

/// flags when constructing the image_t type. see: image, image_from_mem
/// IMAGE_R: 1-channel image (R)
/// IMAGE_RG: 2-channel image (R,G)
/// IMAGE_RGB: 3-channel image (R,G,B)
/// IMAGE_RGBA: 4-channel image (R,G,B,A)
/// IMAGE_FLIP: Flip image vertically
/// IMAGE_FLOAT: Float pixel components
enum IMAGE_FLAGS {
    IMAGE_R     = 0x01000,
    IMAGE_RG    = 0x02000,
    IMAGE_RGB   = 0x04000,
    IMAGE_RGBA  = 0x08000,
    IMAGE_FLIP  = 0x10000,
    IMAGE_FLOAT = 0x20000,
};

/// type that holds linear uncompressed bitmap of any given dimensions.
/// w,h: image dimensions in pixels. `x,y` alias.
/// comps: number of components per pixel. `n` alias.
/// pixels: untyped pointer to linear bitmap data. typed pointers use `pixels8/16/32/f` aliases.
/// see: texture_t
typedef struct image_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned n, comps; };
    union { void *pixels; uint8_t *pixels8; uint16_t *pixels16; uint32_t *pixels32; float *pixelsf; };
} image_t;

API image_t image(const char *pathfile, int flags);
API image_t image_from_mem(const void *ptr, int len, int flags);
API void    image_destroy(image_t *img);

// -----------------------------------------------------------------------------
// textures

enum TEXTURE_FLAGS {
    // UNIT[0..7]

    TEXTURE_BC1 = 8,  // DXT1, RGB with 8:1 compression ratio (+ optional 1bpp for alpha)
    TEXTURE_BC2 = 16, // DXT3, RGBA with 4:1 compression ratio (BC1 for RGB + 4bpp for alpha)
    TEXTURE_BC3 = 32, // DXT5, RGBA with 4:1 compression ratio (BC1 for RGB + BC4 for A)
//  TEXTURE_BC4,     // Alpha

    TEXTURE_NEAREST = 0,
    TEXTURE_LINEAR = 64,
    TEXTURE_MIPMAPS = 128,

    TEXTURE_CLAMP = 0,
    TEXTURE_BORDER = 0x100,
    TEXTURE_REPEAT = 0x200,

    TEXTURE_BYTE = 0,
    TEXTURE_FLOAT = IMAGE_FLOAT,

    TEXTURE_COLOR = 0,
    TEXTURE_DEPTH = 0x800,

    TEXTURE_R = IMAGE_R,
    TEXTURE_RG = IMAGE_RG,
    TEXTURE_RGB = IMAGE_RGB,
    TEXTURE_RGBA = IMAGE_RGBA,
    TEXTURE_FLIP = IMAGE_FLIP,

    // @fixme
    TEXTURE_SRGB = 1 << 24,
    TEXTURE_BGR = 1 << 25,
    TEXTURE_ARRAY = 1 << 26,
};

typedef struct texture_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned z, d; };
    union { unsigned n, bpp; };
    handle id, unit;
    unsigned flags;
    char* filename;
    bool transparent;
} texture_t;

API texture_t texture_compressed(const char *filename, unsigned flags);
API texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags);

API texture_t texture(const char* filename, int flags);
API texture_t texture_from_mem(const void* ptr, int len, int flags);
API texture_t texture_create(unsigned w, unsigned h, unsigned n, void *pixels, int flags);
API texture_t texture_checker();
API void      texture_destroy(texture_t *t);
// textureLod(filename, dir, lod);
// void texture_add_loader( int(*loader)(const char *filename, int *w, int *h, int *bpp, int reqbpp, int flags) );
API unsigned  texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, void *pixels, int flags);

// -----------------------------------------------------------------------------
// brdf

API texture_t brdf_lut();

// -----------------------------------------------------------------------------
// pbr materials

typedef struct colormap_t {
    vec4 color;
    texture_t *texture;
} colormap_t;

API bool colormap( colormap_t *cm, const char *pbr_material_type, bool load_as_srgb );

typedef struct pbr_material_t {
    char* name;
    colormap_t diffuse;
    colormap_t normals;
    colormap_t specular;
    colormap_t albedo;
    colormap_t roughness;
    colormap_t metallic;
    colormap_t ao;
    colormap_t ambient;
    colormap_t emissive;

    float specular_shininess;
} pbr_material_t;

API bool pbr_material(pbr_material_t *pbr, const char *material);
API void pbr_material_destroy(pbr_material_t *m);

// -----------------------------------------------------------------------------
// fullscreen quads

API void  fullscreen_rgb_quad( texture_t texture_rgb, float gamma );
API void  fullscreen_ycbcr_quad( texture_t texture_YCbCr[3], float gamma );

// -----------------------------------------------------------------------------
// sprites

// texture id, position(x,y,depth sort), tint color, rotation angle
API void tile( texture_t texture, float position[3], float rotation /*0*/, uint32_t color /*~0u*/);

// texture id, position(x,y,depth sort), rotation angle, offset(x,y), scale(x,y), is_additive, tint color, spritesheet(frameNumber,X,Y) (frame in a X*Y spritesheet)
API void sprite( texture_t texture, float position[3], float rotation, float offset[2], float scale[2], int is_additive, uint32_t rgba, float spritesheet[3]);

API void sprite_flush();

// -----------------------------------------------------------------------------
// cubemaps

typedef struct cubemap_t {
    unsigned id;    // texture id
    vec3 sh[9];     // precomputed spherical harmonics coefficients
} cubemap_t;

API cubemap_t  cubemap( const image_t image, int flags ); // 1 equirectangular panorama
API cubemap_t  cubemap6( const image_t images[6], int flags ); // 6 cubemap faces
API void       cubemap_destroy(cubemap_t *c);
API cubemap_t* cubemap_get_active();

// -----------------------------------------------------------------------------
// fbos

API unsigned fbo( unsigned texture_color, unsigned texture_depth, int wr_flags );
API void     fbo_bind(unsigned id);
API void     fbo_unbind();
API void     fbo_destroy(unsigned id);

// -----------------------------------------------------------------------------
// shadowmaps

typedef struct shadowmap_t {
    mat44  shadowmatrix;
    mat44  mvp;
    mat44  mv;
    mat44  proj;
    vec4   light_position;
    int    saved_fb;
    int    saved_viewport[4];
    handle fbo, texture;
    int texture_width;
} shadowmap_t;

API shadowmap_t shadowmap(int texture_width); // = 1024
API void shadowmap_destroy(shadowmap_t *s);

API void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection);
API void shadowmap_begin(shadowmap_t *s);
API void shadowmap_end(shadowmap_t *s);

// shadowmap utils

API void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar);
API void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar);

// -----------------------------------------------------------------------------
// shaders

API unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor);
API unsigned shader_bind(unsigned program);
API     void shader_bool(const char *uniform, bool i );
API     void shader_int(const char *uniform, int i);
API     void shader_uint(const char *uniform, unsigned i );
API     void shader_float(const char *uniform, float f);
API     void shader_vec2(const char *uniform, vec2 v);
API     void shader_vec3(const char *uniform, vec3 v);
API     void shader_vec4(const char *uniform, vec4 v);
API     void shader_mat44(const char *uniform, mat44 m);
API     void shader_texture(const char *sampler, texture_t texture);
API     void shader_texture_unit(const char *sampler, unsigned texture, unsigned unit);
API     void shader_colormap(const char *name, colormap_t cm);
API unsigned shader_get_active();
API void     shader_destroy(unsigned shader);


// -----------------------------------------------------------------------------
// meshes (@fixme: deprecate?)

enum MESH_FLAGS {
    MESH_STATIC = 0, // STATIC, DYNAMIC, STREAM // zero|single|many updates per frame
    MESH_STREAM = 1,
    MESH_TRIANGLE_STRIP = 2,
};

typedef struct mesh_t {
    handle vao, vbo, ibo;
    unsigned vertex_count;
    unsigned index_count;
    unsigned flags;
} mesh_t;

API mesh_t mesh_create(const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
API   void mesh_upgrade(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
API   void mesh_render(mesh_t *m);
API   void mesh_destroy(mesh_t *m);
API   aabb mesh_bounds(mesh_t *m);

// -----------------------------------------------------------------------------
// materials

enum MATERIAL_ENUMS {
    MAX_CHANNELS_PER_MATERIAL = 8
};

typedef struct material_t {
    char *name;

    int count;
    struct material_layer_t {
        char   texname[32];
        handle texture;
        float  value;
        vec4   color; // uint32_t
    } layer[MAX_CHANNELS_PER_MATERIAL];

} material_t;

// -----------------------------------------------------------------------------
// models

enum MODEL_FLAGS {
    MODEL_NO_ANIMATIONS = 1,
    MODEL_NO_MESHES = 2,
    MODEL_NO_TEXTURES = 4,
    MODEL_MATCAPS = 8,
};

typedef struct model_t {
    struct iqm_t *iqm; // private

    unsigned num_textures;
    handle *textures;
    char **texture_names;
    array(material_t) materials;

    unsigned num_meshes;
    unsigned num_triangles;
    unsigned num_joints; // num_poses;
    unsigned num_anims;
    unsigned num_frames;
    handle program;
    float curframe;
    mat44 pivot;

    int stride; // usually 60 bytes (12*4+4*3) for a p3 u2 n3 t4 i4B w4B c4B vertex stream
    void *verts;
    int num_verts;
    handle vao, ibo, vbo, vao_instanced;

    unsigned flags;
    unsigned billboard;

    float *instanced_matrices;
    unsigned num_instances;
} model_t;

API model_t  model(const char *filename, int flags);
API model_t  model_from_mem(const void *mem, int sz, int flags);
API float    model_animate(model_t, float curframe);
API float    model_animate_clip(model_t, float curframe, int minframe, int maxframe, bool loop);
API aabb     model_aabb(model_t, mat44 transform);
API void     model_render(model_t, mat44 proj, mat44 view, mat44 model, int shader);
API void     model_render_skeleton(model_t, mat44 model);
API void     model_render_instanced(model_t, mat44 proj, mat44 view, mat44 *models, int shader, unsigned count);
API void     model_set_texture(model_t, texture_t t);
API bool     model_get_bone_pose(model_t m, unsigned joint, mat34 *out);
API void     model_destroy(model_t);

// -----------------------------------------------------------------------------
// skyboxes

typedef struct skybox_t {
    handle program;
    mesh_t geometry;
    cubemap_t cubemap;
    int flags;
} skybox_t;

API skybox_t skybox(const char *panorama_or_cubemap_folder, int flags);
API int      skybox_push_state(skybox_t *sky, mat44 proj, mat44 view);
API int      skybox_pop_state();
API void     skybox_destroy(skybox_t *sky);

// -----------------------------------------------------------------------------
// post-fxs

API void     viewport_color(uint32_t color); // background(uint32_t) instead?
API void     viewport_color3(vec3 color); // background3(vec3) instead?
API void     viewport_clear(bool color, bool depth);
API void     viewport_clip(vec2 from, vec2 to);

API void     fx_load(const char *file);
API void     fx_begin();
API void     fx_end();
API void     fx_enable(int pass, int enabled);
API int      fx_enabled(int pass);
API void     fx_enable_all(int enabled);
API char *   fx_name(int pass);

// -----------------------------------------------------------------------------
// utils

API void*    screenshot(unsigned components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
API void*    screenshot_async(unsigned components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
#line 0

#line 1 "fwk_renderdd.h"
// -----------------------------------------------------------------------------
// debugdraw framework
// - rlyeh, public domain.
//
// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
// [x] arrow, point, text, capsule, aabb, plane, flotilla-style locator, boid,
// [x] line batching
// [*] line width and stipple
// [*] (proper) gizmo,
// [ ] bone (pyramid? two boids?), ring,
// [ ] camera, light bulb, light probe,

API void ddraw_color(unsigned rgb);
API void ddraw_color_push(unsigned rgb);
API void ddraw_color_pop();
//
API void ddraw_ontop_push(int enabled);
API void ddraw_ontop(int enabled);
API void ddraw_ontop_pop();
//
API void ddraw_aabb(vec3 minbb, vec3 maxbb);
API void ddraw_aabb_corners(vec3 minbb, vec3 maxbb);
API void ddraw_arrow(vec3 begin, vec3 end);
API void ddraw_axis(float units);
API void ddraw_boid(vec3 pos, vec3 dir);
API void ddraw_bone(vec3 center, vec3 end); // @todo: use me
API void ddraw_bounds(const vec3 points[8]);
API void ddraw_box(vec3 c, vec3 extents);
API void ddraw_capsule(vec3 from, vec3 to, float radius);
API void ddraw_circle(vec3 pos, vec3 n, float radius);
API void ddraw_cone(vec3 center, vec3 top, float radius);
API void ddraw_cube(vec3 center, float radius);
API void ddraw_cube33(vec3 center, vec3 radius, mat33 M);
API void ddraw_diamond(vec3 from, vec3 to, float size);
API void ddraw_frustum(float projview[16]);
API void ddraw_ground(float scale);
API void ddraw_grid(float scale);
API void ddraw_hexagon(vec3 pos, float radius);
API void ddraw_line(vec3 from, vec3 to);
API void ddraw_line_dashed(vec3 from, vec3 to);
API void ddraw_line_thin(vec3 from, vec3 to);
API void ddraw_normal(vec3 pos, vec3 n);
API void ddraw_pentagon(vec3 pos, float radius);
API void ddraw_plane(vec3 p, vec3 n, float scale);
API void ddraw_point(vec3 from);
API void ddraw_position(vec3 pos, float radius);
API void ddraw_position_dir(vec3 pos, vec3 dir, float radius);
API void ddraw_pyramid(vec3 center, float height, int segments);
API void ddraw_cylinder(vec3 center, float height, int segments);
API void ddraw_sphere(vec3 pos, float radius);
API void ddraw_square(vec3 pos, float radius);
API void ddraw_text(vec3 pos, float scale, const char *text);
API void ddraw_text2d(vec2 pos, float scale, const char *text);
API void ddraw_triangle(vec3 p1, vec3 p2, vec3 p3);
//
API void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments);
//
API void ddraw_demo();
API void ddraw_flush();
#line 0

#line 1 "fwk_scene.h"
// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t {
    mat44 view, proj;
    vec3 position, up, look; // position, updir, lookdir
    float yaw, pitch, speed; // mirror_x, mirror_y;
    vec3 last_look, last_move; // used for friction and smoothing
} camera_t;

API camera_t camera();
API void camera_move(camera_t *cam, float x, float y, float z);
API void camera_fps(camera_t *cam, float yaw, float pitch);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API camera_t *camera_get_active();

// object

typedef struct object_t {
    uint64_t renderbucket;
    mat44 transform;
    quat rot;
    vec3 sca, pos, euler, pivot;
    array(handle) textures;
    model_t model;
    aabb bounds;
    unsigned billboard; // [0..7] x(4),y(2),z(1) masks
} object_t;

API object_t object();
API void object_rotate(object_t *obj, vec3 euler);
API void object_pivot(object_t *obj, vec3 euler);
API void object_teleport(object_t *obj, vec3 pos);
API void object_move(object_t *obj, vec3 inc);
API vec3 object_position(object_t *obj);
API void object_scale(object_t *obj, vec3 sca);
//
API void object_model(object_t *obj, model_t model);
API void object_diffuse(object_t *obj, texture_t tex);
API void object_diffuse_push(object_t *obj, texture_t tex);
API void object_diffuse_pop(object_t *obj);
API void object_billboard(object_t *obj, unsigned mode);

// object_pose(transform); // @todo

// scene

enum SCENE_FLAGS {
    SCENE_WIREFRAME = 1,
    SCENE_CULLFACE = 2,
    SCENE_BACKGROUND = 4,
    SCENE_FOREGROUND = 8,
};

typedef struct scene_t {
    handle program;

    array(object_t) objs;

    // special objects below:
    skybox_t skybox;
    int u_coefficients_sh;
} scene_t;

API scene_t*  scene_push();
API void      scene_pop();
API scene_t*  scene_get_active();

API int       scene_merge(const char *source);
API void      scene_render(int flags);

API object_t* scene_spawn();
API unsigned  scene_count();
API object_t* scene_index(unsigned index);
#line 0

#line 1 "fwk_script.h"
// -----------------------------------------------------------------------------
// script framework
// - rlyeh, public domain

API void script_init();
API void script_run(const char *script);
API void script_runfile(const char *pathfile);

API void script_bind_class(const char *objname, int num_methods, const char **c_names, void **c_functions);
API void script_bind_function(const char *c_name, void *c_function);
API void script_call(const char *lua_function);
#line 0

#line 1 "fwk_string.h"
// string framework
// - rlyeh, public domain

// string: temporary api (stack)
API char*   tempvl(const char *fmt, va_list);
API char*   tempva(const char *fmt, ...);
#define     va(...) ((printf || printf(__VA_ARGS__), tempva(__VA_ARGS__)))  // vs2015 check trick

// string: allocated api (heap). FREE() after use
API char*   strcatf(char **s, const char *buf);
#define     strcatf(s,fmt,...)  strcatf((s), va(fmt, __VA_ARGS__))
#define     stringf(fmt,...)    STRDUP(va(fmt, __VA_ARGS__)) // (strcatf)(0, va(fmt, __VA_ARGS__))


#if defined _MSC_VER || (defined __TINYC__ && defined _WIN32)
#if!defined _MSC_VER
char* strtok_s(char* str,const char* delimiters,char** context); // tcc misses this in <string.h>
#endif
#define strtok_r strtok_s
#endif

#define each_substring(str, delims, keyname) \
    ( int len_ = strlen(str) + 1; len_; len_ = 0 ) \
    for( char buf_[1024], *ptr_ = len_ < 1024 ? buf_ : REALLOC(0, len_), *lit_ = (char*)(str), *_bak = (snprintf(ptr_, len_, "%s", lit_), ptr_); _bak; _bak = 0, (ptr_ == buf_ ? 0 : REALLOC(ptr_, 0)) ) \
    for( char *next_token = 0, *keyname = strtok_r(_bak, delims, &next_token); keyname; keyname = strtok_r(NULL, delims, &next_token) )

// utils

API int          strmatch(const char *s, const char *wildcard);

API int          strcmp_qsort(const void *a, const void *b);
API int          strcmpi_qsort(const void *a, const void *b);

API bool         strbeg(const char *src, const char *sub); // returns true if both strings match at beginning. case sensitive
API bool         strend(const char *src, const char *sub); // returns true if both strings match at end. case sensitive

API bool         strbegi(const char *src, const char *sub);  // returns true if both strings match at beginning. case insensitive
API bool         strendi(const char *src, const char *sub);  // returns true if both strings match at end. case insensitive
API const char * strstri(const char *src, const char *sub);  // returns find first substring in string. case insensitive.
#define          strcmpi  ifdef(cl, _stricmp, strcasecmp)

API char *       strupper(const char *str);
API char *       strlower(const char *str);

API char *       strrepl(char **copy, const char *target, const char *replace); // replace any 'target' as 'repl' in 'copy'. 'copy' may change (heap). returns 'copy'
API char *       strswap(char *copy, const char *target, const char *replace);  // replaced inline only if repl is shorter than target. no allocations.
API char *       strcut(char *copy, const char *target);                        // remove any 'target' in 'copy'. returns 'copy'

API char *       str16to8(const wchar_t *str); // convert from wchar16(win) to utf8/ascii

API const char * strlerp(unsigned numpairs, const char **pairs, const char *str); // using key-value pairs, null-terminated

#ifndef __APPLE__ // BSD provides these
API size_t       strlcat(char *dst, const char *src, size_t dstcap); // concat 2 strings safely. always NUL terminates. may truncate.
API size_t       strlcpy(char *dst, const char *src, size_t dstcap); // copy 2 strings safely. always NUL terminates. truncates if retval>=dstcap
#endif

/// split `string` after any of `delimiters` character is found.
/// returns temporary array of split strings. see: strjoin
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
API array(char*) strsplit(const char *string, const char *delimiters);

/// concatenate all elements within `list`, with `separator` string in between.
/// returns: temporary joint string. see: strsplit
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
/// > char *joint = strjoin(tokens, "+"); // joint="hello+world"
API char*        strjoin(array(char*) list, const char *separator);


API array(uint32_t) strutf32( const char *utf8 );
#line 0

#line 1 "fwk_system.h"
// -----------------------------------------------------------------------------
// system framework utils
// - rlyeh, public domain.
//
// Note: Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// Note: If you are linking your binary using GNU ld you need to add --export-dynamic

API int         argc();
API char*       argv(int);

API int         flag(const char *commalist); // --arg
API const char* option(const char *commalist, const char *defaults); // --arg=key or --arg key
API int         optioni(const char *commalist, int defaults); // argvi() ?
API float       optionf(const char *commalist, float defaults);

// @todo:
// ini(file, key, default);
// inif(file, key, default);

API char*       os_exec_output(); // legacy
API int         os_exec(const char *command); // legacy
#define         os_exec(...) os_exec(file_normalize(va(__VA_ARGS__))) // legacy
API char*       os_exec_(int *retvalue, const char *command); // new
#define         os_exec_(rc, ...) os_exec(rc, file_normalize(va(__VA_ARGS__))) // new

API void        tty_color(unsigned color);
API void        tty_reset();

API int         cpu_cores();

// return battery level [1..100]. also positive if charging (+), negative if discharging (-), and 0 if no battery is present.
API int         battery();

API const char* app_name();
API const char* app_path();
API const char* app_cache();
API const char* app_temp();
API void        app_reload();

API uint64_t    date();       // YYYYMMDDhhmmss
API uint64_t    date_epoch(); // linux epoch
API double      time_hh();
API double      time_mm();
API double      time_ss();
API uint64_t    time_ms();
API uint64_t    time_us();
API uint64_t    time_ns();
API void        sleep_ss(double ss);
API void        sleep_ms(double ms);
API void        sleep_us(double us);
API void        sleep_ns(double us);

API char*       callstack( int traces ); // write callstack into a temporary string. do not delete it.
API int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

API void        die(const char *message);
API void        alert(const char *message);
API void        hexdump( const void *ptr, unsigned len );
API void        hexdumpf( FILE *fp, const void *ptr, unsigned len, int width );
API void        breakpoint(const char *reason);
API bool        has_debugger();

API uint16_t    lil16(uint16_t n); // swap16 as lil
API uint32_t    lil32(uint32_t n); // swap32 as lil
API float       lil32f(float n);   // swap32 as lil
API uint64_t    lil64(uint64_t n); // swap64 as lil
API double      lil64f(double n);  // swap64 as lil
API uint16_t    big16(uint16_t n); // swap16 as big
API uint32_t    big32(uint32_t n); // swap32 as big
API float       big32f(float n);   // swap32 as big
API uint64_t    big64(uint64_t n); // swap64 as big
API double      big64f(double n);  // swap64 as big

API uint16_t*   lil16p(void *n, int sz);
API uint32_t*   lil32p(void *n, int sz);
API float*      lil32pf(void *n, int sz);
API uint64_t*   lil64p(void *n, int sz);
API double*     lil64pf(void *n, int sz);
API uint16_t*   big16p(void *n, int sz);
API uint32_t*   big32p(void *n, int sz);
API float*      big32pf(void *n, int sz);
API uint64_t*   big64p(void *n, int sz);
API double*     big64pf(void *n, int sz);

#define PANIC(...)   PANIC(va(__VA_ARGS__), __FILE__, __LINE__) // die() ?
API int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);
API int (PANIC)(const char *error, const char *file, int line);
#line 0

#line 1 "fwk_ui.h"
// -----------------------------------------------------------------------------
// immediate ui framework
// - rlyeh, public domain
//
// @todo: logger/console

API int  ui_begin(const char *title, int flags);
API int    ui_int(const char *label, int *value);
API int    ui_bool(const char *label, bool *value);
API int    ui_short(const char *label, short *value);
API int    ui_float(const char *label, float *value);
API int    ui_float2(const char *label, float value[2]);
API int    ui_float3(const char *label, float value[3]);
API int    ui_string(const char *label, char *buffer, int buflen);
API int    ui_color3(const char *label, float *color3); //[0..255]
API int    ui_color3f(const char *label, float *color3); //[0..1]
API int    ui_color4(const char *label, float *color4); //[0..255]
API int    ui_color4f(const char *label, float *color4); //[0..1]
API int    ui_button(const char *label);
API int    ui_toggle(const char *label, bool *value);
API int    ui_dialog(const char *title, const char *text, int choices, bool *show); // @fixme: return
API int    ui_list(const char *label, const char **items, int num_items, int *selector);
API int    ui_radio(const char *label, const char **items, int num_items, int *selector);
API int    ui_image(const char *label, handle id, unsigned w, unsigned h); //(w,h) can be 0
API int    ui_colormap(const char *map_name, colormap_t *cm); // returns num member changed: 1 for color, 2 for texture map
API int    ui_separator();
API int    ui_bits8(const char *label, uint8_t *bits);
API int    ui_bits16(const char *label, uint16_t *bits);
API int    ui_clampf(const char *label, float *value, float minf, float maxf);
API int    ui_label(const char *label);
#define    ui_labelf(...) ui_label(va(__VA_ARGS__))
API int    ui_label2(const char *label, const char *caption);
API int    ui_slider(const char *label, float *value);
API int    ui_slider2(const char *label, float *value, const char *caption);
API int    ui_const_bool(const char *label, const double value);
API int    ui_const_float(const char *label, const double value);
API int    ui_const_string(const char *label, const char *value);
#define    ui_const_stringf(label, ...) ui_const_string(label, va(__VA_ARGS__))
API void ui_end();

API int  ui_menu(const char *items); // semicolon- or comma-separated items
API int  ui_item();

API int  ui_hover(); // ui_is_hover()?
API int  ui_active(); // ui_is_active()?
API void ui_demo();
#line 0

#line 1 "fwk_video.h"
// -----------------------------------------------------------------------------
// video decoder (mpeg)
// - rlyeh, public domain
//
// [ref] https://github.com/phoboslab/pl_mpeg/blob/master/pl_mpeg_player.c
// [use] ffmpeg -i infile.mp4 -c:v mpeg1video -c:a mp2 -format mpeg outfile.mpg

enum VIDEO_FLAGS {
    VIDEO_YCBCR = 0,
    VIDEO_RGB = 1,

    VIDEO_AUDIO = 0,
    VIDEO_NO_AUDIO = 2,
};

typedef struct video_t video_t;

API video_t*   video( const char *filename, int flags );
API texture_t*  video_decode( video_t *v ); // decodes next frame, returns associated texture(s)
API texture_t*  video_textures( video_t *v ); // returns last video textures. does not perform any decoding.
API int         video_has_finished(video_t *v);
API double      video_duration(video_t *v);
API int         video_seek(video_t *v, double seek_to);
API double      video_position(video_t *v);
API void        video_pause(video_t *v, bool paused);
API bool        video_is_paused(video_t *v);
API void       video_destroy( video_t *v );

// -----------------------------------------------------------------------------
// video recorder (uses external ffmpeg and fallbacks to built-in mpeg1 encoder)
// - rlyeh, public domain
//
// @fixme: MSAA can cause some artifacts with PBOs: either use glDisable(GL_MULTISAMPLE) before recording or do not create window with WINDOW_MSAA at all.

API void       videorec_start(const char *outfile_mp4);
API bool        videorec_active();
API void       videorec_stop(void);
#line 0

#line 1 "fwk_window.h"
// -----------------------------------------------------------------------------
// window framework
// - rlyeh, public domain
//
// @todo: window_cursor(ico);
// @todo: if WINDOW_PORTRAIT && exist portrait monitor, use that instead of primary one
// @todo: WINDOW_TRAY

enum WINDOW_FLAGS {
    WINDOW_MSAA2 = 0x02,
    WINDOW_MSAA4 = 0x04,
    WINDOW_MSAA8 = 0x08,

    WINDOW_SQUARE = 0x20,
    WINDOW_PORTRAIT = 0x40,
    WINDOW_LANDSCAPE = 0x80,
    WINDOW_FIXED = 0x100,

    WINDOW_VSYNC = 0,
    WINDOW_VSYNC_ADAPTIVE = 0x1000,
    WINDOW_VSYNC_DISABLED = 0x2000,
};

API void     window_create(float zoom, int flags);
API void     window_title(const char *title);
API void     window_icon(const char *file_icon);
API void     window_flush();
API int      window_swap();
API void*    window_handle();

API uint64_t window_frame();
API int      window_width();
API int      window_height();
API double   window_aspect();
API double   window_time();
API double   window_delta();
API double   window_fps();

API bool     window_hook(void (*func)(), void* userdata);
API void     window_unhook(void (*func)());

API void     window_focus(); // window attribute api using haz catz language for now
API int      window_has_focus();
API void     window_fullscreen(int enabled);
API int      window_has_fullscreen();
API void     window_cursor(int visible);
API int      window_has_cursor();
API void     window_pause();
API int      window_has_pause();
API void     window_visible(int visible);
API int      window_has_visible();

API void     window_lock_aspect(unsigned numer, unsigned denom);
API void     window_unlock_aspect();
API void     window_lock_fps(float fps);
API void     window_unlock_fps();

API void     window_screenshot(const char* filename_png);
#line 0

// ----

#ifdef __cplusplus
} // extern "C"
#endif

// for glad
#ifdef _WIN32
//#define GLAD_API_CALL API
#endif
#include "fwk"

#endif // FWK_H
