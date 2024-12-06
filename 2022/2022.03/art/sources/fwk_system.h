// -----------------------------------------------------------------------------
// system framework utils
// - rlyeh, public domain.
//
// Note: Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// Note: If you are linking your binary using GNU ld you need to add --export-dynamic

API int         argc();
API char*       argv(int);

API int         flag(const char *commalist); // --arg
API const char* option(const char *commalist, const char *defaults); // --arg=key or --arg key
API int         optioni(const char *commalist, int defaults); // argvi() ?
API float       optionf(const char *commalist, float defaults);

// @todo:
// ini(file, key, default);
// inif(file, key, default);

API char*       os_exec_output(); // legacy
API int         os_exec(const char *command); // legacy
#define         os_exec(...) os_exec(file_normalize(va(__VA_ARGS__))) // legacy
API char*       os_exec_(int *retvalue, const char *command); // new
#define         os_exec_(rc, ...) os_exec(rc, file_normalize(va(__VA_ARGS__))) // new

API void        tty_color(unsigned color);
API void        tty_reset();

API int         cpu_cores();

// return battery level [1..100]. also positive if charging (+), negative if discharging (-), and 0 if no battery is present.
API int         battery();

API const char* app_name();
API const char* app_path();
API const char* app_cache();
API const char* app_temp();
API void        app_reload();

API uint64_t    date();       // YYYYMMDDhhmmss
API uint64_t    date_epoch(); // linux epoch
API double      time_hh();
API double      time_mm();
API double      time_ss();
API uint64_t    time_ms();
API uint64_t    time_us();
API uint64_t    time_ns();
API void        sleep_ss(double ss);
API void        sleep_ms(double ms);
API void        sleep_us(double us);
API void        sleep_ns(double us);

API char*       callstack( int traces ); // write callstack into a temporary string. do not delete it.
API int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

API void        die(const char *message);
API void        alert(const char *message);
API void        hexdump( const void *ptr, unsigned len );
API void        hexdumpf( FILE *fp, const void *ptr, unsigned len, int width );
API void        breakpoint(const char *reason);
API bool        has_debugger();

API uint16_t    lil16(uint16_t n); // swap16 as lil
API uint32_t    lil32(uint32_t n); // swap32 as lil
API float       lil32f(float n);   // swap32 as lil
API uint64_t    lil64(uint64_t n); // swap64 as lil
API double      lil64f(double n);  // swap64 as lil
API uint16_t    big16(uint16_t n); // swap16 as big
API uint32_t    big32(uint32_t n); // swap32 as big
API float       big32f(float n);   // swap32 as big
API uint64_t    big64(uint64_t n); // swap64 as big
API double      big64f(double n);  // swap64 as big

API uint16_t*   lil16p(void *n, int sz);
API uint32_t*   lil32p(void *n, int sz);
API float*      lil32pf(void *n, int sz);
API uint64_t*   lil64p(void *n, int sz);
API double*     lil64pf(void *n, int sz);
API uint16_t*   big16p(void *n, int sz);
API uint32_t*   big32p(void *n, int sz);
API float*      big32pf(void *n, int sz);
API uint64_t*   big64p(void *n, int sz);
API double*     big64pf(void *n, int sz);

#define PANIC(...)   PANIC(va(__VA_ARGS__), __FILE__, __LINE__) // die() ?
API int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);
API int (PANIC)(const char *error, const char *file, int line);
