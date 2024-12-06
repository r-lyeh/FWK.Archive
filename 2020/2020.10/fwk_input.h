// input framework
// - rlyeh, public domain
//
// @todo: touch, window
// @todo: for extra savings (168->72 bytes), promote bits to real bits (/8 %8) & normalized floats [-1,+1] to shorts or chars
// @todo: GAMEPAD_A|2, MOUSE_L|1, KEY_C|3
// @todo: load/save
// @todo: send virtual presses & outputs (rumble, light, led, text, etc)
// @todo: fix if logger !60 hz

#ifndef INPUT_H
#define INPUT_H

// -- basic api (check input during current frame)

int         input_controller( int id );
float       input( int vk );
vec2        input2( int vk );
float       input_diff( int vk );
vec2        input_diff2( int vk );

// --- extended api (check for specific frame)

float       input_frame( int vk, int frame );
vec2        input_frame2( int vk, int frame );
const char* input_frames( int vk, int frame );

// --- utils

void        input_demo();
void        input_send( int vk ); // @todo
void*       input_save_state( int id, int *size); // @todo
bool        input_load_state( int id, void *ptr, int size); // @todo

// --- events

int         event( int vk ); // 1
int         event_down( int vk ); // OFF -> ON (trigger)
int         event_held( int vk ); // ON -> ON (pressed)
int         event_release( int vk ); // ON -> OFF (release)
int         event_idle( int vk ); // OFF -> OFF

int         event_click( int vk, int ms ); // OFF -> ON -> OFF
int         event_click2( int vk, int ms ); // OFF -> ON -> OFF -> ON -> OFF
int         event_repeat( int vk, int ms ); // [...] ON -> ON -> ON

int         event_chord2( int vk1, int vk2 ); // all vk1 && vk2 are ON
int         event_chord3( int vk1, int vk2, int vk3 ); // all vk1 && vk2 && vk3 are ON
int         event_chord4( int vk1, int vk2, int vk3, int vk4 ); // all vk1 && vk2 && vk3 && vk4 are ON

// --- 1d/2d filters

float       filter_positive( float v ); // [-1..1] -> [0..1]
vec2        filter_positive2( vec2 v ); // [-1..1] -> [0..1]
vec2        filter_deadzone( vec2 v, float deadzone );
vec2        filter_deadzone_4way( vec2 v, float deadzone );

// ---

enum {
    // --- bits: x104 keyboard, x3 mouse, x15 gamepad, x7 window
    // keyboard gaming keys (53-bit): first-class keys for gaming
    KEY_ESC,
    KEY_TICK, KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,  KEY_BS,
    KEY_TAB,   KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,
    KEY_CAPS,     KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L, KEY_ENTER,
    KEY_LSHIFT,       KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,        KEY_RSHIFT,            KEY_UP,
    KEY_LCONTROL,KEY_LALT,            KEY_SPACE,             KEY_RALT,KEY_RCONTROL,  KEY_LEFT,KEY_DOWN,KEY_RIGHT,

    // for completeness, secondary keys below (52-bit). beware!
    KEY_INS,KEY_HOME,KEY_PGUP,KEY_DEL,KEY_END,KEY_PGDN, // beware: different behavior win/osx (also, osx: no home/end).
    KEY_LMETA,KEY_RMETA,KEY_MENU,KEY_PRINT,KEY_PAUSE,KEY_SCROLL,KEY_NUMLOCK, // beware: may trigger unexpected OS behavior. (@todo: add RSHIFT here for win?)
    KEY_MINUS,KEY_EQUAL,KEY_LSQUARE,KEY_RSQUARE,KEY_SEMICOLON,KEY_QUOTE,KEY_HASH,KEY_BAR,KEY_COMMA,KEY_DOT,KEY_SLASH, // beware: non-us keyboard layouts
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12, // beware: complicated on laptops/osx
    KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,KEY_PAD8,KEY_PAD9,KEY_PAD0, // beware: complicated on laptops
    KEY_PADADD,KEY_PADSUB,KEY_PADMUL,KEY_PADDIV,KEY_PADDOT,KEY_PADENTER, // beware: complicated on laptops

    MOUSE_L, MOUSE_M, MOUSE_R,
    GAMEPAD_CONNECTED, GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
    GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_MENU, GAMEPAD_START,
    GAMEPAD_LB, GAMEPAD_RB, GAMEPAD_LTHUMB, GAMEPAD_RTHUMB,
    WINDOW_BLUR, WINDOW_FOCUS, WINDOW_CLOSE, WINDOW_MINIMIZE, WINDOW_MAXIMIZE, WINDOW_FULLSCREEN, WINDOW_WINDOWED,

    // --- floats: x7 gamepad, x3 mouse, x4 touch, x4 window
    GAMEPAD_LPAD, GAMEPAD_LPADX = GAMEPAD_LPAD, GAMEPAD_LPADY,
    GAMEPAD_RPAD, GAMEPAD_RPADX = GAMEPAD_RPAD, GAMEPAD_RPADY,
    GAMEPAD_LT, GAMEPAD_RT, GAMEPAD_BATTERY,
    MOUSE, MOUSE_X = MOUSE, MOUSE_Y, MOUSE_W,
    TOUCH_X1, TOUCH_Y1, TOUCH_X2, TOUCH_Y2,
    WINDOW_RESIZE, WINDOW_RESIZEX = WINDOW_RESIZE, WINDOW_RESIZEY, WINDOW_ORIENTATION, WINDOW_BATTERY,

    // --- strings: x2 gamepad
    GAMEPAD_GUID, GAMEPAD_NAME,
};

#endif


#ifdef INPUT_C
#pragma once

static int controller_id = 0;

static struct controller_t {
    const char* strings[2];
    float floats[7+3+4+4];
    char  bits[104+3+15+7];
} controller[4] = {0}, frame[60] = {0};

static struct controller_t *input_logger(int position, int advance) {
    static int cycle = 0;
    cycle += advance;
    position += cycle;
    position = position >= 0 ? position % 60 : 60-1 + ((position+1) % 60);
    return &frame[position];
}

void input_init() {
    unsigned char* mappings = readfile("./gamecontrollerdb.txt", NULL);
    if( mappings ) { glfwUpdateGamepadMappings(mappings); REALLOC(mappings, 0); }

    void input_update();
    window_hook(input_update, NULL);
}

void input_update() {
    struct controller_t *c = &controller[0]; // @fixme

    char *bits = &c->bits[0];
    float *floats = &c->floats[-GAMEPAD_LPADX];
    const char **strings = &c->strings[-GAMEPAD_GUID];
    float mouse_wheel_old = floats[MOUSE_W];

    struct controller_t clear = {0};
    *c = clear;
    for( int i = 0; i < COUNTOF(c->strings); ++i ) c->strings[i] = "";

    //  glfwSetInputMode(win, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    floats[MOUSE_X] = mx;
    floats[MOUSE_Y] = my;
    struct nk_glfw* glfw = glfwGetWindowUserPointer(win); // from nuklear, because it is overriding glfwSetScrollCallback()
    floats[MOUSE_W] = !glfw ? 0 : mouse_wheel_old + (float)glfw->scroll.x + (float)glfw->scroll.y;
    bits[MOUSE_L] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bits[MOUSE_M] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    bits[MOUSE_R] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    #define k2(VK,GLFW) [KEY_##VK] = GLFW_KEY_##GLFW
    #define k(VK) k2(VK,VK)
    int table[] = {
        k2(ESC,ESCAPE),
        k2(TICK,GRAVE_ACCENT), k(1),k(2),k(3),k(4),k(5),k(6),k(7),k(8),k(9),k(0), k2(BS,BACKSPACE),
        k(TAB),                k(Q),k(W),k(E),k(R),k(T),k(Y),k(U),k(I),k(O),k(P),
        k2(CAPS,CAPS_LOCK),      k(A),k(S),k(D),k(F),k(G),k(H),k(J),k(K),k(L), k(ENTER),
        k2(LSHIFT,LEFT_SHIFT),       k(Z),k(X),k(C),k(V),k(B),k(N),k(M),  k2(RSHIFT,RIGHT_SHIFT),                      k(UP),
        k2(LCONTROL,LEFT_CONTROL),k2(LALT,LEFT_ALT), k(SPACE), k2(RALT,RIGHT_ALT),k2(RCONTROL,RIGHT_CONTROL), k(LEFT),k(DOWN),k(RIGHT),

        k(F1),k(F2),k(F3),k(F4),k(F5),k(F6),k(F7),k(F8),k(F9),k(F10),k(F11),k(F12), k2(PRINT,PRINT_SCREEN),k(PAUSE),
        k2(INS,INSERT),k(HOME),k2(PGUP,PAGE_UP), k2(DEL,DELETE),k(END), k2(PGDN,PAGE_DOWN),
    };
    for(int i = 0; i < COUNTOF(table); ++i) {
        //if( table[i] ) bits[i] = glfwGetKey(win, table[i] ) == GLFW_PRESS;
        bits[i] = ((_GLFWwindow*)win)->keys[ table[i] ];
    }
    #undef k
    #undef k2

    int jid = GLFW_JOYSTICK_1 + 0; // @fixme
    if( glfwGetGamepadName(jid) ) { // glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid) ) {
        bits[GAMEPAD_CONNECTED] = 1;
        strings[GAMEPAD_GUID] = glfwGetJoystickGUID(jid);
        strings[GAMEPAD_NAME] = glfwGetGamepadName(jid);
        floats[GAMEPAD_BATTERY] = 100; //glfwJoystickCurrentPowerLevel(jid);

        GLFWgamepadstate state;
        if (glfwGetGamepadState(jid, &state)) {
            bits[GAMEPAD_A] = state.buttons[GLFW_GAMEPAD_BUTTON_A]; // cross
            bits[GAMEPAD_B] = state.buttons[GLFW_GAMEPAD_BUTTON_B]; // circle
            bits[GAMEPAD_X] = state.buttons[GLFW_GAMEPAD_BUTTON_X]; // square
            bits[GAMEPAD_Y] = state.buttons[GLFW_GAMEPAD_BUTTON_Y]; // triangle

            bits[GAMEPAD_UP] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP];
            bits[GAMEPAD_DOWN] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN];
            bits[GAMEPAD_LEFT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
            bits[GAMEPAD_RIGHT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];

            bits[GAMEPAD_LB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
            bits[GAMEPAD_RB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
            bits[GAMEPAD_MENU] = state.buttons[GLFW_GAMEPAD_BUTTON_BACK];
            bits[GAMEPAD_START] = state.buttons[GLFW_GAMEPAD_BUTTON_START]; // _GUIDE

            bits[GAMEPAD_LTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
            bits[GAMEPAD_RTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB];

            floats[GAMEPAD_LT] = filter_positive(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]); // [-1..+1] -> [0..1]
            floats[GAMEPAD_RT] = filter_positive(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]); // [-1..+1] -> [0..1]

            floats[GAMEPAD_LPADX] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            floats[GAMEPAD_LPADY] = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

            floats[GAMEPAD_RPADX] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            floats[GAMEPAD_RPADY] = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        }
    }

    *input_logger(0,+1) = controller[0];
}

int input_controller(int id) {
    return controller_id >= 0 && controller_id <= 3 ? controller_id = id, 1 : 0;
}

float input_frame( int vk, int frame ) {
    if( controller_id > 0 ) return 0; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    if(vk < GAMEPAD_LPADX) return c->bits[vk]; // if in bits...
    if(vk < GAMEPAD_GUID) return c->floats[vk - GAMEPAD_LPADX]; // if in floats...
    return 0.f; // NAN
}
vec2 input_frame2( int vk, int frame ) {
    return vec2( input_frame(vk, frame), input_frame(vk+1, frame) );
}
const char *input_frames( int vk, int frame ) {
    if( controller_id > 0 ) return ""; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    return vk >= GAMEPAD_GUID ? c->strings[vk - GAMEPAD_GUID] : ""; // if in strings...
}

// --- sugars

float input_diff( int vk ) {
    return input_frame(vk, 0) - input_frame(vk, -1);
}
vec2 input_diff2( int vk ) {
    return vec2( input_diff(vk), input_diff(vk+1) );
}
float input( int vk ) {
    return input_frame( vk, 0 );
}
vec2 input2( int vk ) {
    return vec2( input_frame(vk, 0), input_frame(vk+1, 0) );
}

// --- events

#define MS2FRAME 0.06f // 60 hz/1000 ms

int event( int vk ) {
    float v = input_frame(vk,0);
    return (v * v) > 0;
}
int event_chord2( int vk1, int vk2 ) {
    return event(vk1) && event(vk2);
}
int event_chord3( int vk1, int vk2, int vk3 ) {
    return event(vk1) && event_chord2(vk2, vk3);
}
int event_chord4( int vk1, int vk2, int vk3, int vk4 ) {
    return event(vk1) && event_chord3(vk2, vk3, vk4);
}
int event_down( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) > 0;
} 
int event_held( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) > 0;
} 
int event_release( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) <= 0;
} 
int event_idle( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) <= 0;
} 
int event_repeat( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) > 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) > 0;
}
int event_click( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) <= 0;
}
int event_click2( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME *3/4) > 0 
    && input_frame(vk,-ms * MS2FRAME *2/4) <= 0 && input_frame(vk,-ms * MS2FRAME *1/4) > 0 && input_frame(vk,0) <= 0;
}

#undef MS2FRAME

// --- filters

float filter_positive( float v ) {
    return ( v + 1 ) * 0.5f;
}
vec2  filter_positive2( vec2 v ) {
    return scale2(inc2(v,1), 0.5f);
}

vec2 filter_deadzone( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float mag = sqrt( v.x*v.x + v.y*v.y );
    float nx = v.x / mag, ny = v.y / mag, k = (mag - deadzone) / (1 - deadzone);
    if( k > 1 ) k = 1; // clamp
    // k = k * k; // uncomment for a smoother curve
    return mag < deadzone ? vec2(0, 0) : vec2(nx * k, ny * k);
}

vec2 filter_deadzone_4way( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float v0 = v.x*v.x < deadzone*deadzone ? 0 : v.x;
    float v1 = v.y*v.y < deadzone*deadzone ? 0 : v.y;
    return vec2(v0, v1);
}

// ----------------------------------------------------------------------------

void input_demo() {
    if( ui_begin("Keyboard",0) ) {
        ui_labelb("A", input(KEY_A));
        ui_labelb("B", input(KEY_B));
        ui_labelb("C", input(KEY_C));
        ui_labelb("D...", input(KEY_D));
        ui_end();
    }
    if( ui_begin("Mouse",0) ) {
        ui_labelf("X", input(MOUSE_X));
        ui_labelf("Y", input(MOUSE_Y));
        ui_labelf("W", input(MOUSE_W));
        ui_labelb("L", input(MOUSE_L));
        ui_labelb("M", input(MOUSE_M));
        ui_labelb("R", input(MOUSE_R));
        ui_end();
    }
    if( ui_begin("Pad",0) ) {
        static int gamepad_id = 0;
        const char *list[] = {"1","2","3","4"};
        ui_list("Gamepad", list, 4, &gamepad_id);

        input_controller(gamepad_id);

        ui_labels("Name", input_frames(GAMEPAD_NAME,0));
        ui_labelb("Connected", input(GAMEPAD_CONNECTED));

        ui_separator();

        ui_labelb("A", input(GAMEPAD_A) );
        ui_labelb("B", input(GAMEPAD_B) );
        ui_labelb("X", input(GAMEPAD_X) );
        ui_labelb("Y", input(GAMEPAD_Y) );
        ui_labelb("Up", input(GAMEPAD_UP) );
        ui_labelb("Down", input(GAMEPAD_DOWN) );
        ui_labelb("Left", input(GAMEPAD_LEFT) );
        ui_labelb("Right", input(GAMEPAD_RIGHT) );
        ui_labelb("Menu", input(GAMEPAD_MENU) );
        ui_labelb("Start", input(GAMEPAD_START) );

        ui_separator();

        ui_labelf("Left pad x", input(GAMEPAD_LPADX) );
        ui_labelf("Left pad y", input(GAMEPAD_LPADY) );
        ui_labelf("Left trigger", input(GAMEPAD_LT) );
        ui_labelb("Left bumper", input(GAMEPAD_LB) );
        ui_labelb("Left thumb", input(GAMEPAD_LTHUMB) );

        vec2 v = filter_deadzone( input2(GAMEPAD_LPADX), 0.1f );
        ui_labelf("Filtered pad x", v.x);
        ui_labelf("Filtered pad y", v.y);

        ui_separator();

        ui_labelf("Right pad x", input(GAMEPAD_RPADX) );
        ui_labelf("Right pad y", input(GAMEPAD_RPADY) );
        ui_labelf("Right trigger", input(GAMEPAD_RT) );
        ui_labelb("Right bumper", input(GAMEPAD_RB) );
        ui_labelb("Right thumb", input(GAMEPAD_RTHUMB) );

        vec2 w = filter_deadzone( input2(GAMEPAD_RPADX), 0.1f );
        ui_labelf("Filtered pad x", w.x);
        ui_labelf("Filtered pad y", w.y);

        ui_end();
    }
    if( ui_begin("Event",0) ) {
        ui_labelb("[1] Down", event_down(KEY_1) );
        ui_labelb("[2] Held", event_held(KEY_2) );
        ui_labelb("[3] Release", event_release(KEY_3) );
        ui_labelb("[4] Idle", event_idle(KEY_4) );
        ui_labelb("[5] Click", event_click(KEY_5,250) );
        ui_labelb("[6] Click2", event_click2(KEY_6,500) );
        ui_labelb("[7] Repeat", event_repeat(KEY_7,1000) );
        ui_end();
    }
}

#endif
