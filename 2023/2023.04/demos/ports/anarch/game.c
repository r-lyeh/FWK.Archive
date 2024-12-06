/**
    @file game.c

    This is a quick FWK implementation of the game front end, with a couple of fixes for MSVC.
    by r-lyeh

    Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
    plus a waiver of all other intellectual property. The goal of this work is
    be and remain completely in the public domain forever, available for any use
    whatsoever.
*/

#define FWK_IMPLEMENTATION
#include "fwk.h"

#ifdef _MSC_VER
#define __builtin_expect(expr,val) (expr)
#endif

#define SFG_FPS 60
#define SFG_LOG(str) ;// puts(str);
#define SFG_DITHERED_SHADOW 1
#define SFG_DIMINISH_SPRITES 1
#define SFG_HEADBOB_SHEAR (-1 * SFG_SCREEN_RESOLUTION_Y / 80)
#define SFG_BACKGROUND_BLUR 1
#define SFG_SCREEN_RESOLUTION_X 700
#define SFG_SCREEN_RESOLUTION_Y 512
#define SFG_CAN_EXIT 1

#include "game.h"
#include "sounds.h"

// now implement the Anarch API functions (SFG_*)

void SFG_processEvent(uint8_t event, uint8_t data)
{}

void SFG_sleepMs(uint16_t timeMs) 
{}

uint32_t SFG_getTimeMs() {
    return time_ms();
}

void SFG_getMouseOffset(int16_t *x, int16_t *y)
{}

int8_t SFG_keyPressed(uint8_t key) {
    switch (key) {
        default: return 0;
        case SFG_KEY_UP: return !!input(KEY_UP); 
        case SFG_KEY_DOWN: return !!input(KEY_DOWN);
        case SFG_KEY_RIGHT: return !!input(KEY_RIGHT);
        case SFG_KEY_LEFT: return !!input(KEY_LEFT);
        case SFG_KEY_A: return !!input(KEY_Z) | !!input(KEY_SPACE);
        case SFG_KEY_C: return !!input(KEY_X) | !!input(KEY_LALT);
        case SFG_KEY_B: return !!input(KEY_C) | !!input(KEY_LCTRL);
    }
}

void SFG_save(uint8_t data[SFG_SAVE_SIZE]) {
    if( !file_write("anarch.sav",data,SFG_SAVE_SIZE) ) {
        puts("FWK: could not save the file!");
        return;
    }
}

uint8_t SFG_load(uint8_t data[SFG_SAVE_SIZE]) {
    int len = file_size("anarch.sav");
    for( char *blob = file_read("anarch.sav"); blob && len == SFG_SAVE_SIZE; ) {
        return !!memcpy(data, blob, len);
    }
    puts("FWK: no save file to open");
    return 0;
}

uint32_t screen[SFG_SCREEN_RESOLUTION_X * SFG_SCREEN_RESOLUTION_Y];

void SFG_setPixel(uint16_t x, uint16_t y, uint8_t colorIndex) {
    static unsigned palette32[256];

    do_once
    for (int i = 0; i < 256; ++i) { // precompute RGB palette
        uint16_t col565 = paletteRGB565[i];
        palette32[i] = 0xff000000 | ((col565 << 19) & 0xf80000) | ((col565 << 5) & 0xfc00) | ((col565 >> 8) & 0xf8);
    }

    screen[y * SFG_SCREEN_RESOLUTION_X + x] = palette32[colorIndex];
}

uint8_t musicOn = 1; //< this has to be init to 0 (not 1), else a few samples get played at start
void SFG_setMusic(uint8_t value) {
    if( value == SFG_MUSIC_TURN_ON )  musicOn = 1;
    if( value == SFG_MUSIC_TURN_OFF ) musicOn = 0;
    if( value == SFG_MUSIC_NEXT ) SFG_nextMusicTrack();
}

#define MUSIC_VOLUME 16.f
uint16_t audioBuff[SFG_SFX_SAMPLE_COUNT] = {0};
uint16_t audioPos = 0; // audio position for the next audio buffer fill
uint32_t audioUpdateFrame = 0; // game frame at which audio buffer fill happened

static inline int16_t mixSamples(int16_t sample1, int16_t sample2) {
    return sample1 + sample2;
}

void audioFillCallback(void *userdata, uint8_t *s, int l) {
    uint16_t *s16 = (uint16_t *) s;

    for (int i = 0; i < l / 2; ++i) {
        s16[i] = musicOn ?
            mixSamples(audioBuff[audioPos], MUSIC_VOLUME * (SFG_getNextMusicSample() - SFG_musicTrackAverages[SFG_MusicState.track]))
            : audioBuff[audioPos];

        audioBuff[audioPos] = 0;
        audioPos = (audioPos < SFG_SFX_SAMPLE_COUNT - 1) ? (audioPos + 1) : 0;
    }

    audioUpdateFrame = SFG_game.frame;
}

void SFG_playSound(uint8_t soundIndex, uint8_t volume) {
    uint16_t pos = (audioPos + ((SFG_game.frame - audioUpdateFrame) * SFG_MS_PER_FRAME * 8)) % SFG_SFX_SAMPLE_COUNT;
    uint16_t volumeScale = 1 << (volume / 37);

    for (int i = 0; i < SFG_SFX_SAMPLE_COUNT; ++i) {
        audioBuff[pos] = mixSamples(audioBuff[pos], (128 - SFG_GET_SFX_SAMPLE(soundIndex,i)) * volumeScale);
        pos = (pos < SFG_SFX_SAMPLE_COUNT - 1) ? (pos + 1) : 0;
    }
}

int main() {
    // install signal handlers
    signal_hooks();

    SFG_init();

    window_create(0.75, 0);
    texture_t t = texture_checker();

    while( window_swap() && SFG_mainLoopBody() ) {
        texture_update(&t, SFG_SCREEN_RESOLUTION_X, SFG_SCREEN_RESOLUTION_Y, 4, screen, TEXTURE_RGB|TEXTURE_LINEAR);
        fullscreen_quad_rgb(t, 1.0f);

        uint16_t samples[128]; // 8 KHz 16-bit mono = 8000/60 = 133 samples/frame -> 256 samples/frame 
        audioFillCallback(NULL, (uint8_t*)samples, sizeof(samples));
        audio_queue(samples, countof(samples), AUDIO_8KHZ|AUDIO_16|AUDIO_1CH );
    }
}
