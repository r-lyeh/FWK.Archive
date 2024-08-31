// from randypgaul https://github.com/RandyGaul/kk_slides (likely public domain)

#pragma once

typedef struct coroutine_t {
    float elapsed;
    int index;
} coroutine_t;

#define COROUTINE_BEGIN(co) do { switch (co->index) { default:
#define COROUTINE_CASE(co, name) case __LINE__: name: co->index = __LINE__;
#define COROUTINE_GOTO(co, name) do { goto name; } while (0)
#define COROUTINE_EXIT(co) do { goto __co_end; } while (0)
#define COROUTINE_YIELD(co) do { co->index = __LINE__; COROUTINE_EXIT(co); case __LINE__:; } while (0)
#define COROUTINE_WAIT(co, time, dt) do { case __LINE__: co->index = __LINE__; co->elapsed += dt; if(co->elapsed < time) goto __co_end; co->elapsed = 0; } while (0)
#define COROUTINE_END(co) } co->index = 0; __co_end:; } while (0)

/*
#include <stdio.h>
#include <winsock2.h> // Sleep

void test1(coroutine_t* co) {
    static int loop_count = 0;

    COROUTINE_BEGIN(co);

    COROUTINE_CASE(co, loop_start);
    printf("State 0\n");
    COROUTINE_YIELD(co);
    printf("State 1\n");
    COROUTINE_YIELD(co);
    printf("State 2\n");
    COROUTINE_YIELD(co);
    // [...]
    COROUTINE_GOTO(co, loop_start);

    COROUTINE_END(co);
}

int test2(coroutine_t* co, float dt) {
    int active = 1;

    printf("%.1f\r", co->elapsed);

    COROUTINE_BEGIN(co);

    COROUTINE_WAIT(co, 0.0f, dt);
    printf("State 0\n");
    COROUTINE_WAIT(co, 1.0f, dt);
    printf("State 1\n");
    COROUTINE_WAIT(co, 2.0f, dt);
    printf("State 2\n");
    active = 0;

    COROUTINE_END(co);
    return active;
}

int main() {
    coroutine_t co1 = {0};
    for(int i = 0; i < 6; ++i) test1(&co1);
    puts("---");

    coroutine_t co2 = {0};
    while(test2(&co2, 1.0/60)) Sleep(10); // should be 16 instead of 10; but Sleep() precision sucks
}
*/
