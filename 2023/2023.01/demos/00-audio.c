// audio demo
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // audio (both clips & streams)
    audio_t voice1 = audio_clip( "coin.wav" );
    audio_t voice2 = audio_clip( "pew.sfxr" );
    audio_t stream1 = audio_stream( "wrath_of_the_djinn.xm" );
    audio_t stream2 = audio_stream( "larry.mid" );
    audio_play(voice2, 0);
    audio_play(stream1, 0);

    // demo loop
    while (window_swap() && !input_down(KEY_ESC))
    {
        if( ui_panel("Audio", 0)) {
            static float bgm = 1, sfx = 1, master = 1;
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("SFX", &sfx, va("%.2f", sfx))) audio_volume_clip(sfx);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_label2_toolbar("BGM: Wrath of the Djinn", ICON_MD_VOLUME_UP, 24)) audio_stop(stream2), audio_play(stream1, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("BGM: Leisure Suit Larry 3", ICON_MD_VOLUME_UP, 24)) audio_stop(stream1), audio_play(stream2, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP, 24)) audio_play(voice1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP, 24)) audio_play(voice2, 0);
            ui_panel_end();
        }
    }
}
