// audio framework
// - rlyeh, public domain
//
// fixme: leaks, audio_delete
// fixme: sfxr (hack)
// @todo: destroystream()    if( ss->type == WAV ) drwav_uninit(&ss->wav);
// @todo: destroystream()    if( ss->type == MOD ) jar_mod_unload(&ss->mod);
// @todo: destroystream()    if( ss->type == XM && ss->xm ) jar_xm_free_context(ss->xm);

#ifndef AUDIO_H
#define AUDIO_H

typedef struct audio_handle* audio_t;

audio_t audio_clip( const char *pathfile );
audio_t audio_stream( const char *pathfile );
    int audio_play( audio_t s, int flags );

#endif

#ifdef AUDIO_C
#pragma once

// encapsulate drflac and some buffer with the sts_mixer_stream_t
enum { UNK, WAV, MOD, XM, FLAC, OGG, SFXR, MP1, MP3 };
typedef struct {
    int type;
    union {
        drwav wav;
        jar_mod_context_t mod;
        jar_xm_context_t *xm;
        stb_vorbis *ogg;
        void *opaque;
        drflac*         flac;               // FLAC decoder state
        drmp3           mp3_;
    };
    sts_mixer_stream_t  stream;             // mixer stream
    union {
    int32_t             data[4096*2];       // static sample buffer
    float               dataf[4096*2];
    };
} mystream_t;

static void stereo_float_to_mono( int channels, float *buffer, int samples ) {
    if( channels > 1 ) {
        float mix;
        float *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (float)(mix / channels);
        }
    }
}
static void stereo_s16_to_mono( int channels, short *buffer, int samples ) {
    if( channels > 1 ) {
        float mix;
        short *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (short)(mix / channels);
        }
    }
}

static void *read_file( const char *filename, int *bytes ) {
    void *ptr = 0;
    for( FILE *fp = fopen( filename, "rb" ); fp; (fclose(fp), fp = 0)) {
        fseek( fp, 0L, SEEK_END );
        size_t tell = ftell( fp );
        fseek( fp, 0L, SEEK_SET );
        ptr = REALLOC( ptr, tell + 1 );
        if( ptr ) {
            tell[(char*)ptr] = '\0';
            fread( ptr, 1, tell, fp );
            if( bytes ) {
                *bytes = (int)tell;
            }
        }
    }
    return ptr;
}

// the callback to refill the (stereo) stream data
static void refill_stream(sts_mixer_sample_t* sample, void* userdata) {
    mystream_t* stream = (mystream_t*)userdata;
    switch( stream->type ) {
        default:
        break; case FLAC: {
            int sl = sample->length / 2; /*sample->channels*/;
            if (drflac_read_pcm_frames_f32(stream->flac, sl, stream->dataf) < sl) {
                drflac_seek_to_pcm_frame(stream->flac, 0);
            }
        }
        break; case WAV: {
            if (drwav_read_pcm_frames_s16(&stream->wav, sample->length, (short*)stream->data) < sample->length) {
                drwav_seek_to_pcm_frame(&stream->wav, 0);
            }
        }
        break; case MP3: {
            int sl = sample->length / 2; /*sample->channels*/;
            if (drmp3_read_pcm_frames_f32(&stream->mp3_, sl, stream->dataf) < sl) {
                drmp3_seek_to_pcm_frame(&stream->mp3_, 0);
            }
        }
        break; case OGG: {
            stb_vorbis *ogg = (stb_vorbis*)stream->ogg;
            if( stb_vorbis_get_samples_short_interleaved(ogg, 2, (short*)stream->data, sample->length) == 0 )  {
                stb_vorbis_seek(stream->ogg, 0);
            }
        }
        break; case MOD: {
            jar_mod_context_t *mod = (jar_mod_context_t*)&stream->mod;
            jar_mod_fillbuffer(mod, (ma_int16*)stream->data, sample->length / 2, 0);
        }
        break; case XM: {
            jar_xm_context_t *xm = (jar_xm_context_t*)stream->xm;
            jar_xm_generate_samples_16bit(xm, (short*)stream->data, sample->length / 2 );
        }
    }
}

// load a (stereo) stream
static bool load_stream(mystream_t* stream, const char *filename) {
    int error;
    int HZ = 44100;
    stream->type = UNK;
    if( stream->type == UNK && (stream->ogg = stb_vorbis_open_filename(filename, &error, NULL)) ) {
        stb_vorbis_info info = stb_vorbis_get_info(stream->ogg);
        if( info.channels != 2 ) { puts("cannot stream ogg file. stereo required."); goto end; }
        stream->type = OGG;
        stream->stream.sample.frequency = info.sample_rate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    if( stream->type == UNK && (drwav_init_file(&stream->wav, filename, NULL))) {
        if( stream->wav.channels != 2 ) { puts("cannot stream wav file. stereo required."); goto end; }
        stream->type = WAV;
        stream->stream.sample.frequency = stream->wav.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    if( stream->type == UNK && (stream->flac = drflac_open_file(filename, NULL)) ) {
        if( stream->flac->channels != 2 ) { puts("cannot stream flac file. stereo required."); goto end; }
        stream->type = FLAC;
        stream->stream.sample.frequency = stream->flac->sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_FLOAT;
    }
    if( stream->type == UNK && (jar_xm_create_context_from_file(&stream->xm, HZ, filename) == 0)) {
        stream->type = XM;
        stream->stream.sample.frequency = HZ;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    if( stream->type == UNK && ((jar_mod_init(&stream->mod), jar_mod_load_file(&stream->mod, filename)) != 0) ) {
        stream->type = MOD;
        jar_mod_setcfg(&stream->mod, HZ, 16/*bits*/, 1/*stereo*/, 1/*stereo_separation*/, 1/*filter*/);
        stream->stream.sample.frequency = HZ;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    drmp3_config mp3_cfg = { 2, HZ };
    if( stream->type == UNK && (drmp3_init_file(&stream->mp3_, filename, NULL/*&mp3_cfg*/) != 0) ) {
        stream->type = MP3;
        stream->stream.sample.frequency = stream->mp3_.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_FLOAT;
    }

    if( stream->type == UNK ) {
        return false;
    }

    end:;
    stream->stream.userdata = stream;
    stream->stream.callback = refill_stream;
    stream->stream.sample.length = sizeof(stream->data) / sizeof(stream->data[0]);
    stream->stream.sample.data = stream->data;
    refill_stream(&stream->stream.sample, stream);

    return true;
}

// load a (mono) sample
static bool load_sample(sts_mixer_sample_t* sample, const char *filename) {
    int error;
    int channels = 0;
    if( !channels ) for( drwav w = {0}, *wav = &w; wav && drwav_init_file(wav, filename, NULL); wav = 0 ) {
        channels = wav->channels;
        sample->frequency = wav->sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = wav->totalPCMFrameCount;
        sample->data = REALLOC(0, sample->length * sizeof(short) * channels);
        drwav_read_pcm_frames_s16(wav, sample->length, (short*)sample->data);
        drwav_uninit(wav);
    }
    if( !channels ) for( stb_vorbis *ogg = stb_vorbis_open_filename(filename, &error, NULL); ogg; ogg = 0 ) {
        stb_vorbis_info info = stb_vorbis_get_info(ogg);
        channels = info.channels;
        sample->frequency = info.sample_rate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = (int)stb_vorbis_stream_length_in_samples(ogg);
        stb_vorbis_close(ogg);

        short *buffer;
        int sample_rate;
        stb_vorbis_decode_filename(filename, &channels, &sample_rate, (short **)&buffer);
        sample->data = buffer;
    }
    if( !channels ) for( drflac* flac = drflac_open_file(filename, NULL); flac; flac = 0 ) {
        channels = flac->channels;
        sample->frequency = flac->sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = flac->totalPCMFrameCount;
        sample->data = REALLOC(0, sample->length * sizeof(short) * channels);
        drflac_read_pcm_frames_s16(flac, sample->length, (short*)sample->data);
        drflac_close(flac);
    }
    drmp3_config mp3_cfg = { 2, 44100 };
    drmp3_uint64 mp3_fc;
    if( !channels ) for( short *fbuf = 0; fbuf = drmp3_open_file_and_read_pcm_frames_s16(filename, &mp3_cfg, &mp3_fc, NULL); ) {
        channels = mp3_cfg.channels;
        sample->frequency = mp3_cfg.sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = mp3_fc; //  / sizeof(float) / mp3_cfg.channels;
        sample->data = fbuf;
        break;
    }
    if( !channels ) {
        int inputSize;
        void *inputData = read_file( filename, &inputSize );
        if( inputData ) {
            short *output = 0;
            int outputSize, hz, mp1channels;
            bool ok = jo_read_mp1(inputData, inputSize, &output, &outputSize, &hz, &mp1channels);
            if( ok ) {
                channels = mp1channels;
                sample->frequency = hz;
                sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
                sample->length = outputSize / sizeof(int16_t) / channels;
                sample->data = REALLOC(0, sample->length * sizeof(int16_t) * channels );
                memcpy( sample->data, output, outputSize );
            } 
            REALLOC( inputData, 0 );
        }
    }

#if 0
    if( !channels ) {
        //loadPreset(1, 0);
        //SaveSettings("test.sfxr");
        LoadSettings(filename);
        ExportWAV("sfxr.wav");
        return load_sample(sample, "sfxr.wav");
    }
#endif

    if( !channels ) {
        return false;
    }

    if( channels > 1 ) {
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_FLOAT ) {
            stereo_float_to_mono( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(float));
        }
        else
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_16 ) {
            stereo_s16_to_mono( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(short));
        }
        else {
            puts("error!");
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

static ma_device  device;
static ma_context context;
static sts_mixer_t mixer;

// This is the function that's used for sending more data to the device for playback.
static ma_uint32 audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    int len = frameCount;
    sts_mixer_mix_audio(&mixer, pOutput, len / (sizeof(int32_t) / 4));
    return len / (sizeof(int32_t) / 4);
}

void audio_drop(void) {
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
}

int audio_init( int flags ) {
    atexit(audio_drop);

    // init sts_mixer
    sts_mixer_init(&mixer, 44100, STS_MIXER_SAMPLE_FORMAT_32);

    // The prioritization of backends can be controlled by the application. You need only specify the backends
    // you care about. If the context cannot be initialized for any of the specified backends ma_context_init()
    // will fail.
    ma_backend backends[] = {
        ma_backend_wasapi, // Higest priority.
        ma_backend_dsound,
        ma_backend_winmm,
        ma_backend_pulseaudio,
        ma_backend_alsa,
        ma_backend_oss,
        ma_backend_jack,
        ma_backend_opensl,
        //ma_backend_openal,
        //ma_backend_sdl,
        ma_backend_null    // Lowest priority.
    };

    if (ma_context_init(backends, sizeof(backends)/sizeof(backends[0]), NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.");
        return false;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback); // Or ma_device_type_capture or ma_device_type_duplex.
    config.playback.pDeviceID = NULL; // &myPlaybackDeviceID; // Or NULL for the default playback device.
    config.playback.format    = ma_format_s32;
    config.playback.channels  = 2;
    config.sampleRate         = 44100;
    config.dataCallback       = audio_callback;
    config.pUserData          = NULL;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.");
        ma_context_uninit(&context);
        return false;
    }

    ma_device_start(&device);
    return true;
}

typedef struct audio_handle {
    bool is_clip;
    bool is_stream;
    union {
    sts_mixer_sample_t clip;
    mystream_t         stream;
    };
} audio_handle;

audio_t audio_clip( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_clip = load_sample( &a->clip, pathfile );
    return a;
}
audio_t audio_stream( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_stream = load_stream( &a->stream, pathfile );
    return a;
}

int audio_play( audio_t a, int flags ) {
    if( a->is_clip ) {
        float gain = 1.0f; // randf(); // [0..1]
        float pitch = 1.f; // (0..N]
        float pan =  0; // -1.0f + randf() * 2.0f; // [-1..+1]
        int voice = sts_mixer_play_sample(&mixer, &a->clip, gain, pitch, pan);
        if( voice == -1 ) return 0; // all voices busy
    }
    else if( a->is_stream ) {
        float gain = 0.5f;
        int voice = sts_mixer_play_stream(&mixer, &a->stream.stream, gain);
        if( voice == -1 ) return 0; // all voices busy
    }
    return 1;
}

#endif // AUDIO_C
