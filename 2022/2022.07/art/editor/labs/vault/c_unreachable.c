// - rlyeh, public domain

#pragma once

#ifdef DEBUG
#define UNREACHABLE() assert(!"This line was not meant to be reached")
#elif defined( _MSC_VER )
#define UNREACHABLE() __assume(0)
#elif defined __GNUC__ // && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE()
#endif
