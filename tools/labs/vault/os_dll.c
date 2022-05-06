// - rlyeh, public domain

#ifndef DLL_H
#define DLL_H

#ifdef _WIN32
//# include <winsock2.h>
#   define dlopen(name,mode)    (void*)( (name) ? LoadLibraryA(name) : GetModuleHandle(NULL))
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)handle, symbol )
#   define dlclose(handle)      0
#else
#   include <dlfcn.h>
#endif

#endif

#ifdef DLL_C
#pragma once
void* dllfind(const char *filename, const char *symbol) {
/*
    char *buf;
    if( file_exists(buf = va("%s", filename)) ||
        file_exists(buf = va("%s.dll", filename)) ||
        file_exists(buf = va("%s.so", filename)) ||
        file_exists(buf = va("lib%s.so", filename)) ||
        file_exists(buf = va("%s.dylib", filename)) ) {
    }
*/
    void *dll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    return dll ? dlsym(dll, symbol) : 0;
}
#endif
