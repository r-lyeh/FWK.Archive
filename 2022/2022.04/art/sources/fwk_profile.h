// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if WITH_PROFILE
#   define profile_init() do { map_init(profiler, less_str, hash_str); } while(0)
#   define profile_enable(on) do { profiler_enabled = !!(on); } while(0)
#   define profile(section) for( \
        struct profile_t *found = map_find_or_add(profiler, section "@" FILELINE, (struct profile_t){NAN} ), *dummy = (\
        found->cost = -time_us(), found); found->cost < 0; found->cost += time_us(), found->avg = found->cost * 0.25 + found->avg * 0.75)  ///+
#   define profile_incstat(name, accum) do { if(profiler) { \
        struct profile_t *found = map_find(profiler, name); \
        if(!found) found = map_insert(profiler, name, (struct profile_t){0}); \
        found->stat += accum; \
        } } while(0) ///+
#   define profile_render() if(profiler && profiler_enabled) do { \
        for(float _i = ui_begin("Profiler",0), _r; _i ; ui_end(), _i = 0) { \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( !isnan(val->stat) ) ui_slider2(va("Stat: %s", *key), (_r = val->stat, &_r), va("%.2f", val->stat)), val->stat = 0; \
            ui_separator(); \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( isnan(val->stat) ) ui_slider2(*key, (_r = val->avg/1000.0, &_r), va("%.2f ms", val->avg/1000.0)); \
        } } while(0)
struct profile_t { double stat; int32_t cost, avg; }; ///-
typedef map(char *, struct profile_t) profiler_t; ///-
extern API profiler_t profiler; ///-
extern API int profiler_enabled; ///-
#else
#   define profile_init()               do {} while(0)
#   define profile_enable(x)            do {} while(0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile(...)                 if(1) // for(int _p = 1; _p; _p = 0)
#   define profile_render()
#endif
