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
    audio_init(0);
}
static void fwk_post_init(float refresh_rate) {
    const char *app = app_name();

    // cooker cleanup
    cooker_stop();

    // display window
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    // mount virtual filesystems later (lower priority)
    bool any_mounted = 0;
    for( int i = 0; i < 256; ++i) {
        bool mounted = false; 
        mounted |= !!vfs_mount(va(".art[%02x].zip", i));
        mounted |= !!vfs_mount(va("%s[%02x].zip", app, i));
        mounted |= !!vfs_mount(va("%s%02x.zip", app, i));
        mounted |= !!vfs_mount(va("%s.%02x", app, i));
        any_mounted |= mounted;
        if(!mounted) break;
    }
    // mount physical filesystems first (higher priority)
    if(!any_mounted) vfs_mount(ART);

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
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();
}

#if 0 // @todo: WINDOW_SIGTRAP
#include <signal.h>
void fwk_on_signal(int sn) {
    PRINTF("!Signal caught: %d (SIGFPE=%d SIGBUS=%d SIGSEGV=%d SIGILL=%d SIGABRT=%d)\n", sn, SIGFPE, SIGBUS, SIGSEGV, SIGILL, SIGABRT );
//    signal(sn, fwk_on_signal); // continue
}
void fwk_install_signal_handler() {
    signal(SIGFPE, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGBUS, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGSEGV, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGILL, fwk_on_signal); // fwk_on_signal or SIG_IGN
    signal(SIGABRT, fwk_on_signal); // fwk_on_signal or SIG_IGN
}
/*
void fwk_on_signal_error(int signum, siginfo_t *siginf, void *puc) {
    // [...]
}
void fwk_install_signal_handler() {
    struct sigaction sigact;
    sigact.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigact.sa_sigaction = fwk_on_signal_error;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGFPE, &sigact, NULL);
    sigaction(SIGILL, &sigact, NULL);
    sigaction(SIGSEGV, &sigact, NULL);
    sigaction(SIGBUS, &sigact, NULL);
    sigaction(SIGABRT, &sigact, NULL);
}
*/
#endif

static
void fwk_quit(void) {
    storage_flush();
}

void fwk_init() {
    do_once {
        // printf("-O%d | %s %s\n", O_FLAG, __DATE__, __TIME__); // display optimization flag

        // signal handler
        // fwk_install_signal_handler();

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
        cooker_config(NULL, NULL, NULL);
        if( file_directory(TOOLS) && cooker_jobs() ) {
            cooker_start( "**", 0|COOKER_ASYNC|COOKER_CANCELABLE );
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

