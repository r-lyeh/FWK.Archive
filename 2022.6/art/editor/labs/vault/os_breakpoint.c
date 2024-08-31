void os_breakpoint();

#ifdef BREAKPOINT_C
#pragma once
#include <signal.h>

void os_breakpoint() {
#if _MSC_VER
    __debugbreak();    // msvc
#elif __GNUC__
    __builtin_trap();  // gcc and clang
#else
    raise(SIGTRAP);    // posix
#endif
}

#ifdef BREAKPOINT_DEMO
#include <stdio.h>
int main() {
    puts("trying to invoke debugger... (may crash if debugger is not attached)");
    os_breakpoint();
}
#endif
#endif
