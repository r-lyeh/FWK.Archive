// - rlyeh, public domain

#pragma once

#if __STDC_VERSION__ >= 201112L
#   ifndef _MSC_VER
#   include <stdalign.h>
#   endif
#   define ALIGNAS(bytes) alignas(bytes)
#else
#   define ALIGNAS(bytes)
#endif
