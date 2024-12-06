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
static char videorec_file[512];
static int locked_aspect_ratio = 0;

void window_drop_callback(GLFWwindow* window, int count, const char** paths) {
    // @fixme: win: convert from utf8 to window16 before processing
    // @fixme: remove USERNAME for nonwin32
    // @fixme: wait until any active import (launch) is done

    char pathdir[512]; snprintf(pathdir, 512, "%s/import/%llu_%s/", ART, date(), ifdef(linux, getlogin(), getenv("USERNAME")));
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

static // @todo
void window_create_from_handle(void *handle, int flags) {
    fwk_init();
    if(!t) t = glfwGetTime();

window = handle;
glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
glDebugEnable();

    // 0:disable vsync, 1:enable vsync, <0:adaptive (allow vsync when framerate is higher than syncrate and disable vsync when framerate drops below syncrate)
    flags |= optioni("--with-vsync", 1) || flag("--with-vsync") ? WINDOW_VSYNC : WINDOW_VSYNC_DISABLED;
    flags |= optioni("--with-vsync-adaptive", 0) || flag("--with-vsync-adaptive") ? WINDOW_VSYNC_ADAPTIVE : 0;
    int has_adaptive_vsync = glfwExtensionSupported("WGL_EXT_swap_control_tear") || glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("EXT_swap_control_tear");
    int wants_adaptive_vsync = (flags & WINDOW_VSYNC_ADAPTIVE);
    int interval = has_adaptive_vsync && wants_adaptive_vsync ? -1 : (flags & WINDOW_VSYNC_DISABLED ? 0 : 1);
    glfwSwapInterval(interval);

    GLFWmonitor* monitor = glfwGetWindowMonitor(window); if(!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    PRINTF("Monitor: %s (%dHz, vsync=%d)\n", glfwGetMonitorName(monitor), mode->refreshRate, interval);
    PRINTF("GPU device: %s\n", glGetString(GL_RENDERER));
    PRINTF("GPU driver: %s\n", glGetString(GL_VERSION));

    int lock_aspect_ratio = !!( flags & (WINDOW_SQUARE | WINDOW_PORTRAIT | WINDOW_LANDSCAPE | WINDOW_FIXED));
    if (lock_aspect_ratio) window_lock_aspect(w, h);

    // window_cursor(flags & WINDOW_NO_MOUSE ? false : true);

    glfwSetDropCallback(window, window_drop_callback);

    glfwPollEvents();

    fwk_pre_init_subsystems();

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

    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    fwk_post_init_subsystems();

    hz = mode->refreshRate;

    //    t = glfwGetTime();
}

void window_create(float zoom, int flags) {
    fwk_init();
    if(!t) t = glfwGetTime();

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    int area_width, area_height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &area_width, &area_height);
    float width = area_width, height = area_height, ratio = width / (height + !!height);

#ifdef __APPLE__
//glfwInitHint( GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE );
//glfwWindowHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );
#endif

    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // osx
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // osx, 2:#version150,3:330
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //osx
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

    zoom = (zoom < 1 ? zoom * 100 : zoom > 100 ? 100 : zoom);
    if( zoom >= 100 ) {
        // full screen
        window = glfwCreateWindow(width = mode->width, height = mode->height, "", monitor, NULL);
    }
    else if( zoom >= 99 ) {
        // full screen windowed
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(width = mode->width, height = mode->height, "", monitor, NULL);
    } else {
        // windowed
        if( flags & WINDOW_SQUARE )   width = height = width > height ? height : width;
        if( flags & WINDOW_LANDSCAPE ) if( width < height ) height = width * ratio;
        if( flags & WINDOW_PORTRAIT )  if( width > height ) width = height * (1.f / ratio);

        window = glfwCreateWindow(width * zoom / 100, height * zoom / 100, "", NULL, NULL);
        glfwSetWindowPos(window, xpos = xpos + (area_width - width * zoom / 100) / 2, ypos = ypos + (area_height - height * zoom / 100) / 2);
    }
    wprev = w, hprev = h;
    xprev = xpos, yprev = ypos;

    window_create_from_handle(window, flags);
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

int window_needs_flush = 1;
void window_flush() {
    // flush systems that are batched and need to be rendered before frame swapping. order matters.
    if( window_needs_flush ) {
        window_needs_flush = 0;

        fwk_pre_swap_subsystems();
    }
}
void window_lock_fps(float fps) {
    hz = fps;
}
void window_unlock_fps() {
    hz = 0;
}
int window_swap() {
    int ready = !glfwWindowShouldClose(window);
    if( !ready ) {
        #if OFLAG < 3
        #if WITH_SELFIES

//            static int frame = 100;
            bool do_it = (cooker_progress() >= 100); // && ( frame > 0 && !--frame ); // || input_down(KEY_F12)
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
        if( videorec_file[0] ) {
            for( FILE *fp = fopen(videorec_file, "a+b"); fp; fclose(fp), fp = 0) {
                void* rgb = screenshot(-3); // 3 RGB, -3 BGR
                jo_write_mpeg(fp, rgb, window_width(), window_height(), 24);  // 24fps
            }
            // videorec_file[0] = 0;
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

        // @todo: deprecate me, this is only useful for apps that plan to use ddraw without any camera setup
        // ddraw_flush();

        // run user-defined hooks
        for(int i = 0; i < 64; ++i) {
            if( hooks[i] ) hooks[i]( userdatas[i] );
        }

        first = 0;
    }
    return ready;
}

int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_aspect() {
    return (double)w / (h + !!h);
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
static
GLFWmonitor *window_find_monitor(int wx, int wy) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    // find best monitor given current window coordinates. @todo: select by ocuppied window area inside each monitor instead.
    int num_monitors = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&num_monitors);
    for( int i = 0; i < num_monitors; ++i) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
        monitor = wx >= mx && wx <= (mx+mw) && wy >= my && wy <= (my+mh) ? monitors[i] : monitor;
    }

    return monitor;
}
void window_fullscreen(int enabled) {
    fullscreen = !!enabled;
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
}
int window_has_fullscreen() {
    return fullscreen;
}
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

void window_videorec(const char* filename_mpg) {
    snprintf(videorec_file, 512, "%s", filename_mpg ? filename_mpg : "");
}
int window_has_videorec() {
    return !!videorec_file[0];
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
