#pragma once
#define ONCE static int once##__LINE__=1; for(;once##__LINE__;once##__LINE__=0)

/*
#define once_unique(a) once_join(a, __LINE__)
#define once_join(a,b) once_jo1n(a,b)
#define once_jo1n(a,b) a##b

#define ONCE \
    static int once_unique(once) = 0; for(; !once_unique(once); once_unique(once) = 1)
*/

/*
int main() {
    for( int i = 0; i < 10; ++i ) {
        ONCE {
            puts("hi");
        }
    }
}
*/
