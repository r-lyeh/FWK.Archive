// game framework
// - rlyeh, public domain

#ifndef FWK_H
#define FWK_H

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REALLOC realloc
#define COUNTOF(x) (sizeof(x)/sizeof(0[x]))
#define PANIC(...) exit(-fprintf(stderr,__VA_ARGS__))

//#include "fwk_app.h"
#include "fwk_math.h"
#include "fwk_audio.h"
#include "fwk_input.h"
#include "fwk_network.h"
#include "fwk_render.h"
#include "fwk_renderdd.h"
#include "fwk_script.h"
#include "fwk_ui.h"
#include "fwk_window.h"

#endif

#ifdef FWK_C
#pragma once

#define GLAD_GL_IMPLEMENTATION              // glad
#define GLFW_INCLUDE_NONE                   // glfw
#define LSH_GLFW_IMPLEMENTATION             // glfw
#define _GLFW_WIN                           // glfw
#define lua_c                               // lua
#define MINIAUDIO_IMPLEMENTATION            // miniaudio
#define STB_IMAGE_IMPLEMENTATION            // stbi
#define NK_INCLUDE_FIXED_TYPES              // nuklear
#define NK_INCLUDE_STANDARD_IO              // nuklear
#define NK_INCLUDE_STANDARD_VARARGS         // nuklear
#define NK_INCLUDE_DEFAULT_ALLOCATOR        // nuklear
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT     // nuklear
#define NK_INCLUDE_FONT_BAKING              // nuklear
#define NK_INCLUDE_DEFAULT_FONT             // nuklear
#define NK_IMPLEMENTATION                   // nuklear
#define NK_GLFW_GL3_IMPLEMENTATION          // nuklear
#define NK_KEYSTATE_BASED_INPUT             // nuklear
#define SWRAP_IMPLEMENTATION                // swrap
#define SWRAP_STATIC                        // swrap
#define JAR_MOD_IMPLEMENTATION              // jar_mod
#define JAR_XM_IMPLEMENTATION               // jar_xm
#define STS_MIXER_IMPLEMENTATION            // sts_mixer

#include "3rd/3rd_glad.h"
#include "3rd/3rd_glfw3.h"
#undef timeGetTime

#include "3rd/3rd_swrap.h"

#include "3rd/3rd_jo_mp1.h"
#include "3rd/3rd_sfxr.h"
#define error stb_vorbis_error
#define get_bits stb_vorbis_get_bits
#include "3rd/3rd_stb_vorbis.h"
#undef error
#define channel jar_channel
#include "3rd/3rd_jar_mod.h"
#undef DEBUG
#undef channel
#include "3rd/3rd_jar_xm.h"
#include "3rd/3rd_sts_mixer.h"
#include "3rd/3rd_miniaudio.h"

#undef L
#undef C
#undef R
#define error l_error
#include "3rd/3rd_lua53.h"

#include "3rd/3rd_stb_image.h"

#undef freelist
#include "3rd/3rd_nuklear.h"
#include "3rd/3rd_nuklear_glfw_gl3.h"

#include "3rd/3rd_assimp.h"

// -----------------------------------------------------------------------------

extern struct GLFWwindow *win;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static unsigned char *readfile(const char *pathfile, int *size) {
    char *bin = 0;
    for( FILE *fp = fopen(pathfile,"rb"); fp; fclose(fp), fp = 0) {
        fseek(fp, 0L, SEEK_END);
        size_t len = ftell(fp);
        if(size) *size = (int)len;
        fseek(fp, 0L, SEEK_SET);
        bin = REALLOC(0, len+1);
        if( bin && fread(bin, 1, len, fp) == len ) bin[len] = '\0';
        else REALLOC(bin, 0), bin = 0;
    }
    return bin;
}

void fwk_init() {
    static int once = 1; for(;once;once=0) {
        glfwSetErrorCallback(error_callback);
        glfwInit();
        atexit(glfwTerminate);
    }
}

#define APP_C
#define AUDIO_C
#define INPUT_C
#define MATH_C
#define NETWORK_C
#define RENDER_C
#define RENDERDD_C
#define SCRIPT_C
#define UI_C
#define WINDOW_C

//#include "fwk_app.h"
#include "fwk_audio.h"
#include "fwk_input.h"
#include "fwk_math.h"
#include "fwk_network.h"
#include "fwk_render.h"
#include "fwk_renderdd.h"
#include "fwk_script.h"
#include "fwk_ui.h"
#include "fwk_window.h"

#endif
