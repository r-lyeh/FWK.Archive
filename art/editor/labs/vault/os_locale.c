void os_locale(void);

#ifdef LOCALE_C
#pragma once
#include <locale.h>

void os_locale(void) {
    // @graphitemaster reported speed boosts at least on Linux
    setlocale(LC_ALL, "C");
}

#endif
