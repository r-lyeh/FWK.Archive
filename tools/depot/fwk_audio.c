// @fixme: really shutdown audio & related threads before quitting. drwav crashes.

// encapsulate drwav,drmp3,stbvoribs and some buffer with the sts_mixer_stream_t
enum { UNK, WAV, OGG, MP1, MP3 };
typedef struct {
    int type;
    union {
        drwav wav;
        stb_vorbis *ogg;
        void *opaque;
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
        short *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (short)(mix / channels);
        }
    }
}

// the callback to refill the (stereo) stream data
static void refill_stream(sts_mixer_sample_t* sample, void* userdata) {
    mystream_t* stream = (mystream_t*)userdata;
    switch( stream->type ) {
        default:
        break; case WAV: {
            int sl = sample->length / 2; /*sample->channels*/;
            if (drwav_read_pcm_frames_s16(&stream->wav, sl, (short*)stream->data) < sl) {
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
    drmp3_config mp3_cfg = { 2, 44100 };
    drmp3_uint64 mp3_fc;
    if( !channels ) for( short *fbuf = drmp3_open_file_and_read_pcm_frames_s16(filename, &mp3_cfg, &mp3_fc, NULL); fbuf ; fbuf = 0 ) {
        channels = mp3_cfg.channels;
        sample->frequency = mp3_cfg.sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = mp3_fc; //  / sizeof(float) / mp3_cfg.channels;
        sample->data = fbuf;
    }
    if( !channels ) {
        int inputSize = file_size( filename );
        void *inputData = file_read( filename );
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
    (void)pDevice; (void)pInput;
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
#if 1
        ma_backend_wasapi, // Higest priority.
        ma_backend_dsound,
        ma_backend_winmm,
        ma_backend_pulseaudio,
        ma_backend_alsa,
        ma_backend_oss,
        ma_backend_jack,
        ma_backend_opensl,
        //ma_backend_webaudio,
        //ma_backend_openal,
        //ma_backend_sdl,
        ma_backend_null    // Lowest priority.
#else
        // Highest priority
        ma_backend_wasapi,      // WASAPI      |  Windows Vista+
        ma_backend_dsound,      // DirectSound |  Windows XP+
        ma_backend_winmm,       // WinMM       |  Windows XP+ (may work on older versions, but untested)
        ma_backend_coreaudio,   // Core Audio  |  macOS, iOS 
        ma_backend_pulseaudio,  // PulseAudio  |  Cross Platform (disabled on Windows, BSD and Android)
        ma_backend_alsa,        // ALSA        |  Linux 
        ma_backend_oss,         // OSS         |  FreeBSD 
        ma_backend_jack,        // JACK        |  Cross Platform (disabled on BSD and Android)
        ma_backend_opensl,      // OpenSL ES   |  Android (API level 16+)
        ma_backend_webaudio,    // Web Audio   |  Web (via Emscripten)
        ma_backend_sndio,       // sndio       |  OpenBSD 
        ma_backend_audio4,      // audio(4)    |  NetBSD, OpenBSD 
        ma_backend_aaudio,      // AAudio      |  Android 8+
        ma_backend_custom,      // Custom      |  Cross Platform 
        ma_backend_null,        // Null        |  Cross Platform (not used on Web)
        // Lowest priority
#endif
    };

    if (ma_context_init(backends, countof(backends), NULL, &context) != MA_SUCCESS) {
        PRINTF("%s\n", "Failed to initialize audio context.");
        return false;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback); // Or ma_device_type_capture or ma_device_type_duplex.
    config.playback.pDeviceID = NULL; // &myPlaybackDeviceID; // Or NULL for the default playback device.
    config.playback.format    = ma_format_s32;
    config.playback.channels  = 2;
    config.sampleRate         = 44100;
    config.dataCallback       = (void*)audio_callback; //< @r-lyeh add void* cast
    config.pUserData          = NULL;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.");
        ma_context_uninit(&context);
        return false;
    }

    (void)flags;
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

static array(audio_handle*) audio_instances;

audio_t audio_clip( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_clip = load_sample( &a->clip, vfs_find(pathfile) );
    array_push(audio_instances, a);
    return a;
}
audio_t audio_stream( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_stream = load_stream( &a->stream, vfs_find(pathfile) );
    array_push(audio_instances, a);
    return a;
}


static float volume_clip = 1, volume_stream = 1, volume_master = 1;
float audio_volume_clip(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_clip = gain * gain;
        // patch all live clips
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].sample ) // is_sample?
                mixer.voices[i].gain = volume_clip;
        }
    return sqrt( volume_clip );
}
float audio_volume_stream(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_stream = gain * gain;
        // patch all live streams
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].stream ) // is_stream?
                mixer.voices[i].gain = volume_stream;
        }
    return sqrt( volume_stream );
}
float audio_volume_master(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_master = gain * gain;
        // patch global mixer
        mixer.gain = volume_master;
    return sqrt( volume_master );
}

int audio_play_gain_pitch_pan( audio_t a, int flags, float gain, float pitch, float pan ) {
    if( flags & AUDIO_IGNORE_MIXER_GAIN ) {
        // do nothing, gain used as-is
    } else {
        // apply mixer gains on top
        gain += a->is_clip ? volume_clip : volume_stream;
    }

    // gain: [0..+1], pitch: (0..N], pan: [-1..+1]

    if( a->is_clip ) {
        int voice = sts_mixer_play_sample(&mixer, &a->clip, gain, pitch, pan);
        if( voice == -1 ) return 0; // all voices busy
    }
    if( a->is_stream ) {
        int voice = sts_mixer_play_stream(&mixer, &a->stream.stream, gain);
        if( voice == -1 ) return 0; // all voices busy
    }
    return 1;
}

int audio_play_gain_pitch( audio_t a, int flags, float gain, float pitch ) {
    return audio_play_gain_pitch_pan(a, flags, gain, pitch, 0);
}

int audio_play_gain( audio_t a, int flags, float gain ) {
    return audio_play_gain_pitch(a, flags, gain, 1.f);
}

int audio_play( audio_t a, int flags ) {
    return audio_play_gain(a, flags & ~AUDIO_IGNORE_MIXER_GAIN, 0.f);
}

// -----------------------------------------------------------------------------
// audio queue

#ifndef AUDIO_QUEUE_BUFFERING_MS
#define AUDIO_QUEUE_BUFFERING_MS 100
#endif

#ifndef AUDIO_MAX_QUEUE
#define AUDIO_MAX_QUEUE 2048
#endif

typedef struct audio_queue_t {
    void *data;
    int cursor;
    int avail;
} audio_queue_t;

static thread_mutex_t queue_mutex;
static audio_queue_t audio_queues[AUDIO_MAX_QUEUE] = {0};
static int audio_queue_rd = 0, audio_queue_wr = AUDIO_MAX_QUEUE/2, audio_queue_voice = -1;
static void audio_queue_callback(sts_mixer_sample_t* sample, void* userdata) {
    char *p = sample->data;
    int bytes = sample->length * 4; // * 8; // 2ch * sizeof float

    if( audio_queue_rd == audio_queue_wr ) {
        PRINTF("WARNING: audio buffers exhausted, please increase buffering (%d>>%d)\n", audio_queue_rd, audio_queue_wr);
        audio_queue_wr = (audio_queue_wr + AUDIO_MAX_QUEUE / 2) % AUDIO_MAX_QUEUE;
        // sleep_ms(10);
        return;
    }

    while( bytes > 0 ) {
        thread_mutex_lock(&queue_mutex);

            audio_queue_t* front = &audio_queues[audio_queue_rd];

            int bytes_to_copy = bytes;
            if( bytes_to_copy > front->avail ) bytes_to_copy = front->avail;

if(front->data)
            memcpy(p, &((char*)front->data)[front->cursor], bytes_to_copy);
            p += bytes_to_copy;
            front->cursor += bytes_to_copy;
            front->avail -= bytes_to_copy;
            bytes -= bytes_to_copy;

            if( front->avail <= 0 ) {
                audio_queue_rd = (audio_queue_rd + 1) % AUDIO_MAX_QUEUE;
            }

        thread_mutex_unlock(&queue_mutex);
    }
    (void)userdata;
}
void audio_queue_clear() {
    thread_mutex_lock(&queue_mutex);
//sts_mixer_stop_voice(&mixer, audio_queue_voice);
//audio_queue_voice = -1;
    for (int i = 0; i < AUDIO_MAX_QUEUE; ++i ) {
        audio_queues[i].data = REALLOC(audio_queues[i].data, 0);
    }
    audio_queue_rd = 0;
    audio_queue_wr = AUDIO_MAX_QUEUE/2;
    thread_mutex_unlock(&queue_mutex);
}
int audio_queue( const void *samples, int num_samples, int flags ) {
    float gain = 1; // [0..1]
    float pitch = 1; // (0..N]
    float pan = 0; // [-1..1]

    int bits = flags & AUDIO_16 ? 16 : 32;
    int channels = flags & AUDIO_2CH ? 2 : 1;
    int bytes_per_sample = channels * (bits / 8);
    int bytes = num_samples * bytes_per_sample;
    assert( channels == 2 );

    if( audio_queue_voice < 0 ) {
        static sts_mixer_stream_t q = { 0 };
        q.callback = audio_queue_callback;
        q.sample.frequency = flags & AUDIO_44KHZ ? 44100 : 22050;
        q.sample.audio_format = flags & AUDIO_FLOAT ? STS_MIXER_SAMPLE_FORMAT_FLOAT : STS_MIXER_SAMPLE_FORMAT_16;
        q.sample.length = q.sample.frequency / (1000 / AUDIO_QUEUE_BUFFERING_MS);
        int bytes = q.sample.length /* * channels*/ * bytes_per_sample;
        q.sample.data = memset(REALLOC(q.sample.data, bytes), 0, bytes);
        audio_queue_voice = sts_mixer_play_stream(&mixer, &q, gain * 1.f);
    }

    do_once thread_mutex_init(&queue_mutex);
    thread_mutex_lock(&queue_mutex);
        audio_queue_t *aq = &audio_queues[audio_queue_wr = (audio_queue_wr+1) % AUDIO_MAX_QUEUE];
        aq->cursor = 0;
        aq->avail = bytes;
        aq->data = REALLOC(aq->data, bytes);
        assert(aq->data);
        if( samples ) memcpy(aq->data, samples, bytes);
        else          memset(aq->data,       0, bytes);
    thread_mutex_unlock(&queue_mutex);

    return audio_queue_voice;
}
