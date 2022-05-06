// - rlyeh, public domain

#pragma once
#define ON  1
#define OFF 0
#define WITH(x) ((x)+0)

/*
// cl c_with.c
// cl c_with.c -DALSA
// cl c_with.c -DALSA=ON
// cl c_with.c -DALSA=OFF
// cl c_with.c -DALSA=0
// cl c_with.c -DALSA=1

int main() {
	#if WITH(_MSC_VER)
	puts("msc");
	#endif
	#if WITH(__GNUC__)
	puts("gcc");
	#endif
	#if WITH(ALSA)
	puts("alsa audio");
	#endif
	#if WITH(_WIN32)
	puts("win32");
	#endif
}
*/
