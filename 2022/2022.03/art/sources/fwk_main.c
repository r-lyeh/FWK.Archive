// ----------------------------------------------------------------------------

//static int threadlocal _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

static void fwk_pre_init_subsystems() {
    profile_init();
    ddraw_init();
    sprite_init();

    // window_swap();

    script_init();
    audio_init(0);
}
static int fwk_unlock_pre_swap_events = 0;
static void fwk_post_init_subsystems() {
    // cooker cleanup
    cooker_stop();

    // mount virtual filesystems later (lower priority)
    bool mounted = 0;
    for( int i = 0; i < 16; ++i) {
        mounted |= !!vfs_mount(va(".art[%d].zip", i));
    }
    // mount physical filesystems first (higher priority)
    if(!mounted) vfs_mount(ART);

    // unlock preswap events
    fwk_unlock_pre_swap_events = 1;

    // init more subsystems
    scene_push();           // create an empty scene by default
    input_init();
    network_init();
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;
}
static void fwk_pre_swap_subsystems() {
    if( fwk_unlock_pre_swap_events ) {
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

static
void fwk_error_callback(int error, const char *description) {
    if( is(osx) && error == 65544 ) return; // whitelisted
    PANIC("%s (error %x)", description, error);
}

#if 0
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

void fwk_init() {
    do_once {
        // printf("-O%d | %s %s\n", O_FLAG, __DATE__, __TIME__); // display optimization flag

        // signal handler
        // fwk_install_signal_handler();

        // init glfw
        glfwSetErrorCallback(fwk_error_callback);
        glfwInit();
        atexit(glfwTerminate);

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        chdir(app_path());
        // skip tcc argvs (if invoked as tcc file.c fwk.c -g -run)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            // __argc = 1; __argv[0] = __argv[1]; char *found = strstr(__argv[1], ".c"); if(found) *found = 0;
            // __argc = 1;
        }

        // create or update cook.zip file
        cooker_config(NULL, NULL, NULL);
        if( file_directory(TOOLS) && cooker_jobs() ) {
            cooker_start( "**", 0|COOKER_ASYNC );
        }
    }
}

// Enable more performant GPUs on laptops. Does this work into a dll?
// int NvOptimusEnablement = 1;
// int AmdPowerXpressRequestHighPerformance = 1;

