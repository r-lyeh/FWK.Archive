// - rlyeh, public domain

#ifdef C_C
#pragma once
#define BENCHMARK_C
#define IFDEF_C
#define OVERLOAD_C
#define WITH_C
#endif

#include "c_benchmark.c"
#include "c_ifdef.c"
#include "c_overload.c"
#include "c_with.c"
#include "c_incbin.h"

#include "c_section.c"
#include "c_alignas.c"
#include "c_thread.c"
#include "c_plan.c"

#include "c_once.c"
#include "c_checkva.c"
#include "c_cc4.c" // after c_ifdef
