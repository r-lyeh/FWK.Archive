// PLAN is usually a TODO taskfile, like: AUTORUN { puts("[x] task1"); puts("[ ] task2"); }
// Usage: cl test.c -DPLAN="^<my_plan.h^>" or -DPLAN="\\\"my_plan.h\\\""

#pragma once

#if defined __has_include
#  if __has_include("plan.c")
#    include "plan.c"
#  endif
#elif defined PLAN
#  include PLAN
#endif
