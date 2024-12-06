//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//-----------------------------------------------------------------------------
/* Pure DOOM usage

   Do this:
      #define DOOM_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define DOOM_IMPLEMENTATION
   #include "PureDOOM.h"

*/
//-----------------------------------------------------------------------------


#ifndef __DOOM_H__
#define __DOOM_H__


// Sample rate of sound samples from doom
#define DOOM_SAMPLERATE 11025

// MIDI tick needs to be called 140 times per seconds
#define DOOM_MIDI_RATE 140

// Hide menu options. If for say your platform doesn't support mouse or
// MIDI playback, you can hide these settings from the menu.
#define DOOM_FLAG_HIDE_MOUSE_OPTIONS 1 // Remove mouse options from menu
#define DOOM_FLAG_HIDE_SOUND_OPTIONS 2 // Remove sound options from menu
#define DOOM_FLAG_HIDE_MUSIC_OPTIONS 4 // Remove music options from menu

// Darken background when menu is open, making it more readable. This
// uses a bit more CPU and redraws the HUD every frame
#define DOOM_FLAG_MENU_DARKEN_BG 8


#if __cplusplus
extern "C" {
#endif

typedef enum
{
    DOOM_SEEK_CUR = 1,
    DOOM_SEEK_END = 2,
    DOOM_SEEK_SET = 0
} doom_seek_t;


typedef void(*doom_print_fn)(const char* str);
typedef void*(*doom_malloc_fn)(int size);
typedef void(*doom_free_fn)(void* ptr);
typedef void*(*doom_open_fn)(const char* filename, const char* mode);
typedef void(*doom_close_fn)(void* handle);
typedef int(*doom_read_fn)(void* handle, void *buf, int count);
typedef int(*doom_write_fn)(void* handle, const void *buf, int count);
typedef int(*doom_seek_fn)(void* handle, int offset, doom_seek_t origin);
typedef int(*doom_tell_fn)(void* handle);
typedef int(*doom_eof_fn)(void* handle);
typedef void(*doom_gettime_fn)(int* sec, int* usec);
typedef void(*doom_exit_fn)(int code);
typedef char*(*doom_getenv_fn)(const char* var);


// Doom key mapping
typedef enum
{
    DOOM_KEY_UNKNOWN = -1,
    DOOM_KEY_TAB = 9,
    DOOM_KEY_ENTER = 13,
    DOOM_KEY_ESCAPE = 27,
    DOOM_KEY_SPACE = 32,
    DOOM_KEY_APOSTROPHE = '\'',
    DOOM_KEY_MULTIPLY = '*',
    DOOM_KEY_COMMA = ',',
    DOOM_KEY_MINUS = 0x2d,
    DOOM_KEY_PERIOD = '.',
    DOOM_KEY_SLASH = '/',
    DOOM_KEY_0 = '0',
    DOOM_KEY_1 = '1',
    DOOM_KEY_2 = '2',
    DOOM_KEY_3 = '3',
    DOOM_KEY_4 = '4',
    DOOM_KEY_5 = '5',
    DOOM_KEY_6 = '6',
    DOOM_KEY_7 = '7',
    DOOM_KEY_8 = '8',
    DOOM_KEY_9 = '9',
    DOOM_KEY_SEMICOLON = ';',
    DOOM_KEY_EQUALS = 0x3d,
    DOOM_KEY_LEFT_BRACKET = '[',
    DOOM_KEY_RIGHT_BRACKET = ']',
    DOOM_KEY_A = 'a',
    DOOM_KEY_B = 'b',
    DOOM_KEY_C = 'c',
    DOOM_KEY_D = 'd',
    DOOM_KEY_E = 'e',
    DOOM_KEY_F = 'f',
    DOOM_KEY_G = 'g',
    DOOM_KEY_H = 'h',
    DOOM_KEY_I = 'i',
    DOOM_KEY_J = 'j',
    DOOM_KEY_K = 'k',
    DOOM_KEY_L = 'l',
    DOOM_KEY_M = 'm',
    DOOM_KEY_N = 'n',
    DOOM_KEY_O = 'o',
    DOOM_KEY_P = 'p',
    DOOM_KEY_Q = 'q',
    DOOM_KEY_R = 'r',
    DOOM_KEY_S = 's',
    DOOM_KEY_T = 't',
    DOOM_KEY_U = 'u',
    DOOM_KEY_V = 'v',
    DOOM_KEY_W = 'w',
    DOOM_KEY_X = 'x',
    DOOM_KEY_Y = 'y',
    DOOM_KEY_Z = 'z',
    DOOM_KEY_BACKSPACE = 127,
    DOOM_KEY_CTRL = (0x80 + 0x1d), // Both left and right
    DOOM_KEY_LEFT_ARROW = 0xac,
    DOOM_KEY_UP_ARROW = 0xad,
    DOOM_KEY_RIGHT_ARROW = 0xae,
    DOOM_KEY_DOWN_ARROW = 0xaf,
    DOOM_KEY_SHIFT = (0x80 + 0x36), // Both left and right
    DOOM_KEY_ALT = (0x80 + 0x38), // Both left and right
    DOOM_KEY_F1 = (0x80 + 0x3b),
    DOOM_KEY_F2 = (0x80 + 0x3c),
    DOOM_KEY_F3 = (0x80 + 0x3d),
    DOOM_KEY_F4 = (0x80 + 0x3e),
    DOOM_KEY_F5 = (0x80 + 0x3f),
    DOOM_KEY_F6 = (0x80 + 0x40),
    DOOM_KEY_F7 = (0x80 + 0x41),
    DOOM_KEY_F8 = (0x80 + 0x42),
    DOOM_KEY_F9 = (0x80 + 0x43),
    DOOM_KEY_F10 = (0x80 + 0x44),
    DOOM_KEY_F11 = (0x80 + 0x57),
    DOOM_KEY_F12 = (0x80 + 0x58),
    DOOM_KEY_PAUSE = 0xff
} doom_key_t;


// Mouse button mapping
typedef enum
{
    DOOM_LEFT_BUTTON = 0,
    DOOM_RIGHT_BUTTON = 1,
    DOOM_MIDDLE_BUTTON = 2
} doom_button_t;


// For the software renderer. Default is 320x200
void doom_set_resolution(int width, int height);

// Set default configurations. Lets say, changing arrows to WASD as default controls
void doom_set_default_int(const char* name, int value);
void doom_set_default_string(const char* name, const char* value);

// set callbacks
void doom_set_print(doom_print_fn print_fn);
void doom_set_malloc(doom_malloc_fn malloc_fn, doom_free_fn free_fn);
void doom_set_file_io(doom_open_fn open_fn,
                      doom_close_fn close_fn,
                      doom_read_fn read_fn,
                      doom_write_fn write_fn,
                      doom_seek_fn seek_fn,
                      doom_tell_fn tell_fn,
                      doom_eof_fn eof_fn);
void doom_set_gettime(doom_gettime_fn gettime_fn);
void doom_set_exit(doom_exit_fn exit_fn);
void doom_set_getenv(doom_getenv_fn getenv_fn);

// Initializes DOOM and start things up. Call only call one
void doom_init(int argc, char** argv, int flags);

// Call this every frame
void doom_update();

// Channels: 1 = indexed, 3 = RGB, 4 = RGBA
const unsigned char* doom_get_framebuffer(int channels);

// It is always 2048 bytes in size
short* doom_get_sound_buffer();

// Call this 140 times per second. Or about every 7ms. 
// Returns midi message. Keep calling it until it returns 0.
unsigned long doom_tick_midi();

// Events
void doom_key_down(doom_key_t key);
void doom_key_up(doom_key_t key);
void doom_button_down(doom_button_t button);
void doom_button_up(doom_button_t button);
void doom_mouse_move(int delta_x, int delta_y);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
