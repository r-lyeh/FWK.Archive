// PureDOOM port based on existing SDL port by @daivuk
// - rlyeh, public domain.

#include "fwk.h"

#define DOOM_IMPLEMENTATION
#include "PureDOOM.h"

// Resolution DOOM renders at
enum { DOOM_WIDTH = 320, DOOM_HEIGHT = 200 };

// ----------------------------------------------------------------------------

doom_key_t scancode_to_doom_key(int scancode) {
    switch (scancode) {
        case KEY_TAB: return DOOM_KEY_TAB;
        case KEY_ENTER: return DOOM_KEY_ENTER;
        case KEY_ESC: return DOOM_KEY_ESCAPE;
        case KEY_SPACE: return DOOM_KEY_SPACE;
        case KEY_COMMA: return DOOM_KEY_COMMA;
        case KEY_MINUS: return DOOM_KEY_MINUS;
        case KEY_SLASH: return DOOM_KEY_SLASH;
        case KEY_0: return DOOM_KEY_0;
        case KEY_1: return DOOM_KEY_1;
        case KEY_2: return DOOM_KEY_2;
        case KEY_3: return DOOM_KEY_3;
        case KEY_4: return DOOM_KEY_4;
        case KEY_5: return DOOM_KEY_5;
        case KEY_6: return DOOM_KEY_6;
        case KEY_7: return DOOM_KEY_7;
        case KEY_8: return DOOM_KEY_8;
        case KEY_9: return DOOM_KEY_9;
        case KEY_SEMICOLON: return DOOM_KEY_SEMICOLON;
        case KEY_A: return DOOM_KEY_A;
        case KEY_B: return DOOM_KEY_B;
        case KEY_C: return DOOM_KEY_C;
        case KEY_D: return DOOM_KEY_D;
        case KEY_E: return DOOM_KEY_E;
        case KEY_F: return DOOM_KEY_F;
        case KEY_G: return DOOM_KEY_G;
        case KEY_H: return DOOM_KEY_H;
        case KEY_I: return DOOM_KEY_I;
        case KEY_J: return DOOM_KEY_J;
        case KEY_K: return DOOM_KEY_K;
        case KEY_L: return DOOM_KEY_L;
        case KEY_M: return DOOM_KEY_M;
        case KEY_N: return DOOM_KEY_N;
        case KEY_O: return DOOM_KEY_O;
        case KEY_P: return DOOM_KEY_P;
        case KEY_Q: return DOOM_KEY_Q;
        case KEY_R: return DOOM_KEY_R;
        case KEY_S: return DOOM_KEY_S;
        case KEY_T: return DOOM_KEY_T;
        case KEY_U: return DOOM_KEY_U;
        case KEY_V: return DOOM_KEY_V;
        case KEY_W: return DOOM_KEY_W;
        case KEY_X: return DOOM_KEY_X;
        case KEY_Y: return DOOM_KEY_Y;
        case KEY_Z: return DOOM_KEY_Z;
        case KEY_BS: return DOOM_KEY_BACKSPACE;
        case KEY_LCTRL:
        case KEY_RCTRL: return DOOM_KEY_CTRL;
        case KEY_LEFT: return DOOM_KEY_LEFT_ARROW;
        case KEY_UP: return DOOM_KEY_UP_ARROW;
        case KEY_RIGHT: return DOOM_KEY_RIGHT_ARROW;
        case KEY_DOWN: return DOOM_KEY_DOWN_ARROW;
        case KEY_LSHIFT:
        case KEY_RSHIFT: return DOOM_KEY_SHIFT;
        case KEY_LALT:
        case KEY_RALT: return DOOM_KEY_ALT;
        case KEY_F1: return DOOM_KEY_F1;
        case KEY_F2: return DOOM_KEY_F2;
        case KEY_F3: return DOOM_KEY_F3;
        case KEY_F4: return DOOM_KEY_F4;
        case KEY_F5: return DOOM_KEY_F5;
        case KEY_F6: return DOOM_KEY_F6;
        case KEY_F7: return DOOM_KEY_F7;
        case KEY_F8: return DOOM_KEY_F8;
        case KEY_F9: return DOOM_KEY_F9;
        case KEY_F10: return DOOM_KEY_F10;
        case KEY_F11: return DOOM_KEY_F11;
        case KEY_F12: return DOOM_KEY_F12;
        case KEY_PAUSE: return DOOM_KEY_PAUSE;
        case MOUSE_L: return DOOM_LEFT_BUTTON;
        case MOUSE_R: return DOOM_RIGHT_BUTTON;
        case MOUSE_M: return DOOM_MIDDLE_BUTTON;
        default: return DOOM_KEY_UNKNOWN;
    }
    return DOOM_KEY_UNKNOWN;
}

void keyboard() {
    int keys[] = {
        KEY_ESC,
        KEY_TICK, KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,  KEY_BS,
        KEY_TAB,   KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,
        KEY_CAPS,     KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L, KEY_ENTER,
        KEY_LSHIFT,       KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,        KEY_RSHIFT,            KEY_UP,
        KEY_LCTRL,KEY_LALT,               KEY_SPACE,                KEY_RALT,KEY_RCTRL,  KEY_LEFT,KEY_DOWN,KEY_RIGHT,
    };
    for( int i = 0; i < countof(keys); ++i ) {
        int doom_vk = scancode_to_doom_key(keys[i]);
        if( doom_vk != DOOM_KEY_UNKNOWN ) {
        /**/ if( input_down(keys[i]) ) doom_key_down(doom_vk);
        else if( input_up(keys[i]) )   doom_key_up(doom_vk);
        }
    }
}

void mouse() {
    if( !window_has_cursor() ) { // Dev allow us to take mouse out of window
        int motion_x = input_diff(MOUSE_X);
        int motion_y = input_diff(MOUSE_Y);            

        const float MOUSE_SENSITIVITY = 3.0f;
        doom_mouse_move(motion_x * MOUSE_SENSITIVITY, motion_y * MOUSE_SENSITIVITY);            

        /**/ if( input_down(MOUSE_L) ) doom_button_down(scancode_to_doom_key(MOUSE_L));
        else if( input_up(MOUSE_L) )   doom_button_up(scancode_to_doom_key(MOUSE_L));
        /**/ if( input_down(MOUSE_R) ) doom_button_down(scancode_to_doom_key(MOUSE_R));
        else if( input_up(MOUSE_R) )   doom_button_up(scancode_to_doom_key(MOUSE_R));
    }
}

void logic() {
    doom_update();
}

void blit() {
    static texture_t screen; do_once screen = texture_checker();
    texture_update(&screen, DOOM_WIDTH, DOOM_HEIGHT, 4, doom_get_framebuffer(4), TEXTURE_RGBA/*|TEXTURE_LINEAR*/);
    fullscreen_quad_rgb(screen, 1.0f); // gamma(1)
}

void audio() {
    // doom_get_sound_buffer() is always 2048 bytes sized, 11khz 16bit stereo, 512 samples
    static int target_frame = 60.0 * ( 512. / 11025 );
    static int curr_frame = 0; if( curr_frame++ >= target_frame ) { curr_frame = 0;
    audio_queue(doom_get_sound_buffer(), 512, 0|AUDIO_16|AUDIO_2CH|AUDIO_11KHZ);
    }
}

unsigned midi(unsigned interval, void *param) { // this is async called every interval, via timer
    unsigned midi_msg;
    while( (midi_msg = doom_tick_midi()) != 0 ) midi_send(midi_msg);
    return 1000 / DOOM_MIDI_RATE ifdef(osx, -1); // Weirdly, on Apple music is too slow
}

// ----------------------------------------------------------------------------

int main(int argc, char** argv) {
    // install signal handlers
    signal_hooks();

    // Change default bindings to modern
    doom_set_default_int("key_up", DOOM_KEY_W);
    doom_set_default_int("key_down", DOOM_KEY_S);
    doom_set_default_int("key_strafeleft", DOOM_KEY_A);
    doom_set_default_int("key_straferight", DOOM_KEY_D);
    doom_set_default_int("key_use", DOOM_KEY_E);
    doom_set_default_int("mouse_move", 0); // Mouse will not move forward

    // Change resolution
    doom_set_resolution(DOOM_WIDTH, DOOM_HEIGHT);

    // Initialize doom
    doom_init(argc, argv, DOOM_FLAG_MENU_DARKEN_BG);

    // Init app
    window_create(0.75, 0);
    window_aspect_lock(DOOM_WIDTH, DOOM_HEIGHT);
    window_title("Pure DOOM");
    window_cursor(false);
    timer(0, midi, 0);
    
    // Loop app
    while( window_swap() ) {
        if( input_down(KEY_F10) ) tty_attach();
        if( input_down(KEY_F11) ) window_fullscreen(!window_has_fullscreen());

        keyboard();
        mouse();
        logic();
        blit();
        audio();
    }
}
