// -----------------------------------------------------------------------------
// data framework (json5) @todo:xml,kvdb
// - rlyeh, public domain
//
// @todo: vec2,vec3,vec4

typedef union data_t {
    char* s;
    double f;
    int64_t i;
    uintptr_t p;
    array(union data_t) arr;
} data_t;

// data api

API bool    data_push(const char *source);
API data_t*     data_find(const char *type_keypath); // @todo, array(data_t) data_array();
API data_t      data_get(const char *type_keypath); // @todo, array(data_t) data_array();
API int         data_count(const char *keypath);
#define         data_int(...)    (data_get(va("i" __VA_ARGS__)).i)
#define         data_float(...)  (data_get(va("f" __VA_ARGS__)).f)
#define         data_string(...) (data_get(va("s" __VA_ARGS__)).s)
#define         data_count(...)   data_count(va(__VA_ARGS__))
API bool    data_pop();
