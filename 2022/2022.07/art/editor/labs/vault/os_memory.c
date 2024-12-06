// - rlyeh, public domain

#ifndef MEMORY_H
#define MEMORY_H

#include <string.h>
#include <stdlib.h>

#ifndef REALLOC
#define REALLOC        realloc
#endif

#define MALLOC(n)      REALLOC(0, n)
#define FREE(p)        REALLOC(p, 0)
#define CALLOC(c, n)   memset(MALLOC((c)*(n)), 0, (c)*(n))
#define STRDUP(s)      strcpy(MALLOC(strlen(s)+1), (s))

#ifndef MSIZE
#   if defined _OSX || defined _BSD
#       define MSIZE malloc_size
#   elif defined _AND
#       define MSIZE dlmalloc_usable_size
#   elif defined _WIN
#       define MSIZE _msize
#   else
#       define MSIZE malloc_usable_size // glibc
#   endif
#endif

#endif
