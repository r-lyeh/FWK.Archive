//-----------------------------------------------------------------------------
// fps locking

static volatile float framerate = 0;
static volatile unsigned fps_active, timer_counter, loop_counter;
static
int fps__timing_thread(void *arg) {
    int64_t ns_excess = 0;
    while( fps_active ) {
        if( framerate <= 0 ) {
            loop_counter = timer_counter = 0;
            sleep_ms(250);
        } else {
            timer_counter++;
            int64_t tt = (int64_t)(1e9/(float)framerate) - ns_excess;
            uint64_t took = -time_ns();
        #ifdef _WIN32
            timeBeginPeriod(1); Sleep( tt > 0 ? tt/1e6 : 0 );
        #else
            sleep_ns( (float)tt );
        #endif
            took += time_ns();
            ns_excess = took - tt;
            if( ns_excess < 0 ) ns_excess = 0;
            //puts( strf("%lld", ns_excess) );
        }
    }
    fps_active = 1;

    (void)arg;
    return thread_exit(0), 0;
}
static
void fps_locker( int on ) {
    if( on ) {
        // private threaded timer
        fps_active = 1, timer_counter = loop_counter = 0;
        thread_init( fps__timing_thread, 0, "fps__timing_thread()", 0 );
    } else {
        fps_active = 0;
    }
}
// function that locks render to desired `framerate` framerate (in FPS).
// - assumes fps_locker() was enabled beforehand.
// - returns true if must render, else 0.
static
int fps_wait() {
    if( framerate <= 0 ) return 1;
    if( !fps_active ) return 1;

    // if we throttled too much, cpu idle wait
    while( fps_active && (loop_counter > timer_counter) ) {
        //thread_yield();
        sleep_ns(100);
    }

    // max auto frameskip is 10: ie, even if speed is low paint at least one frame every 10
    enum { maxframeskip = 10 };
    if( timer_counter > loop_counter + maxframeskip ) {
        loop_counter = timer_counter;
    }
    loop_counter++;

    // only draw if we are fast enough, otherwise skip the frame
    return loop_counter >= timer_counter;
}
static
void window_vsync(float hz) {
    if( hz <= 0 ) return;
    do_once fps_locker(1);
    framerate = hz;
    fps_wait();
}

//-----------------------------------------------------------------------------

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

static GLFWwindow *window;
static int w, h, xpos, ypos, paused;
static int fullscreen, xprev, yprev, wprev, hprev;
static uint64_t frame_count;
static double t, dt, fps, hz = 0.00;
static char title[128] = {0};
static char screenshot_file[512];
static int locked_aspect_ratio = 0;

// -----------------------------------------------------------------------------
// glfw

struct app {
    GLFWwindow *window;
    int width, height, keep_running;

    struct nk_context *ctx;
    struct nk_glfw *glfw;
} appHandle = {0}, *g;

static void glfw_error_callback(int error, const char *description) {
    if( is(osx) && error == 65544 ) return; // whitelisted
    PANIC("%s (error %x)", description, error);
}

void glfw_quit(void) {
    do_once {
        if(g->glfw) nk_glfw3_shutdown(g->glfw);
        glfwTerminate();
    }
}

void glfw_init() {
    do_once {
        g = &appHandle;

        glfwSetErrorCallback(glfw_error_callback);
        int ok = !!glfwInit();
        assert(ok); // if(!ok) PANIC("cannot initialize glfw");

        atexit(glfw_quit); //glfwTerminate);
    }
}

void window_drop_callback(GLFWwindow* window, int count, const char** paths) {
    // @fixme: win: convert from utf8 to window16 before processing
    // @fixme: wait until any active import (launch) is done

    char pathdir[512]; snprintf(pathdir, 512, "%s/import/%llu_%s/", ART, (unsigned long long)date(), ifdef(linux, getlogin(), getenv("USERNAME")));
    mkdir( pathdir, 0777 );

    int errors = 0;
    for( int i = 0; i < count; ++i ) {
        const char *src = paths[i];
        const char *dst = va("%s%s", pathdir, file_name(src));
        errors += file_copy(src, dst) ? 0 : 1;
    }

    if( errors ) PANIC("%d errors found during file dropping", errors);
    else  app_reload();

    (void)window;
}

void window_hints(unsigned flags) {
    #ifdef __APPLE__
    //glfwInitHint( GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );
    #endif

    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // osx
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // osx, 2:#version150,3:330
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //osx+ems
    glfwWindowHint(GLFW_STENCIL_BITS, 8); //osx
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //glfwWindowHint( GLFW_RED_BITS, 8 );
    //glfwWindowHint( GLFW_GREEN_BITS, 8 );
    //glfwWindowHint( GLFW_BLUE_BITS, 8 );
    //glfwWindowHint( GLFW_ALPHA_BITS, 8 );
    //glfwWindowHint( GLFW_DEPTH_BITS, 24 );

    //glfwWindowHint(GLFW_AUX_BUFFERS, Nth);
    //glfwWindowHint(GLFW_STEREO, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    // Prevent fullscreen window minimize on focus loss
    glfwWindowHint( GLFW_AUTO_ICONIFY, GL_FALSE );

    // Fix SRGB on intels
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // makes it non-resizable
    if(flags & WINDOW_MSAA2) glfwWindowHint(GLFW_SAMPLES, 2); // x2 AA
    if(flags & WINDOW_MSAA4) glfwWindowHint(GLFW_SAMPLES, 4); // x4 AA
    if(flags & WINDOW_MSAA8) glfwWindowHint(GLFW_SAMPLES, 8); // x8 AA
}

enum app_window_mode {
    APP_WIN_NORMAL = 'w', // normal window mode, user can resize/maximize
    APP_WIN_MAXIMIZED = 'm', // like normal, but starts maximized
    APP_WIN_FIXED = 'x', // fixed size mode. User can't resize/maximize
    APP_WIN_FULLSCREEN = 'f', // real fullscreen. In almost all frontends changes the screen's resolution
    APP_WIN_FULLSCREEN_DESKTOP = 'd' // "virtual" fullscreen. Removes windows decoration. expands window to full size, changes window size
};

struct nk_glfw *window_handle_glfw() {
    return g->glfw;
}

bool window_poll() {
    glfwPollEvents();
    nk_glfw3_new_frame(g->glfw);
    if( glfwWindowShouldClose(g->window) ) {
        return 0;
    }
    return 1;
}

bool window_create_from_handle(void *handle, float scale, unsigned flags) {
    glfw_init();
    fwk_init();
    if(!t) t = glfwGetTime();

    #ifdef __EMSCRIPTEN__
    scale = 100.f;
    #endif

    scale = (scale < 1 ? scale * 100 : scale);
    bool FLAGS_FULLSCREEN = scale > 100;
    bool FLAGS_FULLSCREEN_DESKTOP = scale == 100;
    bool FLAGS_WINDOWED = scale < 100;
    scale = (scale > 100 ? 100 : scale) / 100.f;
    int winWidth = window_canvas().w * scale;
    int winHeight = window_canvas().h * scale;

    window_hints(flags);

    GLFWmonitor* monitor = NULL;
    #ifndef __EMSCRIPTEN__
    if( FLAGS_FULLSCREEN || FLAGS_FULLSCREEN_DESKTOP ) {
        monitor = glfwGetPrimaryMonitor();
    }
    if( FLAGS_FULLSCREEN_DESKTOP ) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        winWidth = mode->width;
        winHeight = mode->height;
    }
    if( FLAGS_WINDOWED ) {
        // windowed
        float ratio = (float)winWidth / (winHeight + !winHeight);
        if( flags & WINDOW_SQUARE )    winWidth = winHeight = winWidth > winHeight ? winHeight : winWidth;
        //if( flags & WINDOW_LANDSCAPE ) if( winWidth < winHeight ) winHeight = winWidth * ratio;
        if( flags & WINDOW_PORTRAIT )  if( winWidth > winHeight ) winWidth = winHeight * (1.f / ratio);
    }
    #endif

    window = handle ? handle : glfwCreateWindow(winWidth, winHeight, "", monitor, NULL);
    if( !window ) return PANIC("GLFW Window creation failed"), false;

    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    if( flags & WINDOW_FIXED ) { // disable resizing
        glfwSetWindowSizeLimits(window, w, h, w, h);
    }
    if( flags & (WINDOW_SQUARE | WINDOW_PORTRAIT | WINDOW_LANDSCAPE | WINDOW_ASPECT) ) { // keep aspect ratio
        window_lock_aspect(w, h);
    }

    #ifndef __EMSCRIPTEN__
    if( FLAGS_WINDOWED ) {
        // center window
        monitor = monitor ? monitor : glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        int area_width = mode->width, area_height = mode->height;
        glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &area_width, &area_height);
        glfwSetWindowPos(window, xpos = xpos + (area_width - winWidth) / 2, ypos = ypos + (area_height - winHeight) / 2);
        //printf("%dx%d @(%d,%d) [res:%dx%d]\n", winWidth, winHeight, xpos,ypos, area_width, area_height );

        wprev = w, hprev = h;
        xprev = xpos, yprev = ypos;
    }
    #endif

    glfwMakeContextCurrent(window);

    #ifdef __EMSCRIPTEN__
    if( FLAGS_FULLSCREEN ) window_fullscreen(1);
    #else
    gladLoadGL(glfwGetProcAddress);
    #endif

    glDebugEnable();
    
    ui_ctx = nk_glfw3_init(&ui_glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
    
    //glEnable(GL_TEXTURE_2D);

    // 0:disable vsync, 1:enable vsync, <0:adaptive (allow vsync when framerate is higher than syncrate and disable vsync when framerate drops below syncrate)
    flags |= optioni("--with-vsync", 1) || flag("--with-vsync") ? WINDOW_VSYNC : WINDOW_VSYNC_DISABLED;
    flags |= optioni("--with-vsync-adaptive", 0) || flag("--with-vsync-adaptive") ? WINDOW_VSYNC_ADAPTIVE : 0;
    int has_adaptive_vsync = glfwExtensionSupported("WGL_EXT_swap_control_tear") || glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("EXT_swap_control_tear");
    int wants_adaptive_vsync = (flags & WINDOW_VSYNC_ADAPTIVE);
    int interval = has_adaptive_vsync && wants_adaptive_vsync ? -1 : (flags & WINDOW_VSYNC_DISABLED ? 0 : 1);
    glfwSwapInterval(interval);

    const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());
    PRINTF("Monitor: %s (%dHz, vsync=%d)\n", glfwGetMonitorName(monitor ? monitor : glfwGetPrimaryMonitor()), mode->refreshRate, interval);
    PRINTF("GPU device: %s\n", glGetString(GL_RENDERER));
    PRINTF("GPU driver: %s\n", glGetString(GL_VERSION));

    g->ctx = ui_ctx;
    g->glfw = &ui_glfw;
    g->window = window;
    g->width = window_width();
    g->height = window_height();

    // window_cursor(flags & WINDOW_NO_MOUSE ? false : true);
    glfwSetDropCallback(window, window_drop_callback);

    // camera inits for fwk_pre_init() -> ddraw_flush() -> get_active_camera()
    // static camera_t cam = {0}; id44(cam.view); id44(cam.proj); extern camera_t *last_camera; last_camera = &cam;
    fwk_pre_init();

        // display a progress bar meanwhile cooker is working in the background
        // Sleep(500);
        if( file_directory(TOOLS) && cooker_jobs() )
        while( cooker_progress() < 100 ) {
            for( int frames = 0; frames < 10 && window_swap(); frames += cooker_progress() >= 100 ) {
                window_title(va("Cooking assets %.2d%%", cooker_progress()));

                glfwGetFramebufferSize(window, &w, &h);
                glNewFrame();

                static float previous[100] = {0};

                #define draw_cooker_progress_bar(JOB_ID, JOB_MAX, PERCENT) do { \
                   /* NDC coordinates (2d): bottom-left(-1,-1), center(0,0), top-right(+1,+1) */ \
                   float progress = (PERCENT+1) / 100.f; if(progress > 1) progress = 1; \
                   float speed = progress < 1 ? 0.2f : 0.5f; \
                   float smooth = previous[JOB_ID] = progress * speed + previous[JOB_ID] * (1-speed); \
                   \
                   float pixel = 2.f / window_height(), dist = smooth*2-1, y = pixel*3*JOB_ID; \
                   if(JOB_ID==0)ddraw_line(vec3(-1,y-pixel*2,0), vec3(1,   y-pixel*2,0)); /* full line */ \
                   ddraw_line(vec3(-1,y-pixel  ,0), vec3(dist,y-pixel  ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+0      ,0), vec3(dist,y+0      ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+pixel  ,0), vec3(dist,y+pixel  ,0)); /* progress line */ \
                   if(JOB_ID==JOB_MAX-1)ddraw_line(vec3(-1,y+pixel*2,0), vec3(1,   y+pixel*2,0)); /* full line */ \
                } while(0)

                for(int i = 0; i < cooker_jobs(); ++i) draw_cooker_progress_bar(i, cooker_jobs(), jobs[i].progress);
                // draw_cooker_progress_bar(0, 1, cooker_progress());

                ddraw_flush();

                do_once window_visible(1);
            }
            // set black screen
            glfwGetFramebufferSize(window, &w, &h);
            glNewFrame();
            window_swap();
            window_title("");
        }

    fwk_post_init(mode->refreshRate);
    return true;
}

bool window_create(float scale, unsigned flags) {
    return window_create_from_handle(NULL, scale, flags);
}

static double boot_time = 0;

static
char* window_stats() {
    static double num_frames = 0, begin = FLT_MAX, prev_frame = 0;

    double now = time_ss();
    if( boot_time < 0 ) boot_time = now;

    if( begin > now ) {
        begin = now;
        num_frames = 0;
    }
    if( (now - begin) >= 0.25f ) {
        fps = num_frames * (1.f / (now - begin));
    }
    if( (now - begin) > 1 ) {
        begin = now + ((now - begin) - 1);
        num_frames = 0;
    }

    // @todo: print %used/%avail kib mem, %used/%avail objs as well
    static char buf[192];
    snprintf(buf, 192, "%s | boot %.2fs | %5.2ffps (%.2fms)", title, !boot_time ? now : boot_time, fps, (now - prev_frame) * 1000.f);

    prev_frame = now;
    ++num_frames;

    return buf + 3 * (buf[0] == ' ');
}

static int window_needs_flush = 1;
static int unlock_preswap_events = 0;
void window_flush() {

#ifdef __EMSCRIPTEN__
    vec3 background = vec3(0,0,0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glClearColor(background.x / 255, background.y / 255, background.z / 255, 1 );

    /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    nk_glfw3_render(g->glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    glfwSwapBuffers(g->window);

    return;
#endif

    // flush batching systems that need to be rendered before frame swapping. order matters.
    if( window_needs_flush ) {
        window_needs_flush = 0;

        if( unlock_preswap_events ) {
            // flush all batched sprites before swap
            sprite_flush();

            // queue ui drawcalls for profiler
            // hack: skip first frame, because of conflicts with ui_menu & ui_begin auto-row order
            static int once = 0; if(once) profile_render(); once = 1;

            // flush all debugdraw calls before swap
            dd_ontop = 0;
            ddraw_flush();
            glClear(GL_DEPTH_BUFFER_BIT);
            dd_ontop = 1;
            ddraw_flush();

            font_goto(0,0);

            // flush all batched ui before swap (creates single dummy if no batches are found)
            ui_create();
            ui_render();
        }
    }
}
void window_lock_fps(float fps) {
    hz = fps;
}
void window_unlock_fps() {
    hz = 0;
}
int window_swap() {

#ifdef __EMSCRIPTEN__ // @fixme, add all missing features below

    static bool first_frame = 1;
    if( first_frame ) {
        first_frame = 0;
    } else {
        window_flush();
    }

    if( !window_poll() ) {
        window_loop_exit();
        return 0;
    }
    return 1;

    {
        int ready = !glfwWindowShouldClose(window);
        if( ready ) {
            window_flush();
            glfwPollEvents();
            glfwSwapBuffers(window);
        } else {
            window_loop_exit(); // finish emscripten loop automatically
        }
        return ready;
    }

#endif

    int ready = !glfwWindowShouldClose(window);
    if( !ready ) {

        #if OFLAG < 3
        #if WITH_SELFIES

//            static int frame = 100;
            bool do_it = cooker_progress() >= 100; // && ( frame > 0 && !--frame ); // || input_down(KEY_F12)
            if(do_it) {
               snprintf(screenshot_file, 512, "%s.png", app_name());

               int n = 3;
               void *rgb = screenshot(n);
               stbi_flip_vertically_on_write(true);
               if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
                   PANIC("!could not write screenshot file `%s`\n", screenshot_file);
               }
               screenshot_file[0] = 0;
            }

        #endif
        #endif

        window_loop_exit(); // finish emscripten loop automatically

    }
    else {
        static int first = 1;

        window_flush();

        glfwPollEvents();

        // input_update(); // already hooked!

        double now = paused ? t : glfwGetTime();
        dt = now - t;
        t = now;

#if 0
        static unsigned frames = 0;
        static double t = 0;
        t += window_delta();
        if( t >= 1.0 ) {
            fps = frames / t;
            glfwSetWindowTitle(window, va("%s | %5.2f fps (%.2fms)", title, fps, (t*1000.0) / fps));
            frames = 0;
            t = 0;
        }
        ++frames;
#else
        char *st = window_stats();
        static double t = 0;
        t += window_delta();
        if( t >= 0.25 ) {
            glfwSetWindowTitle(window, st);
            t = 0;
        }
#endif

        // save screenshot if queued
        if( screenshot_file[0] ) {
            int n = 3;
            void *rgb = screenshot(n);
            stbi_flip_vertically_on_write(true);
            if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
                PANIC("!could not write screenshot file `%s`\n", screenshot_file);
            }
            screenshot_file[0] = 0;
        }
        if( videorec_active() ) {
            void videorec_frame();
            videorec_frame();
        }

        if( !first ) {
            // glFinish();
            window_vsync(hz);
            glfwSwapBuffers(window);
            ++frame_count;
        }

        glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);
        glNewFrame();
        window_needs_flush = 1;

        // run user-defined hooks
        for(int i = 0; i < 64; ++i) {
            if( hooks[i] ) hooks[i]( userdatas[i] );
        }

        first = 0;
    }
    return ready;
}

void window_loop(void (*function)(void* loopArg), void* loopArg ) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(function, loopArg, 0, 1);
#else
    g->keep_running = true;
    while (g->keep_running)
        function(loopArg);
#endif /* __EMSCRIPTEN__ */
}

void window_loop_exit() {
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#else
    g->keep_running = false;
#endif /* __EMSCRIPTEN__ */
}

vec2 window_canvas() {
#ifdef __EMSCRIPTEN__
    int width = EM_ASM_INT_V(return window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth);
    int height = EM_ASM_INT_V(return window.innerHeight|| document.documentElement.clientHeight|| document.body.clientHeight);
    return vec2(width, height);
#else
    glfw_init();
    const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
    assert( mode );
    return vec2(mode->width, mode->height);
#endif /* __EMSCRIPTEN__ */
}

int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_aspect() {
    return (double)w / (h + !h);
}
double window_time() {
    return t;
}
double window_delta() {
    return dt;
}
double window_fps() {
    return fps;
}
uint64_t window_frame() {
    return frame_count;
}
void window_title(const char *title_) {
    snprintf(title, 128, "%s", title_);
    if( !title[0] ) glfwSetWindowTitle(window, title);
}
void window_icon(const char *file_icon) {
    void *data = vfs_read(file_icon); data = data ? data : file_read(file_icon);
    if( data ) {
        unsigned len = vfs_size(file_icon); len = len ? len : file_size(file_icon);
        if( len ) {
            image_t img = image_from_mem(data, len, IMAGE_RGBA);
            if( img.w && img.h && img.pixels ) {
                GLFWimage images[1];
                images[0].width = img.w;
                images[0].height = img.h;
                images[0].pixels = img.pixels;
                glfwSetWindowIcon(window, 1, images);
                return;
            }
        }
    }
#ifdef _WIN32
    HANDLE hIcon = LoadImageA(0, file_icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if( hIcon ) {
        HWND hWnd = glfwGetWin32Window(window);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        return;
    }
#endif
}
void* window_handle() {
    return window;
}

// -----------------------------------------------------------------------------
// fullscreen

#if 0 // to deprecate

static
GLFWmonitor *window_find_monitor(int wx, int wy) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    // find best monitor given current window coordinates. @todo: select by ocuppied window area inside each monitor instead.
    int num_monitors = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&num_monitors);
#ifdef __EMSCRIPTEN__
    return *monitors;
#else
    for( int i = 0; i < num_monitors; ++i) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
        monitor = wx >= mx && wx <= (mx+mw) && wy >= my && wy <= (my+mh) ? monitors[i] : monitor;
    }
    return monitor;
#endif
}
void window_fullscreen(int enabled) {
    fullscreen = !!enabled;
#ifndef __EMSCRIPTEN__
    if( fullscreen ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
#endif
}
int window_has_fullscreen() {
    return fullscreen;
}

#else

int window_has_fullscreen() {
#ifdef __EMSCRIPTEN__
    EmscriptenFullscreenChangeEvent fsce;
    emscripten_get_fullscreen_status(&fsce);
    return !!fsce.isFullscreen;
#else
    return !!glfwGetWindowMonitor(g->window);
#endif /* __EMSCRIPTEN__ */
}

void window_fullscreen(int enabled) {
    if( window_has_fullscreen() == !!enabled ) return;

#ifdef __EMSCRIPTEN__
    if( enabled ) {
        emscripten_exit_soft_fullscreen();

        /* Workaround https://github.com/kripken/emscripten/issues/5124#issuecomment-292849872 */
        EM_ASM(JSEvents.inEventHandler = true);
        EM_ASM(JSEvents.currentEventHandler = {allowsDeferredCalls:true});

        EmscriptenFullscreenStrategy strategy = {
            .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
            .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE,
            .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
            .canvasResizedCallback = NULL,
            .canvasResizedCallbackUserData = NULL
        };
        /*emscripten_request_fullscreen_strategy(NULL, EM_TRUE, &strategy);*/
        emscripten_enter_soft_fullscreen(NULL, &strategy);
    } else {            
        emscripten_exit_fullscreen();
    }
#else
    if( enabled ) {
        /*glfwGetWindowPos(g->window, &g->window_xpos, &g->window_ypos);*/
        glfwGetWindowSize(g->window, &g->width, &g->height);
        glfwSetWindowMonitor(g->window, glfwGetPrimaryMonitor(), 0, 0, g->width, g->height, GLFW_DONT_CARE);
    } else {            
        glfwSetWindowMonitor(g->window, NULL, 0, 0, g->width, g->height, GLFW_DONT_CARE);
    }
#endif
}

#endif

void window_pause(int enabled) {
    paused = enabled;
}
int window_has_pause() {
    return paused;
}
void window_focus() {
    glfwFocusWindow(window);
}
int window_has_focus() {
    return !!glfwGetWindowAttrib(window, GLFW_FOCUSED);
}
void window_cursor(int visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
int window_has_cursor() {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}
void window_visible(int visible) {
    if(!window) return;
    //if(window) (visible ? glfwRestoreWindow : glfwIconifyWindow)(window);
    (visible ? glfwShowWindow : glfwHideWindow)(window);
    // call glfwpollevents in linux to flush visiblity changes that would happen in next frame otherwise
    #if is(linux) || is(osx)
    glfwPollEvents();
    #endif
}
int window_has_visible() {
    return glfwGetWindowAttrib(window, GLFW_VISIBLE);
}

void window_screenshot(const char* filename_png) {
    snprintf(screenshot_file, 512, "%s", filename_png ? filename_png : "");
}

void window_lock_aspect(unsigned numer, unsigned denom) {
    if(!window) return;
    if( numer * denom ) {
        glfwSetWindowAspectRatio(window, numer, denom);
    } else {
        glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
}
void window_unlock_aspect() {
    if(!window) return;
    window_lock_aspect(0, 0);
}
