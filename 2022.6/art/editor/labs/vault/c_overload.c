// msvc trick to expand vargs properly by Braden Steffaniak
// - see https://stackoverflow.com/a/24028231

#pragma once
#define GLUE(x, y) x y

#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5(...) EXPAND_ARGS((__VA_ARGS__, 5, 4, 3, 2, 1, 0))

#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)

#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, COUNT_ARGS_MAX5(__VA_ARGS__)), (__VA_ARGS__))

/* usage:
#define ASSERT1(expr)             assert1(expr)
#define ASSERT2(expr, errormsg)   assert2(expr, errormsg)
#define ASSERT(...) CALL_OVERLOAD(ASSERT, __VA_ARGS__)
*/
