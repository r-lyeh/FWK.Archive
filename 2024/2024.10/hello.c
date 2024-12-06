// playground tests
// - rlyeh, public domain
//
// # quickstart
// - win/vc       : cl hello.c
// - win/clang-cl : clang-cl  hello.c
// - win/tcc      : tools\tcc hello.c -m64
// - win/mingw    : gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - win/clang    : clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - linux        : cc  hello.c -lm -ldl -lpthread -lX11
// - linux/tcc    : tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
// - osx          : cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox

#include "engine.h"

#pragma warning(disable : 4716)
#define main static macro(x); const char *concat(app,__COUNTER__) = __FILE__; void concat(run,__COUNTER__)

#include "demos/hello.c"
#include "demos/00-loop.c"
#include "demos/00-script.c"
#include "demos/01-demo2d.c"
#include "demos/01-easing.c"
#include "demos/01-font.c"
#include "demos/01-ui.c"
#include "demos/02-ddraw.c"
#include "demos/02-frustum.c"
#include "demos/03-anims.c"
#include "demos/03-batching.c"
#include "demos/03-mesh.c"
#include "demos/04-actor.c"
#include "demos/06-material.c"
#include "demos/06-parallax.c"
#include "demos/06-scene-sorting.c"
#include "demos/06-scene.c"
#include "demos/06-sorting.c"
#include "demos/07-netsync.c"
#include "demos/07-network.c"
#include "demos/08-audio.c"
#include "demos/08-video.c"
#include "demos/09-cubemap.c"
#include "demos/09-envmap.c"
#include "demos/09-lights.c"
#include "demos/09-shadertoy.c"
#include "demos/09-shadows-scene.c"
#include "demos/09-shadows.c"
#include "demos/99-bt.c"
#include "demos/99-compute.c"
#include "demos/99-controller.c"
#include "demos/99-geom.c"
#include "demos/99-gizmo.c"
#include "demos/99-gui.c"
#include "demos/99-lod.c"
#include "demos/99-pathfind.c"
#include "demos/99-spine.c"
#include "demos/99-splines.c"
#include "demos/99-sponza.c"
#include "demos/99-sprite.c"
#include "demos/99-sprite3d.c"
#include "demos/99-steam.c"

#undef main

const char *trim(const char *x) {
    const char *a = strrchr(x, '\\');
    const char *b = strrchr(x, '/');
    return a > b ? a+1 : b > a ? b+1 : x;
}

int main() {
    const char *apps[] = {
        app10,app12,app14,app16,app18,app20,app22,app24,app26,app28,
        app30,app32,app34,app36,app38,app40,app42,app44,app46,app48,
        app50,app52,app54,app56,app58,app60,app62,app64,app66,app68,
        app70,app72,app74,app76,app78,app80,app82,app84,app86,app88,
        app90,app92,
    };
    void (*runs[])() = {
        run11,run13,run15,run17,run19,run21,run23,run25,run27,run29,
        run31,run33,run35,run37,run39,run41,run43,run45,run47,run49,
        run51,run53,run55,run57,run59,run61,run63,run65,run67,run69,
        run71,run73,run75,run77,run79,run81,run83,run85,run87,run89,
        run91,run93,
    };

    int demo = argc() > 1 && argv(1)[0] >= '0' && argv(1)[0] <= '9' ? atoi(argv(1)) : -1;
    if( demo >= 0 && demo < countof(runs) ) return runs[demo](), 0;

    window_create(100, WINDOW_TRANSPARENT|WINDOW_MSAA2);

    static int open = 1;
    while( window_swap() && !input(KEY_ESC) && open ) {
        if( ui_window("Demo Launcher", &open) ) {
            for( int i = 0; i < countof(runs); ++i) {
            int choice = ui_buttons(1, trim(apps[i]), "...");
            if( choice == 1 ) window_visible(0), system(va("%s %d", argv(0), i)), window_visible(1);
            if( choice == 2 ) system(va("%s \"%s\"", ifdef(win32,"start \"\"",ifdef(osx,"open","xdg-open")), apps[i]));
            }

            ui_window_end();
        }
    }
    return 0;
}
