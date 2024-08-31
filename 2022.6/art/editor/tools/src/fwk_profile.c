
#if WITH_PROFILE
profiler_t profiler;
int profiler_enabled = 1;

void (profile_init)() { map_init(profiler, less_str, hash_str); }
void (profile_enable)(bool on) { profiler_enabled = !!(on); }
void (profile_render)() { 
    if(!profiler) return;
    if(!profiler_enabled) return;

    int has_menu = ui_has_menubar();
    if( !has_menu ) {
        // render profiler, unless we are in the cooking stage 
        // also, we defer profile from rendering some initial frames, so the user may have chance to actually call any UI call before us
        // (given the UI policy nature, first-called first-served, we dont want profile tab to be on top of all other tabs)
        static unsigned frames = 0; if(frames <= 3) frames += cooker_progress() >= 100;
        if( frames <= 3 ) return;
    }

    if( has_menu ? ui_window("Profiler", 0) : ui_begin("Profiler", 0) ) {

        for each_map_ptr(profiler, const char *, key, struct profile_t, val ) {
            if( !isnan(val->stat) ) {
                float v = val->stat;
                ui_slider2(va("Stat: %s", *key), &v, va("%.2f", val->stat));
                val->stat = 0;
            }
        }

        ui_separator();

        for each_map_ptr(profiler, const char *, key, struct profile_t, val ) {
            if( isnan(val->stat) ) {
                float v = val->avg/1000.0;
                ui_slider2(*key, &v, va("%.2f ms", val->avg/1000.0));
            }
        }

        (has_menu ? ui_window_end : ui_end)();
    }
}
#endif

