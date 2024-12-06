// shadertoy viewer
// - rlyeh, public domain

#include "fwk.h"

int main() {
    window_create(75, 0); // WINDOW_MSAA8);
    window_title(__FILE__);

    const char **list = file_list("demos/art/shadertoys/", "**.fs");
    if(!list[0]) exit(-1);
    array(char*) browser = 0;
    while(*list) array_push(browser, STRDUP(file_name(*list++)));
    int browser_count = array_count(browser);

    shadertoy_t sh = {0};

    while(window_swap()) {
        // selector
        int next = input_down(KEY_UP) || input_down(KEY_LEFT);
        int prev = input_down(KEY_DOWN) || input_down(KEY_RIGHT);
        static int selector = 0;
        static int reload = 1;
        if( next ) if( selector > 0 ) --selector, reload = 1;
        if( prev ) if( selector < browser_count - 1 ) ++selector, reload = 1;
        if( reload ) {
            reload = 0;
            window_title(va("FWK - %s", browser[selector]));
            sh = shadertoy( browser[selector], 0 );
        }

        // draw
        shadertoy_render(&sh, window_delta());

        // UI
        if( ui_panel("Shadertoy", 0)) {
            if( ui_list("In use", (const char**)browser, browser_count, &selector) ) {
                reload = 1;
            }
            ui_panel_end();
        }
    }
}
