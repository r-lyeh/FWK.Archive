// audio demo
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // audio (both clips & streams)
    audio_t voice = audio_clip("coin.wav"); // "pew.sfxr"
    audio_t stream = audio_stream("wrath_of_the_djinn.xm"); // "larry.mid"
    audio_play(voice, 0);
    audio_play(stream, 0);

    // demo loop
    while (window_swap() && !input_down(KEY_ESC))
    {
        if( ui_begin("Audio", 0)) {
            static float fx = 1, bgm = 1, master = 1;
            if( ui_slider2("FX", &fx, va("%.2f", fx))) audio_volume_clip(fx);
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_button("Coin FX") ) audio_play(voice, 0);
            ui_end();
        }
    }
}
