// portable os argc/v pairs
// - rlyeh, public domain

int    os_argc(void);
char*  os_argv(unsigned);

#ifdef ARGS_C
#pragma once
#include <stdlib.h>
#include <string.h>

// provide __argc and __argv for those systems that dont.
#  if defined __MINGW32__
#   define __argc _argc
#   define __argv _argv
#elif defined __GLIBC__
#   define __argc __libc_argc
#   define __argv __libc_argv
#elif defined __hpux
#   define __argc __argc_value
#   define __argv __argv_value
#elif defined __GNUC__ // also, clang
    int __argc;
    char **__argv;
    __attribute__((constructor)) void init_argcv(int argc, char **argv) {
        __argc = argc;
        __argv = argv;
    }
#endif

int os_argc(void) {
    return __argc;
}
char* os_argv(unsigned slot) {
    // normalize argv[0]
    static int ever = 0; if(!ever) { ever = 1;
        // remove extension
        int arg0len = strlen( __argv[0] );
        if( arg0len > 4 ) {
            char *dot = &__argv[0][arg0len - 4];
            if( 0 == strcmp( dot, ".exe" ) || 0 == strcmp( dot, ".EXE" ) ) {
                *dot = '\0';
            }
        }
        // remove path
        char *s1 = strrchr(__argv[0], '/'), *s2 = strrchr(__argv[0], '\\');
        __argv[0] = s1>s2 ? s1+1 : s2>s1 ? s2+1 : __argv[0];
    }
    return __argv[slot];
}

#ifdef ARGS_DEMO
#include <stdio.h>
int main() {
    for( int i = 0; i < os_argc(); ++i ) {
        puts(os_argv(i));
    }
}
#define main main__
#endif // ARGS_DEMO
#endif // ARGS_C
