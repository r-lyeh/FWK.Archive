// script framework
// - rlyeh, public domain

void script_init();
void script_run(const char *script);
void script_runfile(const char *pathfile);


#ifdef SCRIPT_C
#pragma once

/* the Lua interpreter */
lua_State* L;

void script_quit(void) {
    lua_close(L);
}
void script_init() {
    fwk_init();

    /* initialize Lua */
    L = luaL_newstate();

    /* load various Lua libraries */
    luaL_openlibs(L);
    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);

    atexit(script_quit);
}

void script_run(const char *script) {
    luaL_dostring(L, script);
}

void script_runfile(const char *pathfile) {
    luaL_dofile(L, pathfile);
}

#endif
