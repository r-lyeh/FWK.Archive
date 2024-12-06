// window framework
// - rlyeh, public domain

void   window_create(float zoom, int flags);
void   window_title(const char *title);
int    window_swap(void **pixels);
int    window_width();
int    window_height();
int    window_has_focus();
void   window_focus();
double window_time();
double window_delta();
void*  window_handle();

bool   window_hook(void (*func)(), void* userdata);
void   window_unhook(void (*func)());


#ifdef WINDOW_C
#pragma once

static GLFWwindow *win;
static int w, h;
static double t, dt;
static char win_title[128] = {0};

void* window_handle() {
    return win;
}

static void (*hooks[64])() = {0};
static void *userdatas[64] = {0};

bool window_hook(void (*func)(), void* user) {
    window_unhook( func );
    for( int i = 0; i < 64; ++i ) {
        if( !hooks[i] ) {
            hooks[i] = func;
            userdatas[i] = user;
            return true;
        }
    }
    return false;
}
void window_unhook(void (*func)()) {
    for( int i = 0; i < 64; ++i ) {
        if(hooks[i] == func) {
            hooks[i] = 0;
            userdatas[i] = 0;
        }
    }
}

void window_create(float zoom, int flags) {
    zoom = (zoom < 1 ? zoom * 100 : zoom > 100 ? 100 : zoom);

    fwk_init();

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // 3
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int keep_aspect_ratio = 1;
    // glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // makes it non-resizable
    glfwWindowHint(GLFW_SAMPLES, 4); // x4 AA

    if( zoom >= 100 ) {
    // full screen
    win = glfwCreateWindow(mode->width, mode->height, "", monitor, NULL);
    }
    else if( zoom >= 99 ) {
    // full screen windowed
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    win = glfwCreateWindow(mode->width, mode->height, "", monitor, NULL);
    } else {
    // windowed
    win = glfwCreateWindow(width * zoom / 100, height * zoom / 100, "", NULL, NULL);
    glfwSetWindowPos(win, xpos + (width - width * zoom / 100) / 2, ypos + (height - height * zoom / 100) / 2);
    }

    glfwMakeContextCurrent(win);

    gladLoadGL(glfwGetProcAddress);
    glDebugEnable();

    printf("Monitor: %s\n", glfwGetMonitorName(monitor));
    printf("GPU device: %s\n", glGetString(GL_RENDERER));
    printf("GPU driver: %s\n", glGetString(GL_VERSION));

    glfwSwapInterval(1);

    if(keep_aspect_ratio)
    glfwSetWindowAspectRatio(win, mode->width, mode->height);

    glfwPollEvents();

    window_swap(NULL);
    ddraw_init();
    input_init();
    script_init();
    audio_init(0);

    t = glfwGetTime();
}
int window_swap(void **pixels) {
    int ready = !glfwWindowShouldClose(win);
    if( ready ) {
        glfwPollEvents();

        // input_update(); // already hooked!

        double now = glfwGetTime();
        dt = now - t;
        t = now;

        static unsigned frames = 0;
        static double t = 0;
        t += window_delta();
        if( t >= 1.0 ) {
            char buf[256];
            snprintf(buf, 256, "%s | %5.2f fps", win_title, frames / t);
            glfwSetWindowTitle(win, buf);
            frames = 0;
            t = 0;
        }
        ++frames;

        static int first = 1;
        if( !first ) glfwSwapBuffers(win);
        first = 0;

        glfwGetFramebufferSize(win, &w, &h); //glfwGetWindowSize(win, &w, &h);
        glNewFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // run user-defined hooks
        for(int i = 0; i < 64; ++i) {
            if( hooks[i] ) hooks[i]( userdatas[i] );
        }
    }
    return ready;
}
int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_time() {
    return t;
}
double window_delta() {
    return dt;
}
void window_title(const char *title) {
    if( !win_title[0] ) glfwSetWindowTitle(win, title);
    snprintf(win_title, 128, "%s", title);
}
void window_focus() {
    glfwFocusWindow(win);
}
int window_has_focus() {
    return !!glfwGetWindowAttrib(win, GLFW_FOCUSED);
}

#endif
