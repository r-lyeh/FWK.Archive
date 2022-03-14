// gotta love linux
#ifdef __linux
#undef KEY_ESC
#undef KEY_TICK
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9
#undef KEY_0
#undef KEY_BS
#undef KEY_TAB
#undef KEY_Q
#undef KEY_W
#undef KEY_E
#undef KEY_R
#undef KEY_T
#undef KEY_Y
#undef KEY_U
#undef KEY_I
#undef KEY_O
#undef KEY_P
#undef KEY_CAPS
#undef KEY_A
#undef KEY_S
#undef KEY_D
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_ENTER
#undef KEY_LSHIFT
#undef KEY_Z
#undef KEY_X
#undef KEY_C
#undef KEY_V
#undef KEY_B
#undef KEY_N
#undef KEY_M
#undef KEY_RSHIFT
#undef KEY_UP
#undef KEY_LCTRL
#undef KEY_LALT
#undef KEY_SPACE
#undef KEY_RALT
#undef KEY_RCTRL
#undef KEY_LEFT
#undef KEY_DOWN
#undef KEY_RIGHT
#undef KEY_INS
#undef KEY_HOME
#undef KEY_PGUP
#undef KEY_DEL
#undef KEY_END
#undef KEY_PGDN
#undef KEY_LMETA
#undef KEY_RMETA
#undef KEY_MENU
#undef KEY_PRINT
#undef KEY_PAUSE
#undef KEY_SCROLL
#undef KEY_NUMLOCK
#undef KEY_MINUS
#undef KEY_EQUAL
#undef KEY_LSQUARE
#undef KEY_RSQUARE
#undef KEY_SEMICOLON
#undef KEY_QUOTE
#undef KEY_HASH
#undef KEY_BAR
#undef KEY_COMMA
#undef KEY_DOT
#undef KEY_SLASH
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#undef KEY_PAD1
#undef KEY_PAD2
#undef KEY_PAD3
#undef KEY_PAD4
#undef KEY_PAD5
#undef KEY_PAD6
#undef KEY_PAD7
#undef KEY_PAD8
#undef KEY_PAD9
#undef KEY_PAD0
#undef KEY_PADADD
#undef KEY_PADSUB
#undef KEY_PADMUL
#undef KEY_PADDIV
#undef KEY_PADDOT
#undef KEY_PADENTER
#undef MOUSE_L
#undef MOUSE_M
#undef MOUSE_R
#undef GAMEPAD_CONNECTED
#undef GAMEPAD_A
#undef GAMEPAD_B
#undef GAMEPAD_X
#undef GAMEPAD_Y
#undef GAMEPAD_UP
#undef GAMEPAD_DOWN
#undef GAMEPAD_LEFT
#undef GAMEPAD_RIGHT
#undef GAMEPAD_MENU
#undef GAMEPAD_START
#undef GAMEPAD_LB
#undef GAMEPAD_RB
#undef GAMEPAD_LTHUMB
#undef GAMEPAD_RTHUMB
#undef WINDOW_BLUR
#undef WINDOW_FOCUS
#undef WINDOW_CLOSE
#undef WINDOW_MINIMIZE
#undef WINDOW_MAXIMIZE
#undef WINDOW_FULLSCREEN
#undef WINDOW_WINDOWED
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPADY
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPADY
#undef GAMEPAD_LT
#undef GAMEPAD_RT
#undef GAMEPAD_BATTERY
#undef MOUSE
#undef MOUSE
#undef MOUSE_Y
#undef MOUSE_W
#undef TOUCH_X1
#undef TOUCH_Y1
#undef TOUCH_X2
#undef TOUCH_Y2
#undef WINDOW_RESIZE
#undef WINDOW_RESIZE
#undef WINDOW_RESIZEY
#undef WINDOW_ORIENTATION
#undef WINDOW_BATTERY
#undef GAMEPAD_GUID
#undef GAMEPAD_NAME
#endif

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

void input_mappings() {
    char* mappings = vfs_read("gamecontrollerdb.txt");
    if( mappings ) { glfwUpdateGamepadMappings(mappings); /*REALLOC(mappings, 0);*/ }
}

void input_init() {
    do_once {
        input_mappings();
    }
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
    for( int i = 0; i < countof(c->strings); ++i ) c->strings[i] = "";

    struct GLFWwindow *win = window_handle();
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

#if !is(ems) // error: use of undeclared identifier '_GLFWwindow', 'GLFW_GAMEPAD_BUTTON_A' [etc.]
    #define k2(VK,GLFW) [KEY_##VK] = GLFW_KEY_##GLFW
    #define k(VK) k2(VK,VK)
    int table[] = {
        k2(ESC,ESCAPE),
        k2(TICK,GRAVE_ACCENT), k(1),k(2),k(3),k(4),k(5),k(6),k(7),k(8),k(9),k(0), k2(BS,BACKSPACE),
        k(TAB),                k(Q),k(W),k(E),k(R),k(T),k(Y),k(U),k(I),k(O),k(P),
        k2(CAPS,CAPS_LOCK),      k(A),k(S),k(D),k(F),k(G),k(H),k(J),k(K),k(L), k(ENTER),
        k2(LSHIFT,LEFT_SHIFT),       k(Z),k(X),k(C),k(V),k(B),k(N),k(M),  k2(RSHIFT,RIGHT_SHIFT),                      k(UP),
        k2(LCTRL,LEFT_CONTROL),k2(LALT,LEFT_ALT), k(SPACE), k2(RALT,RIGHT_ALT),k2(RCTRL,RIGHT_CONTROL), k(LEFT),k(DOWN),k(RIGHT),

        k(F1),k(F2),k(F3),k(F4),k(F5),k(F6),k(F7),k(F8),k(F9),k(F10),k(F11),k(F12), k2(PRINT,PRINT_SCREEN),k(PAUSE),
        k2(INS,INSERT),k(HOME),k2(PGUP,PAGE_UP), k2(DEL,DELETE),k(END), k2(PGDN,PAGE_DOWN),
    };
    for(int i = 0; i < countof(table); ++i) {
        //if( table[i] ) bits[i] = glfwGetKey(win, table[i] ) == GLFW_PRESS;
        bits[i] = glfwGetKeys(win)[ table[i] ];
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

            floats[GAMEPAD_LT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]); // [-1..+1] -> [0..1]
            floats[GAMEPAD_RT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]); // [-1..+1] -> [0..1]

            floats[GAMEPAD_LPADX] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            floats[GAMEPAD_LPADY] = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

            floats[GAMEPAD_RPADX] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            floats[GAMEPAD_RPADY] = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        }
    }
#endif

    *input_logger(0,+1) = controller[0];
}

int input_use(int id) {
    return controller_id >= 0 && controller_id <= 3 ? controller_id = id, 1 : 0;
}

float input_frame( int vk, int frame ) {
    if( controller_id > 0 ) return 0; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    if(vk < GAMEPAD_LPADX) return c->bits[vk]; // if in bits...
    if(vk < GAMEPAD_GUID) return c->floats[vk - GAMEPAD_LPADX]; // if in floats...
    return 0.f; // NAN?
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

const float MS2FRAME = 0.06f; // 60 hz/1000 ms

int event( int vk ) {
    float v = input_frame(vk,0);
    return (v * v) > 0;
}
int input_chord2( int vk1, int vk2 ) {
    return event(vk1) && event(vk2);
}
int input_chord3( int vk1, int vk2, int vk3 ) {
    return event(vk1) && input_chord2(vk2, vk3);
}
int input_chord4( int vk1, int vk2, int vk3, int vk4 ) {
    return event(vk1) && input_chord3(vk2, vk3, vk4);
}
int input_down( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) > 0;
}
int input_held( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) > 0;
}
int input_up( int vk ) {
    return input_frame(vk,-1) > 0 && input_frame(vk,0) <= 0;
}
int input_idle( int vk ) {
    return input_frame(vk,-1) <= 0 && input_frame(vk,0) <= 0;
}
int input_repeat( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) > 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) > 0;
}
int input_click( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) <= 0;
}
int input_click2( int vk, int ms ) {
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME *3/4) > 0
    && input_frame(vk,-ms * MS2FRAME *2/4) <= 0 && input_frame(vk,-ms * MS2FRAME *1/4) > 0 && input_frame(vk,0) <= 0;
}

#undef MS2FRAME

// --- filters

float input_filter_positive( float v ) {
    return ( v + 1 ) * 0.5f;
}
vec2  input_filter_positive2( vec2 v ) {
    return scale2(inc2(v,1), 0.5f);
}

vec2 input_filter_deadzone( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float mag = sqrt( v.x*v.x + v.y*v.y );
    float nx = v.x / mag, ny = v.y / mag, k = (mag - deadzone) / (1 - deadzone);
    if( k > 1 ) k = 1; // clamp
    // k = k * k; // uncomment for a smoother curve
    return mag < deadzone ? vec2(0, 0) : vec2(nx * k, ny * k);
}

vec2 input_filter_deadzone_4way( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float v0 = v.x*v.x < deadzone*deadzone ? 0 : v.x;
    float v1 = v.y*v.y < deadzone*deadzone ? 0 : v.y;
    return vec2(v0, v1);
}

// ----------------------------------------------------------------------------

void input_demo() {
    if( ui_begin("Keyboard",0) ) {
        ui_const_bool("[Key 1]", input(KEY_1));
        ui_const_bool("[Key 2]", input(KEY_2));
        ui_const_bool("[Key 3]", input(KEY_3));
        ui_const_bool("[Key 4]", input(KEY_4));
        ui_const_bool("[Key 5]", input(KEY_5));
        ui_const_bool("[Key 6]", input(KEY_6));
        ui_const_bool("[Key 7]", input(KEY_7));
        ui_separator();
        ui_const_bool("[Key 1] Down event", input_down(KEY_1) );
        ui_const_bool("[Key 2] Held event", input_held(KEY_2) );
        ui_const_bool("[Key 3] Up event", input_up(KEY_3) );
        ui_const_bool("[Key 4] Idle event", input_idle(KEY_4) );
        ui_const_bool("[Key 5] Click event", input_click(KEY_5,500) );
        ui_const_bool("[Key 6] Click2 event", input_click2(KEY_6,1000) );
        ui_const_bool("[Key 7] Repeat event", input_repeat(KEY_7,750) );
        ui_end();
    }
    if( ui_begin("Mouse",0) ) {
        ui_const_float("X", input(MOUSE_X));
        ui_const_float("Y", input(MOUSE_Y));
        ui_separator();
        ui_const_float("Wheel", input(MOUSE_W));
        ui_separator();
        ui_const_bool("Left", input(MOUSE_L));
        ui_const_bool("Middle", input(MOUSE_M));
        ui_const_bool("Right", input(MOUSE_R));
        ui_end();
    }
    if( ui_begin("GamePad",0) ) {
        static int gamepad_id = 0;
        const char *list[] = {"1","2","3","4"};
        ui_list("Gamepad", list, 4, &gamepad_id);

        input_use(gamepad_id);

        ui_const_string("Name", input_frames(GAMEPAD_NAME,0));
        ui_const_bool("Connected", input(GAMEPAD_CONNECTED));

        ui_separator();

        ui_const_bool("A", input(GAMEPAD_A) );
        ui_const_bool("B", input(GAMEPAD_B) );
        ui_const_bool("X", input(GAMEPAD_X) );
        ui_const_bool("Y", input(GAMEPAD_Y) );
        ui_const_bool("Up", input(GAMEPAD_UP) );
        ui_const_bool("Down", input(GAMEPAD_DOWN) );
        ui_const_bool("Left", input(GAMEPAD_LEFT) );
        ui_const_bool("Right", input(GAMEPAD_RIGHT) );
        ui_const_bool("Menu", input(GAMEPAD_MENU) );
        ui_const_bool("Start", input(GAMEPAD_START) );

        ui_separator();

        ui_const_float("Left pad x", input(GAMEPAD_LPADX) );
        ui_const_float("Left pad y", input(GAMEPAD_LPADY) );
        ui_const_float("Left trigger", input(GAMEPAD_LT) );
        ui_const_bool("Left bumper", input(GAMEPAD_LB) );
        ui_const_bool("Left thumb", input(GAMEPAD_LTHUMB) );

        vec2 v = input_filter_deadzone( input2(GAMEPAD_LPADX), 0.1f );
        ui_const_float("Filtered pad x", v.x);
        ui_const_float("Filtered pad y", v.y);

        ui_separator();

        ui_const_float("Right pad x", input(GAMEPAD_RPADX) );
        ui_const_float("Right pad y", input(GAMEPAD_RPADY) );
        ui_const_float("Right trigger", input(GAMEPAD_RT) );
        ui_const_bool("Right bumper", input(GAMEPAD_RB) );
        ui_const_bool("Right thumb", input(GAMEPAD_RTHUMB) );

        vec2 w = input_filter_deadzone( input2(GAMEPAD_RPADX), 0.1f );
        ui_const_float("Filtered pad x", w.x);
        ui_const_float("Filtered pad y", w.y);

        ui_end();
    }
}
