// # tst ######################################################################
// @todo: measure time with overrides as well, so it works as benchmark.
// double (*benchmark)() = 0;
// unit() { now = benchmark(); }
// test() { ... }

int unit(const char *name);
int test(int expr);

#ifdef TEST_C
#pragma once
#include <stdio.h>

static __thread int right = 0, wrong = 0;
static __thread const char *suite = "";

int unit(const char *name) {
    suite = name;
    right = wrong = 0;
    return 1;
}
int test(int expr) {
    right = right + !!expr;
    wrong = wrong +  !expr;

    char buffer[1024];
    sprintf(buffer, "%s%c Unit '%s': test %d/%d %s", // (%5.2f%%)",
        !expr ? "!" : "",
        "NY"[expr && !wrong],
        suite,
        right, right+wrong,
        !expr ? "FAILED" : "passed"
        //,100.f * right / (right+wrong)
    );

#if 1
    fprintf(stderr, "%s\n", buffer[0] == '!' ? buffer+1 : buffer);
    fflush(stderr);
#else
    LOG(TEST, "%s", buffer);
#endif

    return !!expr;
}

#ifdef TEST_DEMO
int main() {
    if( unit( "1st unit" ) ) {
        test( 1 < 4 );
        test( 2 < 4 );
        test( 3 < 4 );
    }
    if( unit( "2nd unit") ) {
        test( 10 < 10 ); // <-- shall fail here
    }
    if( unit( "3rd unit" ) ) {
        test( 1 < 2 );
    }
}
#define main main__ 
#endif // TEST_DEMO
#endif // TEST_C
