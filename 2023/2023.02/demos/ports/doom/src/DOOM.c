#if defined(WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "DOOM.h"

#include "d_main.h"
#include "doomdef.h"
#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"


extern byte* screens[5];
extern unsigned char screen_palette[256 * 3];
extern doom_boolean is_wiping_screen;
extern default_t defaults[];
extern int numdefaults;
extern signed short mixbuffer[2048];


static unsigned char* screen_buffer = 0;
static unsigned char* final_screen_buffer = 0;
static int last_update_time = 0;
static int button_states[3] = { 0 };
static char itoa_buf[20];


char error_buf[260];
int doom_flags = 0;
doom_print_fn doom_print = 0;
doom_malloc_fn doom_malloc = 0;
doom_free_fn doom_free = 0;
doom_open_fn doom_open = 0;
doom_close_fn doom_close = 0;
doom_read_fn doom_read = 0;
doom_write_fn doom_write = 0;
doom_seek_fn doom_seek = 0;
doom_tell_fn doom_tell = 0;
doom_eof_fn doom_eof = 0;
doom_gettime_fn doom_gettime = 0;
doom_exit_fn doom_exit = 0;
doom_getenv_fn doom_getenv = 0;


void D_DoomLoop(void);
void D_UpdateWipe(void);
void I_UpdateSound();
unsigned long I_TickSong();


#if defined(DOOM_IMPLEMENT_PRINT)
#include <stdio.h>
static void doom_print_impl(const char* str)
{
    printf("%s", str);
}
#else
static void doom_print_impl(const char* str) {}
#endif

#if defined(DOOM_IMPLEMENT_MALLOC)
#include <stdlib.h>
static void* doom_malloc_impl(int size)
{
    return malloc((size_t)size);
}
static void doom_free_impl(void* ptr)
{
    free(ptr);
}
#else
static void* doom_malloc_impl(int size) { return 0; }
static void doom_free_impl(void* ptr) {}
#endif


#if defined(DOOM_IMPLEMENT_FILE_IO)
#include <stdio.h>
void* doom_open_impl(const char* filename, const char* mode)
{
    return fopen(filename, mode);
}
void doom_close_impl(void* handle)
{
    fclose(handle);
}
int doom_read_impl(void* handle, void *buf, int count)
{
    return (int)fread(buf, 1, count, handle);
}
int doom_write_impl(void* handle, const void *buf, int count)
{
    return (int)fwrite(buf, 1, count, handle);
}
int doom_seek_impl(void* handle, int offset, doom_seek_t origin)
{
    return fseek(handle, offset, origin);
}
int doom_tell_impl(void* handle)
{
    return (int)ftell(handle);
}
int doom_eof_impl(void* handle)
{
    return feof(handle);
}
#else
void* doom_open_impl(const char* filename, const char* mode)
{
    return 0;
}
void doom_close_impl(void* handle) {}
int doom_read_impl(void* handle, void *buf, int count)
{
    return -1;
}
int doom_write_impl(void* handle, const void *buf, int count)
{
    return -1;
}
int doom_seek_impl(void* handle, int offset, doom_seek_t origin)
{
    return -1;
}
int doom_tell_impl(void* handle)
{
    return -1;
}
int doom_eof_impl(void* handle)
{
    return 1;
}
#endif


#if defined(DOOM_IMPLEMENT_GETTIME)
#if defined(WIN32)
#include <winsock.h>
#else
#include <sys/time.h>
#endif
void doom_gettime_impl(int* sec, int* usec)
{
#if defined(WIN32)
    static const unsigned long long EPOCH = ((unsigned long long)116444736000000000ULL);
    SYSTEMTIME system_time;
    FILETIME file_time;
    unsigned long long time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((unsigned long long)file_time.dwLowDateTime);
    time += ((unsigned long long)file_time.dwHighDateTime) << 32;
    *sec = (int)((time - EPOCH) / 10000000L);
    *usec = (int)(system_time.wMilliseconds * 1000);
#else
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);
    *sec = tp.tv_sec;
    *usec = tp.tv_usec;
#endif
}
#else
void doom_gettime_impl(int* sec, int* usec)
{
    *sec = 0;
    *usec = 0;
}
#endif


#if defined(DOOM_IMPLEMENT_EXIT)
#include <stdlib.h>
void doom_exit_impl(int code)
{
    exit(code);
}
#else
void doom_exit_impl(int code) {}
#endif


#if defined(DOOM_IMPLEMENT_GETENV)
#include <stdlib.h>
char* doom_getenv_impl(const char* var)
{
    return getenv(var);
}
#else
char* doom_getenv_impl(const char* var) {
    return (char*)0; //< @r-lyeh add missing return
}
#endif


void doom_memset(void* ptr, int value, int num)
{
    unsigned char* p = ptr;
    for (int i = 0; i < num; ++i, ++p)
    {
        *p = (unsigned char)value;
    }
}


void* doom_memcpy(void* destination, const void* source, int num)
{
    unsigned char* dst = destination;
    const unsigned char* src = source;

    for (int i = 0; i < num; ++i, ++dst, ++src)
    {
        *dst = *src;
    }

    return destination;
}


int doom_strlen(const char* str)
{
    int len = 0;
    while (*str++) ++len;
    return len;
}


char* doom_concat(char* dst, const char* src)
{
    char* ret = dst;
    dst += doom_strlen(dst);

    while (*src) *dst++ = *src++;
    *dst = *src; // \0

    return ret;
}


char* doom_strcpy(char* dst, const char* src)
{
    char* ret = dst;

    while (*src) *dst++ = *src++;
    *dst = *src; // \0

    return ret;
}


char* doom_strncpy(char* dst, const char* src, int num)
{
    int i = 0;

    for (; i < num; ++i)
    {
        if (!src[i]) break;
        dst[i] = src[i];
    }

    while (i < num) dst[i++] = '\0';

    return dst;
}


int doom_strcmp(const char* str1, const char* str2)
{
    int ret = 0;

    while (!(ret = *(unsigned char*)str1 - *(unsigned char*) str2) && *str1)
        ++str1, ++str2;

    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;

    return (ret);
}


int doom_strncmp(const char* str1, const char* str2, int n)
{
    int ret = 0;
    int count = 1;

    while (!(ret = *(unsigned char*)str1 - *(unsigned char*) str2) && *str1 && count++ < n)
        ++str1, ++str2;

    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;

    return (ret);
}


int doom_toupper(int c)
{
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}


int doom_strcasecmp(const char* str1, const char* str2)
{
    int ret = 0;

    while (!(ret = doom_toupper(*(unsigned char*)str1) - doom_toupper(*(unsigned char*)str2)) && *str1)
        ++str1, ++str2;

    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;

    return (ret);
}


int doom_strncasecmp(const char* str1, const char* str2, int n)
{
    int ret = 0;
    int count = 1;

    while (!(ret = doom_toupper(*(unsigned char*)str1) - doom_toupper(*(unsigned char*)str2)) && *str1 && count++ < n)
        ++str1, ++str2;

    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;

    return (ret);
}


int doom_atoi(const char* str)
{
    int i = 0;
    int c;

    while ((c = *str++) != 0)
    {
        i *= 10;
        i += c - '0';
    }

    return i;
}


int doom_atox(const char* str)
{
    int i = 0;
    int c;

    while ((c = *str++) != 0)
    {
        i *= 16;
        if (c >= '0' && c <= '9')
            i += c - '0';
        else
            i += c - 'A' + 10;
    }

    return i;
}


const char* doom_itoa(int k, int radix)
{
    int i = k < 0 ? -k : k;
    if (i == 0)
    {
        itoa_buf[0] = '0';
        itoa_buf[1] = '\0';
        return itoa_buf;
    }

    int idx = k < 0 ? 1 : 0;
    int j = i;
    while (j)
    {
        j /= radix;
        idx++;
    }
    itoa_buf[idx] = '\0';

    if (radix == 10)
    {
        while (i)
        {
            itoa_buf[--idx] = '0' + (i % 10);
            i /= 10;
        }
    }
    else
    {
        while (i)
        {
            int k = (i & 0xF);
            if (k >= 10)
                itoa_buf[--idx] = 'A' + ((i & 0xF) - 10);
            else
                itoa_buf[--idx] = '0' + (i & 0xF);
            i >>= 4;
        }
    }

    if (k < 0) itoa_buf[0] = '-';

    return itoa_buf;
}


const char* doom_ctoa(char c)
{
    itoa_buf[0] = c;
    itoa_buf[1] = '\0';
    return itoa_buf;
}


const char* doom_ptoa(void* p)
{
    int idx = 0;
    unsigned long long i = (unsigned long long)p;

    itoa_buf[idx++] = '0';
    itoa_buf[idx++] = 'x';

    while (i)
    {
        int k = (i & 0xF);
        if (k >= 10)
            itoa_buf[idx++] = 'A' + ((i & 0xF) - 10);
        else
            itoa_buf[idx++] = '0' + (i & 0xF);
        i >>= 4;
    }

    itoa_buf[idx] = '\0';
    return itoa_buf;
}


int doom_fprint(void* handle, const char* str)
{
    return doom_write(handle, str, doom_strlen(str));
}


static default_t* get_default(const char* name)
{
    for (int i = 0; i < numdefaults; ++i)
    {
        if (doom_strcmp(defaults[i].name, name) == 0) return &defaults[i];
    }
    return 0;
}


void doom_set_resolution(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    // SCREENWIDTH = width;
    // SCREENHEIGHT = height;
}


void doom_set_default_int(const char* name, int value)
{
    default_t* def = get_default(name);
    if (!def) return;
    def->defaultvalue = value;
}


void doom_set_default_string(const char* name, const char* value)
{
    default_t* def = get_default(name);
    if (!def) return;
    def->default_text_value = (char*)value;
}


void doom_set_print(doom_print_fn print_fn)
{
    doom_print = print_fn;
}


void doom_set_malloc(doom_malloc_fn malloc_fn, doom_free_fn free_fn)
{
    doom_malloc = malloc_fn;
    doom_free = free_fn;
}


void doom_set_file_io(doom_open_fn open_fn,
                      doom_close_fn close_fn,
                      doom_read_fn read_fn,
                      doom_write_fn write_fn,
                      doom_seek_fn seek_fn,
                      doom_tell_fn tell_fn,
                      doom_eof_fn eof_fn)
{
    doom_open = open_fn;
    doom_close = close_fn;
    doom_read = read_fn;
    doom_write = write_fn;
    doom_seek = seek_fn;
    doom_tell = tell_fn;
    doom_eof = eof_fn;
}


void doom_set_gettime(doom_gettime_fn gettime_fn)
{
    doom_gettime = gettime_fn;
}


void doom_set_exit(doom_exit_fn exit_fn)
{
    doom_exit = exit_fn;
}


void doom_set_getenv(doom_getenv_fn getenv_fn)
{
    doom_getenv = getenv_fn;
}


void doom_init(int argc, char** argv, int flags)
{
    if (!doom_print) doom_print = doom_print_impl;
    if (!doom_malloc) doom_malloc = doom_malloc_impl;
    if (!doom_free) doom_free = doom_free_impl;
    if (!doom_open) doom_open = doom_open_impl;
    if (!doom_close) doom_close = doom_close_impl;
    if (!doom_read) doom_read = doom_read_impl;
    if (!doom_write) doom_write = doom_write_impl;
    if (!doom_seek) doom_seek = doom_seek_impl;
    if (!doom_tell) doom_tell = doom_tell_impl;
    if (!doom_eof) doom_eof = doom_eof_impl;
    if (!doom_gettime) doom_gettime = doom_gettime_impl;
    if (!doom_exit) doom_exit = doom_exit_impl;
    if (!doom_getenv) doom_getenv = doom_getenv_impl;

    screen_buffer = doom_malloc(SCREENWIDTH * SCREENHEIGHT);
    final_screen_buffer = doom_malloc(SCREENWIDTH * SCREENHEIGHT * 4);
    last_update_time = I_GetTime();

    myargc = argc;
    myargv = argv;
    doom_flags = flags;

    D_DoomMain();
}


void doom_update()
{
    int now = I_GetTime();
    int delta_time = now - last_update_time;

    while (delta_time-- > 0)
    {
        if (is_wiping_screen)
            D_UpdateWipe();
        else
            D_DoomLoop();
    }

    last_update_time = now;
}


const unsigned char* doom_get_framebuffer(int channels)
{
    int i, len;

    doom_memcpy(screen_buffer, screens[0], SCREENWIDTH * SCREENHEIGHT);

    extern doom_boolean menuactive;
    extern gamestate_t gamestate; 
    extern doom_boolean automapactive;
    extern int crosshair;

    // Draw crosshair
    if (crosshair && 
        !menuactive &&
        gamestate == GS_LEVEL &&
        !automapactive)
    {
        int y;
        extern int setblocks;
        if (setblocks == 11) y = SCREENHEIGHT / 2 + 8;
        else y = SCREENHEIGHT / 2 - 8;
        for (i = 0; i < 2; ++i)
        {
            screen_buffer[SCREENWIDTH / 2 - 2 - i + y * SCREENWIDTH] = 4;
            screen_buffer[SCREENWIDTH / 2 + 2 + i + y * SCREENWIDTH] = 4;
        }
        for (i = 0; i < 2; ++i)
        {
            screen_buffer[SCREENWIDTH / 2 + (y - 2 - i) * SCREENWIDTH] = 4;
            screen_buffer[SCREENWIDTH / 2 + (y + 2 + i) * SCREENWIDTH] = 4;
        }
    }

    if (channels == 1)
    {
        return screen_buffer;
    }
    else if (channels == 3)
    {
        for (i = 0, len = SCREENWIDTH * SCREENHEIGHT; i < len; ++i)
        {
            int k = i * 3;
            int kpal = screen_buffer[i] * 3;
            final_screen_buffer[k + 0] = screen_palette[kpal + 0];
            final_screen_buffer[k + 1] = screen_palette[kpal + 1];
            final_screen_buffer[k + 2] = screen_palette[kpal + 2];
        }
        return final_screen_buffer;
    }
    else if (channels == 4)
    {
        for (i = 0, len = SCREENWIDTH * SCREENHEIGHT; i < len; ++i)
        {
            int k = i * 4;
            int kpal = screen_buffer[i] * 3;
            final_screen_buffer[k + 0] = screen_palette[kpal + 0];
            final_screen_buffer[k + 1] = screen_palette[kpal + 1];
            final_screen_buffer[k + 2] = screen_palette[kpal + 2];
            final_screen_buffer[k + 3] = 255;
        }
        return final_screen_buffer;
    }
    else
    {
        return 0;
    }
}


unsigned long doom_tick_midi()
{
    return I_TickSong();
}


short* doom_get_sound_buffer()
{
    I_UpdateSound();
    return mixbuffer;
}


void doom_key_down(doom_key_t key)
{
    event_t event;
    event.type = ev_keydown;
    event.data1 = (int)key;
    D_PostEvent(&event);
}


void doom_key_up(doom_key_t key)
{
    event_t event;
    event.type = ev_keyup;
    event.data1 = (int)key;
    D_PostEvent(&event);
}


void doom_button_down(doom_button_t button)
{
    button_states[button] = 1;

    event_t event;
    event.type = ev_mouse;
    event.data1 =
        (button_states[0]) |
        (button_states[1] ? 2 : 0) |
        (button_states[2] ? 4 : 0);
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}


void doom_button_up(doom_button_t button)
{
    button_states[button] = 0;

    event_t event;
    event.type = ev_mouse;
    event.data1 =
        (button_states[0]) |
        (button_states[1] ? 2 : 0) |
        (button_states[2] ? 4 : 0);

    event.data1 =
        event.data1
        ^ (button_states[0] ? 1 : 0)
        ^ (button_states[1] ? 2 : 0)
        ^ (button_states[2] ? 4 : 0);

    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}


void doom_mouse_move(int delta_x, int delta_y)
{
    event_t event;

    event.type = ev_mouse;
    event.data1 =
        (button_states[0]) |
        (button_states[1] ? 2 : 0) |
        (button_states[2] ? 4 : 0);
    event.data2 = delta_x;
    event.data3 = -delta_y;

    if (event.data2 || event.data3)
    {
        D_PostEvent(&event);
    }
}
