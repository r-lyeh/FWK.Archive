// - rlyeh, public domain

#ifdef SYN_C
#pragma once
#define ATOMIC_C
#define CHANNEL_C
#define CONDV_C
#define MCMP_C
#define MUTEX_C
#define SEMAPHORE_C
#define SLEEP_C
#define THREAD_C
#define TLS_C
#define CORO_C
#define FIBER_C
#endif

#include "c.c"
#include "os.c"
#include "syn_mutex.c"
#include "syn_condv.c"   // after mutex
#include "syn_channel.c" // after condv+mutex
#include "syn_tls.c"
#include "syn_thread.c"  // after tls
#include "syn_atomic.c"
#include "syn_mcmp.c"
#include "syn_semaphore.c"
#include "syn_sleep.c"

#include "syn_coro.c"
#include "syn_fiber.c"
