// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if WITH_PROFILE
#   define profile(section) for( \
        struct profile_t *found = map_find_or_add(profiler, section "@" FILELINE, (struct profile_t){NAN} ), *dummy = (\
        found->cost = -time_us(), found); found->cost < 0; found->cost += time_us(), found->avg = found->cost * 0.25 + found->avg * 0.75)  ///+
#   define profile_incstat(name, accum) do { if(profiler) { \
        struct profile_t *found = map_find(profiler, name); \
        if(!found) found = map_insert(profiler, name, (struct profile_t){0}); \
        found->stat += accum; \
        } } while(0) ///+
struct profile_t { double stat; int32_t cost, avg; }; ///-
typedef map(char *, struct profile_t) profiler_t; ///-
extern API profiler_t profiler; ///-
extern API int profiler_enabled; ///-
#else
#   define profile_init()               do {} while(0)
#   define profile_enable(x)            do {} while(0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile(...)                 for(int macro(i) = 1; macro(i); macro(i) = 0)
#   define profile_render()
#endif
