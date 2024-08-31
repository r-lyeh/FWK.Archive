// [x] if/n/def hell + system headers: here, rather than in every header.
// - rlyeh, public domain

#ifndef OS_H
#define OS_H

#  if defined __ANDROID_API__
#define _AND
#elif defined __APPLE__
#define _OSX
#elif defined __FreeBSD__
#define _BSD
#elif defined _WIN32
#define _WIN
#else
#define _LIN
#endif

#  if defined _MSC_VER
#define _MSC
#elif defined __clang__
#define _CLA
#elif defined __TINYC__
#define _TCC
#else
#define _GCC
#endif

#ifdef _AND
#include <dlfcn.h>
#include <malloc.h>
size_t dlmalloc_usable_size(void*); // (android)
#endif

#ifdef _BSD
#include <dlfcn.h>
#include <malloc/malloc.h>
#endif

#ifdef _IOS
#include <mach-o/dyld.h>
#include <malloc/malloc.h>
#endif

#ifdef _LIN
#include <dlfcn.h>
#include <malloc.h>
#endif

#ifdef _OSX
#include <mach-o/dyld.h>
#include <malloc/malloc.h>
#endif

#ifdef _WIN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // for CONDITION_VARIABLE
#endif
#ifdef _TCC
#include <io.h>
#include <windows.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include <malloc.h>
#endif

// augment vendor compatibility

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS // <stdint.h>: UINT32_MAX...
#endif

// all required standard headers

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// builtins

#if !defined __thread && (defined _MSC || defined _TCC)
#define __thread __declspec(thread)
#endif

#if !defined inline && defined _MSC && !defined __cplusplus
#define inline _inline
#endif

// autorun initializers for C
// - rlyeh, public domain
//
// note: based on code by Joe Lowe (public domain).
// note: XIU for C initializers, XCU for C++ initializers, XTU for C deinitializers

#ifndef AUTORUN
    #ifdef __cplusplus
    #define AUTORUN \
        static void AUTORUN_U(autorun)(void); \
        static const int AUTORUN_J(AUTORUN_U(autorun),__1) = (AUTORUN_U(autorun)(), 1); \
        static void AUTORUN_U(autorun)(void)
    #elif defined _MSC
    #define AUTORUN \
        static void AUTORUN_U(autorun)(void); \
        static int AUTORUN_J(AUTORUN_U(autorun),__1) (){ AUTORUN_U(autorun)(); return 0; } \
        __pragma(section(".CRT$XIU", long, read)) \
        __declspec(allocate(".CRT$XIU")) \
        static int(* AUTORUN_J(AUTORUN_U(autorun),__2) )() = AUTORUN_J(AUTORUN_U(autorun),__1); \
        static void AUTORUN_U(autorun)(void)
    #else
    #define AUTORUN \
        __attribute__((constructor)) \
        static void AUTORUN_U(autorun)(void)
    #endif

    // helpers: join + unique macros

    #define AUTORUN_j(a, b) a##b
    #define AUTORUN_J(a, b) AUTORUN_j(a, b)
    #define AUTORUN_U(x)    AUTORUN_J(x, __LINE__)
#endif

#ifdef AUTORUN_DEMO
    #include <stdio.h>
    #include <stdlib.h>
    int main() {
        puts("during main()");
    }
    void my_quit(void) {
        puts("after main()");
    }
    AUTORUN {
        puts("before main() [1]");
    }
    AUTORUN {
        puts("before main() [2]");
        atexit( my_quit );
    }
    #define main main__
#endif

#endif // OS_H

// -----------------------------------------------------------------------------

#ifdef OS_C
#define MEMORY_C
#define DLL_C
#define PLUGIN_C
#define TIME_C
#define TRACE_C
#define PANIC_C
#define ASSERT_C
#define MIME_C
#define FILE_C
#define EXEC_C
#define ICON_C
#define TRAY_C
#define SINGLETON_C
#define ANSI_C
#define ENTROPY_C
#define INI_C
#define TITLE_C
#define ARGS_C
#define EXIT_C
#define ENV_C
#define BREAKPOINT_C
#define DIALOG_C
#define DIE_C
#define CPU_C
#define DATE_C
#define TRAP_C
#define TEST_C
#define LOGGER_C
#define LOGGER2_C
#define LOGGER3_C
#define LOCALE_C
#endif

#include "os_memory.c"
#include "os_assert.c"
#include "os_dll.c"
#include "os_plugin.c"
#include "os_time.c"
#include "os_logger.c"
#include "os_logger2.c"
#include "os_trace.c" // after os_logger
#include "os_die.c"
#include "os_panic.c"
#include "os_mime.c"
#include "os_file.c"
#include "os_exec.c"
#include "os_icon.c"
#include "os_tray.c" // after os_icon
#include "os_singleton.c"
#include "os_ansi.c"
#include "os_entropy.c"
#include "os_ini.c"
#include "os_title.c"
#include "os_args.c"
#include "os_exit.c"
#include "os_env.c"
#include "os_breakpoint.c"
#include "os_dialog.c"
#include "os_cpu.c"
#include "os_date.c"
#include "os_trap.c" // after os_panic
#include "os_test.c"
#include "os_locale.c"

#include "os_logger3.c" // after os_logger2+os_trace+os_ansi
