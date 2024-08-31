// compiled with:
// tcc cook.c -I ../split
// cl  cook.c -I ../split /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /DFINAL /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
// cc -ObjC cook.c -o cook.osx -I ../split -framework Cocoa -framework IOKit

#define FWK_COOKER_STANDALONE
#include "fwk.c"
