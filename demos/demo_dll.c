// fwk built as dll.
// compile with: cl fwk.c /LD /DAPI=EXPORT && cl demo_dll.c fwk.lib /DAPI=IMPORT

#include "fwk.h"

int main() {
    window_create(75.0,0);
    while( window_swap() ) {
        ddraw_grid(10);
        ddraw_flush();
    }
}
