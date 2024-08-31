#pragma once

#define CC4(abcd)  ((0[#abcd "\0\0\0\0"] << 24) | (1[#abcd "\0\0\0\0"] << 16) | (2[#abcd "\0\0\0\0"] << 8) | (3[#abcd "\0\0\0\0"] << 0))
#if LITTLE
#define CC4_FMT(i) ((i)>>24)&255,((i)>>16)&255,((i)>> 8)&255,((i)>> 0)&255
#else
#define CC4_FMT(i) ((i)>> 0)&255,((i)>> 8)&255,((i)>>16)&255,((i)>>24)&255
#endif

/*
int main() {
    int x = CC4(user);
    printf("%#x %c%c%c%c\n", x, CC4_FMT(x));
}
*/
