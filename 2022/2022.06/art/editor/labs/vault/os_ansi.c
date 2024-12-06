// - rlyeh, public domain

void os_ansi(void);
void os_beep(void);
void os_color(uint8_t r, uint8_t g, uint8_t b);
void os_reset(void);
int  os_columns(void);

// ----------------------------------------------------------------------------

#ifdef ANSI_C
#pragma once

#ifndef _WIN32
#   include <sys/ioctl.h>
#   include <termios.h>
#   include <unistd.h>
#endif

int os_columns(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Window width
    }
#elif defined(TIOCGSIZE)
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols + 1;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    return ts.ws_col + 1;
#endif
    return 1;
}

void os_ansi(void) {
#ifdef _WIN32
    /* Best effort enable ANSI escape processing. */
    HANDLE handle;
    DWORD mode;
    handle = GetStdHandle(-11); /* STD_OUTPUT_HANDLE */
    if (GetConsoleMode(handle, &mode)) {
        mode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
        SetConsoleMode(handle, mode); /* ignore errors */
    }
#endif
}

void os_beep(void) {
    fputc('\x7', stdout); // putc(0x7, stdout)
}

void os_reset(void) {
    static int once = 0; if( !once ) { once = 1; atexit(os_reset); }
#ifdef _WIN32
    static CONSOLE_SCREEN_BUFFER_INFO csbi = {0}, *ever = 0;
    if( !ever ) {
        const HANDLE console = GetStdHandle( STD_OUTPUT_HANDLE );
        if (console == INVALID_HANDLE_VALUE) {
            return;
        }
        if (!GetConsoleScreenBufferInfo(console, &csbi)) {
            return;
        }
        ever = &csbi;
    }
    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(console, csbi.wAttributes);
    }
#else
    puts("\x1B[39;49m");
#endif
}

void os_color(uint8_t r, uint8_t g, uint8_t b) {
    static int once = 0; if( !once ) { once = 1; os_reset(); }
#ifdef _WIN32
    const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    auto color = ( r > 127 ? FOREGROUND_RED : 0 ) |
                 ( g > 127 ? FOREGROUND_GREEN : 0 ) |
                 ( b > 127 ? FOREGROUND_BLUE : 0 );
    if(!(r+g+b)) color=BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE; // bright white on red
    SetConsoleTextAttribute(stdout_handle, color | FOREGROUND_INTENSITY);
#else
    if(!(r+g+b)) { printf("\033[41;97m"); return; } // bright white on red
    // 24-bit console ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
    // 256-color console ESC[38;5;<fgcode>m
    // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
    // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
    // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
    // 0xE8-0xFF:  grayscale from black to white in 24 steps
    r /= 51, g /= 51, b /= 51; // [0..5]
    printf("\033[38;5;%dm", r*36+g*6+b+16); // "\033[0;3%sm", color_code);
#endif
}

#endif // OS_ANSI

