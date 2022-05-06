// - rlyeh, public domain

#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef _MSC_VER // _MSC
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#endif

#ifdef PLUGIN_DEMO
#pragma once

// plugin compile (dll): cl files /LD

int my_plugin_init(int argc, char **argv) {
    printf("my_init: %s(%d)\n", argv[0], argc);
    return 0;
}

EXPORT
void *plugin(int method) {
    if(method == 'info') return "my_plugin";
    if(method == 'vers') return "1.0.0";
    if(method == 'deps') return "";
    if(method == 'init') return my_plugin_init;
    return 0;
}

// app
// #include "dll.c" // dllfind
int main(int argc, char **argv) {
    const char *dllname = argc > 1 ? argv[1] : "plugin.dll";
    void*(*plugin)(int) = dllfind(dllname, "plugin");
    if(!plugin) { printf("cannot find %s:plugin\n", dllname); exit(-1); }
    int(*pi)(int,char**) = plugin('init');
    pi(argc, argv);
}

#define main main__
#endif
