//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
/* Pure DOOM usage

   Do this:
      #define DOOM_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define DOOM_IMPLEMENTATION
   #include "PureDOOM.h"

*/
//-----------------------------------------------------------------------------


#ifndef __DOOM_H__
#define __DOOM_H__


// Sample rate of sound samples from doom
#define DOOM_SAMPLERATE 11025

// MIDI tick needs to be called 140 times per seconds
#define DOOM_MIDI_RATE 140

// Hide menu options. If for say your platform doesn't support mouse or
// MIDI playback, you can hide these settings from the menu.
#define DOOM_FLAG_HIDE_MOUSE_OPTIONS 1 // Remove mouse options from menu
#define DOOM_FLAG_HIDE_SOUND_OPTIONS 2 // Remove sound options from menu
#define DOOM_FLAG_HIDE_MUSIC_OPTIONS 4 // Remove music options from menu

// Darken background when menu is open, making it more readable. This
// uses a bit more CPU and redraws the HUD every frame
#define DOOM_FLAG_MENU_DARKEN_BG 8


#if __cplusplus
extern "C" {
#endif

typedef enum
{
    DOOM_SEEK_CUR = 1,
    DOOM_SEEK_END = 2,
    DOOM_SEEK_SET = 0
} doom_seek_t;


typedef void(*doom_print_fn)(const char* str);
typedef void*(*doom_malloc_fn)(int size);
typedef void(*doom_free_fn)(void* ptr);
typedef void*(*doom_open_fn)(const char* filename, const char* mode);
typedef void(*doom_close_fn)(void* handle);
typedef int(*doom_read_fn)(void* handle, void *buf, int count);
typedef int(*doom_write_fn)(void* handle, const void *buf, int count);
typedef int(*doom_seek_fn)(void* handle, int offset, doom_seek_t origin);
typedef int(*doom_tell_fn)(void* handle);
typedef int(*doom_eof_fn)(void* handle);
typedef void(*doom_gettime_fn)(int* sec, int* usec);
typedef void(*doom_exit_fn)(int code);
typedef char*(*doom_getenv_fn)(const char* var);


// Doom key mapping
typedef enum
{
    DOOM_KEY_UNKNOWN = -1,
    DOOM_KEY_TAB = 9,
    DOOM_KEY_ENTER = 13,
    DOOM_KEY_ESCAPE = 27,
    DOOM_KEY_SPACE = 32,
    DOOM_KEY_APOSTROPHE = '\'',
    DOOM_KEY_MULTIPLY = '*',
    DOOM_KEY_COMMA = ',',
    DOOM_KEY_MINUS = 0x2d,
    DOOM_KEY_PERIOD = '.',
    DOOM_KEY_SLASH = '/',
    DOOM_KEY_0 = '0',
    DOOM_KEY_1 = '1',
    DOOM_KEY_2 = '2',
    DOOM_KEY_3 = '3',
    DOOM_KEY_4 = '4',
    DOOM_KEY_5 = '5',
    DOOM_KEY_6 = '6',
    DOOM_KEY_7 = '7',
    DOOM_KEY_8 = '8',
    DOOM_KEY_9 = '9',
    DOOM_KEY_SEMICOLON = ';',
    DOOM_KEY_EQUALS = 0x3d,
    DOOM_KEY_LEFT_BRACKET = '[',
    DOOM_KEY_RIGHT_BRACKET = ']',
    DOOM_KEY_A = 'a',
    DOOM_KEY_B = 'b',
    DOOM_KEY_C = 'c',
    DOOM_KEY_D = 'd',
    DOOM_KEY_E = 'e',
    DOOM_KEY_F = 'f',
    DOOM_KEY_G = 'g',
    DOOM_KEY_H = 'h',
    DOOM_KEY_I = 'i',
    DOOM_KEY_J = 'j',
    DOOM_KEY_K = 'k',
    DOOM_KEY_L = 'l',
    DOOM_KEY_M = 'm',
    DOOM_KEY_N = 'n',
    DOOM_KEY_O = 'o',
    DOOM_KEY_P = 'p',
    DOOM_KEY_Q = 'q',
    DOOM_KEY_R = 'r',
    DOOM_KEY_S = 's',
    DOOM_KEY_T = 't',
    DOOM_KEY_U = 'u',
    DOOM_KEY_V = 'v',
    DOOM_KEY_W = 'w',
    DOOM_KEY_X = 'x',
    DOOM_KEY_Y = 'y',
    DOOM_KEY_Z = 'z',
    DOOM_KEY_BACKSPACE = 127,
    DOOM_KEY_CTRL = (0x80 + 0x1d), // Both left and right
    DOOM_KEY_LEFT_ARROW = 0xac,
    DOOM_KEY_UP_ARROW = 0xad,
    DOOM_KEY_RIGHT_ARROW = 0xae,
    DOOM_KEY_DOWN_ARROW = 0xaf,
    DOOM_KEY_SHIFT = (0x80 + 0x36), // Both left and right
    DOOM_KEY_ALT = (0x80 + 0x38), // Both left and right
    DOOM_KEY_F1 = (0x80 + 0x3b),
    DOOM_KEY_F2 = (0x80 + 0x3c),
    DOOM_KEY_F3 = (0x80 + 0x3d),
    DOOM_KEY_F4 = (0x80 + 0x3e),
    DOOM_KEY_F5 = (0x80 + 0x3f),
    DOOM_KEY_F6 = (0x80 + 0x40),
    DOOM_KEY_F7 = (0x80 + 0x41),
    DOOM_KEY_F8 = (0x80 + 0x42),
    DOOM_KEY_F9 = (0x80 + 0x43),
    DOOM_KEY_F10 = (0x80 + 0x44),
    DOOM_KEY_F11 = (0x80 + 0x57),
    DOOM_KEY_F12 = (0x80 + 0x58),
    DOOM_KEY_PAUSE = 0xff
} doom_key_t;


// Mouse button mapping
typedef enum
{
    DOOM_LEFT_BUTTON = 0,
    DOOM_RIGHT_BUTTON = 1,
    DOOM_MIDDLE_BUTTON = 2
} doom_button_t;


// For the software renderer. Default is 320x200
void doom_set_resolution(int width, int height);

// Set default configurations. Lets say, changing arrows to WASD as default controls
void doom_set_default_int(const char* name, int value);
void doom_set_default_string(const char* name, const char* value);

// set callbacks
void doom_set_print(doom_print_fn print_fn);
void doom_set_malloc(doom_malloc_fn malloc_fn, doom_free_fn free_fn);
void doom_set_file_io(doom_open_fn open_fn,
                      doom_close_fn close_fn,
                      doom_read_fn read_fn,
                      doom_write_fn write_fn,
                      doom_seek_fn seek_fn,
                      doom_tell_fn tell_fn,
                      doom_eof_fn eof_fn);
void doom_set_gettime(doom_gettime_fn gettime_fn);
void doom_set_exit(doom_exit_fn exit_fn);
void doom_set_getenv(doom_getenv_fn getenv_fn);

// Initializes DOOM and start things up. Call only call one
void doom_init(int argc, char** argv, int flags);

// Call this every frame
void doom_update();

// Channels: 1 = indexed, 3 = RGB, 4 = RGBA
const unsigned char* doom_get_framebuffer(int channels);

// It is always 2048 bytes in size
short* doom_get_sound_buffer();

// Call this 140 times per second. Or about every 7ms. 
// Returns midi message. Keep calling it until it returns 0.
unsigned long doom_tick_midi();

// Events
void doom_key_down(doom_key_t key);
void doom_key_up(doom_key_t key);
void doom_button_down(doom_button_t button);
void doom_button_up(doom_button_t button);
void doom_mouse_move(int delta_x, int delta_y);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
#ifndef __D_ENGLSH__
#define __D_ENGLSH__


//
// Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR "Development mode ON.\n"
#define D_CDROM "CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//        M_Menu.C
//
#define PRESSKEY "press a key."
#define PRESSYN "press y or n."
#define QUITMSG "are you sure you want to\nquit this great game?"
#define LOADNET "you can't do load while in a net game!\n\n" PRESSKEY
#define QLOADNET "you can't quickload during a netgame!\n\n" PRESSKEY
#define QSAVESPOT "you haven't picked a quicksave slot yet!\n\n" PRESSKEY
#define SAVEDEAD "you can't save if you aren't playing!\n\n" PRESSKEY
#define QSPROMPT "quicksave over your game named\n\n'%s'?\n\n" PRESSYN
#define QLPROMPT "do you want to quickload the game named\n\n'%s'?\n\n" PRESSYN

#define QSPROMPT_1 "quicksave over your game named\n\n'"
#define QSPROMPT_2 "'?\n\n" PRESSYN
#define QLPROMPT_1 "do you want to quickload the game named\n\n'%s"
#define QLPROMPT_2 "'?\n\n" PRESSYN

#define NEWGAME \
"you can't start a new game\n"\
"while in a network game.\n\n" PRESSKEY

#define NIGHTMARE \
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\n" PRESSYN

#define SWSTRING \
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\n" PRESSKEY

#define MSGOFF "Messages OFF"
#define MSGON "Messages ON"
#define CROSSOFF "Crosshair OFF"
#define CROSSON "Crosshair ON"
#define ALWAYSRUNOFF "Always run OFF"
#define ALWAYSRUNON "Always run ON"
#define NETEND "you can't end a netgame!\n\n" PRESSKEY
#define ENDGAME "are you sure you want to end the game?\n\n" PRESSYN

#define DOSY "(press y to quit)"

#define DETAILHI "High detail"
#define DETAILLO "Low detail"
#define GAMMALVL0 "Gamma correction OFF"
#define GAMMALVL1 "Gamma correction level 1"
#define GAMMALVL2 "Gamma correction level 2"
#define GAMMALVL3 "Gamma correction level 3"
#define GAMMALVL4 "Gamma correction level 4"
#define EMPTYSTRING "empty slot"

//
// P_inter.C
//
#define GOTARMOR    "Picked up the armor."
#define GOTMEGA     "Picked up the MegaArmor!"
#define GOTHTHBONUS "Picked up a health bonus."
#define GOTARMBONUS "Picked up an armor bonus."
#define GOTSTIM     "Picked up a stimpack."
#define GOTMEDINEED "Picked up a medikit that you REALLY need!"
#define GOTMEDIKIT  "Picked up a medikit."
#define GOTSUPER    "Supercharge!"

#define GOTBLUECARD "Picked up a blue keycard."
#define GOTYELWCARD "Picked up a yellow keycard."
#define GOTREDCARD  "Picked up a red keycard."
#define GOTBLUESKUL "Picked up a blue skull key."
#define GOTYELWSKUL "Picked up a yellow skull key."
#define GOTREDSKULL "Picked up a red skull key."

#define GOTINVUL    "Invulnerability!"
#define GOTBERSERK  "Berserk!"
#define GOTINVIS    "Partial Invisibility"
#define GOTSUIT     "Radiation Shielding Suit"
#define GOTMAP      "Computer Area Map"
#define GOTVISOR    "Light Amplification Visor"
#define GOTMSPHERE  "MegaSphere!"

#define GOTCLIP     "Picked up a clip."
#define GOTCLIPBOX  "Picked up a box of bullets."
#define GOTROCKET   "Picked up a rocket."
#define GOTROCKBOX  "Picked up a box of rockets."
#define GOTCELL     "Picked up an energy cell."
#define GOTCELLBOX  "Picked up an energy cell pack."
#define GOTSHELLS   "Picked up 4 shotgun shells."
#define GOTSHELLBOX "Picked up a box of shotgun shells."
#define GOTBACKPACK "Picked up a backpack full of ammo!"

#define GOTBFG9000  "You got the BFG9000!  Oh, yes."
#define GOTCHAINGUN "You got the chaingun!"
#define GOTCHAINSAW "A chainsaw!  Find some meat!"
#define GOTLAUNCHER "You got the rocket launcher!"
#define GOTPLASMA   "You got the plasma gun!"
#define GOTSHOTGUN  "You got the shotgun!"
#define GOTSHOTGUN2 "You got the super shotgun!"

//
// P_Doors.C
//
#define PD_BLUEO    "You need a blue key to activate this object"
#define PD_REDO     "You need a red key to activate this object"
#define PD_YELLOWO  "You need a yellow key to activate this object"
#define PD_BLUEK    "You need a blue key to open this door"
#define PD_REDK     "You need a red key to open this door"
#define PD_YELLOWK  "You need a yellow key to open this door"

//
// G_game.C
//
#define GGSAVED     "game saved."

//
// HU_stuff.C
//
#define HUSTR_MSGU "[Message unsent]"

#define HUSTR_E1M1 "E1M1: Hangar"
#define HUSTR_E1M2 "E1M2: Nuclear Plant"
#define HUSTR_E1M3 "E1M3: Toxin Refinery"
#define HUSTR_E1M4 "E1M4: Command Control"
#define HUSTR_E1M5 "E1M5: Phobos Lab"
#define HUSTR_E1M6 "E1M6: Central Processing"
#define HUSTR_E1M7 "E1M7: Computer Station"
#define HUSTR_E1M8 "E1M8: Phobos Anomaly"
#define HUSTR_E1M9 "E1M9: Military Base"

#define HUSTR_E2M1 "E2M1: Deimos Anomaly"
#define HUSTR_E2M2 "E2M2: Containment Area"
#define HUSTR_E2M3 "E2M3: Refinery"
#define HUSTR_E2M4 "E2M4: Deimos Lab"
#define HUSTR_E2M5 "E2M5: Command Center"
#define HUSTR_E2M6 "E2M6: Halls of the Damned"
#define HUSTR_E2M7 "E2M7: Spawning Vats"
#define HUSTR_E2M8 "E2M8: Tower of Babel"
#define HUSTR_E2M9 "E2M9: Fortress of Mystery"

#define HUSTR_E3M1 "E3M1: Hell Keep"
#define HUSTR_E3M2 "E3M2: Slough of Despair"
#define HUSTR_E3M3 "E3M3: Pandemonium"
#define HUSTR_E3M4 "E3M4: House of Pain"
#define HUSTR_E3M5 "E3M5: Unholy Cathedral"
#define HUSTR_E3M6 "E3M6: Mt. Erebus"
#define HUSTR_E3M7 "E3M7: Limbo"
#define HUSTR_E3M8 "E3M8: Dis"
#define HUSTR_E3M9 "E3M9: Warrens"

#define HUSTR_E4M1 "E4M1: Hell Beneath"
#define HUSTR_E4M2 "E4M2: Perfect Hatred"
#define HUSTR_E4M3 "E4M3: Sever The Wicked"
#define HUSTR_E4M4 "E4M4: Unruly Evil"
#define HUSTR_E4M5 "E4M5: They Will Repent"
#define HUSTR_E4M6 "E4M6: Against Thee Wickedly"
#define HUSTR_E4M7 "E4M7: And Hell Followed"
#define HUSTR_E4M8 "E4M8: Unto The Cruel"
#define HUSTR_E4M9 "E4M9: Fear"

#define HUSTR_1 "level 1: entryway"
#define HUSTR_2 "level 2: underhalls"
#define HUSTR_3 "level 3: the gantlet"
#define HUSTR_4 "level 4: the focus"
#define HUSTR_5 "level 5: the waste tunnels"
#define HUSTR_6 "level 6: the crusher"
#define HUSTR_7 "level 7: dead simple"
#define HUSTR_8 "level 8: tricks and traps"
#define HUSTR_9 "level 9: the pit"
#define HUSTR_10 "level 10: refueling base"
#define HUSTR_11 "level 11: 'o' of destruction!"

#define HUSTR_12 "level 12: the factory"
#define HUSTR_13 "level 13: downtown"
#define HUSTR_14 "level 14: the inmost dens"
#define HUSTR_15 "level 15: industrial zone"
#define HUSTR_16 "level 16: suburbs"
#define HUSTR_17 "level 17: tenements"
#define HUSTR_18 "level 18: the courtyard"
#define HUSTR_19 "level 19: the citadel"
#define HUSTR_20 "level 20: gotcha!"

#define HUSTR_21 "level 21: nirvana"
#define HUSTR_22 "level 22: the catacombs"
#define HUSTR_23 "level 23: barrels o' fun"
#define HUSTR_24 "level 24: the chasm"
#define HUSTR_25 "level 25: bloodfalls"
#define HUSTR_26 "level 26: the abandoned mines"
#define HUSTR_27 "level 27: monster condo"
#define HUSTR_28 "level 28: the spirit world"
#define HUSTR_29 "level 29: the living end"
#define HUSTR_30 "level 30: icon of sin"

#define HUSTR_31 "level 31: wolfenstein"
#define HUSTR_32 "level 32: grosse"

#define PHUSTR_1 "level 1: congo"
#define PHUSTR_2 "level 2: well of souls"
#define PHUSTR_3 "level 3: aztec"
#define PHUSTR_4 "level 4: caged"
#define PHUSTR_5 "level 5: ghost town"
#define PHUSTR_6 "level 6: baron's lair"
#define PHUSTR_7 "level 7: caughtyard"
#define PHUSTR_8 "level 8: realm"
#define PHUSTR_9 "level 9: abattoire"
#define PHUSTR_10 "level 10: onslaught"
#define PHUSTR_11 "level 11: hunted"

#define PHUSTR_12 "level 12: speed"
#define PHUSTR_13 "level 13: the crypt"
#define PHUSTR_14 "level 14: genesis"
#define PHUSTR_15 "level 15: the twilight"
#define PHUSTR_16 "level 16: the omen"
#define PHUSTR_17 "level 17: compound"
#define PHUSTR_18 "level 18: neurosphere"
#define PHUSTR_19 "level 19: nme"
#define PHUSTR_20 "level 20: the death domain"

#define PHUSTR_21 "level 21: slayer"
#define PHUSTR_22 "level 22: impossible mission"
#define PHUSTR_23 "level 23: tombstone"
#define PHUSTR_24 "level 24: the final frontier"
#define PHUSTR_25 "level 25: the temple of darkness"
#define PHUSTR_26 "level 26: bunker"
#define PHUSTR_27 "level 27: anti-christ"
#define PHUSTR_28 "level 28: the sewers"
#define PHUSTR_29 "level 29: odyssey of noises"
#define PHUSTR_30 "level 30: the gateway of hell"

#define PHUSTR_31 "level 31: cyberden"
#define PHUSTR_32 "level 32: go 2 it"

#define THUSTR_1 "level 1: system control"
#define THUSTR_2 "level 2: human bbq"
#define THUSTR_3 "level 3: power control"
#define THUSTR_4 "level 4: wormhole"
#define THUSTR_5 "level 5: hanger"
#define THUSTR_6 "level 6: open season"
#define THUSTR_7 "level 7: prison"
#define THUSTR_8 "level 8: metal"
#define THUSTR_9 "level 9: stronghold"
#define THUSTR_10 "level 10: redemption"
#define THUSTR_11 "level 11: storage facility"

#define THUSTR_12 "level 12: crater"
#define THUSTR_13 "level 13: nukage processing"
#define THUSTR_14 "level 14: steel works"
#define THUSTR_15 "level 15: dead zone"
#define THUSTR_16 "level 16: deepest reaches"
#define THUSTR_17 "level 17: processing area"
#define THUSTR_18 "level 18: mill"
#define THUSTR_19 "level 19: shipping/respawning"
#define THUSTR_20 "level 20: central processing"

#define THUSTR_21 "level 21: administration center"
#define THUSTR_22 "level 22: habitat"
#define THUSTR_23 "level 23: lunar mining project"
#define THUSTR_24 "level 24: quarry"
#define THUSTR_25 "level 25: baron's den"
#define THUSTR_26 "level 26: ballistyx"
#define THUSTR_27 "level 27: mount pain"
#define THUSTR_28 "level 28: heck"
#define THUSTR_29 "level 29: river styx"
#define THUSTR_30 "level 30: last call"

#define THUSTR_31 "level 31: pharaoh"
#define THUSTR_32 "level 32: caribbean"

#define HUSTR_CHATMACRO1 "I'm ready to kick butt!"
#define HUSTR_CHATMACRO2 "I'm OK."
#define HUSTR_CHATMACRO3 "I'm not looking too good!"
#define HUSTR_CHATMACRO4 "Help!"
#define HUSTR_CHATMACRO5 "You suck!"
#define HUSTR_CHATMACRO6 "Next time, scumbag..."
#define HUSTR_CHATMACRO7 "Come here!"
#define HUSTR_CHATMACRO8 "I'll take care of it."
#define HUSTR_CHATMACRO9 "Yes"
#define HUSTR_CHATMACRO0 "No"

#define HUSTR_TALKTOSELF1 "You mumble to yourself"
#define HUSTR_TALKTOSELF2 "Who's there?"
#define HUSTR_TALKTOSELF3 "You scare yourself"
#define HUSTR_TALKTOSELF4 "You start to rave"
#define HUSTR_TALKTOSELF5 "You've lost it..."

#define HUSTR_MESSAGESENT "[Message Sent]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define HUSTR_PLRGREEN  "Green: "
#define HUSTR_PLRINDIGO "Indigo: "
#define HUSTR_PLRBROWN  "Brown: "
#define HUSTR_PLRRED    "Red: "

#define HUSTR_KEYGREEN  'g'
#define HUSTR_KEYINDIGO 'i'
#define HUSTR_KEYBROWN  'b'
#define HUSTR_KEYRED    'r'

//
// AM_map.C
//

#define AMSTR_FOLLOWON  "Follow Mode ON"
#define AMSTR_FOLLOWOFF "Follow Mode OFF"

#define AMSTR_GRIDON    "Grid ON"
#define AMSTR_GRIDOFF   "Grid OFF"

#define AMSTR_MARKEDSPOT    "Marked Spot"
#define AMSTR_MARKSCLEARED  "All Marks Cleared"

//
// ST_stuff.C
//

#define STSTR_MUS       "Music Change"
#define STSTR_NOMUS     "IMPOSSIBLE SELECTION"
#define STSTR_DQDON     "Degreelessness Mode On"
#define STSTR_DQDOFF    "Degreelessness Mode Off"

#define STSTR_KFAADDED  "Very Happy Ammo Added"
#define STSTR_FAADDED   "Ammo (no keys) Added"

#define STSTR_NCON      "No Clipping Mode ON"
#define STSTR_NCOFF     "No Clipping Mode OFF"

#define STSTR_BEHOLD    "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define STSTR_BEHOLDX   "Power-up Toggled"

#define STSTR_CHOPPERS  "... doesn't suck - GM"
#define STSTR_CLEV      "Changing Level..."

//
// F_Finale.C
//
#define E1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base.  Looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"


#define E2TEXT \
"You've done it! The hideous cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But ... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to  the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's on to the final chapter of\n"\
"DOOM! -- Inferno."


#define E3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair -- for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you ..."


#define E4TEXT \
"the spider mastermind must have sent forth\n"\
"its legions of hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from hell.  but you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"besides, someone was gonna pay for what\n"\
"happened to daisy, your pet rabbit.\n"\
"\n"\
"but now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"next stop, hell on earth!"


// after level 6, put this:
#define C1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n" \
"STARPORT. BUT SOMETHING IS WRONG. THE\n" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\n" \
"\n"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" \
"OF THE STARBASE AND FIND THE CONTROLLING\n" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\n" \
"HOSTAGE."

// After level 11, put this:
#define C2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\n"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"\
"THAT YOU HAVE SAVED YOUR SPECIES.\n"\
"\n"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\n"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"\
"UP AND RETURN TO THE FRAY."


// After level 20, put this:
#define C3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"\
"TEETH AND PLUNGE THROUGH IT.\n"\
"\n"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\n"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"\
"GOT TO GO THROUGH HELL TO GET TO IT?"


// After level 29, put this:
#define C4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\n"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"\
"UP AND DIES, ITS THRASHING LIMBS\n"\
"DEVASTATING UNTOLD MILES OF HELL'S\n"\
"SURFACE.\n"\
"\n"\
"YOU'VE DONE IT. THE INVASION IS OVER.\n"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\n"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"\
"HOME. REBUILDING EARTH OUGHT TO BE A\n"\
"LOT MORE FUN THAN RUINING IT WAS.\n"



// Before level 31, put this:
#define C5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."


// Before level 32, put this:
#define C6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"


// after map 06        
#define P1TEXT  \
"You gloat over the steaming carcass of the\n"\
"Guardian.  With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell.  You relax and glance around the\n"\
"room.  Damn!  There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."


// after map 11
#define P2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."


// after map 20
#define P3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive.  Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth.  Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"

// after map 30
#define P4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place.  As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons.  You're done. Hell\n"\
"has gone back to pounding bad dead folks \n"\
"instead of good live ones.  Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up ..."

// before map 31
#define P5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32
#define P6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."


#define T1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs.   It seems that UAC has\n"\
"once again gulped it down.  With their\n"\
"high turnover, it must be hard for poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays..\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."


#define T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define T4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeeze, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."


#define T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define T6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"Maybe someone else can kick Hell's ass\n"\
"next time around. Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a cyberdemon's iron shoe."


//
// Character cast strings F_FINALE.C
//
#define CC_ZOMBIE   "ZOMBIEMAN"
#define CC_SHOTGUN  "SHOTGUN GUY"
#define CC_HEAVY    "HEAVY WEAPON DUDE"
#define CC_IMP      "IMP"
#define CC_DEMON    "DEMON"
#define CC_LOST     "LOST SOUL"
#define CC_CACO     "CACODEMON"
#define CC_HELL     "HELL KNIGHT"
#define CC_BARON    "BARON OF HELL"
#define CC_ARACH    "ARACHNOTRON"
#define CC_PAIN     "PAIN ELEMENTAL"
#define CC_REVEN    "REVENANT"
#define CC_MANCU    "MANCUBUS"
#define CC_ARCH     "ARCH-VILE"
#define CC_SPIDER   "THE SPIDER MASTERMIND"
#define CC_CYBER    "THE CYBERDEMON"
#define CC_HERO     "OUR HERO"


#endif


#ifndef __D_THINK__
#define __D_THINK__


//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//
typedef void (*actionf_v)();
typedef void (*actionf_p1)(void*);
typedef void (*actionf_p2)(void*, void*);

typedef union
{
    actionf_p1 acp1;
    actionf_v  acv;
    actionf_p2 acp2;
} actionf_t;


// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
typedef actionf_t think_t;


// Doubly linked list of actors.
typedef struct thinker_s
{
    struct thinker_s* prev;
    struct thinker_s* next;
    think_t function;
} thinker_t;


#endif


#ifndef __DOOM_CONFIG_H__
#define __DOOM_CONFIG_H__


#if defined(WIN32)
#define DOOM_WIN32
#elif defined(__APPLE__)
#define DOOM_APPLE
#else
#define DOOM_LINUX
#endif


//#include "DOOM.h"


#define doom_abs(x) ((x) < 0 ? -(x) : (x))


extern char error_buf[260];
extern int doom_flags;
extern doom_print_fn doom_print;
extern doom_malloc_fn doom_malloc;
extern doom_free_fn doom_free;
extern doom_open_fn doom_open;
extern doom_close_fn doom_close;
extern doom_read_fn doom_read;
extern doom_write_fn doom_write;
extern doom_seek_fn doom_seek;
extern doom_tell_fn doom_tell;
extern doom_eof_fn doom_eof;
extern doom_gettime_fn doom_gettime;
extern doom_exit_fn doom_exit;
extern doom_getenv_fn doom_getenv;


const char* doom_itoa(int i, int radix);
const char* doom_ctoa(char c);
const char* doom_ptoa(void* p);
void doom_memset(void* ptr, int value, int num);
void* doom_memcpy(void* destination, const void* source, int num);
int doom_fprint(void* handle, const char* str);
int doom_strlen(const char* str);
char* doom_concat(char* dst, const char* src);
char* doom_strcpy(char* destination, const char* source);
char* doom_strncpy(char* destination, const char* source, int num);
int doom_strcmp(const char* str1, const char* str2);
int doom_strncmp(const char* str1, const char* str2, int n);
int doom_strcasecmp(const char* str1, const char* str2);
int doom_strncasecmp(const char* str1, const char* str2, int n);
int doom_atoi(const char* str);
int doom_atox(const char* str);
int doom_toupper(int c);


#endif
#ifndef __DOOMDEF__
#define __DOOMDEF__






//
// Global parameters/defines.
//
// DOOM version
enum
{
    VERSION = 110
};


// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum
{
    shareware,      // DOOM 1 shareware, E1, M9
    registered,     // DOOM 1 registered, E3, M27
    commercial,     // DOOM 2 retail, E1 M34
    // DOOM 2 german edition not handled
    retail,         // DOOM 1 retail, E4, M36
    indetermined    // Well, no IWAD found.
} GameMode_t;


// Mission packs - might be useful for TC stuff?
typedef enum
{
    doom,       // DOOM 1
    doom2,      // DOOM 2
    pack_tnt,   // TNT mission pack
    pack_plut,  // Plutonia pack
    none
} GameMission_t;


// Identify language to use, software localization.
typedef enum
{
    english,
    french,
    german,
    unknown
} Language_t;


// If rangecheck is undefined,
// most parameter validation debugging code will not be compiled
#define RANGECHECK

//
// For resize of screen, at start of game.
// It will not work dynamically, see visplanes.
//
#define BASE_WIDTH 320

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.
#define SCREEN_MUL 1
#define INV_ASPECT_RATIO 0.625 // 0.75, ideally

// Defines suck. C sucks.
// C++ might sucks for OOP, but it sure is a better C.
// So there.
// extern int SCREENWIDTH;
#define SCREENWIDTH 320
#define SCREENHEIGHT 200

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS 4

// State updates, number of tics / second.
#if defined(DOOM_FAST_TICK)
#define TICKMUL 2
#else
#define TICKMUL 1
#endif
#define TICRATE (35 * TICKMUL)


// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or a demo. 
typedef enum
{
    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,
    GS_DEMOSCREEN
} gamestate_t;


//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define MTF_EASY    1
#define MTF_NORMAL  2
#define MTF_HARD    4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH  8


typedef enum
{
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;


//
// Key cards.
//
typedef enum
{
    it_bluecard,
    it_yellowcard,
    it_redcard,
    it_blueskull,
    it_yellowskull,
    it_redskull,
    NUMCARDS
} card_t;


// The defined weapons,
// including a marker indicating
// user has not changed weapon.
typedef enum
{
    wp_fist,
    wp_pistol,
    wp_shotgun,
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg,
    wp_chainsaw,
    wp_supershotgun,
    NUMWEAPONS,
    // No pending weapon change.
    wp_nochange
} weapontype_t;


// Ammunition types defined.
typedef enum
{
    am_clip,    // Pistol / chaingun ammo.
    am_shell,   // Shotgun / double barreled shotgun.
    am_cell,    // Plasma rifle, BFG.
    am_misl,    // Missile launcher.
    NUMAMMO,
    am_noammo   // Unlimited for chainsaw / fist.        
} ammotype_t;


// Power up artifacts.
typedef enum
{
    pw_invulnerability,
    pw_strength,
    pw_invisibility,
    pw_ironfeet,
    pw_allmap,
    pw_infrared,
    NUMPOWERS
} powertype_t;


//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
    INVULNTICS = (30 * TICRATE),
    INVISTICS = (60 * TICRATE),
    INFRATICS = (120 * TICRATE),
    IRONTICS = (60 * TICRATE)
} powerduration_t;


//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#if 0 //< @r-lyeh
#define KEY_RIGHTARROW  0xae
#define KEY_LEFTARROW   0xac
#define KEY_UPARROW     0xad
#define KEY_DOWNARROW   0xaf
#define KEY_ESCAPE      27
#define KEY_ENTER       13
#define KEY_TAB         9
#define KEY_F1          (0x80+0x3b)
#define KEY_F2          (0x80+0x3c)
#define KEY_F3          (0x80+0x3d)
#define KEY_F4          (0x80+0x3e)
#define KEY_F5          (0x80+0x3f)
#define KEY_F6          (0x80+0x40)
#define KEY_F7          (0x80+0x41)
#define KEY_F8          (0x80+0x42)
#define KEY_F9          (0x80+0x43)
#define KEY_F10         (0x80+0x44)
#define KEY_F11         (0x80+0x57)
#define KEY_F12         (0x80+0x58)

#define KEY_BACKSPACE   127
#define KEY_PAUSE       0xff

#define KEY_EQUALS      0x3d
#define KEY_MINUS       0x2d

#define KEY_RSHIFT      (0x80+0x36)
#define KEY_RCTRL       (0x80+0x1d)
#define KEY_RALT        (0x80+0x38)

#define KEY_LALT        KEY_RALT
#endif


#endif          // __DOOMDEF__


#ifndef __D_ITEMS__
#define __D_ITEMS__


//#include "doomdef.h"


// Weapon info: sprite frames, ammunition use.
typedef struct
{
    ammotype_t ammo;
    int upstate;
    int downstate;
    int readystate;
    int atkstate;
    int flashstate;
} weaponinfo_t;


extern weaponinfo_t weaponinfo[NUMWEAPONS];


#endif


#ifndef __DOOMTYPE__
#define __DOOMTYPE__


// Fixed to use builtin bool type with C++.
#ifdef __cplusplus
typedef bool doom_boolean;
#else
#if !defined(false) && !defined(true)
typedef enum
{
    false, true
} doom_boolean;
#else
typedef int doom_boolean;
#endif
#endif


typedef unsigned char byte;


#define DOOM_MAXCHAR    ((char)0x7f)
#define DOOM_MAXSHORT   ((short)0x7fff)

// Max pos 32-bit int.
#define DOOM_MAXINT     ((int)0x7fffffff)        
#define DOOM_MAXLONG    ((long)0x7fffffff)
#define DOOM_MINCHAR    ((char)0x80)
#define DOOM_MINSHORT   ((short)0x8000)

// Max negative 32-bit integer.
#define DOOM_MININT     ((int)0x80000000)        
#define DOOM_MINLONG    ((long)0x80000000)


#endif


#ifndef __D_EVENT__
#define __D_EVENT__


//#include "doomtype.h"


//
// Event handling.
//

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_joystick
} evtype_t;


// Event structure.
typedef struct
{
    evtype_t type;
    int data1;  // keys / mouse/joystick buttons
    int data2;  // mouse/joystick x move
    int data3;  // mouse/joystick y move
} event_t;


typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot
} gameaction_t;


//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK = 1,
    // Use button, to open doors, activate switches.
    BT_USE = 2,

    // Flag: game events, not really buttons.
    BT_SPECIAL = 128,
    BT_SPECIALMASK = 3,

    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE = 4,
    // The 3bit weapon mask and shift, convenience.
    BT_WEAPONMASK = (8 + 16 + 32),
    BT_WEAPONSHIFT = 3,

    // Pause the game.
    BTS_PAUSE = 1,
    // Save the game at each console.
    BTS_SAVEGAME = 2,

    // Savegame slot numbers
    //  occupy the second byte of buttons.    
    BTS_SAVEMASK = (4 + 8 + 16),
    BTS_SAVESHIFT = 2,
} buttoncode_t;


//
// GLOBAL VARIABLES
//
#define MAXEVENTS (64 * 64) // [pd] Crank up the number because we pump them faster

extern event_t events[MAXEVENTS];
extern int eventhead;
extern int eventtail;

extern gameaction_t gameaction;


#endif


#ifndef __AMMAP_H__
#define __AMMAP_H__


//#include "d_event.h"


// Used by ST StatusBar stuff.
#define AM_MSGHEADER (('a'<<24)+('m'<<16))
#define AM_MSGENTERED (AM_MSGHEADER | ('e'<<8))
#define AM_MSGEXITED (AM_MSGHEADER | ('x'<<8))


// Called by main loop.
doom_boolean AM_Responder(event_t* ev);

// Called by main loop.
void AM_Ticker(void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer(void);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop(void);


#endif


#ifndef __D_MAIN__
#define __D_MAIN__


//#include "d_event.h"


#define MAXWADFILES 20


extern char* wadfiles[MAXWADFILES];


void D_AddFile(char* file);

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overrided by user input, calls N_AdvanceDemo.
//
void D_DoomMain(void);

// Called by IO functions when input is detected.
void D_PostEvent(event_t* ev);

//
// BASE LEVEL
//
void D_PageTicker(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
void D_StartTitle(void);


#endif
#ifndef __D_TEXTUR__
#define __D_TEXTUR__


//#include "doomtype.h"


//
// Flats?
//
// a pic is an unmasked block of pixels
typedef struct
{
    byte width;
    byte height;
    byte data;
} pic_t;


#endif


#ifndef __D_TICCMD__
#define __D_TICCMD__


//#include "doomtype.h"


// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.
typedef struct
{
    char forwardmove;   // *2048 for move
    char sidemove;      // *2048 for move
    short angleturn;    // <<16 for angle delta
    short consistancy;  // checks for net game
    byte chatchar;
    byte buttons;
} ticcmd_t;


#endif


#ifndef __DOOMDATA__
#define __DOOMDATA__


// The most basic types we use, portability.
//#include "doomtype.h"

// Some global defines, that configure the game.
//#include "doomdef.h"


//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
    ML_LABEL,       // A separator, name, ExMx or MAPxx
    ML_THINGS,      // Monsters, items..
    ML_LINEDEFS,    // LineDefs, from editing
    ML_SIDEDEFS,    // SideDefs, from editing
    ML_VERTEXES,    // Vertices, edited and BSP splits generated
    ML_SEGS,        // LineSegs, from LineDefs split by BSP
    ML_SSECTORS,    // SubSectors, list of LineSegs
    ML_NODES,       // BSP nodes
    ML_SECTORS,     // Sectors, from editing
    ML_REJECT,      // LUT, sector-sector visibility        
    ML_BLOCKMAP     // LUT, motion clipping, walls/grid element
};


// A single Vertex.
typedef struct
{
    short x;
    short y;
} mapvertex_t;


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
    short textureoffset;
    short rowoffset;
    char toptexture[8];
    char bottomtexture[8];
    char midtexture[8];
    // Front sector, towards viewer.
    short sector;
} mapsidedef_t;


// A LineDef, as used for editing, and as input
// to the BSP builder.
typedef struct
{
    short v1;
    short v2;
    short flags;
    short special;
    short tag;
    // sidenum[1] will be -1 if one sided
    short sidenum[2];
} maplinedef_t;


//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING 1

// Blocks monsters only.
#define ML_BLOCKMONSTERS 2

// Backside will not be present at all
//  if not two sided.
#define ML_TWOSIDED 4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP 8

// lower texture unpegged
#define ML_DONTPEGBOTTOM 16        

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET 32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK 64

// Don't draw on the automap at all.
#define ML_DONTDRAW 128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED 256


// Sector definition, from editing.
typedef struct
{
    short floorheight;
    short ceilingheight;
    char floorpic[8];
    char ceilingpic[8];
    short lightlevel;
    short special;
    short tag;
} mapsector_t;


// SubSector, as generated by BSP.
typedef struct
{
    short numsegs;
    // Index of first one, segs are stored sequentially.
    short firstseg;
} mapsubsector_t;


// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
    short v1;
    short v2;
    short angle;
    short linedef;
    short side;
    short offset;
} mapseg_t;


//
// BSP node structure.
//

// Indicate a leaf.
#define NF_SUBSECTOR 0x8000

typedef struct
{
    // Partition line from (x,y) to x+dx,y+dy)
    short x;
    short y;
    short dx;
    short dy;

    // Bounding box for each child,
    // clip against view frustum.
    short bbox[2][4];

    // If NF_SUBSECTOR its a subsector,
    // else it's a node of another subtree.
    unsigned short children[2];
} mapnode_t;


// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct
{
    short x;
    short y;
    short angle;
    short type;
    short options;
} mapthing_t;


#endif // __DOOMDATA__


#ifndef __DSTRINGS__
#define __DSTRINGS__


// All important printed strings.
// Language selection (message strings).
// Use -DFRENCH etc.

#ifdef FRENCH
//#include  "d_french.h" // Leave the extra space there, to throw off regex in PureDOOM.h creation
#else
//#include "d_englsh.h"
#endif


// Misc. other strings.
#define SAVEGAMENAME "doomsav"

//
// File locations,
//  relative to current position.
// Path names are OS-sensitive.
//
#define DEVMAPS "devmaps"
#define DEVDATA "devdata"

// Not done in french?

// QuitDOOM messages
#define NUM_QUITMESSAGES   22


extern char* endmsg[];


#endif


#ifndef __F_FINALE__
#define __F_FINALE__


//#include "doomtype.h"
//#include "d_event.h"


//
// FINALE
//

// Called by main loop.
doom_boolean F_Responder(event_t* ev);

// Called by main loop.
void F_Ticker(void);

// Called by main loop.
void F_Drawer(void);

void F_StartFinale(void);


#endif


#ifndef __F_WIPE_H__
#define __F_WIPE_H__


//
// SCREEN WIPE PACKAGE
//

enum
{
    // simple gradual pixel change for 8-bit only
    wipe_ColorXForm,

    // weird screen melt
    wipe_Melt,

    wipe_NUMWIPES
};


int wipe_StartScreen(int x, int y, int width, int height);
int wipe_EndScreen(int x, int y, int width, int height);
int wipe_ScreenWipe(int wipeno, int x, int y, int width, int height, int ticks);


#endif


#ifndef __G_GAME__
#define __G_GAME__


//#include "doomdef.h"
//#include "d_event.h"


//
// GAME
//
void G_DeathMatchSpawnPlayer(int playernum);

void G_InitNew(skill_t skill, int episode, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew(skill_t skill, int episode, int map);

void G_DeferedPlayDemo(char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame(char* name);

void G_DoLoadGame(void);

// Called by M_Responder.
void G_SaveGame(int slot, char* description);

// Only called by startup code.
void G_RecordDemo(char* name);

void G_BeginRecording(void);

void G_TimeDemo(char* name);
doom_boolean G_CheckDemoStatus(void);

void G_ExitLevel(void);
void G_SecretExitLevel(void);

void G_WorldDone(void);

void G_Ticker(void);
doom_boolean G_Responder(event_t* ev);

void G_ScreenShot(void);


#endif


#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__


//#include "d_event.h"


//
// Globally visible constants.
//
#define HU_FONTSTART '!'    // the first font characters
#define HU_FONTEND '_'      // the last font characters

// Calculate # of glyphs in font.
#define HU_FONTSIZE (HU_FONTEND - HU_FONTSTART + 1)        

#define HU_BROADCAST 5

#define HU_MSGREFRESH KEY_ENTER
#define HU_MSGX 0
#define HU_MSGY 0
#define HU_MSGWIDTH 64  // in characters
#define HU_MSGHEIGHT 1  // in lines

#define HU_MSGTIMEOUT (4*TICRATE)


//
// HEADS UP TEXT
//

void HU_Init(void);
void HU_Start(void);
doom_boolean HU_Responder(event_t* ev);
void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);


#endif


#ifndef __I_NET__
#define __I_NET__


// Called by D_DoomMain.

void I_InitNetwork (void);
void I_NetCmd (void);


#endif


#ifndef __I_SYSTEM__
#define __I_SYSTEM__


//#include "d_ticcmd.h"
//#include "d_event.h"


// Called by DoomMain.
void I_Init(void);

// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte* I_ZoneBase(int* size);

// Called by D_DoomLoop,
// returns current time in tics.
int I_GetTime(void);

// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
void I_StartFrame(void);

// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic(void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd(void);

// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit(void);

// Allocates from low memory under dos,
// just mallocs under unix
byte* I_AllocLow(int length);

void I_Tactile(int on, int off, int total);

void I_Error(char* error);


#endif


#ifndef __I_VIDEO__
#define __I_VIDEO__


//#include "doomtype.h"


// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette(byte* palette);

void I_UpdateNoBlit(void);
void I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen(byte* scr);

void I_BeginRead(void);
void I_EndRead(void);


#endif


#ifndef __INFO__
#define __INFO__


// Needed for action function pointer handling.
//#include "d_think.h"


typedef enum
{
    SPR_TROO,
    SPR_SHTG,
    SPR_PUNG,
    SPR_PISG,
    SPR_PISF,
    SPR_SHTF,
    SPR_SHT2,
    SPR_CHGG,
    SPR_CHGF,
    SPR_MISG,
    SPR_MISF,
    SPR_SAWG,
    SPR_PLSG,
    SPR_PLSF,
    SPR_BFGG,
    SPR_BFGF,
    SPR_BLUD,
    SPR_PUFF,
    SPR_BAL1,
    SPR_BAL2,
    SPR_PLSS,
    SPR_PLSE,
    SPR_MISL,
    SPR_BFS1,
    SPR_BFE1,
    SPR_BFE2,
    SPR_TFOG,
    SPR_IFOG,
    SPR_PLAY,
    SPR_POSS,
    SPR_SPOS,
    SPR_VILE,
    SPR_FIRE,
    SPR_FATB,
    SPR_FBXP,
    SPR_SKEL,
    SPR_MANF,
    SPR_FATT,
    SPR_CPOS,
    SPR_SARG,
    SPR_HEAD,
    SPR_BAL7,
    SPR_BOSS,
    SPR_BOS2,
    SPR_SKUL,
    SPR_SPID,
    SPR_BSPI,
    SPR_APLS,
    SPR_APBX,
    SPR_CYBR,
    SPR_PAIN,
    SPR_SSWV,
    SPR_KEEN,
    SPR_BBRN,
    SPR_BOSF,
    SPR_ARM1,
    SPR_ARM2,
    SPR_BAR1,
    SPR_BEXP,
    SPR_FCAN,
    SPR_BON1,
    SPR_BON2,
    SPR_BKEY,
    SPR_RKEY,
    SPR_YKEY,
    SPR_BSKU,
    SPR_RSKU,
    SPR_YSKU,
    SPR_STIM,
    SPR_MEDI,
    SPR_SOUL,
    SPR_PINV,
    SPR_PSTR,
    SPR_PINS,
    SPR_MEGA,
    SPR_SUIT,
    SPR_PMAP,
    SPR_PVIS,
    SPR_CLIP,
    SPR_AMMO,
    SPR_ROCK,
    SPR_BROK,
    SPR_CELL,
    SPR_CELP,
    SPR_SHEL,
    SPR_SBOX,
    SPR_BPAK,
    SPR_BFUG,
    SPR_MGUN,
    SPR_CSAW,
    SPR_LAUN,
    SPR_PLAS,
    SPR_SHOT,
    SPR_SGN2,
    SPR_COLU,
    SPR_SMT2,
    SPR_GOR1,
    SPR_POL2,
    SPR_POL5,
    SPR_POL4,
    SPR_POL3,
    SPR_POL1,
    SPR_POL6,
    SPR_GOR2,
    SPR_GOR3,
    SPR_GOR4,
    SPR_GOR5,
    SPR_SMIT,
    SPR_COL1,
    SPR_COL2,
    SPR_COL3,
    SPR_COL4,
    SPR_CAND,
    SPR_CBRA,
    SPR_COL6,
    SPR_TRE1,
    SPR_TRE2,
    SPR_ELEC,
    SPR_CEYE,
    SPR_FSKU,
    SPR_COL5,
    SPR_TBLU,
    SPR_TGRN,
    SPR_TRED,
    SPR_SMBT,
    SPR_SMGT,
    SPR_SMRT,
    SPR_HDB1,
    SPR_HDB2,
    SPR_HDB3,
    SPR_HDB4,
    SPR_HDB5,
    SPR_HDB6,
    SPR_POB1,
    SPR_POB2,
    SPR_BRS1,
    SPR_TLMP,
    SPR_TLP2,
    NUMSPRITES
} spritenum_t;


typedef enum
{
    S_NULL,
    S_LIGHTDONE,
    S_PUNCH,
    S_PUNCHDOWN,
    S_PUNCHUP,
    S_PUNCH1,
    S_PUNCH2,
    S_PUNCH3,
    S_PUNCH4,
    S_PUNCH5,
    S_PISTOL,
    S_PISTOLDOWN,
    S_PISTOLUP,
    S_PISTOL1,
    S_PISTOL2,
    S_PISTOL3,
    S_PISTOL4,
    S_PISTOLFLASH,
    S_SGUN,
    S_SGUNDOWN,
    S_SGUNUP,
    S_SGUN1,
    S_SGUN2,
    S_SGUN3,
    S_SGUN4,
    S_SGUN5,
    S_SGUN6,
    S_SGUN7,
    S_SGUN8,
    S_SGUN9,
    S_SGUNFLASH1,
    S_SGUNFLASH2,
    S_DSGUN,
    S_DSGUNDOWN,
    S_DSGUNUP,
    S_DSGUN1,
    S_DSGUN2,
    S_DSGUN3,
    S_DSGUN4,
    S_DSGUN5,
    S_DSGUN6,
    S_DSGUN7,
    S_DSGUN8,
    S_DSGUN9,
    S_DSGUN10,
    S_DSNR1,
    S_DSNR2,
    S_DSGUNFLASH1,
    S_DSGUNFLASH2,
    S_CHAIN,
    S_CHAINDOWN,
    S_CHAINUP,
    S_CHAIN1,
    S_CHAIN2,
    S_CHAIN3,
    S_CHAINFLASH1,
    S_CHAINFLASH2,
    S_MISSILE,
    S_MISSILEDOWN,
    S_MISSILEUP,
    S_MISSILE1,
    S_MISSILE2,
    S_MISSILE3,
    S_MISSILEFLASH1,
    S_MISSILEFLASH2,
    S_MISSILEFLASH3,
    S_MISSILEFLASH4,
    S_SAW,
    S_SAWB,
    S_SAWDOWN,
    S_SAWUP,
    S_SAW1,
    S_SAW2,
    S_SAW3,
    S_PLASMA,
    S_PLASMADOWN,
    S_PLASMAUP,
    S_PLASMA1,
    S_PLASMA2,
    S_PLASMAFLASH1,
    S_PLASMAFLASH2,
    S_BFG,
    S_BFGDOWN,
    S_BFGUP,
    S_BFG1,
    S_BFG2,
    S_BFG3,
    S_BFG4,
    S_BFGFLASH1,
    S_BFGFLASH2,
    S_BLOOD1,
    S_BLOOD2,
    S_BLOOD3,
    S_PUFF1,
    S_PUFF2,
    S_PUFF3,
    S_PUFF4,
    S_TBALL1,
    S_TBALL2,
    S_TBALLX1,
    S_TBALLX2,
    S_TBALLX3,
    S_RBALL1,
    S_RBALL2,
    S_RBALLX1,
    S_RBALLX2,
    S_RBALLX3,
    S_PLASBALL,
    S_PLASBALL2,
    S_PLASEXP,
    S_PLASEXP2,
    S_PLASEXP3,
    S_PLASEXP4,
    S_PLASEXP5,
    S_ROCKET,
    S_BFGSHOT,
    S_BFGSHOT2,
    S_BFGLAND,
    S_BFGLAND2,
    S_BFGLAND3,
    S_BFGLAND4,
    S_BFGLAND5,
    S_BFGLAND6,
    S_BFGEXP,
    S_BFGEXP2,
    S_BFGEXP3,
    S_BFGEXP4,
    S_EXPLODE1,
    S_EXPLODE2,
    S_EXPLODE3,
    S_TFOG,
    S_TFOG01,
    S_TFOG02,
    S_TFOG2,
    S_TFOG3,
    S_TFOG4,
    S_TFOG5,
    S_TFOG6,
    S_TFOG7,
    S_TFOG8,
    S_TFOG9,
    S_TFOG10,
    S_IFOG,
    S_IFOG01,
    S_IFOG02,
    S_IFOG2,
    S_IFOG3,
    S_IFOG4,
    S_IFOG5,
    S_PLAY,
    S_PLAY_RUN1,
    S_PLAY_RUN2,
    S_PLAY_RUN3,
    S_PLAY_RUN4,
    S_PLAY_ATK1,
    S_PLAY_ATK2,
    S_PLAY_PAIN,
    S_PLAY_PAIN2,
    S_PLAY_DIE1,
    S_PLAY_DIE2,
    S_PLAY_DIE3,
    S_PLAY_DIE4,
    S_PLAY_DIE5,
    S_PLAY_DIE6,
    S_PLAY_DIE7,
    S_PLAY_XDIE1,
    S_PLAY_XDIE2,
    S_PLAY_XDIE3,
    S_PLAY_XDIE4,
    S_PLAY_XDIE5,
    S_PLAY_XDIE6,
    S_PLAY_XDIE7,
    S_PLAY_XDIE8,
    S_PLAY_XDIE9,
    S_POSS_STND,
    S_POSS_STND2,
    S_POSS_RUN1,
    S_POSS_RUN2,
    S_POSS_RUN3,
    S_POSS_RUN4,
    S_POSS_RUN5,
    S_POSS_RUN6,
    S_POSS_RUN7,
    S_POSS_RUN8,
    S_POSS_ATK1,
    S_POSS_ATK2,
    S_POSS_ATK3,
    S_POSS_PAIN,
    S_POSS_PAIN2,
    S_POSS_DIE1,
    S_POSS_DIE2,
    S_POSS_DIE3,
    S_POSS_DIE4,
    S_POSS_DIE5,
    S_POSS_XDIE1,
    S_POSS_XDIE2,
    S_POSS_XDIE3,
    S_POSS_XDIE4,
    S_POSS_XDIE5,
    S_POSS_XDIE6,
    S_POSS_XDIE7,
    S_POSS_XDIE8,
    S_POSS_XDIE9,
    S_POSS_RAISE1,
    S_POSS_RAISE2,
    S_POSS_RAISE3,
    S_POSS_RAISE4,
    S_SPOS_STND,
    S_SPOS_STND2,
    S_SPOS_RUN1,
    S_SPOS_RUN2,
    S_SPOS_RUN3,
    S_SPOS_RUN4,
    S_SPOS_RUN5,
    S_SPOS_RUN6,
    S_SPOS_RUN7,
    S_SPOS_RUN8,
    S_SPOS_ATK1,
    S_SPOS_ATK2,
    S_SPOS_ATK3,
    S_SPOS_PAIN,
    S_SPOS_PAIN2,
    S_SPOS_DIE1,
    S_SPOS_DIE2,
    S_SPOS_DIE3,
    S_SPOS_DIE4,
    S_SPOS_DIE5,
    S_SPOS_XDIE1,
    S_SPOS_XDIE2,
    S_SPOS_XDIE3,
    S_SPOS_XDIE4,
    S_SPOS_XDIE5,
    S_SPOS_XDIE6,
    S_SPOS_XDIE7,
    S_SPOS_XDIE8,
    S_SPOS_XDIE9,
    S_SPOS_RAISE1,
    S_SPOS_RAISE2,
    S_SPOS_RAISE3,
    S_SPOS_RAISE4,
    S_SPOS_RAISE5,
    S_VILE_STND,
    S_VILE_STND2,
    S_VILE_RUN1,
    S_VILE_RUN2,
    S_VILE_RUN3,
    S_VILE_RUN4,
    S_VILE_RUN5,
    S_VILE_RUN6,
    S_VILE_RUN7,
    S_VILE_RUN8,
    S_VILE_RUN9,
    S_VILE_RUN10,
    S_VILE_RUN11,
    S_VILE_RUN12,
    S_VILE_ATK1,
    S_VILE_ATK2,
    S_VILE_ATK3,
    S_VILE_ATK4,
    S_VILE_ATK5,
    S_VILE_ATK6,
    S_VILE_ATK7,
    S_VILE_ATK8,
    S_VILE_ATK9,
    S_VILE_ATK10,
    S_VILE_ATK11,
    S_VILE_HEAL1,
    S_VILE_HEAL2,
    S_VILE_HEAL3,
    S_VILE_PAIN,
    S_VILE_PAIN2,
    S_VILE_DIE1,
    S_VILE_DIE2,
    S_VILE_DIE3,
    S_VILE_DIE4,
    S_VILE_DIE5,
    S_VILE_DIE6,
    S_VILE_DIE7,
    S_VILE_DIE8,
    S_VILE_DIE9,
    S_VILE_DIE10,
    S_FIRE1,
    S_FIRE2,
    S_FIRE3,
    S_FIRE4,
    S_FIRE5,
    S_FIRE6,
    S_FIRE7,
    S_FIRE8,
    S_FIRE9,
    S_FIRE10,
    S_FIRE11,
    S_FIRE12,
    S_FIRE13,
    S_FIRE14,
    S_FIRE15,
    S_FIRE16,
    S_FIRE17,
    S_FIRE18,
    S_FIRE19,
    S_FIRE20,
    S_FIRE21,
    S_FIRE22,
    S_FIRE23,
    S_FIRE24,
    S_FIRE25,
    S_FIRE26,
    S_FIRE27,
    S_FIRE28,
    S_FIRE29,
    S_FIRE30,
    S_SMOKE1,
    S_SMOKE2,
    S_SMOKE3,
    S_SMOKE4,
    S_SMOKE5,
    S_TRACER,
    S_TRACER2,
    S_TRACEEXP1,
    S_TRACEEXP2,
    S_TRACEEXP3,
    S_SKEL_STND,
    S_SKEL_STND2,
    S_SKEL_RUN1,
    S_SKEL_RUN2,
    S_SKEL_RUN3,
    S_SKEL_RUN4,
    S_SKEL_RUN5,
    S_SKEL_RUN6,
    S_SKEL_RUN7,
    S_SKEL_RUN8,
    S_SKEL_RUN9,
    S_SKEL_RUN10,
    S_SKEL_RUN11,
    S_SKEL_RUN12,
    S_SKEL_FIST1,
    S_SKEL_FIST2,
    S_SKEL_FIST3,
    S_SKEL_FIST4,
    S_SKEL_MISS1,
    S_SKEL_MISS2,
    S_SKEL_MISS3,
    S_SKEL_MISS4,
    S_SKEL_PAIN,
    S_SKEL_PAIN2,
    S_SKEL_DIE1,
    S_SKEL_DIE2,
    S_SKEL_DIE3,
    S_SKEL_DIE4,
    S_SKEL_DIE5,
    S_SKEL_DIE6,
    S_SKEL_RAISE1,
    S_SKEL_RAISE2,
    S_SKEL_RAISE3,
    S_SKEL_RAISE4,
    S_SKEL_RAISE5,
    S_SKEL_RAISE6,
    S_FATSHOT1,
    S_FATSHOT2,
    S_FATSHOTX1,
    S_FATSHOTX2,
    S_FATSHOTX3,
    S_FATT_STND,
    S_FATT_STND2,
    S_FATT_RUN1,
    S_FATT_RUN2,
    S_FATT_RUN3,
    S_FATT_RUN4,
    S_FATT_RUN5,
    S_FATT_RUN6,
    S_FATT_RUN7,
    S_FATT_RUN8,
    S_FATT_RUN9,
    S_FATT_RUN10,
    S_FATT_RUN11,
    S_FATT_RUN12,
    S_FATT_ATK1,
    S_FATT_ATK2,
    S_FATT_ATK3,
    S_FATT_ATK4,
    S_FATT_ATK5,
    S_FATT_ATK6,
    S_FATT_ATK7,
    S_FATT_ATK8,
    S_FATT_ATK9,
    S_FATT_ATK10,
    S_FATT_PAIN,
    S_FATT_PAIN2,
    S_FATT_DIE1,
    S_FATT_DIE2,
    S_FATT_DIE3,
    S_FATT_DIE4,
    S_FATT_DIE5,
    S_FATT_DIE6,
    S_FATT_DIE7,
    S_FATT_DIE8,
    S_FATT_DIE9,
    S_FATT_DIE10,
    S_FATT_RAISE1,
    S_FATT_RAISE2,
    S_FATT_RAISE3,
    S_FATT_RAISE4,
    S_FATT_RAISE5,
    S_FATT_RAISE6,
    S_FATT_RAISE7,
    S_FATT_RAISE8,
    S_CPOS_STND,
    S_CPOS_STND2,
    S_CPOS_RUN1,
    S_CPOS_RUN2,
    S_CPOS_RUN3,
    S_CPOS_RUN4,
    S_CPOS_RUN5,
    S_CPOS_RUN6,
    S_CPOS_RUN7,
    S_CPOS_RUN8,
    S_CPOS_ATK1,
    S_CPOS_ATK2,
    S_CPOS_ATK3,
    S_CPOS_ATK4,
    S_CPOS_PAIN,
    S_CPOS_PAIN2,
    S_CPOS_DIE1,
    S_CPOS_DIE2,
    S_CPOS_DIE3,
    S_CPOS_DIE4,
    S_CPOS_DIE5,
    S_CPOS_DIE6,
    S_CPOS_DIE7,
    S_CPOS_XDIE1,
    S_CPOS_XDIE2,
    S_CPOS_XDIE3,
    S_CPOS_XDIE4,
    S_CPOS_XDIE5,
    S_CPOS_XDIE6,
    S_CPOS_RAISE1,
    S_CPOS_RAISE2,
    S_CPOS_RAISE3,
    S_CPOS_RAISE4,
    S_CPOS_RAISE5,
    S_CPOS_RAISE6,
    S_CPOS_RAISE7,
    S_TROO_STND,
    S_TROO_STND2,
    S_TROO_RUN1,
    S_TROO_RUN2,
    S_TROO_RUN3,
    S_TROO_RUN4,
    S_TROO_RUN5,
    S_TROO_RUN6,
    S_TROO_RUN7,
    S_TROO_RUN8,
    S_TROO_ATK1,
    S_TROO_ATK2,
    S_TROO_ATK3,
    S_TROO_PAIN,
    S_TROO_PAIN2,
    S_TROO_DIE1,
    S_TROO_DIE2,
    S_TROO_DIE3,
    S_TROO_DIE4,
    S_TROO_DIE5,
    S_TROO_XDIE1,
    S_TROO_XDIE2,
    S_TROO_XDIE3,
    S_TROO_XDIE4,
    S_TROO_XDIE5,
    S_TROO_XDIE6,
    S_TROO_XDIE7,
    S_TROO_XDIE8,
    S_TROO_RAISE1,
    S_TROO_RAISE2,
    S_TROO_RAISE3,
    S_TROO_RAISE4,
    S_TROO_RAISE5,
    S_SARG_STND,
    S_SARG_STND2,
    S_SARG_RUN1,
    S_SARG_RUN2,
    S_SARG_RUN3,
    S_SARG_RUN4,
    S_SARG_RUN5,
    S_SARG_RUN6,
    S_SARG_RUN7,
    S_SARG_RUN8,
    S_SARG_ATK1,
    S_SARG_ATK2,
    S_SARG_ATK3,
    S_SARG_PAIN,
    S_SARG_PAIN2,
    S_SARG_DIE1,
    S_SARG_DIE2,
    S_SARG_DIE3,
    S_SARG_DIE4,
    S_SARG_DIE5,
    S_SARG_DIE6,
    S_SARG_RAISE1,
    S_SARG_RAISE2,
    S_SARG_RAISE3,
    S_SARG_RAISE4,
    S_SARG_RAISE5,
    S_SARG_RAISE6,
    S_HEAD_STND,
    S_HEAD_RUN1,
    S_HEAD_ATK1,
    S_HEAD_ATK2,
    S_HEAD_ATK3,
    S_HEAD_PAIN,
    S_HEAD_PAIN2,
    S_HEAD_PAIN3,
    S_HEAD_DIE1,
    S_HEAD_DIE2,
    S_HEAD_DIE3,
    S_HEAD_DIE4,
    S_HEAD_DIE5,
    S_HEAD_DIE6,
    S_HEAD_RAISE1,
    S_HEAD_RAISE2,
    S_HEAD_RAISE3,
    S_HEAD_RAISE4,
    S_HEAD_RAISE5,
    S_HEAD_RAISE6,
    S_BRBALL1,
    S_BRBALL2,
    S_BRBALLX1,
    S_BRBALLX2,
    S_BRBALLX3,
    S_BOSS_STND,
    S_BOSS_STND2,
    S_BOSS_RUN1,
    S_BOSS_RUN2,
    S_BOSS_RUN3,
    S_BOSS_RUN4,
    S_BOSS_RUN5,
    S_BOSS_RUN6,
    S_BOSS_RUN7,
    S_BOSS_RUN8,
    S_BOSS_ATK1,
    S_BOSS_ATK2,
    S_BOSS_ATK3,
    S_BOSS_PAIN,
    S_BOSS_PAIN2,
    S_BOSS_DIE1,
    S_BOSS_DIE2,
    S_BOSS_DIE3,
    S_BOSS_DIE4,
    S_BOSS_DIE5,
    S_BOSS_DIE6,
    S_BOSS_DIE7,
    S_BOSS_RAISE1,
    S_BOSS_RAISE2,
    S_BOSS_RAISE3,
    S_BOSS_RAISE4,
    S_BOSS_RAISE5,
    S_BOSS_RAISE6,
    S_BOSS_RAISE7,
    S_BOS2_STND,
    S_BOS2_STND2,
    S_BOS2_RUN1,
    S_BOS2_RUN2,
    S_BOS2_RUN3,
    S_BOS2_RUN4,
    S_BOS2_RUN5,
    S_BOS2_RUN6,
    S_BOS2_RUN7,
    S_BOS2_RUN8,
    S_BOS2_ATK1,
    S_BOS2_ATK2,
    S_BOS2_ATK3,
    S_BOS2_PAIN,
    S_BOS2_PAIN2,
    S_BOS2_DIE1,
    S_BOS2_DIE2,
    S_BOS2_DIE3,
    S_BOS2_DIE4,
    S_BOS2_DIE5,
    S_BOS2_DIE6,
    S_BOS2_DIE7,
    S_BOS2_RAISE1,
    S_BOS2_RAISE2,
    S_BOS2_RAISE3,
    S_BOS2_RAISE4,
    S_BOS2_RAISE5,
    S_BOS2_RAISE6,
    S_BOS2_RAISE7,
    S_SKULL_STND,
    S_SKULL_STND2,
    S_SKULL_RUN1,
    S_SKULL_RUN2,
    S_SKULL_ATK1,
    S_SKULL_ATK2,
    S_SKULL_ATK3,
    S_SKULL_ATK4,
    S_SKULL_PAIN,
    S_SKULL_PAIN2,
    S_SKULL_DIE1,
    S_SKULL_DIE2,
    S_SKULL_DIE3,
    S_SKULL_DIE4,
    S_SKULL_DIE5,
    S_SKULL_DIE6,
    S_SPID_STND,
    S_SPID_STND2,
    S_SPID_RUN1,
    S_SPID_RUN2,
    S_SPID_RUN3,
    S_SPID_RUN4,
    S_SPID_RUN5,
    S_SPID_RUN6,
    S_SPID_RUN7,
    S_SPID_RUN8,
    S_SPID_RUN9,
    S_SPID_RUN10,
    S_SPID_RUN11,
    S_SPID_RUN12,
    S_SPID_ATK1,
    S_SPID_ATK2,
    S_SPID_ATK3,
    S_SPID_ATK4,
    S_SPID_PAIN,
    S_SPID_PAIN2,
    S_SPID_DIE1,
    S_SPID_DIE2,
    S_SPID_DIE3,
    S_SPID_DIE4,
    S_SPID_DIE5,
    S_SPID_DIE6,
    S_SPID_DIE7,
    S_SPID_DIE8,
    S_SPID_DIE9,
    S_SPID_DIE10,
    S_SPID_DIE11,
    S_BSPI_STND,
    S_BSPI_STND2,
    S_BSPI_SIGHT,
    S_BSPI_RUN1,
    S_BSPI_RUN2,
    S_BSPI_RUN3,
    S_BSPI_RUN4,
    S_BSPI_RUN5,
    S_BSPI_RUN6,
    S_BSPI_RUN7,
    S_BSPI_RUN8,
    S_BSPI_RUN9,
    S_BSPI_RUN10,
    S_BSPI_RUN11,
    S_BSPI_RUN12,
    S_BSPI_ATK1,
    S_BSPI_ATK2,
    S_BSPI_ATK3,
    S_BSPI_ATK4,
    S_BSPI_PAIN,
    S_BSPI_PAIN2,
    S_BSPI_DIE1,
    S_BSPI_DIE2,
    S_BSPI_DIE3,
    S_BSPI_DIE4,
    S_BSPI_DIE5,
    S_BSPI_DIE6,
    S_BSPI_DIE7,
    S_BSPI_RAISE1,
    S_BSPI_RAISE2,
    S_BSPI_RAISE3,
    S_BSPI_RAISE4,
    S_BSPI_RAISE5,
    S_BSPI_RAISE6,
    S_BSPI_RAISE7,
    S_ARACH_PLAZ,
    S_ARACH_PLAZ2,
    S_ARACH_PLEX,
    S_ARACH_PLEX2,
    S_ARACH_PLEX3,
    S_ARACH_PLEX4,
    S_ARACH_PLEX5,
    S_CYBER_STND,
    S_CYBER_STND2,
    S_CYBER_RUN1,
    S_CYBER_RUN2,
    S_CYBER_RUN3,
    S_CYBER_RUN4,
    S_CYBER_RUN5,
    S_CYBER_RUN6,
    S_CYBER_RUN7,
    S_CYBER_RUN8,
    S_CYBER_ATK1,
    S_CYBER_ATK2,
    S_CYBER_ATK3,
    S_CYBER_ATK4,
    S_CYBER_ATK5,
    S_CYBER_ATK6,
    S_CYBER_PAIN,
    S_CYBER_DIE1,
    S_CYBER_DIE2,
    S_CYBER_DIE3,
    S_CYBER_DIE4,
    S_CYBER_DIE5,
    S_CYBER_DIE6,
    S_CYBER_DIE7,
    S_CYBER_DIE8,
    S_CYBER_DIE9,
    S_CYBER_DIE10,
    S_PAIN_STND,
    S_PAIN_RUN1,
    S_PAIN_RUN2,
    S_PAIN_RUN3,
    S_PAIN_RUN4,
    S_PAIN_RUN5,
    S_PAIN_RUN6,
    S_PAIN_ATK1,
    S_PAIN_ATK2,
    S_PAIN_ATK3,
    S_PAIN_ATK4,
    S_PAIN_PAIN,
    S_PAIN_PAIN2,
    S_PAIN_DIE1,
    S_PAIN_DIE2,
    S_PAIN_DIE3,
    S_PAIN_DIE4,
    S_PAIN_DIE5,
    S_PAIN_DIE6,
    S_PAIN_RAISE1,
    S_PAIN_RAISE2,
    S_PAIN_RAISE3,
    S_PAIN_RAISE4,
    S_PAIN_RAISE5,
    S_PAIN_RAISE6,
    S_SSWV_STND,
    S_SSWV_STND2,
    S_SSWV_RUN1,
    S_SSWV_RUN2,
    S_SSWV_RUN3,
    S_SSWV_RUN4,
    S_SSWV_RUN5,
    S_SSWV_RUN6,
    S_SSWV_RUN7,
    S_SSWV_RUN8,
    S_SSWV_ATK1,
    S_SSWV_ATK2,
    S_SSWV_ATK3,
    S_SSWV_ATK4,
    S_SSWV_ATK5,
    S_SSWV_ATK6,
    S_SSWV_PAIN,
    S_SSWV_PAIN2,
    S_SSWV_DIE1,
    S_SSWV_DIE2,
    S_SSWV_DIE3,
    S_SSWV_DIE4,
    S_SSWV_DIE5,
    S_SSWV_XDIE1,
    S_SSWV_XDIE2,
    S_SSWV_XDIE3,
    S_SSWV_XDIE4,
    S_SSWV_XDIE5,
    S_SSWV_XDIE6,
    S_SSWV_XDIE7,
    S_SSWV_XDIE8,
    S_SSWV_XDIE9,
    S_SSWV_RAISE1,
    S_SSWV_RAISE2,
    S_SSWV_RAISE3,
    S_SSWV_RAISE4,
    S_SSWV_RAISE5,
    S_KEENSTND,
    S_COMMKEEN,
    S_COMMKEEN2,
    S_COMMKEEN3,
    S_COMMKEEN4,
    S_COMMKEEN5,
    S_COMMKEEN6,
    S_COMMKEEN7,
    S_COMMKEEN8,
    S_COMMKEEN9,
    S_COMMKEEN10,
    S_COMMKEEN11,
    S_COMMKEEN12,
    S_KEENPAIN,
    S_KEENPAIN2,
    S_BRAIN,
    S_BRAIN_PAIN,
    S_BRAIN_DIE1,
    S_BRAIN_DIE2,
    S_BRAIN_DIE3,
    S_BRAIN_DIE4,
    S_BRAINEYE,
    S_BRAINEYESEE,
    S_BRAINEYE1,
    S_SPAWN1,
    S_SPAWN2,
    S_SPAWN3,
    S_SPAWN4,
    S_SPAWNFIRE1,
    S_SPAWNFIRE2,
    S_SPAWNFIRE3,
    S_SPAWNFIRE4,
    S_SPAWNFIRE5,
    S_SPAWNFIRE6,
    S_SPAWNFIRE7,
    S_SPAWNFIRE8,
    S_BRAINEXPLODE1,
    S_BRAINEXPLODE2,
    S_BRAINEXPLODE3,
    S_ARM1,
    S_ARM1A,
    S_ARM2,
    S_ARM2A,
    S_BAR1,
    S_BAR2,
    S_BEXP,
    S_BEXP2,
    S_BEXP3,
    S_BEXP4,
    S_BEXP5,
    S_BBAR1,
    S_BBAR2,
    S_BBAR3,
    S_BON1,
    S_BON1A,
    S_BON1B,
    S_BON1C,
    S_BON1D,
    S_BON1E,
    S_BON2,
    S_BON2A,
    S_BON2B,
    S_BON2C,
    S_BON2D,
    S_BON2E,
    S_BKEY,
    S_BKEY2,
    S_RKEY,
    S_RKEY2,
    S_YKEY,
    S_YKEY2,
    S_BSKULL,
    S_BSKULL2,
    S_RSKULL,
    S_RSKULL2,
    S_YSKULL,
    S_YSKULL2,
    S_STIM,
    S_MEDI,
    S_SOUL,
    S_SOUL2,
    S_SOUL3,
    S_SOUL4,
    S_SOUL5,
    S_SOUL6,
    S_PINV,
    S_PINV2,
    S_PINV3,
    S_PINV4,
    S_PSTR,
    S_PINS,
    S_PINS2,
    S_PINS3,
    S_PINS4,
    S_MEGA,
    S_MEGA2,
    S_MEGA3,
    S_MEGA4,
    S_SUIT,
    S_PMAP,
    S_PMAP2,
    S_PMAP3,
    S_PMAP4,
    S_PMAP5,
    S_PMAP6,
    S_PVIS,
    S_PVIS2,
    S_CLIP,
    S_AMMO,
    S_ROCK,
    S_BROK,
    S_CELL,
    S_CELP,
    S_SHEL,
    S_SBOX,
    S_BPAK,
    S_BFUG,
    S_MGUN,
    S_CSAW,
    S_LAUN,
    S_PLAS,
    S_SHOT,
    S_SHOT2,
    S_COLU,
    S_STALAG,
    S_BLOODYTWITCH,
    S_BLOODYTWITCH2,
    S_BLOODYTWITCH3,
    S_BLOODYTWITCH4,
    S_DEADTORSO,
    S_DEADBOTTOM,
    S_HEADSONSTICK,
    S_GIBS,
    S_HEADONASTICK,
    S_HEADCANDLES,
    S_HEADCANDLES2,
    S_DEADSTICK,
    S_LIVESTICK,
    S_LIVESTICK2,
    S_MEAT2,
    S_MEAT3,
    S_MEAT4,
    S_MEAT5,
    S_STALAGTITE,
    S_TALLGRNCOL,
    S_SHRTGRNCOL,
    S_TALLREDCOL,
    S_SHRTREDCOL,
    S_CANDLESTIK,
    S_CANDELABRA,
    S_SKULLCOL,
    S_TORCHTREE,
    S_BIGTREE,
    S_TECHPILLAR,
    S_EVILEYE,
    S_EVILEYE2,
    S_EVILEYE3,
    S_EVILEYE4,
    S_FLOATSKULL,
    S_FLOATSKULL2,
    S_FLOATSKULL3,
    S_HEARTCOL,
    S_HEARTCOL2,
    S_BLUETORCH,
    S_BLUETORCH2,
    S_BLUETORCH3,
    S_BLUETORCH4,
    S_GREENTORCH,
    S_GREENTORCH2,
    S_GREENTORCH3,
    S_GREENTORCH4,
    S_REDTORCH,
    S_REDTORCH2,
    S_REDTORCH3,
    S_REDTORCH4,
    S_BTORCHSHRT,
    S_BTORCHSHRT2,
    S_BTORCHSHRT3,
    S_BTORCHSHRT4,
    S_GTORCHSHRT,
    S_GTORCHSHRT2,
    S_GTORCHSHRT3,
    S_GTORCHSHRT4,
    S_RTORCHSHRT,
    S_RTORCHSHRT2,
    S_RTORCHSHRT3,
    S_RTORCHSHRT4,
    S_HANGNOGUTS,
    S_HANGBNOBRAIN,
    S_HANGTLOOKDN,
    S_HANGTSKULL,
    S_HANGTLOOKUP,
    S_HANGTNOBRAIN,
    S_COLONGIBS,
    S_SMALLPOOL,
    S_BRAINSTEM,
    S_TECHLAMP,
    S_TECHLAMP2,
    S_TECHLAMP3,
    S_TECHLAMP4,
    S_TECH2LAMP,
    S_TECH2LAMP2,
    S_TECH2LAMP3,
    S_TECH2LAMP4,
    NUMSTATES
} statenum_t;


typedef struct
{
    spritenum_t sprite;
    long frame;
    long tics;
    actionf_t action;
    statenum_t nextstate;
    long misc1, misc2;
} state_t;


extern state_t states[NUMSTATES];
extern char* sprnames[NUMSPRITES + 1];


typedef enum
{
    MT_PLAYER,
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_VILE,
    MT_FIRE,
    MT_UNDEAD,
    MT_TRACER,
    MT_SMOKE,
    MT_FATSO,
    MT_FATSHOT,
    MT_CHAINGUY,
    MT_TROOP,
    MT_SERGEANT,
    MT_SHADOWS,
    MT_HEAD,
    MT_BRUISER,
    MT_BRUISERSHOT,
    MT_KNIGHT,
    MT_SKULL,
    MT_SPIDER,
    MT_BABY,
    MT_CYBORG,
    MT_PAIN,
    MT_WOLFSS,
    MT_KEEN,
    MT_BOSSBRAIN,
    MT_BOSSSPIT,
    MT_BOSSTARGET,
    MT_SPAWNSHOT,
    MT_SPAWNFIRE,
    MT_BARREL,
    MT_TROOPSHOT,
    MT_HEADSHOT,
    MT_ROCKET,
    MT_PLASMA,
    MT_BFG,
    MT_ARACHPLAZ,
    MT_PUFF,
    MT_BLOOD,
    MT_TFOG,
    MT_IFOG,
    MT_TELEPORTMAN,
    MT_EXTRABFG,
    MT_MISC0,
    MT_MISC1,
    MT_MISC2,
    MT_MISC3,
    MT_MISC4,
    MT_MISC5,
    MT_MISC6,
    MT_MISC7,
    MT_MISC8,
    MT_MISC9,
    MT_MISC10,
    MT_MISC11,
    MT_MISC12,
    MT_INV,
    MT_MISC13,
    MT_INS,
    MT_MISC14,
    MT_MISC15,
    MT_MISC16,
    MT_MEGA,
    MT_CLIP,
    MT_MISC17,
    MT_MISC18,
    MT_MISC19,
    MT_MISC20,
    MT_MISC21,
    MT_MISC22,
    MT_MISC23,
    MT_MISC24,
    MT_MISC25,
    MT_CHAINGUN,
    MT_MISC26,
    MT_MISC27,
    MT_MISC28,
    MT_SHOTGUN,
    MT_SUPERSHOTGUN,
    MT_MISC29,
    MT_MISC30,
    MT_MISC31,
    MT_MISC32,
    MT_MISC33,
    MT_MISC34,
    MT_MISC35,
    MT_MISC36,
    MT_MISC37,
    MT_MISC38,
    MT_MISC39,
    MT_MISC40,
    MT_MISC41,
    MT_MISC42,
    MT_MISC43,
    MT_MISC44,
    MT_MISC45,
    MT_MISC46,
    MT_MISC47,
    MT_MISC48,
    MT_MISC49,
    MT_MISC50,
    MT_MISC51,
    MT_MISC52,
    MT_MISC53,
    MT_MISC54,
    MT_MISC55,
    MT_MISC56,
    MT_MISC57,
    MT_MISC58,
    MT_MISC59,
    MT_MISC60,
    MT_MISC61,
    MT_MISC62,
    MT_MISC63,
    MT_MISC64,
    MT_MISC65,
    MT_MISC66,
    MT_MISC67,
    MT_MISC68,
    MT_MISC69,
    MT_MISC70,
    MT_MISC71,
    MT_MISC72,
    MT_MISC73,
    MT_MISC74,
    MT_MISC75,
    MT_MISC76,
    MT_MISC77,
    MT_MISC78,
    MT_MISC79,
    MT_MISC80,
    MT_MISC81,
    MT_MISC82,
    MT_MISC83,
    MT_MISC84,
    MT_MISC85,
    MT_MISC86,
    NUMMOBJTYPES
} mobjtype_t;


typedef struct
{
    int        doomednum;
    int        spawnstate;
    int        spawnhealth;
    int        seestate;
    int        seesound;
    int        reactiontime;
    int        attacksound;
    int        painstate;
    int        painchance;
    int        painsound;
    int        meleestate;
    int        missilestate;
    int        deathstate;
    int        xdeathstate;
    int        deathsound;
    int        speed;
    int        radius;
    int        height;
    int        mass;
    int        damage;
    int        activesound;
    int        flags;
    int        raisestate;
} mobjinfo_t;


extern mobjinfo_t mobjinfo[NUMMOBJTYPES];


#endif


#ifndef __M_ARGV__
#define __M_ARGV__


//
// MISC
//
extern int myargc;
extern char** myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm(char* check);


#endif


#ifndef __M_CHEAT__
#define __M_CHEAT__


//
// CHEAT SEQUENCE PACKAGE
//

#define SCRAMBLE(a) \
((((a)&1)<<7) + (((a)&2)<<5) + ((a)&4) + (((a)&8)<<1) \
 + (((a)&16)>>1) + ((a)&32) + (((a)&64)>>5) + (((a)&128)>>7))


typedef struct
{
    unsigned char* sequence;
    unsigned char* p;
} cheatseq_t;


int cht_CheckCheat(cheatseq_t* cht, char key);
void cht_GetParam(cheatseq_t* cht, char* buffer);


#endif


#ifndef __M_FIXED__
#define __M_FIXED__


//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)


typedef int fixed_t;


fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);
fixed_t FixedDiv2(fixed_t a, fixed_t b);


#endif


#ifndef __M_BBOX__
#define __M_BBOX__


//#include "doomtype.h"
//#include "m_fixed.h"


// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
}; // bbox coordinates


// Bounding box functions.
void M_ClearBox(fixed_t* box);
void M_AddToBox(fixed_t* box, fixed_t x, fixed_t y);


#endif


#ifndef __M_MENU__
#define __M_MENU__


//#include "d_event.h"


//
// MENUS
//
 
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
doom_boolean M_Responder(event_t* ev);

// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker(void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer(void);

// Called by D_DoomMain,
// loads the config file.
void M_Init(void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel(void);


#endif


#ifndef __M_MISC__
#define __M_MISC__


//#include "doomtype.h"


//
// MISC
//
typedef struct
{
    char* name;
    int* location;
    int defaultvalue;
    int scantranslate; // PC scan code hack
    int untranslated; // lousy hack
    char** text_location; // [pd] int* location was used to store text pointer. Can't change to intptr_t unless we change all settings type
    char* default_text_value; // [pd] So we don't change defaultvalue behavior for int to intptr_t
} default_t;


doom_boolean M_WriteFile(char const* name, void* source, int length);
int M_ReadFile(char const* name, byte** buffer);
void M_ScreenShot(void);
void M_LoadDefaults(void);
void M_SaveDefaults(void);
int M_DrawText(int x, int y, doom_boolean direct, char* string);


#endif


#ifndef __M_RANDOM__
#define __M_RANDOM__


//#include "doomtype.h"


// Returns a number from 0 to 255,
// from a lookup table.
int M_Random(void);

// As M_Random, but used only by the play simulation.
int P_Random(void);

// Fix randoms for demos.
void M_ClearRandom(void);


#endif


#ifndef __M_SWAP__
#define __M_SWAP__


// Endianess handling.
// WAD files are stored little endian.
#ifdef __BIG_ENDIAN__
unsigned short SwapSHORT(unsigned short);
unsigned long SwapLONG(unsigned long);
#define SHORT(x) ((short)SwapSHORT((unsigned short) (x)))
#define LONG(x) ((long)SwapLONG((unsigned long) (x)))
#else
#define SHORT(x) (x)
#define LONG(x) (x)
#endif


#endif


#ifndef __P_SAVEG__
#define __P_SAVEG__


//#include "doomtype.h"


// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);

extern byte* save_p;


#endif


#ifndef __P_SETUP__
#define __P_SETUP__


// NOT called by W_Ticker. Fixme.
void P_SetupLevel(int episode, int map, int playermask, skill_t skill);

// Called by startup code.
void P_Init(void);


#endif


#ifndef __P_TICK__
#define __P_TICK__


// Called by C_Ticker,
// can call G_PlayerExited.
// Carries out all thinking of monsters and players.
void P_Ticker(void);


#endif


#ifndef __R_SKY__
#define __R_SKY__

// SKY, store the number for name.
#define SKYFLATNAME "F_SKY1"

// The sky map is 256*128*4 maps.
#define ANGLETOSKYSHIFT 22

extern int skytexture;
extern int skytexturemid;

// Called whenever the view size changes.
void R_InitSkyMap(void);

#endif


#ifndef __S_SOUND__
#define __S_SOUND__


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
// allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(void* origin, int sound_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(void* origin, int sound_id, int volume);

// Stop sound for thing at <origin>
void S_StopSound(void* origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
// and set whether looping
void S_ChangeMusic(int music_id, int looping);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);

//
// Updates music & sounds
//
void S_UpdateSounds(void* listener);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);


#endif


#ifndef __SOUNDS__
#define __SOUNDS__


//
// SoundFX struct.
//
typedef struct sfxinfo_struct sfxinfo_t;

struct sfxinfo_struct
{
    // up to 6-character name
    char* name;

    // Sfx singularity (only one at a time)
    int singularity;

    // Sfx priority
    int priority;

    // referenced sound if a link
    sfxinfo_t* link;

    // pitch if a link
    int pitch;

    // volume if a link
    int volume;

    // sound data
    void* data;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int usefulness;

    // lump number of sfx
    int lumpnum;
};


//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    char* name;

    // lump number of music
    int lumpnum;

    // music data
    void* data;

    // music handle once registered
    int handle;
} musicinfo_t;


// the complete set of sound effects
extern sfxinfo_t S_sfx[];

// the complete set of music
extern musicinfo_t S_music[];

//
// Identifiers for all music in game.
//
typedef enum
{
    mus_None,
    mus_e1m1,
    mus_e1m2,
    mus_e1m3,
    mus_e1m4,
    mus_e1m5,
    mus_e1m6,
    mus_e1m7,
    mus_e1m8,
    mus_e1m9,
    mus_e2m1,
    mus_e2m2,
    mus_e2m3,
    mus_e2m4,
    mus_e2m5,
    mus_e2m6,
    mus_e2m7,
    mus_e2m8,
    mus_e2m9,
    mus_e3m1,
    mus_e3m2,
    mus_e3m3,
    mus_e3m4,
    mus_e3m5,
    mus_e3m6,
    mus_e3m7,
    mus_e3m8,
    mus_e3m9,
    mus_inter,
    mus_intro,
    mus_bunny,
    mus_victor,
    mus_introa,
    mus_runnin,
    mus_stalks,
    mus_countd,
    mus_betwee,
    mus_doom,
    mus_the_da,
    mus_shawn,
    mus_ddtblu,
    mus_in_cit,
    mus_dead,
    mus_stlks2,
    mus_theda2,
    mus_doom2,
    mus_ddtbl2,
    mus_runni2,
    mus_dead2,
    mus_stlks3,
    mus_romero,
    mus_shawn2,
    mus_messag,
    mus_count2,
    mus_ddtbl3,
    mus_ampie,
    mus_theda3,
    mus_adrian,
    mus_messg2,
    mus_romer2,
    mus_tense,
    mus_shawn3,
    mus_openin,
    mus_evil,
    mus_ultima,
    mus_read_m,
    mus_dm2ttl,
    mus_dm2int,
    NUMMUSIC
} musicenum_t;


//
// Identifiers for all sfx in game.
//
typedef enum
{
    sfx_None,
    sfx_pistol,
    sfx_shotgn,
    sfx_sgcock,
    sfx_dshtgn,
    sfx_dbopn,
    sfx_dbcls,
    sfx_dbload,
    sfx_plasma,
    sfx_bfg,
    sfx_sawup,
    sfx_sawidl,
    sfx_sawful,
    sfx_sawhit,
    sfx_rlaunc,
    sfx_rxplod,
    sfx_firsht,
    sfx_firxpl,
    sfx_pstart,
    sfx_pstop,
    sfx_doropn,
    sfx_dorcls,
    sfx_stnmov,
    sfx_swtchn,
    sfx_swtchx,
    sfx_plpain,
    sfx_dmpain,
    sfx_popain,
    sfx_vipain,
    sfx_mnpain,
    sfx_pepain,
    sfx_slop,
    sfx_itemup,
    sfx_wpnup,
    sfx_oof,
    sfx_telept,
    sfx_posit1,
    sfx_posit2,
    sfx_posit3,
    sfx_bgsit1,
    sfx_bgsit2,
    sfx_sgtsit,
    sfx_cacsit,
    sfx_brssit,
    sfx_cybsit,
    sfx_spisit,
    sfx_bspsit,
    sfx_kntsit,
    sfx_vilsit,
    sfx_mansit,
    sfx_pesit,
    sfx_sklatk,
    sfx_sgtatk,
    sfx_skepch,
    sfx_vilatk,
    sfx_claw,
    sfx_skeswg,
    sfx_pldeth,
    sfx_pdiehi,
    sfx_podth1,
    sfx_podth2,
    sfx_podth3,
    sfx_bgdth1,
    sfx_bgdth2,
    sfx_sgtdth,
    sfx_cacdth,
    sfx_skldth,
    sfx_brsdth,
    sfx_cybdth,
    sfx_spidth,
    sfx_bspdth,
    sfx_vildth,
    sfx_kntdth,
    sfx_pedth,
    sfx_skedth,
    sfx_posact,
    sfx_bgact,
    sfx_dmact,
    sfx_bspact,
    sfx_bspwlk,
    sfx_vilact,
    sfx_noway,
    sfx_barexp,
    sfx_punch,
    sfx_hoof,
    sfx_metal,
    sfx_chgun,
    sfx_tink,
    sfx_bdopn,
    sfx_bdcls,
    sfx_itmbk,
    sfx_flame,
    sfx_flamst,
    sfx_getpow,
    sfx_bospit,
    sfx_boscub,
    sfx_bossit,
    sfx_bospn,
    sfx_bosdth,
    sfx_manatk,
    sfx_mandth,
    sfx_sssit,
    sfx_ssdth,
    sfx_keenpn,
    sfx_keendt,
    sfx_skeact,
    sfx_skesit,
    sfx_skeatk,
    sfx_radio,
    NUMSFX
} sfxenum_t;

#endif


#ifndef __STSTUFF_H__
#define __STSTUFF_H__

//#include "doomtype.h"
//#include "d_event.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT (32 * SCREEN_MUL)
#define ST_WIDTH SCREENWIDTH
#define ST_Y (SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// Called by main loop.
doom_boolean ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(doom_boolean fullscreen, doom_boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);


// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
} st_chatstateenum_t;


doom_boolean ST_Responder(event_t* ev);

#endif


#ifndef __TABLES__
#define __TABLES__


#ifdef LINUX

#else
#define PI 3.141592657
#endif

//#include "m_fixed.h"

#define FINEANGLES 8192
#define FINEMASK (FINEANGLES-1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT 19                

// Effective size is 10240.
extern fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 pahse shift.
extern fixed_t* finecosine;


// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES / 2];

// Binary Angle Measument, BAM.
#define ANG45 0x20000000
#define ANG90 0x40000000
#define ANG180 0x80000000
#define ANG270 0xc0000000

#define SLOPERANGE 2048
#define SLOPEBITS 11
#define DBITS (FRACBITS - SLOPEBITS)

typedef unsigned angle_t;

// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern angle_t                tantoangle[SLOPERANGE + 1];


// Utility function,
//  called by R_PointToAngle.
int SlopeDiv(unsigned num, unsigned den);


#endif


#ifndef __P_MOBJ__
#define __P_MOBJ__


// Basics.
//#include "tables.h"
//#include "m_fixed.h"

// We need the thinker_t stuff.
//#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
//#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
//#include "info.h"


//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.  
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL = 1,
    // Blocks.
    MF_SOLID = 2,
    // Can be hit.
    MF_SHOOTABLE = 4,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR = 8,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP = 16,

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH = 32,
    // Will try to attack right back.
    MF_JUSTHIT = 64,
    // Will take at least one step before attacking.
    MF_JUSTATTACKED = 128,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING = 256,
    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY = 512,

    // Movement flags.
    // This allows jumps from high places.
    MF_DROPOFF = 0x400,
    // For players, will pick up items.
    MF_PICKUP = 0x800,
    // Player cheat. ???
    MF_NOCLIP = 0x1000,
    // Player: keep info about sliding along walls.
    MF_SLIDE = 0x2000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT = 0x4000,
    // Don't cross lines
    //   ??? or look at heights on teleport.
    MF_TELEPORT = 0x8000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE = 0x10000,
    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED = 0x20000,
    // Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    MF_SHADOW = 0x40000,
    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD = 0x80000,
    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE = 0x100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT = 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL = 0x400000,

    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM = 0x800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY = 0x1000000,

    // Don't spawn this object
    //  in death match mode (e.g. key cards).
    MF_NOTDMATCH = 0x2000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x4 0x8 or 0xc,
    //  use a translation table for player colormaps
    MF_TRANSLATION = 0xc000000,
    // Hmm ???.
    MF_TRANSSHIFT = 26
} mobjflag_t;


// Map Object definition.
typedef struct mobj_s
{
    // List: thinker links.
    thinker_t thinker;

    // Info for drawing: position.
    fixed_t x;
    fixed_t y;
    fixed_t z;

    // More list: links in sector (if needed)
    struct mobj_s* snext;
    struct mobj_s* sprev;

    //More drawing info: to determine current sprite.
    angle_t angle;        // orientation
    spritenum_t sprite;        // used to find patch_t and flip value
    int frame;        // might be ORed with FF_FULLBRIGHT

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s* bnext;
    struct mobj_s* bprev;

    struct subsector_s* subsector;

    // The closest interval over all contacted Sectors.
    fixed_t floorz;
    fixed_t ceilingz;

    // For movement checking.
    fixed_t radius;
    fixed_t height;

    // Momentums, used to update position.
    fixed_t momx;
    fixed_t momy;
    fixed_t momz;

    // If == validcount, already checked.
    int validcount;

    mobjtype_t type;
    mobjinfo_t* info; // &mobjinfo[mobj->type]

    int tics; // state tic counter
    state_t* state;
    int flags;
    int health;

    // Movement direction, movement generation (zig-zagging).
    int movedir; // 0-7
    int movecount; // when 0, select a new dir

    // Thing being chased/attacked (or 0),
    // also the originator for missiles.
    struct mobj_s* target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int reactiontime;

    // If >0, the target will be chased
    // no matter what (even if shot)
    int threshold;

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s* player;

    // Player number last looked for.
    int lastlook;

    // For nightmare respawn.
    mapthing_t spawnpoint;

    // Thing being chased/attacked for tracers.
    struct mobj_s* tracer;
} mobj_t;


#endif


#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
//#include "m_fixed.h"
//#include "tables.h"

// Needs to include the precompiled
// sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
//#include "info.h"

//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//
#define FF_FULLBRIGHT 0x8000 // flag in thing->frame
#define FF_FRAMEMASK 0x7fff


//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES
} psprnum_t;


typedef struct
{
    state_t* state;        // a 0 state means not active
    int tics;
    fixed_t sx;
    fixed_t sy;
} pspdef_t;


#endif


#ifndef __D_PLAYER__
#define __D_PLAYER__


// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
//#include "d_items.h"
//#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
//#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
//#include "d_ticcmd.h"


//
// Player states.
//
typedef enum
{
    // Playing or camping.
    PST_LIVE,
    // Dead on the ground, view follows killer.
    PST_DEAD,
    // Ready to restart/respawn???
    PST_REBORN
} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
    // No clipping, walk through barriers.
    CF_NOCLIP = 1,
    // No damage, no health loss.
    CF_GODMODE = 2,
    // Not really a cheat, just a debug aid.
    CF_NOMOMENTUM = 4
} cheat_t;


//
// Extended player object info: player_t
//
typedef struct player_s
{
    mobj_t* mo;
    playerstate_t playerstate;
    ticcmd_t cmd;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
    fixed_t viewz;
    // Base height above floor for viewz.
    fixed_t viewheight;
    // Bob/squat speed.
    fixed_t deltaviewheight;
    // bounded/scaled total momentum.
    fixed_t bob;

    // This is only used between levels,
    // mo->health is used during levels.
    int health;
    int armorpoints;
    // Armor type is 0-2.
    int armortype;

    // Power ups. invinc and invis are tic counters.
    int powers[NUMPOWERS];
    doom_boolean cards[NUMCARDS];
    doom_boolean backpack;

    // Frags, kills of other players.
    int frags[MAXPLAYERS];
    weapontype_t readyweapon;

    // Is wp_nochange if not changing.
    weapontype_t pendingweapon;

    doom_boolean weaponowned[NUMWEAPONS];
    int ammo[NUMAMMO];
    int maxammo[NUMAMMO];

    // True if button down last tic.
    int attackdown;
    int usedown;

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int cheats;

    // Refired shots are less accurate.
    int refire;

    // For intermission stats.
    int killcount;
    int itemcount;
    int secretcount;

    // Hint messages.
    char* message;

    // For screen flashing (red or bright).
    int damagecount;
    int bonuscount;

    // Who did damage (0 for floors/ceilings).
    mobj_t* attacker;

    // So gun flashes light up areas.
    int extralight;

    // Current PLAYPAL, ???
    //  can be set to REDCOLORMAP for pain, etc.
    int fixedcolormap;

    // Player skin colorshift,
    //  0-3 for which color to draw player.
    int colormap;

    // Overlay view sprites (gun, etc).
    pspdef_t psprites[NUMPSPRITES];

    // True if secret level has been done.
    doom_boolean didsecret;
} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
    doom_boolean in;        // whether the player is in game

    // Player stats, kills, collected items etc.
    int skills;
    int sitems;
    int ssecret;
    int stime;
    int frags[4];
    int score;        // current score on entry, modified on return
} wbplayerstruct_t;


typedef struct
{
    int epsd;        // episode # (0-2)

    // if true, splash the secret level
    doom_boolean didsecret;

    // previous and next levels, origin 0
    int last;
    int next;

    int maxkills;
    int maxitems;
    int maxsecret;
    int maxfrags;

    // the par time
    int partime;

    // index of this player in game
    int pnum;

    wbplayerstruct_t plyr[MAXPLAYERS];
} wbstartstruct_t;


#endif


#ifndef __D_NET__
#define __D_NET__


//#include "d_player.h"


//
// Network play related stuff.
// There is a data struct that stores network
// communication related stuff, and another
// one that defines the actual packets to
// be transmitted.
//

#define DOOMCOM_ID                0x12345678l

// Max computers/players in a game.
#define MAXNETNODES                8

// Networking and tick handling related.
#define BACKUPTICS                12


typedef enum
{
    CMD_SEND = 1,
    CMD_GET = 2
} command_t;


//
// Network packet data.
//
typedef struct
{
    // High bit is retransmit request.
    unsigned checksum;
    // Only valid if NCMD_RETRANSMIT.
    byte retransmitfrom;

    byte starttic;
    byte player;
    byte numtics;
    ticcmd_t cmds[BACKUPTICS];
} doomdata_t;


typedef struct
{
    // Supposed to be DOOMCOM_ID?
    long id;

    // DOOM executes an int to execute commands.
    short intnum;
    // Communication between DOOM and the driver.
    // Is CMD_SEND or CMD_GET.
    short command;
    // Is dest for send, set by get (-1 = no packet).
    short remotenode;

    // Number of bytes in doomdata to be sent
    short datalength;

    // Info common to all nodes.
    // Console is allways node 0.
    short numnodes;
    // Flag: 1 = no duplication, 2-5 = dup for slow nets.
    short ticdup;
    // Flag: 1 = send a backup tic in every packet.
    short extratics;
    // Flag: 1 = deathmatch.
    short deathmatch;
    // Flag: -1 = new game, 0-5 = load savegame
    short savegame;
    short episode;  // 1-3
    short map;      // 1-9
    short skill;    // 1-5

    // Info specific to this node.
    short consoleplayer;
    short numplayers;

    // These are related to the 3-display mode,
    //  in which two drones looking left and right
    //  were used to render two additional views
    //  on two additional computers.
    // Probably not operational anymore.
    // 1 = left, 0 = center, -1 = right
    short angleoffset;
    // 1 = drone
    short drone;

    // The packet data to be sent.
    doomdata_t data;
} doomcom_t;


// Create any new ticcmds and broadcast to other players.
void NetUpdate(void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame(void);

//? how many ticks to run?
void TryRunTics(void);


#endif


#ifndef __D_STATE__
#define __D_STATE__


// We need globally shared data structures,
//  for defining the global state variables.
//#include "doomdata.h"
//#include "d_net.h"

// We need the playr data structure as well.
//#include "d_player.h"


// ------------------------
// Command line parameters.
//
extern doom_boolean nomonsters;     // checkparm of -nomonsters
extern doom_boolean respawnparm;    // checkparm of -respawn
extern doom_boolean fastparm;       // checkparm of -fast
extern doom_boolean devparm;        // DEBUG: launched with -devparm


// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t gamemode;
extern GameMission_t gamemission;

// Set if homebrew PWAD stuff has been added.
extern doom_boolean modifiedgame;


// -------------------------------------------
// Language.
extern Language_t language;


// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t startskill;
extern int startepisode;
extern int startmap;

extern doom_boolean autostart;

// Selected by user. 
extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;

// Nightmare mode flag, single player.
extern doom_boolean respawnmonsters;

// Netgame? Only true if >1 player.
extern doom_boolean netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern doom_boolean deathmatch;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
// but are not (yet) supported with Linux
// (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
// Sound FX volume has default, 0 - 15
// Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;      // maximum volume for sound
extern int snd_MusicVolume;    // maximum volume for music

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int snd_MusicDevice;
extern int snd_SfxDevice;

// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern doom_boolean statusbaractive;

extern doom_boolean automapactive;  // In AutoMap mode?
extern doom_boolean menuactive;     // Menu overlayed?
extern doom_boolean paused;         // Game Pause?

extern doom_boolean viewactive;

extern doom_boolean nodrawers;
extern doom_boolean noblit;

extern int viewwindowx;
extern int viewwindowy;
extern int viewheight;
extern int viewwidth;
extern int scaledviewwidth;


// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern int viewangleoffset;

// Player taking events, and displaying.
extern int consoleplayer;
extern int displayplayer;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int totalkills;
extern int totalitems;
extern int totalsecret;

// Timer, for scores.
extern int levelstarttic;   // gametic at level start
extern int leveltime;       // tics in game play for par


// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern doom_boolean usergame;

//?
extern doom_boolean demoplayback;
extern doom_boolean demorecording;

// Quit after playing a demo from cmdline.
extern doom_boolean singledemo;

//?
extern gamestate_t gamestate;


//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
// according to user inputs. Partly load from
// WAD, partly set at startup time.

extern int gametic;


// Bookkeeping on players - state.
extern player_t players[MAXPLAYERS];

// Alive? Disconnected?
extern doom_boolean playeringame[MAXPLAYERS];


// Player spawn spots for deathmatch.
#define MAX_DM_STARTS 10
extern mapthing_t deathmatchstarts[MAX_DM_STARTS];
extern mapthing_t* deathmatch_p;

// Player spawn spots.
extern mapthing_t playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;


// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern int maxammo[NUMAMMO];


//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char basedefault[1024];
extern void* debugfile;

// if true, load all graphics at level load
extern doom_boolean precache;

// wipegamestate can be set to -1
// to force a wipe on the next draw
extern gamestate_t wipegamestate;

extern int mouseSensitivity;
//?
// debug flag to cancel adaptiveness
extern doom_boolean singletics;

extern int bodyqueslot;


// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int skyflatnum;


// Netgame stuff (buffers and pointers, i.e. indices).

// This is ???
extern doomcom_t* doomcom;

// This points inside doomcom.
extern doomdata_t* netbuffer;


extern ticcmd_t localcmds[BACKUPTICS];
extern int rndindex;

extern int maketic;
extern int nettics[MAXNETNODES];

extern ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];
extern int ticdup;


#endif


#ifndef __I_SOUND__
#define __I_SOUND__


//#include "doomdef.h"



//#include "doomstat.h"
//#include "sounds.h"


// Init at program start...
void I_InitSound();

// ... update sound buffer and audio device at runtime...
void I_UpdateSound(void);
void I_SubmitSound(void);

// ... shut down and relase at program termination.
void I_ShutdownSound(void);


//
//  SFX I/O
//

// Initialize channels?
void I_SetChannels();

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum(sfxinfo_t* sfxinfo);

// Starts a sound in a particular sound channel.
int I_StartSound(int id, int vol, int sep, int pitch, int priority);

// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);


//
//  MUSIC I/O
//
void I_InitMusic(void);
void I_ShutdownMusic(void);

// Volume.
void I_SetMusicVolume(int volume);

// PAUSE game handling.
void I_PauseSong(int handle);
void I_ResumeSong(int handle);

// Registers a song handle to song data.
int I_RegisterSong(void* data);

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void I_PlaySong(int handle, int looping);

// Stops a song over 3 seconds.
void I_StopSong(int handle);

// See above (register), then think backwards
void I_UnRegisterSong(int handle);

// Get next MIDI message
unsigned long I_TickSong();


#endif


#ifndef __P_INTER__
#define __P_INTER__


//#include "d_player.h"


doom_boolean P_GivePower(player_t*, int);


#endif


#ifndef __R_DEFS__
#define __R_DEFS__


// Screenwidth.
//#include "doomdef.h"

// Some more or less basic data types
// we depend on.
//#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
//#include "d_think.h"
// SECTORS do store MObjs anyway.
//#include "p_mobj.h"


// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE 0
#define SIL_BOTTOM 1
#define SIL_TOP 2
#define SIL_BOTH 3

#define MAXDRAWSEGS 256


//
// INTERNAL MAP TYPES
// used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") did.
//
typedef struct
{
    fixed_t        x;
    fixed_t        y;
} vertex_t;


// Forward of LineDefs, for Sectors.
struct line_s;

// Each sector has a degenmobj_t in its center
// for sound origin purposes.
// I suppose this does not handle sound from
// moving objects (doppler), because
// position is prolly just buffered, not
// updated.
typedef struct
{
    thinker_t                thinker;        // not used for anything
    fixed_t                x;
    fixed_t                y;
    fixed_t                z;
} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct
{
    fixed_t floorheight;
    fixed_t ceilingheight;
    short floorpic;
    short ceilingpic;
    short lightlevel;
    short special;
    short tag;

    // 0 = untraversed, 1,2 = sndlines -1
    int soundtraversed;

    // thing that made a sound (or null)
    mobj_t* soundtarget;

    // mapblock bounding box for height changes
    int blockbox[4];

    // origin for any sounds played by the sector
    degenmobj_t soundorg;

    // if == validcount, already checked
    int validcount;

    // list of mobjs in sector
    mobj_t* thinglist;

    // thinker_t for reversable actions
    void* specialdata;

    int linecount;
    struct line_s** lines;        // [linecount] size
} sector_t;


//
// The SideDef.
//
typedef struct
{
    // add this to the calculated texture column
    fixed_t textureoffset;

    // add this to the calculated texture top
    fixed_t rowoffset;

    // Texture indices.
    // We do not maintain names here. 
    short toptexture;
    short bottomtexture;
    short midtexture;

    // Sector the SideDef is facing.
    sector_t* sector;
} side_t;


//
// Move clipping aid for LineDefs.
//
typedef enum
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE
} slopetype_t;


typedef struct line_s
{
    // Vertices, from v1 to v2.
    vertex_t* v1;
    vertex_t* v2;

    // Precalculated v2 - v1 for side checking.
    fixed_t dx;
    fixed_t dy;

    // Animation related.
    short flags;
    short special;
    short tag;

    // Visual appearance: SideDefs.
    // sidenum[1] will be -1 if one sided
    short sidenum[2];

    // Neat. Another bounding box, for the extent
    // of the LineDef.
    fixed_t bbox[4];

    // To aid move clipping.
    slopetype_t slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    sector_t* frontsector;
    sector_t* backsector;

    // if == validcount, already checked
    int validcount;

    // thinker_t for reversable actions
    void* specialdata;
} line_t;


//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
// indicating the visible walls that define
// (all or some) sides of a convex BSP leaf.
//
typedef struct subsector_s
{
    sector_t* sector;
    short numlines;
    short firstline;
} subsector_t;


//
// The LineSeg.
//
typedef struct
{
    vertex_t* v1;
    vertex_t* v2;

    fixed_t offset;

    angle_t angle;

    side_t* sidedef;
    line_t* linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is 0 for one sided lines
    sector_t* frontsector;
    sector_t* backsector;
} seg_t;


//
// BSP node.
//
typedef struct
{
    // Partition line.
    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;

    // Bounding box for each child.
    fixed_t bbox[2][4];

    // If NF_SUBSECTOR its a subsector.
    unsigned short children[2];
} node_t;


// posts are runs of non masked source pixels
typedef struct
{
    byte topdelta;        // -1 is the last post in a column
    byte length;         // length data bytes follows
} post_t;

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;


//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even us emore than 32 levels.
typedef byte lighttable_t;


//
// ?
//
typedef struct drawseg_s
{
    seg_t* curline;
    int x1;
    int x2;

    fixed_t scale1;
    fixed_t scale2;
    fixed_t scalestep;

    // 0=none, 1=bottom, 2=top, 3=both
    int silhouette;

    // do not clip sprites above this
    fixed_t bsilheight;

    // do not clip sprites below this
    fixed_t tsilheight;

    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    short* sprtopclip;
    short* sprbottomclip;
    short* maskedtexturecol;
} drawseg_t;


// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
typedef struct
{
    short width;                // bounding box size 
    short height;
    short leftoffset;        // pixels to the left of origin 
    short topoffset;        // pixels below the origin 
    int columnofs[8];        // only [width] used
    // the [0] is &columnofs[width] 
} patch_t;


// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
    // Doubly linked list.
    struct vissprite_s* prev;
    struct vissprite_s* next;

    int x1;
    int x2;

    // for line side calculation
    fixed_t gx;
    fixed_t gy;

    // global bottom / top for silhouette clipping
    fixed_t gz;
    fixed_t gzt;

    // horizontal position of x1
    fixed_t startfrac;

    fixed_t scale;

    // negative if flipped
    fixed_t xiscale;

    fixed_t texturemid;
    int patch;

    // for color translation and shadow draw,
    //  maxbright frames as well
    lighttable_t* colormap;

    int mobjflags;
} vissprite_t;


//        
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct
{
    // If false use 0 for any position.
    // Note: as eight entries are available,
    //  we might as well insert the same name eight times.
    doom_boolean rotate;

    // Lump to use for view angles 0-7.
    short lump[8];

    // Flip bit (1 = flip) to use for view angles 0-7.
    byte flip[8];
} spriteframe_t;


//
// A sprite definition:
//  a number of animation frames.
//
typedef struct
{
    int numframes;
    spriteframe_t* spriteframes;
} spritedef_t;


//
// Now what is a visplane, anyway?
// 
typedef struct
{
    fixed_t height;
    int picnum;
    int lightlevel;
    int minx;
    int maxx;

    // leave pads for [minx-1]/[maxx+1]

    byte pad1;
    // Here lies the rub for all
    //  dynamic resize/change of resolution.
    byte top[SCREENWIDTH];
    byte pad2;
    byte pad3;
    // See above.
    byte bottom[SCREENWIDTH];
    byte pad4;
} visplane_t;


#endif


#ifndef __HULIB__
#define __HULIB__


// We are referring to patches.
//#include "r_defs.h"


// background and foreground screen numbers
// different from other modules.
#define BG 1
#define FG 0

// font stuff
#define HU_CHARERASE KEY_BACKSPACE

#define HU_MAXLINES 4
#define HU_MAXLINELENGTH 80


//
// Typedefs of widgets
//

// Text Line widget
//  (parent of Scrolling Text and Input Text widgets)
typedef struct
{
    // left-justified position of scrolling text window
    int x;
    int y;

    patch_t** f;                        // font
    int sc;                        // start character
    char l[HU_MAXLINELENGTH + 1];        // line of text
    int len;                              // current line length

    // whether this line needs to be udpated
    int needsupdate;
} hu_textline_t;


// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
    hu_textline_t l[HU_MAXLINES];        // text lines to draw
    int h;                // height in lines
    int cl;                // current line number

    // pointer to doom_boolean stating whether to update window
    doom_boolean* on;
    doom_boolean laston;                // last value of *->on.
} hu_stext_t;


// Input Text Line widget
//  (child of Text Line widget)
typedef struct
{
    hu_textline_t l;                // text line to input on

     // left margin past which I am not to delete characters
    int lm;

    // pointer to doom_boolean stating whether to update window
    doom_boolean* on;
    doom_boolean laston; // last value of *->on;
} hu_itext_t;


//
// Widget creation, access, and update routines
//

// initializes heads-up widget library
void HUlib_init(void);

//
// textline code
//

// clear a line of text
void HUlib_clearTextLine(hu_textline_t* t);

void HUlib_initTextLine(hu_textline_t* t, int x, int y, patch_t** f, int sc);

// returns success
doom_boolean HUlib_addCharToTextLine(hu_textline_t* t, char ch);

// returns success
doom_boolean HUlib_delCharFromTextLine(hu_textline_t* t);

// draws tline
void HUlib_drawTextLine(hu_textline_t* l, doom_boolean drawcursor);

// erases text line
void HUlib_eraseTextLine(hu_textline_t* l);


//
// Scrolling Text window widget routines
//

// ?
void HUlib_initSText(hu_stext_t* s,
                     int x,
                     int y,
                     int h,
                     patch_t** font,
                     int startchar,
                     doom_boolean* on);

// add a new line
void HUlib_addLineToSText(hu_stext_t* s);

// ?
void HUlib_addMessageToSText(hu_stext_t* s, char* prefix, char* msg);

// draws stext
void HUlib_drawSText(hu_stext_t* s);

// erases all stext lines
void HUlib_eraseSText(hu_stext_t* s);

// Input Text Line widget routines
void HUlib_initIText(hu_itext_t* it,
                     int x,
                     int y,
                     patch_t** font,
                     int startchar,
                     doom_boolean* on);

// enforces left margin
void HUlib_delCharFromIText(hu_itext_t* it);

// enforces left margin
void HUlib_eraseLineFromIText(hu_itext_t* it);

// resets line and left margin
void HUlib_resetIText(hu_itext_t* it);

// left of left-margin
void HUlib_addPrefixToIText(hu_itext_t* it, char* str);

// whether eaten
doom_boolean HUlib_keyInIText(hu_itext_t* it, unsigned char ch);

void HUlib_drawIText(hu_itext_t* it);

// erases all itext lines
void HUlib_eraseIText(hu_itext_t* it);


#endif


#ifndef __P_SPEC__
#define __P_SPEC__


//#include "p_mobj.h"
//#include "r_defs.h"


//
// End-level timer (-TIMER option)
//
extern doom_boolean levelTimer;
extern int levelTimeCount;


// Define values for map objects
#define MO_TELEPORTMAN          14


// at game start
void P_InitPicAnims(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
doom_boolean P_UseSpecialLine(mobj_t* thing, line_t* line, int side);

void P_ShootSpecialLine(mobj_t* thing, line_t* line);
void P_CrossSpecialLine(int linenum, int side, mobj_t* thing);
void P_PlayerInSpecialSector(player_t* player);
int twoSided(int sector, int line);
sector_t* getSector(int currentSector, int line, int side);
side_t* getSide(int currentSector, int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t* sec);
fixed_t P_FindHighestFloorSurrounding(sector_t* sec);
fixed_t P_FindNextHighestFloor(sector_t* sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec);
int P_FindSectorFromLineTag(line_t* line, int start);
int P_FindMinSurroundingLight(sector_t* sector, int max);
sector_t* getNextSector(line_t* line, sector_t* sec);

//
// SPECIAL
//
int EV_DoDonut(line_t* line);


//
// P_LIGHTS
//
typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
} fireflicker_t;


typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;


typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;
} strobe_t;


typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int minlight;
    int maxlight;
    int direction;
} glow_t;


#define GLOWSPEED 8
#define STROBEBRIGHT 5
#define FASTDARK 15
#define SLOWDARK 35

void P_SpawnFireFlicker(sector_t* sector);
void T_LightFlash(lightflash_t* flash);
void P_SpawnLightFlash(sector_t* sector);
void T_StrobeFlash(strobe_t* flash);

void P_SpawnStrobeFlash(sector_t* sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing(line_t* line);
void EV_TurnTagLightsOff(line_t* line);

void EV_LightTurnOn(line_t* line, int bright);

void T_Glow(glow_t* g);
void P_SpawnGlowingLight(sector_t* sector);


//
// P_SWITCH
//
typedef struct
{
    char name1[9];
    char name2[9];
    short episode;
} switchlist_t;


typedef enum
{
    top,
    middle,
    bottom
} bwhere_e;


typedef struct
{
    line_t* line;
    bwhere_e where;
    int btexture;
    int btimer;
    mobj_t* soundorg;
} button_t;


// max # of wall switches in a level
#define MAXSWITCHES 50

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS 16

// 1 second, in ticks. 
#define BUTTONTIME 35             


extern button_t buttonlist[MAXBUTTONS];


void P_ChangeSwitchTexture(line_t* line, int useAgain);
void P_InitSwitchList(void);


//
// P_PLATS
//
typedef enum
{
    up,
    down,
    waiting,
    in_stasis
} plat_e;


typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
} plattype_e;


typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plat_e oldstatus;
    doom_boolean crush;
    int tag;
    plattype_e type;
} plat_t;


#define PLATWAIT 3
#define PLATSPEED FRACUNIT
#define MAXPLATS 30


extern plat_t* activeplats[MAXPLATS];


void T_PlatRaise(plat_t* plat);
int EV_DoPlat(line_t* line, plattype_e type, int amount);
void P_AddActivePlat(plat_t* plat);
void P_RemoveActivePlat(plat_t* plat);
void EV_StopPlat(line_t* line);
void P_ActivateInStasis(int tag);


//
// P_DOORS
//
typedef enum
{
    door_normal,
    close30ThenOpen,
    door_close,
    door_open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose
} vldoor_e;


typedef struct
{
    thinker_t thinker;
    vldoor_e type;
    sector_t* sector;
    fixed_t topheight;
    fixed_t speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int direction;

    // tics to wait at the top
    int topwait;

    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int topcountdown;
} vldoor_t;


#define VDOORSPEED                FRACUNIT*2
#define VDOORWAIT                150


void EV_VerticalDoor(line_t* line, mobj_t* thing);
int EV_DoDoor(line_t* line, vldoor_e type);
int EV_DoLockedDoor(line_t* line, vldoor_e type, mobj_t* thing);
void T_VerticalDoor(vldoor_t* door);
void P_SpawnDoorCloseIn30(sector_t* sec);
void P_SpawnDoorRaiseIn5Mins(sector_t* sec, int secnum);


//
// P_CEILNG
//
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise
} ceiling_e;


typedef struct
{
    thinker_t thinker;
    ceiling_e type;
    sector_t* sector;
    fixed_t bottomheight;
    fixed_t topheight;
    fixed_t speed;
    doom_boolean crush;

    // 1 = up, 0 = waiting, -1 = down
    int direction;

    // ID
    int tag;
    int olddirection;
} ceiling_t;


#define CEILSPEED FRACUNIT
#define CEILWAIT 150
#define MAXCEILINGS 30


extern ceiling_t* activeceilings[MAXCEILINGS];


int EV_DoCeiling(line_t* line, ceiling_e type);
void T_MoveCeiling(ceiling_t* ceiling);
void P_AddActiveCeiling(ceiling_t* c);
void P_RemoveActiveCeiling(ceiling_t* c);
int EV_CeilingCrushStop(line_t* line);
void P_ActivateInStasisCeiling(line_t* line);


//
// P_FLOOR
//
typedef enum
{
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    turboLower,

    // raise floor to lowest surrounding CEILING
    raiseFloor,

    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,

    // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512
} floor_e;


typedef enum
{
    build8,        // slowly build by 8
    turbo16        // quickly build by 16
} stair_e;


typedef struct
{
    thinker_t thinker;
    floor_e type;
    doom_boolean crush;
    sector_t* sector;
    int direction;
    int newspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;
} floormove_t;


#define FLOORSPEED FRACUNIT


typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;


result_e T_MovePlane(sector_t* sector, fixed_t speed, fixed_t dest, doom_boolean crush, int floorOrCeiling, int direction);
int EV_BuildStairs(line_t* line, stair_e type);
int EV_DoFloor(line_t* line, floor_e floortype);
void T_MoveFloor(floormove_t* floor);

//
// P_TELEPT
//
int EV_Teleport(line_t* line, int side, mobj_t* thing);


#endif


#ifndef __R_BSP__
#define __R_BSP__


//#include "r_defs.h"


extern seg_t* curline;
extern side_t* sidedef;
extern line_t* linedef;
extern sector_t* frontsector;
extern sector_t* backsector;

extern int rw_x;
extern int rw_stopx;

extern doom_boolean segtextured;

// false if the back side is the same plane
extern doom_boolean markfloor;
extern doom_boolean markceiling;

extern doom_boolean skymap;

extern drawseg_t drawsegs[MAXDRAWSEGS];
extern drawseg_t* ds_p;

extern lighttable_t** hscalelight;
extern lighttable_t** vscalelight;
extern lighttable_t** dscalelight;


typedef void (*drawfunc_t) (int start, int stop);


// BSP?
void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);
void R_RenderBSPNode(int bspnum);


#endif


#ifndef __R_DRAW__
#define __R_DRAW__


//#include "r_defs.h"


extern lighttable_t* dc_colormap;
extern int dc_x;
extern int dc_yl;
extern int dc_yh;
extern fixed_t dc_iscale;
extern fixed_t dc_texturemid;

// first pixel in a column
extern byte* dc_source;


// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void R_DrawColumn(void);
void R_DrawColumnLow(void);

// The Spectre/Invisibility effect.
void R_DrawFuzzColumn(void);
void R_DrawFuzzColumnLow(void);

// Draw with color translation tables,
//  for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void R_DrawTranslatedColumn(void);
void R_DrawTranslatedColumnLow(void);

void R_VideoErase(unsigned ofs, int count);

extern int ds_y;
extern int ds_x1;
extern int ds_x2;

extern lighttable_t* ds_colormap;

extern fixed_t ds_xfrac;
extern fixed_t ds_yfrac;
extern fixed_t ds_xstep;
extern fixed_t ds_ystep;

// start of a 64*64 tile image
extern byte* ds_source;

extern byte* translationtables;
extern byte* dc_translation;


// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void R_DrawSpan(void);

// Low resolution mode, 160x200?
void R_DrawSpanLow(void);

void R_InitBuffer(int width, int height);


// Initialize color translation tables,
//  for player rendering etc.
void R_InitTranslationTables(void);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);


#endif


#ifndef __R_SEGS__
#define __R_SEGS__


//#include "r_defs.h"


void R_RenderMaskedSegRange(drawseg_t* ds, int x1, int x2);


#endif


#ifndef __R_STATE__
#define __R_STATE__


//#include "d_player.h" // Need data structure definitions.
//#include "r_defs.h"


//
// Refresh internal data structures,
// for rendering.
//

// needed for texture pegging
extern fixed_t* textureheight;

// needed for pre rendering (fracs)
extern fixed_t* spritewidth;

extern fixed_t* spriteoffset;
extern fixed_t* spritetopoffset;

extern lighttable_t* colormaps;

extern int viewwidth;
extern int scaledviewwidth;
extern int viewheight;

extern int firstflat;

// for global animation
extern int* flattranslation;
extern int* texturetranslation;

// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;

//
// Lookup tables for map data.
//
extern int numsprites;
extern spritedef_t* sprites;

extern int numvertexes;
extern vertex_t* vertexes;

extern int numsegs;
extern seg_t* segs;

extern int numsectors;
extern sector_t* sectors;

extern int numsubsectors;
extern subsector_t* subsectors;

extern int numnodes;
extern node_t* nodes;

extern int numlines;
extern line_t* lines;

extern int numsides;
extern side_t* sides;

//
// POV data.
//
extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;

extern angle_t viewangle;
extern player_t* viewplayer;

// ?
extern angle_t clipangle;

extern int viewangletox[FINEANGLES / 2];
extern angle_t xtoviewangle[SCREENWIDTH + 1];

extern fixed_t rw_distance;
extern angle_t rw_normalangle;

// angle to line origin
extern int rw_angle1;

// Segs count?
extern int sscount;

extern visplane_t* floorplane;
extern visplane_t* ceilingplane;


#endif


#ifndef __R_DATA__
#define __R_DATA__


//#include "r_defs.h"
//#include "r_state.h"


// Retrieve column data for span blitting.
byte* R_GetColumn(int tex, int col);

// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);

// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName(char* name);

// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(char* name);
int R_CheckTextureNumForName(char* name);

#endif


#ifndef __R_MAIN__
#define __R_MAIN__

//#include "d_player.h"
//#include "r_data.h"


//
// POV related.
//
extern fixed_t viewcos;
extern fixed_t viewsin;

extern int viewwidth;
extern int viewheight;
extern int viewwindowx;
extern int viewwindowy;

extern int centerx;
extern int centery;

extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;

extern int validcount;

extern int linecount;
extern int loopcount;


//
// Lighting LUT.
// Used for z-depth cuing per column/row,
// and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
#define LIGHTLEVELS 16
#define LIGHTSEGSHIFT 4

#define MAXLIGHTSCALE 48
#define LIGHTSCALESHIFT 12
#define MAXLIGHTZ 128
#define LIGHTZSHIFT 20

extern lighttable_t* scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t* scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t* zlight[LIGHTLEVELS][MAXLIGHTZ];

extern int extralight;
extern lighttable_t* fixedcolormap;


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS 32


// Blocky/low detail mode.
//B remove this?
//  0 = high, 1 = low
extern int detailshift;


//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void (*colfunc) (void);
extern void (*basecolfunc) (void);
extern void (*fuzzcolfunc) (void);
// No shadow effects on floors.
extern void (*spanfunc) (void);


//
// Utility functions.
int R_PointOnSide(fixed_t x, fixed_t y, node_t* node);
int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t* line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
fixed_t R_PointToDist(fixed_t x, fixed_t y);
fixed_t R_ScaleFromGlobalAngle(angle_t visangle);
subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);
void R_AddPointToBox(int x, int y, fixed_t* box);


//
// REFRESH - the actual rendering functions.
//

// Called by G_Drawer.
void R_RenderPlayerView(player_t* player);

// Called by startup code.
void R_Init(void);

// Called by M_Responder.
void R_SetViewSize(int blocks, int detail);

#endif


#ifndef __R_PLANE__
#define __R_PLANE__


//#include "r_data.h"


// Visplane related.
extern short* lastopening;

typedef void (*planefunction_t) (int top, int bottom);

extern planefunction_t floorfunc;
extern planefunction_t ceilingfunc_t;

extern short floorclip[SCREENWIDTH];
extern short ceilingclip[SCREENWIDTH];

extern fixed_t yslope[SCREENHEIGHT];
extern fixed_t distscale[SCREENWIDTH];

void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_MapPlane(int y, int x1, int x2);
void R_MakeSpans(int x, int t1, int b1, int t2, int b2);
void R_DrawPlanes(void);
visplane_t* R_FindPlane(fixed_t height, int picnum, int lightlevel);
visplane_t* R_CheckPlane(visplane_t* pl, int start, int stop);


#endif


#ifndef __R_THINGS__
#define __R_THINGS__


//#include "r_defs.h"


#define MAXVISSPRITES 128


extern vissprite_t vissprites[MAXVISSPRITES];
extern vissprite_t* vissprite_p;
extern vissprite_t vsprsortedhead;

// Constant arrays used for psprite clipping
// and initializing clipping.
extern short negonearray[SCREENWIDTH];
extern short screenheightarray[SCREENWIDTH];

// vars for R_DrawMaskedColumn
extern short* mfloorclip;
extern short* mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;

extern fixed_t pspritescale;
extern fixed_t pspriteiscale;


void R_DrawMaskedColumn(column_t* column);
void R_SortVisSprites(void);
void R_AddSprites(sector_t* sec);
void R_InitSprites(char** namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);


#endif


#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
//#include "tables.h"

// Screen size related parameters.
//#include "doomdef.h"

// Include the refresh/render data structs.
//#include "r_data.h"

//
// Separate header file for each module.
//
//#include "r_main.h"
//#include "r_bsp.h"
//#include "r_segs.h"
//#include "r_plane.h"
//#include "r_data.h"
//#include "r_things.h"
//#include "r_draw.h"

#endif // __R_LOCAL__


#ifndef __P_LOCAL__
#define __P_LOCAL__


#ifndef __R_LOCAL__
//#include "r_local.h"
#endif


#define FLOATSPEED (FRACUNIT*4)

#define MAXHEALTH 100
#define VIEWHEIGHT (41*FRACUNIT)

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS 128
#define MAPBLOCKSIZE (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT (FRACBITS+7)
#define MAPBMASK (MAPBLOCKSIZE-1)
#define MAPBTOFRAC (MAPBLOCKSHIFT-FRACBITS)


// player radius for movement checking
#define PLAYERRADIUS 16*FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS 32*FRACUNIT

#define GRAVITY FRACUNIT
#define MAXMOVE (30*FRACUNIT)

#define USERANGE (64*FRACUNIT)
#define MELEERANGE (64*FRACUNIT)
#define MISSILERANGE (32*64*FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD 100


//
// P_TICK
//

// both the head and tail of the thinker list
extern thinker_t thinkercap;


void P_InitThinkers(void);
void P_AddThinker(thinker_t* thinker);
void P_RemoveThinker(thinker_t* thinker);


//
// P_PSPR
//
void P_SetupPsprites(player_t* curplayer);
void P_MovePsprites(player_t* curplayer);
void P_DropWeapon(player_t* player);


//
// P_USER
//
void P_PlayerThink(player_t* player);


//
// P_MOBJ
//
#define ONFLOORZ DOOM_MININT
#define ONCEILINGZ DOOM_MAXINT

// Time interval for item respawning.
#define ITEMQUESIZE                128


extern mapthing_t itemrespawnque[ITEMQUESIZE];
extern int itemrespawntime[ITEMQUESIZE];
extern int iquehead;
extern int iquetail;


void P_RespawnSpecials(void);
mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_RemoveMobj(mobj_t* th);
doom_boolean P_SetMobjState(mobj_t* mobj, statenum_t state);
void P_MobjThinker(mobj_t* mobj);
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage);
mobj_t* P_SpawnMissile(mobj_t* source, mobj_t* dest, mobjtype_t type);
void P_SpawnPlayerMissile(mobj_t* source, mobjtype_t type);


//
// P_ENEMY
//
void P_NoiseAlert(mobj_t* target, mobj_t* emmiter);


//
// P_MAPUTL
//
typedef struct
{
    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;
} divline_t;


typedef struct
{
    fixed_t frac; // along trace line
    doom_boolean isaline;
    union
    {
        mobj_t* thing;
        line_t* line;
    } d;
} intercept_t;


#define MAXINTERCEPTS        128
extern intercept_t intercepts[MAXINTERCEPTS];
extern intercept_t* intercept_p;


typedef doom_boolean(*traverser_t) (intercept_t* in);


fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t* line);
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t* line);
void P_MakeDivline(line_t* li, divline_t* dl);
fixed_t P_InterceptVector(divline_t* v2, divline_t* v1);
int P_BoxOnLineSide(fixed_t* tmbox, line_t* ld);


extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;


void P_LineOpening(line_t* linedef);


doom_boolean P_BlockLinesIterator(int x, int y, doom_boolean(*func)(line_t*));
doom_boolean P_BlockThingsIterator(int x, int y, doom_boolean(*func)(mobj_t*));


#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4


extern divline_t trace;


doom_boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, doom_boolean(*trav) (intercept_t*));
void P_UnsetThingPosition(mobj_t* thing);
void P_SetThingPosition(mobj_t* thing);


//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern doom_boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;

extern line_t* ceilingline;


doom_boolean P_CheckPosition(mobj_t* thing, fixed_t x, fixed_t y);
doom_boolean P_TryMove(mobj_t* thing, fixed_t x, fixed_t y);
doom_boolean P_TeleportMove(mobj_t* thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t* mo);
doom_boolean P_CheckSight(mobj_t* t1, mobj_t* t2);
void P_UseLines(player_t* player);
doom_boolean P_ChangeSector(sector_t* sector, doom_boolean crunch);


extern mobj_t* linetarget;        // who got hit (or 0)


fixed_t P_AimLineAttack(mobj_t* t1, angle_t angle, fixed_t distance);
void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage);
void P_RadiusAttack(mobj_t* spot, mobj_t* source, int damage);


//
// P_SETUP
//
extern byte* rejectmatrix; // for fast sight rejection
extern short* blockmaplump; // offsets in blockmap are from here
extern short* blockmap;
extern int bmapwidth;
extern int bmapheight; // in mapblocks
extern fixed_t bmaporgx;
extern fixed_t bmaporgy; // origin of block map
extern mobj_t** blocklinks; // for thing chains


//
// P_INTER
//
extern int maxammo[NUMAMMO];
extern int clipammo[NUMAMMO];


void P_TouchSpecialThing(mobj_t* special, mobj_t* toucher);
void P_DamageMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage);


//
// P_SPEC
//
//#include "p_spec.h"


#endif // __P_LOCAL__


#ifndef __STLIB__
#define __STLIB__


// We are referring to patches.
//#include "r_defs.h"


//
// Background and foreground screen numbers
//
#define STLIB_BG 4
#define STLIB_FG 0


//
// Typedefs of widgets
//

// Number widget
typedef struct
{
    // upper right-hand corner
    //  of the number (right-justified)
    int x;
    int y;

    // max # of digits in number
    int width;

    // last number value
    int oldnum;

    // pointer to current value
    int* num;

    // pointer to doom_boolean stating
    //  whether to update number
    doom_boolean* on;

    // list of patches for 0-9
    patch_t** p;

    // user data
    int data;
} st_number_t;


// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
    // number information
    st_number_t n;

    // percent sign graphic
    patch_t* p;
} st_percent_t;


// Multiple Icon widget
typedef struct
{
    // center-justified location of icons
    int x;
    int y;

    // last icon number
    int oldinum;

    // pointer to current icon
    int* inum;

    // pointer to doom_boolean stating
    //  whether to update icon
    doom_boolean* on;

    // list of icons
    patch_t** p;

    // user data
    int data;
} st_multicon_t;


// Binary Icon widget
typedef struct
{
    // center-justified location of icon
    int x;
    int y;

    // last icon value
    int oldval;

    // pointer to current icon status
    doom_boolean* val;

    // pointer to doom_boolean
    //  stating whether to update icon
    doom_boolean* on;


    patch_t* p; // icon
    int data;   // user data

} st_binicon_t;


//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
// everything else is done somewhere else.
//
void STlib_init(void);

// Number widget routines
void STlib_initNum(st_number_t* n,
                   int x,
                   int y,
                   patch_t** pl,
                   int* num,
                   doom_boolean* on,
                   int width);

void STlib_updateNum(st_number_t* n, doom_boolean refresh);

// Percent widget routines
void STlib_initPercent(st_percent_t* p,
                       int x,
                       int y,
                       patch_t** pl,
                       int* num,
                       doom_boolean* on,
                       patch_t* percent);

void STlib_updatePercent(st_percent_t* per, int refresh);

// Multiple Icon widget routines
void STlib_initMultIcon(st_multicon_t* mi,
                        int x,
                        int y,
                        patch_t** il,
                        int* inum,
                        doom_boolean* on);

void STlib_updateMultIcon(st_multicon_t* mi, doom_boolean refresh);

// Binary Icon widget routines
void STlib_initBinIcon(st_binicon_t* b,
                       int x,
                       int y,
                       patch_t* i,
                       doom_boolean* val,
                       doom_boolean* on);

void STlib_updateBinIcon(st_binicon_t* bi, doom_boolean refresh);

#endif


#ifndef __V_VIDEO__
#define __V_VIDEO__

//#include "doomtype.h"
//#include "doomdef.h"
//#include "r_data.h" // Needed because we are refering to patches.

//
// VIDEO
//

#define CENTERY (SCREENHEIGHT / 2)


// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.
extern byte* screens[5];
extern int dirtybox[4];
extern byte gammatable[5][256];
extern int usegamma;


// Allocates buffer screens, call before R_Init.
void V_Init(void);

void V_CopyRect(int srcx,
                int srcy,
                int srcscrn,
                int width,
                int height,
                int destx,
                int desty,
                int destscrn);

void V_DrawPatch(int x,
                 int y,
                 int scrn,
                 patch_t* patch);

void V_DrawPatchDirect(int x, 
                       int y, 
                       int scrn, 
                       patch_t* patch);

void V_DrawPatchRectDirect(int x, int y, int scrn, patch_t* patch, int src_x, int src_w);

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x,
                 int y,
                 int scrn,
                 int width,
                 int height,
                 byte* src);

// Reads a linear block of pixels into the view buffer.
void V_GetBlock(int x,
                int y,
                int scrn,
                int width,
                int height,
                byte* dest);

void V_MarkRect(int x,
                int y,
                int width,
                int height);

#endif


#ifndef __W_WAD__
#define __W_WAD__


//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char identification[4];
    int numlumps;
    int infotableofs;
} wadinfo_t;


typedef struct
{
    int filepos;
    int size;
    char name[8];
} filelump_t;


//
// WADFILE I/O related stuff.
//
typedef struct
{
    char name[8];
    void* handle;
    int position;
    int size;
} lumpinfo_t;


extern void** lumpcache;
extern lumpinfo_t* lumpinfo;
extern int numlumps;

void W_InitMultipleFiles(char** filenames);
void W_Reload(void);

int W_CheckNumForName(char* name);
int W_GetNumForName(char* name);

int W_LumpLength(int lump);
void W_ReadLump(int lump, void* dest);

void* W_CacheLumpNum(int lump, int tag);
void* W_CacheLumpName(char* name, int tag);


#endif


#ifndef __WI_STUFF__
#define __WI_STUFF__

//#include "doomdef.h"

// States for the intermission
typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc
} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker(void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer(void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t* wbstartstruct);

#endif


#ifndef __Z_ZONE__
#define __Z_ZONE__




//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC       1        // static entire execution time
#define PU_SOUND        2        // static while playing
#define PU_MUSIC        3        // static while playing
#define PU_DAVE         4        // anything else Dave wants static
#define PU_LEVEL        50       // static until level exited
#define PU_LEVSPEC      51       // a special thinker in a level
// Tags >= 100 are purgable whenever needed.
#define PU_PURGELEVEL   100
#define PU_CACHE        101


void  Z_Init(void);
void* Z_Malloc(int size, int tag, void* ptr);
void  Z_Free(void* ptr);
void  Z_FreeTags(int lowtag, int hightag);
void  Z_DumpHeap(int lowtag, int hightag);
void  Z_FileDumpHeap(void* f);
void  Z_CheckHeap(void);
void  Z_ChangeTag2(void* ptr, int tag);
int   Z_FreeMemory(void);


typedef struct memblock_s
{
    int size;       // including the header and possibly tiny fragments
    void** user;    // 0 if a free block
    int tag;        // purgelevel
    int id;         // should be ZONEID
    struct memblock_s* next;
    struct memblock_s* prev;
} memblock_t;

//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#define Z_ChangeTag(p,t) \
{ \
    if (( (memblock_t *)( (byte *)(p) - sizeof(memblock_t)))->id!=0x1d4a11) \
    { \
        /*I_Error("Error: Z_CT at "__FILE__":%i",__LINE__);*/ \
        char buf[260]; \
        doom_strcpy(buf, "Error: Z_CT at " __FILE__ ":"); \
        doom_concat(buf, doom_itoa(__LINE__, 10)); \
        I_Error(buf); \
    } \
    Z_ChangeTag2(p,t); \
};


#endif



#if defined(DOOM_IMPLEMENTATION)
#endif // DOOM_IMPLEMENTATION
