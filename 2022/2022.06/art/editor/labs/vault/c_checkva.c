#pragma once

// MSVC2015 trick by doynax (check+dead code elimination)
// see: https://stackoverflow.com/a/42313782
#define CHECK_VA(...) \
(printf || printf(__VA_ARGS__), (__VA_ARGS__))

// usage:
// #define my_printf(...) my_printf(CHECK_VA(__VA_ARGS__))
