// ----------------------------------------------------------------------------

static void fwk_pre_init() {
    window_icon(va("%s.png", app_name()));
    window_icon(va("%s.ico", app_name()));

    glfwPollEvents();

    profile_init();
    ddraw_init();
    sprite_init();
    kit_init();

    storage_mount("save/"); // for ems
    storage_read(); // for ems
    touch_init(); // for ems
    // window_swap();

    script_init();
}
static void fwk_post_init(float refresh_rate) {
    // cook cleanup
    cook_stop();

    // initialize audio after cooking
    // reasoning for this: do not launch audio threads while cooks are in progress, so there is more cpu for cooking actually
    audio_init(0);
    midi_init();

    // mount all vfs
    vfs_reload();

    // config nuklear UI (after VFS mounting, as UI needs cooked fonts here)
    nk_config_custom_fonts();
    nk_config_custom_style();

    // @todo
    // window_icon(vfs_file(va("%s.png", app)));
    // window_icon(vfs_file(va("%s.ico", app)));

    // init more subsystems
#ifndef __EMSCRIPTEN__ // @fixme ems -> shaders
    scene_push();      // create an empty scene by default
#endif
    input_init();
    network_init();

    // display window
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    randset(time_ns());
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();
}

// ----------------------------------------------------------------------------

static
void fwk_quit(void) {
    storage_flush();
    midi_quit();
}

void fwk_init() {
    do_once {
        // install signal handlers
        ifdef(debug, signals_install());

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // init glfw
        glfw_init();

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        chdir(app_path());

        // skip tcc argvs (if invoked as tcc file.c fwk.c -g -run)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            __argc = 0;
        }

        // create or update cook.zip file
        if( /* !COOK_ON_DEMAND && */ file_exist(COOK_INI) && cook_jobs() ) {
            cook_start(COOK_INI, "**", 0|COOK_ASYNC|COOK_CANCELABLE );
        }

        atexit(fwk_quit);
    }
}

// Enable more performant GPUs on laptops. Does this work into a dll?
// int NvOptimusEnablement = 1;
// int AmdPowerXpressRequestHighPerformance = 1;

#if is(linux) && is(tcc) // fixes `tcc: error: undefined symbol '__dso_handle'`
int __dso_handle; // compiled with: `tcc demo.c fwk.c -D__STDC_NO_VLA__ -lX11`
#endif

#if is(win32) && is(tcc) // fixes `tcc: error: undefined symbol '_InterlockedExchangeAdd'` when compiling with `-m64` flag
__CRT_INLINE LONG _InterlockedExchangeAdd(LONG volatile *add, LONG val) {
    LONG old;
    do old = *add; while( InterlockedCompareExchange(add, old + val, old) != old );
    return old;
}
__CRT_INLINE LONGLONG _InterlockedExchangeAdd64(LONGLONG volatile *add, LONGLONG val) { // 64bit version, for completeness
    LONGLONG old;
    do old = *add; while( InterlockedCompareExchange64(add, old + val, old) != old );
    return old;
}
#endif

#ifdef ZIG_CC
static int IN6_IS_ADDR_V4MAPPED(const struct in6_addr *a) { return ((a->s6_words[0]==0) && (a->s6_words[1]==0) && (a->s6_words[2]==0) && (a->s6_words[3]==0) && (a->s6_words[4]==0) && (a->s6_words[5]==0xffff)); }
const struct in6_addr in6addr_any; // = IN6ADDR_ANY_INIT;
//static const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
#endif

