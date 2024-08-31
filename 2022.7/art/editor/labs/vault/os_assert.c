// [x] assert: which works in release builds.
// [x] test: test & autotest macros.
// - rlyeh, public domain

// ----------------------------------------------------------------------------

#ifndef ASSERT_H
#define ASSERT_H <assert.h>
#endif

#ifndef NDEBUG
#   include ASSERT_H
#else
#   undef NDEBUG
#   include ASSERT_H
#   define NDEBUG
#endif

#ifdef SHIPPING
#   undef  assert
#   define assert(expr) (void)0
#endif

// ----------------------------------------------------------------------------

#ifndef TEST_H
#define TEST_H

static __thread int tests = 0, fails = 0;

#ifdef NDEBUG
#define TEST(...) (void)0
#define AUTOTEST  static void AUTORUN_U(unused_test)(void)
#else
#define TEST(...) (printf(" #%02d (%s) %s:%d ", ++tests, #__VA_ARGS__, __FILE__, __LINE__), printf("\r%c\n", "NY"[!!(__VA_ARGS__)]))
#define AUTOTEST  AUTORUN
#endif

#endif
