// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//        System interface for sound.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "z_zone.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"


// Needed for calling the actual sound output.
#define SAMPLECOUNT 512
#define NUM_CHANNELS 8
// It is 2 for 16bit, and 2 for two channels.
#define BUFMUL 4
#define MIXBUFFERSIZE (SAMPLECOUNT*BUFMUL)

#define SAMPLERATE 11025 // Hz
#define SAMPLESIZE 2 // 16bit

#define MAX_QUEUED_MIDI_MSGS 256

#define EVENT_RELEASE_NOTE 0
#define EVENT_PLAY_NOTE 1
#define EVENT_PITCH_BEND 2
#define EVENT_SYSTEM_EVENT 3
#define EVENT_CONTROLLER 4
#define EVENT_END_OF_MEASURE 5
#define EVENT_FINISH 6
#define EVENT_UNUSED 7

#define CONTROLLER_EVENT_ALL_SOUNDS_OFF 10
#define CONTROLLER_EVENT_ALL_NOTES_OFF 11
#define CONTROLLER_EVENT_MONO 12
#define CONTROLLER_EVENT_POLY 13
#define CONTROLLER_EVENT_RESET_ALL_CONTROLLERS 14
#define CONTROLLER_EVENT_EVENT 15

#define CONTROLLER_CHANGE_INSTRUMENT 0
#define CONTROLLER_BANK_SELECT 1
#define CONTROLLER_MODULATION 2
#define CONTROLLER_VOLUME 3
#define CONTROLLER_PAN 4
#define CONTROLLER_EXPRESSION 5
#define CONTROLLER_REVERB 6
#define CONTROLLER_CHORUS 7
#define CONTROLLER_SUSTAIN 8
#define CONTROLLER_SOFT 9


typedef struct
{
    char ID[4];
    unsigned short scoreLen;
    unsigned short scoreStart;
    unsigned short channels;
    unsigned short sec_channels;
    unsigned short instrCnt;
    unsigned short dummy;
} mus_header_t;


// A quick hack to establish a protocol between
// synchronous mix buffer updates and asynchronous
// audio writes. Probably redundant with gametic.
static int flag = 0;

static unsigned char* mus_data = 0;
static mus_header_t mus_header;
static int mus_offset = 0;
static int mus_delay = 0;
static doom_boolean mus_loop = false;
static doom_boolean mus_playing = false;
static int mus_volume = 127;
static int mus_channel_volumes[16] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };

static int looping = 0;
static int musicdies = -1;


// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

// The actual lengths of all sound effects.
int lengths[NUMSFX];

// The actual output device.
int audio_fd;

// The global mixing buffer.
// Basically, samples from all active internal channels
//  are modifed and added, and stored in the buffer
//  that is submitted to the audio device.
signed short mixbuffer[MIXBUFFERSIZE];


// The channel step amount...
unsigned int channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
unsigned int channelstepremainder[NUM_CHANNELS];

// The channel data pointers, start and end.
unsigned char* channels[NUM_CHANNELS];
unsigned char* channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
int channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
int channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
int channelids[NUM_CHANNELS];

// Pitch to stepping lookup, unused.
int steptable[256];

// Volume lookups.
int vol_lookup[128 * 256];

// Hardware left and right channel volume lookup.
int* channelleftvol_lookup[NUM_CHANNELS];
int* channelrightvol_lookup[NUM_CHANNELS];

unsigned long queued_midi_msgs[MAX_QUEUED_MIDI_MSGS];
int queue_midi_head = 0;
int queue_midi_tail = 0;


void TickSong();


//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
void* getsfx(char* sfxname, int* len)
{
    unsigned char* sfx;
    unsigned char* paddedsfx;
    int i;
    int size;
    int paddedsize;
    char name[20];
    int sfxlump;

    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    //doom_sprintf(name, "ds%s", sfxname);
    doom_strcpy(name, "ds");
    doom_concat(name, sfxname);

    // Now, there is a severe problem with the
    //  sound handling, in it is not (yet/anymore)
    //  gamemode aware. That means, sounds from
    //  DOOM II will be requested even with DOOM
    //  shareware.
    // The sound list is wired into sounds.c,
    //  which sets the external variable.
    // I do not do runtime patches to that
    //  variable. Instead, we will use a
    //  default sound for replacement.
    if (W_CheckNumForName(name) == -1)
        sfxlump = W_GetNumForName("dspistol");
    else
        sfxlump = W_GetNumForName(name);

    size = W_LumpLength(sfxlump);

    sfx = (unsigned char*)W_CacheLumpNum(sfxlump, PU_STATIC);

    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    paddedsfx = (unsigned char*)Z_Malloc(paddedsize + 8, PU_STATIC, 0);
    // ddt: (unsigned char *) realloc(sfx, paddedsize+8);
    // This should interfere with zone memory handling,
    //  which does not kick in in the soundserver.

    // Now copy and pad.
    doom_memcpy(paddedsfx, sfx, size);
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free(sfx);

    // Preserve padded length.
    *len = paddedsize;

    // Return allocated padded data.
    return (void*)(paddedsfx + 8);
}


//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
int addsfx(int sfxid, int volume, int step, int seperation)
{
    static unsigned short handlenums = 0;

    int i;
    int rc = -1;

    int oldest = gametic;
    int oldestnum = 0;
    int slot;

    int rightvol;
    int leftvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (sfxid == sfx_sawup
        || sfxid == sfx_sawidl
        || sfxid == sfx_sawful
        || sfxid == sfx_sawhit
        || sfxid == sfx_stnmov
        || sfxid == sfx_pistol)
    {
        // Loop all channels, check.
        for (i = 0; i < NUM_CHANNELS; i++)
        {
            // Active, and using the same SFX?
            if ((channels[i])
                && (channelids[i] == sfxid))
            {
                // Reset.
                channels[i] = 0;
                // We are sure that iff,
                //  there will only be one.
                break;
            }
        }
    }

    // Loop all channels to find oldest SFX.
    for (i = 0; (i < NUM_CHANNELS) && (channels[i]); i++)
    {
        if (channelstart[i] < oldest)
        {
            oldestnum = i;
            oldest = channelstart[i];
        }
    }

    // Tales from the cryptic.
    // If we found a channel, fine.
    // If not, we simply overwrite the first one, 0.
    // Probably only happens at startup.
    if (i == NUM_CHANNELS)
        slot = oldestnum;
    else
        slot = i;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char*)S_sfx[sfxid].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[sfxid];

    // Reset current handle number, limited to 0..100.
    if (!handlenums)
        handlenums = 100;

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = rc = handlenums++;

    // Set stepping???
    // Kinda getting the impression this is never used.
    channelstep[slot] = step;
    // ???
    channelstepremainder[slot] = 0;
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol =
        volume - ((volume * seperation * seperation) >> 16); ///(256*256);
    seperation = seperation - 257;
    rightvol =
        volume - ((volume * seperation * seperation) >> 16);

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
        I_Error("Error: rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
        I_Error("Error: leftvol out of bounds");

    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    // You tell me.
    return rc;
}


//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels()
{
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process. 
    int i;
    int j;

    int* steptablemid = steptable + 128;

    // Okay, reset internal mixing channels to zero.
    /*for (i=0; i<NUM_CHANNELS; i++)
    {
      channels[i] = 0;
    }*/

    // This table provides step widths for pitch parameters.
    // I fail to see that this is currently used.
#if 0
    for (i = -128; i < 128; i++)
    {
        doom_print("steptablemid[");
        doom_print(doom_itoa(i, 10));
        doom_print("] = ");
        doom_print(doom_itoa((int)(pow(2.0, (i / 64.0)) * 65536.0), 10));
        doom_print(";\n");
        //steptablemid[i] = (int)(pow(2.0, (i / 64.0)) * 65536.0);
    }
#endif

    steptablemid[-128] = 16384;
    steptablemid[-127] = 16562;
    steptablemid[-126] = 16742;
    steptablemid[-125] = 16925;
    steptablemid[-124] = 17109;
    steptablemid[-123] = 17295;
    steptablemid[-122] = 17484;
    steptablemid[-121] = 17674;
    steptablemid[-120] = 17866;
    steptablemid[-119] = 18061;
    steptablemid[-118] = 18258;
    steptablemid[-117] = 18456;
    steptablemid[-116] = 18657;
    steptablemid[-115] = 18861;
    steptablemid[-114] = 19066;
    steptablemid[-113] = 19274;
    steptablemid[-112] = 19483;
    steptablemid[-111] = 19696;
    steptablemid[-110] = 19910;
    steptablemid[-109] = 20127;
    steptablemid[-108] = 20346;
    steptablemid[-107] = 20568;
    steptablemid[-106] = 20792;
    steptablemid[-105] = 21018;
    steptablemid[-104] = 21247;
    steptablemid[-103] = 21478;
    steptablemid[-102] = 21712;
    steptablemid[-101] = 21949;
    steptablemid[-100] = 22188;
    steptablemid[-99] = 22429;
    steptablemid[-98] = 22673;
    steptablemid[-97] = 22920;
    steptablemid[-96] = 23170;
    steptablemid[-95] = 23422;
    steptablemid[-94] = 23677;
    steptablemid[-93] = 23935;
    steptablemid[-92] = 24196;
    steptablemid[-91] = 24459;
    steptablemid[-90] = 24726;
    steptablemid[-89] = 24995;
    steptablemid[-88] = 25267;
    steptablemid[-87] = 25542;
    steptablemid[-86] = 25820;
    steptablemid[-85] = 26102;
    steptablemid[-84] = 26386;
    steptablemid[-83] = 26673;
    steptablemid[-82] = 26964;
    steptablemid[-81] = 27257;
    steptablemid[-80] = 27554;
    steptablemid[-79] = 27854;
    steptablemid[-78] = 28157;
    steptablemid[-77] = 28464;
    steptablemid[-76] = 28774;
    steptablemid[-75] = 29087;
    steptablemid[-74] = 29404;
    steptablemid[-73] = 29724;
    steptablemid[-72] = 30048;
    steptablemid[-71] = 30375;
    steptablemid[-70] = 30706;
    steptablemid[-69] = 31040;
    steptablemid[-68] = 31378;
    steptablemid[-67] = 31720;
    steptablemid[-66] = 32065;
    steptablemid[-65] = 32415;
    steptablemid[-64] = 32768;
    steptablemid[-63] = 33124;
    steptablemid[-62] = 33485;
    steptablemid[-61] = 33850;
    steptablemid[-60] = 34218;
    steptablemid[-59] = 34591;
    steptablemid[-58] = 34968;
    steptablemid[-57] = 35348;
    steptablemid[-56] = 35733;
    steptablemid[-55] = 36122;
    steptablemid[-54] = 36516;
    steptablemid[-53] = 36913;
    steptablemid[-52] = 37315;
    steptablemid[-51] = 37722;
    steptablemid[-50] = 38132;
    steptablemid[-49] = 38548;
    steptablemid[-48] = 38967;
    steptablemid[-47] = 39392;
    steptablemid[-46] = 39821;
    steptablemid[-45] = 40254;
    steptablemid[-44] = 40693;
    steptablemid[-43] = 41136;
    steptablemid[-42] = 41584;
    steptablemid[-41] = 42037;
    steptablemid[-40] = 42494;
    steptablemid[-39] = 42957;
    steptablemid[-38] = 43425;
    steptablemid[-37] = 43898;
    steptablemid[-36] = 44376;
    steptablemid[-35] = 44859;
    steptablemid[-34] = 45347;
    steptablemid[-33] = 45841;
    steptablemid[-32] = 46340;
    steptablemid[-31] = 46845;
    steptablemid[-30] = 47355;
    steptablemid[-29] = 47871;
    steptablemid[-28] = 48392;
    steptablemid[-27] = 48919;
    steptablemid[-26] = 49452;
    steptablemid[-25] = 49990;
    steptablemid[-24] = 50535;
    steptablemid[-23] = 51085;
    steptablemid[-22] = 51641;
    steptablemid[-21] = 52204;
    steptablemid[-20] = 52772;
    steptablemid[-19] = 53347;
    steptablemid[-18] = 53928;
    steptablemid[-17] = 54515;
    steptablemid[-16] = 55108;
    steptablemid[-15] = 55709;
    steptablemid[-14] = 56315;
    steptablemid[-13] = 56928;
    steptablemid[-12] = 57548;
    steptablemid[-11] = 58175;
    steptablemid[-10] = 58809;
    steptablemid[-9] = 59449;
    steptablemid[-8] = 60096;
    steptablemid[-7] = 60751;
    steptablemid[-6] = 61412;
    steptablemid[-5] = 62081;
    steptablemid[-4] = 62757;
    steptablemid[-3] = 63440;
    steptablemid[-2] = 64131;
    steptablemid[-1] = 64830;
    steptablemid[0] = 65536;
    steptablemid[1] = 66249;
    steptablemid[2] = 66971;
    steptablemid[3] = 67700;
    steptablemid[4] = 68437;
    steptablemid[5] = 69182;
    steptablemid[6] = 69936;
    steptablemid[7] = 70697;
    steptablemid[8] = 71467;
    steptablemid[9] = 72245;
    steptablemid[10] = 73032;
    steptablemid[11] = 73827;
    steptablemid[12] = 74631;
    steptablemid[13] = 75444;
    steptablemid[14] = 76265;
    steptablemid[15] = 77096;
    steptablemid[16] = 77935;
    steptablemid[17] = 78784;
    steptablemid[18] = 79642;
    steptablemid[19] = 80509;
    steptablemid[20] = 81386;
    steptablemid[21] = 82272;
    steptablemid[22] = 83168;
    steptablemid[23] = 84074;
    steptablemid[24] = 84989;
    steptablemid[25] = 85915;
    steptablemid[26] = 86850;
    steptablemid[27] = 87796;
    steptablemid[28] = 88752;
    steptablemid[29] = 89718;
    steptablemid[30] = 90695;
    steptablemid[31] = 91683;
    steptablemid[32] = 92681;
    steptablemid[33] = 93691;
    steptablemid[34] = 94711;
    steptablemid[35] = 95742;
    steptablemid[36] = 96785;
    steptablemid[37] = 97839;
    steptablemid[38] = 98904;
    steptablemid[39] = 99981;
    steptablemid[40] = 101070;
    steptablemid[41] = 102170;
    steptablemid[42] = 103283;
    steptablemid[43] = 104408;
    steptablemid[44] = 105545;
    steptablemid[45] = 106694;
    steptablemid[46] = 107856;
    steptablemid[47] = 109030;
    steptablemid[48] = 110217;
    steptablemid[49] = 111418;
    steptablemid[50] = 112631;
    steptablemid[51] = 113857;
    steptablemid[52] = 115097;
    steptablemid[53] = 116351;
    steptablemid[54] = 117618;
    steptablemid[55] = 118898;
    steptablemid[56] = 120193;
    steptablemid[57] = 121502;
    steptablemid[58] = 122825;
    steptablemid[59] = 124162;
    steptablemid[60] = 125514;
    steptablemid[61] = 126881;
    steptablemid[62] = 128263;
    steptablemid[63] = 129660;
    steptablemid[64] = 131072;
    steptablemid[65] = 132499;
    steptablemid[66] = 133942;
    steptablemid[67] = 135400;
    steptablemid[68] = 136875;
    steptablemid[69] = 138365;
    steptablemid[70] = 139872;
    steptablemid[71] = 141395;
    steptablemid[72] = 142935;
    steptablemid[73] = 144491;
    steptablemid[74] = 146064;
    steptablemid[75] = 147655;
    steptablemid[76] = 149263;
    steptablemid[77] = 150888;
    steptablemid[78] = 152531;
    steptablemid[79] = 154192;
    steptablemid[80] = 155871;
    steptablemid[81] = 157569;
    steptablemid[82] = 159284;
    steptablemid[83] = 161019;
    steptablemid[84] = 162772;
    steptablemid[85] = 164545;
    steptablemid[86] = 166337;
    steptablemid[87] = 168148;
    steptablemid[88] = 169979;
    steptablemid[89] = 171830;
    steptablemid[90] = 173701;
    steptablemid[91] = 175592;
    steptablemid[92] = 177504;
    steptablemid[93] = 179437;
    steptablemid[94] = 181391;
    steptablemid[95] = 183367;
    steptablemid[96] = 185363;
    steptablemid[97] = 187382;
    steptablemid[98] = 189422;
    steptablemid[99] = 191485;
    steptablemid[100] = 193570;
    steptablemid[101] = 195678;
    steptablemid[102] = 197809;
    steptablemid[103] = 199963;
    steptablemid[104] = 202140;
    steptablemid[105] = 204341;
    steptablemid[106] = 206566;
    steptablemid[107] = 208816;
    steptablemid[108] = 211090;
    steptablemid[109] = 213388;
    steptablemid[110] = 215712;
    steptablemid[111] = 218061;
    steptablemid[112] = 220435;
    steptablemid[113] = 222836;
    steptablemid[114] = 225262;
    steptablemid[115] = 227715;
    steptablemid[116] = 230195;
    steptablemid[117] = 232702;
    steptablemid[118] = 235236;
    steptablemid[119] = 237797;
    steptablemid[120] = 240387;
    steptablemid[121] = 243004;
    steptablemid[122] = 245650;
    steptablemid[123] = 248325;
    steptablemid[124] = 251029;
    steptablemid[125] = 253763;
    steptablemid[126] = 256526;
    steptablemid[127] = 259320;

    // Generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples.
    for (i = 0; i < 128; i++)
        for (j = 0; j < 256; j++)
            vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
}


void I_SetSfxVolume(int volume)
{
    // Identical to DOS.
    // Basically, this should propagate
    //  the menu/config file setting
    //  to the state variable used in
    //  the mixing.
    snd_SfxVolume = volume;
}


// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    snd_MusicVolume = volume;
    mus_volume = snd_MusicVolume * 8;

    for (int i = 0; i < 16; ++i)
    {
        queued_midi_msgs[(queue_midi_tail++) % MAX_QUEUED_MIDI_MSGS] = (0x000000B0 | i | 0x0700 | (((mus_channel_volumes[i] * mus_volume) / 127) << 16));
    }
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    //doom_sprintf(namebuf, "ds%s", sfx->name);
    doom_strcpy(namebuf, "ds");
    doom_concat(namebuf, sfx->name);
    return W_GetNumForName(namebuf);
}


//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
    // Returns a handle (not used).
    id = addsfx(id, vol, steptable[pitch], sep);
    return id;
}


void I_StopSound(int handle)
{
    // You need the handle returned by StartSound.
    // Would be looping all channels,
    //  tracking down the handle,
    //  an setting the channel to zero.
}


int I_SoundIsPlaying(int handle)
{
    // Ouch.
    return gametic < handle;
}


//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//
void I_UpdateSound(void)
{
    static int song_tick_progress = 0;

    // Mix current sound data.
    // Data, from raw sound, for right and left.
    register unsigned int sample;
    register int dl;
    register int dr;

    // Pointers in global mixbuffer, left, right, end.
    signed short* leftout;
    signed short* rightout;
    signed short* leftend;
    // Step in mixbuffer, left and right, thus two.
    int step;

    // Mixing channel index.
    int chan;

    // Do music first. [dsl] TODO: if we have embedded synth
      //static int song_progress = 0;
      //song_progress += 512;
      //while (song_progress > 0)
      //{
      //    TickSong(); // About 7 music ticks per sound sampling (X for doubt)
      //    song_progress -= 78;
      //}

    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = mixbuffer;
    rightout = mixbuffer + 1;
    step = 2;

    // Determine end, for left channel only
    //  (right channel is implicit).
    leftend = mixbuffer + SAMPLECOUNT * step;

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT,
    //  that is 512 values for two channels.
    while (leftout != leftend)
    {
        // Reset left/right value. 
        dl = 0;
        dr = 0;

        // Love thy L2 chache - made this a loop.
        // Now more channels could be set at compile time
        //  as well. Thus loop those  channels.
        for (chan = 0; chan < NUM_CHANNELS; chan++)
        {
            // Check channel, if active.
            if (channels[chan])
            {
                // Get the raw data from the channel. 
                sample = *channels[chan];
                // Add left and right part
                //  for this channel (sound)
                //  to the current data.
                // Adjust volume accordingly.
                dl += channelleftvol_lookup[chan][sample];
                dr += channelrightvol_lookup[chan][sample];
                // Increment index ???
                channelstepremainder[chan] += channelstep[chan];
                // MSB is next sample???
                channels[chan] += channelstepremainder[chan] >> 16;
                // Limit to LSB???
                channelstepremainder[chan] &= 65536 - 1;

                // Check whether we are done.
                if (channels[chan] >= channelsend[chan])
                    channels[chan] = 0;
            }
        }

        // Clamp to range. Left hardware channel.
        // Has been char instead of short.
        // if (dl > 127) *leftout = 127;
        // else if (dl < -128) *leftout = -128;
        // else *leftout = dl;

        if (dl > 0x7fff)
            *leftout = 0x7fff;
        else if (dl < -0x8000)
            *leftout = -0x8000;
        else
            *leftout = dl;

        // Same for right hardware channel.
        if (dr > 0x7fff)
            *rightout = 0x7fff;
        else if (dr < -0x8000)
            *rightout = -0x8000;
        else
            *rightout = dr;

        // Increment current pointers in mixbuffer.
        leftout += step;
        rightout += step;
    }
}


//extern doom_sound_callbacks_t doom_sound_callbacks;
//extern doom_boolean skip_next_sound_update;

// 
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime. 
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//
void I_SubmitSound(void)
{
}


void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    // I fail too see that this is used.
    // Would be using the handle to identify
    //  on which channel the sound might be active,
    //  and resetting the channel parameters.
}


void I_ShutdownSound(void)
{
    // Wait till all pending sounds are finished.
    int done = 0;
    int i;

    // FIXME (below).
    doom_print("I_ShutdownSound: NOT finishing pending sounds\n");

    while (!done)
    {
        for (i = 0; i < 8 && !channels[i]; i++);

        // FIXME. No proper channel output.
        //if (i==8)
        done = 1;
    }

    // Done.
    return;
}


void I_InitSound()
{
    int i;

    // Secure and configure sound device first.
    doom_print("I_InitSound: ");

    // Initialize external data (all sounds) at start, keep static.
    doom_print("I_InitSound: ");

    for (i = 1; i < NUMSFX; i++)
    {
        // Alias? Example is the chaingun sound linked to pistol.
        if (!S_sfx[i].link)
        {
            // Load data from WAD file.
            S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
        }
        else
        {
            // Previously loaded already?
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
        }
    }

    doom_print(" pre-cached all sound data\n");

    // Now initialize mixbuffer with zero.
    for (i = 0; i < MIXBUFFERSIZE; i++)
        mixbuffer[i] = 0;

    // Finished initialization.
    doom_print("I_InitSound: sound module ready\n");
}


//
// MUSIC API.
//
void I_InitMusic(void)
{
}


void I_ShutdownMusic(void)
{
}


void I_PlaySong(int handle, int looping)
{
    musicdies = gametic + TICRATE * 30;

    mus_loop = looping ? true : false;
    mus_playing = true;
}


void I_PauseSong(int handle)
{
    mus_playing = false;
}


void I_ResumeSong(int handle)
{
    if (mus_data) mus_playing = true;
}


static void reset_all_channels()
{
    for (int i = 0; i < 16; ++i)
        queued_midi_msgs[(queue_midi_tail++) % MAX_QUEUED_MIDI_MSGS] = 0b10110000 | i | (123 << 8);
}


void I_StopSong(int handle)
{
    mus_data = 0;
    mus_delay = 0;
    mus_offset = 0;
    mus_playing = false;

    reset_all_channels();
}


void I_UnRegisterSong(int handle)
{
    I_StopSong(handle);
}


int I_RegisterSong(void* data)
{
    doom_memcpy(&mus_header, data, sizeof(mus_header_t));
    if (doom_strncmp(mus_header.ID, "MUS", 3) != 0 || mus_header.ID[3] != 0x1A) return 0;

    mus_data = (unsigned char*)data;
    mus_delay = 0;
    mus_offset = mus_header.scoreStart;
    mus_playing = false;

    return 1;
}


// Is the song playing?
int I_QrySongPlaying(int handle)
{
    return mus_playing;
}


unsigned long I_TickSong()
{
    unsigned long midi_event = 0;

    // Dequeue MIDI events
    if (queue_midi_head != queue_midi_tail)
        return queued_midi_msgs[(queue_midi_head++) % MAX_QUEUED_MIDI_MSGS];

    if (!mus_playing || !mus_data) return 0;

    if (mus_delay <= 0)
    {
        int event = (int)mus_data[mus_offset++];
        int type = (event & 0b01110000) >> 4;
        int channel = event & 0b00001111;

        if (channel == 15) channel = 9; // Percussion is 9 on GM
        else if (channel == 9) channel = 15;

        switch (type)
        {
            case EVENT_RELEASE_NOTE:
            {
                int note = (int)mus_data[mus_offset++] & 0b01111111;
                midi_event = (0x00000080 | channel | (note << 8));
                break;
            }
            case EVENT_PLAY_NOTE:
            {
                int note_bytes = (int)mus_data[mus_offset++];
                int note = note_bytes & 0b01111111;
                int vol = 127;
                if (note_bytes & 0b10000000) vol = (int)mus_data[mus_offset++] & 0b01111111;
                midi_event = (0x00000090 | channel | (note << 8) | (vol << 16));
                break;
            }
            case EVENT_PITCH_BEND:
            {
                int bend_amount = (int)mus_data[mus_offset++] * 64;
                int l = bend_amount & 0b01111111;
                int m = (bend_amount & 0b1111111110000000) >> 7;
                midi_event = (0x000000E0 | channel | (l << 8) | (m << 16));
                break;
            }
            case EVENT_SYSTEM_EVENT:
            {
                int controller = (int)mus_data[mus_offset++] & 0b01111111;
                switch (controller)
                {
                    case CONTROLLER_EVENT_ALL_SOUNDS_OFF:
                        midi_event = (0x000000B0 | channel | (120 << 8));
                        break;
                    case CONTROLLER_EVENT_ALL_NOTES_OFF:
                        midi_event = (0x000000B0 | channel | (123 << 8));
                        break;
                    case CONTROLLER_EVENT_MONO:
                        midi_event = (0x000000B0 | channel | (126 << 8));
                        break;
                    case CONTROLLER_EVENT_POLY:
                        midi_event = (0x000000B0 | channel | (127 << 8));
                        break;
                    case CONTROLLER_EVENT_RESET_ALL_CONTROLLERS:
                        midi_event = (0x000000B0 | channel | (121 << 8));
                        break;
                    case CONTROLLER_EVENT_EVENT: // Doom never implemented
                        break;
                }
                break;
            }
            case EVENT_CONTROLLER:
            {
                int controller = (int)mus_data[mus_offset++] & 0b01111111;
                int value = (int)mus_data[mus_offset++] & 0b01111111;
                switch (controller)
                {
                    case CONTROLLER_CHANGE_INSTRUMENT:
                        midi_event = (0x000000C0 | channel | (value << 8));
                        break;
                    case CONTROLLER_BANK_SELECT:
                        midi_event = (0x000000B0 | channel | 0x2000 | (value << 16));
                        break;
                    case CONTROLLER_MODULATION:
                        midi_event = (0x000000B0 | channel | 0x0100 | (value << 16));
                        break;
                    case CONTROLLER_VOLUME:
                        mus_channel_volumes[channel] = value;
                        midi_event = (0x000000B0 | channel | 0x0700 | (((mus_channel_volumes[channel] * mus_volume) / 127) << 16));
                        break;
                    case CONTROLLER_PAN:
                        midi_event = (0x000000B0 | channel | 0x0A00 | (value << 16));
                        break;
                    case CONTROLLER_EXPRESSION:
                        midi_event = (0x000000B0 | channel | 0x0B00 | (value << 16));
                        break;
                    case CONTROLLER_REVERB:
                        midi_event = (0x000000B0 | channel | 0x5B00 | (value << 16));
                        break;
                    case CONTROLLER_CHORUS:
                        midi_event = (0x000000B0 | channel | 0x5D00 | (value << 16));
                        break;
                    case CONTROLLER_SUSTAIN:
                        midi_event = (0x000000B0 | channel | 0x4000 | (value << 16));
                        break;
                    case CONTROLLER_SOFT:
                        midi_event = (0x000000B0 | channel | 0x4300 | (value << 16));
                        break;
                }
                break;
            }
            case EVENT_END_OF_MEASURE:
            {
                break;
            }
            case EVENT_FINISH:
            {
                // Loop
                if (mus_loop)
                {
                    mus_delay = 0;
                    mus_offset = mus_header.scoreStart;
                    break;
                }
                else
                {
                    mus_playing = false;
                    return 0;
                }
            }
            case EVENT_UNUSED:
            {
                int dummy = (int)mus_data[mus_offset++];
                break;
            }
        }

        if (event & 0b10000000) // Followed by delay
        {
            mus_delay = 0;
            int delay_byte = 0;
            do
            {
                delay_byte = mus_data[mus_offset++];
                mus_delay = mus_delay * 128 + delay_byte & 0b01111111;
            } while (delay_byte & 0b10000000);

            return midi_event;
        }
    }

    mus_delay--;

    return midi_event;
}
