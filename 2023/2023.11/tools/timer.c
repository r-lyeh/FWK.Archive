#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <omp.h>
#define clock omp_get_wtime
#define clock_t double
#define CLOCKS_PER_SEC 1.0
#else
#include <time.h>
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

int main(int argc, char **argv) {
    if(!_isatty(fileno(stdin))) {
        clock_t t1 = clock();
        while(!feof(stdin)) putc(getc(stdin), stdout);
        clock_t t2 = clock();
        fprintf(stderr, "%.3fs\n", (t2-t1)*1.0/CLOCKS_PER_SEC);
        return 0;
    }
    if( argc > 1 ) {
        char buf[16384] = {0}; // LEAK = calloc(1, 2*1024*1024);
        for(int i = 1; i < argc; ++i) {
            strcat(buf, argv[i]);
            strcat(buf, " ");
        }
        puts(buf);
        clock_t t1 = clock();
        int rc = system(buf);
        clock_t t2 = clock();
        fprintf(stderr, "%.3fs - return code: %d\n", (t2-t1)*1.0/CLOCKS_PER_SEC, rc);
        return rc;
    }
    return 0;
}
