// video player
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // 75% window, msaa x8
    window_create( 75, WINDOW_MSAA8 );

    // load video
    int is_rgb = flag("--rgb") ? 1 : 0;
    video_t *v = video( "bjork-all-is-full-of-love.mp4", is_rgb ? VIDEO_RGB : VIDEO_YCBCR );

    while( window_swap() ) {
        // decode video frame and get associated textures (audio is automatically sent to audiomixer)
        texture_t *textures;
        profile( Video decoder ) {
            textures = video_decode( v );
        }

        // present decoded textures as a fullscreen composed quad
        profile( Video quad ) {
            if(is_rgb) fullscreen_rgb_quad( textures[0], 1.3f );
            else fullscreen_ycbcr_quad( textures, 1.3f );
        }

        // input controls
        if( input(KEY_ESC) ) break;

        // ui video
        if( ui_begin("Video", 0) ) {
            if( ui_button("Rewind") )  video_seek(v, video_position(v) - 3);
            if( ui_button("Pause") )   video_pause(v, video_is_paused(v) ^ 1);
            if( ui_button("Forward") ) video_seek(v, video_position(v) + 3);
            ui_end();
        }
        // audio
        if( ui_begin("Audio", 0)) {
            static float master = 1;
            if( ui_slider2("Master", &master, stringf("%.2f", master))) audio_volume_master(master);
            ui_end();
        }
    }
}
