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
//        DOOM selection menu, options, episode etc.
//        Sliders and icons. Kinda widget stuff.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_swap.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h" // Data.
#include "m_menu.h"


#define SAVESTRINGSIZE 24
#define SKULLXOFF -32
#define LINEHEIGHT 16


//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short status;

    char name[10];

    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void (*routine)(int choice);

    // hotkey in menu
    char alphaKey;
} menuitem_t;


typedef struct menu_s
{
    short numitems; // # of menu items
    struct menu_s* prevMenu; // previous menu
    menuitem_t* menuitems; // menu items
    void (*routine)(); // draw routine
    short x;
    short y; // x,y of menu
    short lastOn; // last item user was on in menu
} menu_t;


typedef struct
{
    char* lump;
    int x, w;
    int offx;
    int offy;
} menu_custom_text_seg_t;


typedef struct
{
    char* name;
    menu_custom_text_seg_t segs[16];
} menu_custom_text_t;


extern int doom_flags;

extern patch_t* hu_font[HU_FONTSIZE];
extern doom_boolean message_dontfuckwithme;

extern doom_boolean chat_on; // in heads-up code

extern int mousemove;


//
// defaulted values
//
int mouseSensitivity; // has default

// Show messages has default, 0 = off, 1 = on
int showMessages;


// Blocky mode, has default, 0 = high, 1 = normal
int detailLevel;
int screenblocks; // has default

// temp for screenblocks (0-9)
int screenSize;

// -1 = no quicksave slot picked!
int quickSaveSlot;

// 1 = message to be printed
int messageToPrint;
// ...and here is the message string!
char* messageString;

// message x & y
int messx;
int messy;
int messageLastMenuActive;

// timed message = no input from user
doom_boolean messageNeedsInput;

void (*messageRoutine)(int response);

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int saveStringEnter;
int saveSlot; // which slot to save in
int saveCharIndex; // which char we're editing
// old save description before edit
char saveOldString[SAVESTRINGSIZE];

doom_boolean inhelpscreens;
doom_boolean menuactive;

extern doom_boolean sendpause;
char savegamestrings[10][SAVESTRINGSIZE];

char endstring[160];

short itemOn; // menu item skull is on
short skullAnimCounter; // skull animation counter
short whichSkull; // which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char skullName[2][/*8*/9] = { "M_SKULL1","M_SKULL2" };

// current menudef
menu_t* currentMenu;

// We create new menu text by cutting into existing graphics and pasting them to create the new text.
// This way we don't ship code with embeded graphics that come from WAD files.
menu_custom_text_t menu_custom_texts[] =
{
    {"TXT_MMOV", {
        {"M_MSENS", 0, 74, 0, 0}, // Mouse
        {"M_MSENS", 0, 31, 83, 0}, // Mo
        {"M_MSENS", 160, 14, 83 + 31, 0}, // v
        {"M_MSENS", 60, 14, 83 + 31 + 14, 0}, // e
        {"M_DETAIL", 169, 5, 83 + 31 + 14 + 14, 0}, // :
        {0}
    }},
    {"TXT_MOPT", {
        {"M_MSENS", 0, 74, 0, 0}, // Mouse
        {"M_OPTION", 0, 92, 74 + 9, 0}, // Options
        {0}
    }},
    {"TXT_CROS", {
        {"M_SKILL", 0, 16, 0, 0}, // C
        {"M_DETAIL", 14, 15, 16, 0}, // r
        {"M_SKILL", 46, 30, 16 + 15, 0}, // os
        {"M_SKILL", 62, 14, 16 + 15 + 30, 0}, // s
        {"M_SKILL", 16, 15, 16 + 15 + 30 + 14, 0}, // h
        {"M_DETAIL", 140, 19, 16 + 15 + 30 + 14 + 15, 0}, // ai
        {"M_DETAIL", 14, 15, 16 + 15 + 30 + 14 + 15 + 19, 0}, // r
        {"M_DETAIL", 169, 5, 16 + 15 + 30 + 14 + 15 + 19 + 15, 0}, // :
        {0}
    }},
    {"TXT_ARUN", {
        {"M_SGTTL", 90, 17, 0, 0}, // A
        {"M_GDLOW", 0, 10, 17, 3}, // l
        {"M_GDLOW", 26, 16, 17 + 10, 3}, // 
        {"M_DISP", 57, 30, 17 + 10 + 16, 0}, // ay
        {"M_RDTHIS", 99, 14, 17 + 10 + 16 + 30, 0}, // s
        {"M_RDTHIS", 0, 16, 17 + 10 + 16 + 30 + 14 + 7, 0}, // R
        {"M_SFXVOL", 90, 15, 17 + 10 + 16 + 30 + 14 + 7 + 16, 0}, // u
        {"M_OPTION", 62, 15, 17 + 10 + 16 + 30 + 14 + 7 + 16 + 15, 0}, // n
        {"M_DETAIL", 169, 5, 17 + 10 + 16 + 30 + 14 + 7 + 16 + 15 + 15, 0}, // :
        {0}
    }},
};

const int custom_texts_count = sizeof(menu_custom_texts) / sizeof(menu_custom_text_t);

char tempstring[80];
int epi;
char detailNames[2][9] = { "M_GDHIGH","M_GDLOW" };
char msgNames[2][9] = { "M_MSGOFF","M_MSGON" };

int quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};


//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_MouseOptions(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);
void M_ChangeCrosshair(int choice);
void M_ChangeAlwaysRun(int choice);

void M_MouseMove(int choice);
void M_ChangeSensitivity(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x, int y);
void M_SetupNextMenu(menu_t* menudef);
void M_DrawThermo(int x, int y, int thermWidth, int thermDot);
void M_DrawEmptyCell(menu_t* menu, int item);
void M_DrawSelCell(menu_t* menu, int item);
void M_WriteText(int x, int y, char* string);
int  M_StringWidth(char* string);
int  M_StringHeight(char* string);
void M_StartControlPanel(void);
void M_StartMessage(char* string, void* routine, doom_boolean input);
void M_StopMessage(void);
void M_ClearMenus(void);
void M_DrawMouseOptions(void);


//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[] =
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'r'},
    {1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    0,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[] =
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
    ep_end,                // # of menu items
    &MainDef,                // previous menu
    EpisodeMenu,        // menuitem_t ->
    M_DrawEpisode,        // drawing routine ->
    48,63,              // x,y
    ep1                        // lastOn
};


//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[] =
{
    {1,"M_JKILL",        M_ChooseSkill, 'i'},
    {1,"M_ROUGH",        M_ChooseSkill, 'h'},
    {1,"M_HURT",        M_ChooseSkill, 'h'},
    {1,"M_ULTRA",        M_ChooseSkill, 'u'},
    {1,"M_NMARE",        M_ChooseSkill, 'n'}
};

menu_t  NewDef =
{
    newg_end,                // # of menu items
    &EpiDef,                // previous menu
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,        // drawing routine ->
    48,63,              // x,y
    hurtme                // lastOn
};


//
// OPTIONS MENU
//
menuitem_t* OptionsMenu;

enum
{
    endgame,
    messages,
    crosshair_opt,
    always_run_opt,
    //detail, // Details do nothing?
    scrnsize,
    option_empty1,
    mouseoptions,
    soundvol,
    opt_end
} options_e;

menuitem_t OptionsMenuFull[] =
{
    {1,"M_ENDGAM",  M_EndGame,'e'},
    {1,"M_MESSG",   M_ChangeMessages,'m'},
    {1,"TXT_CROS",  M_ChangeCrosshair,'c'},
    {1,"TXT_ARUN",  M_ChangeAlwaysRun,'r'},
    //{1,"M_DETAIL", M_ChangeDetail,'g'},  // Details do nothing?
    {2,"M_SCRNSZ",  M_SizeDisplay,'s'},
    {-1,"",0},
    {1,"TXT_MOPT",  M_MouseOptions,'f'},
    {1,"M_SVOL",    M_Sound,'s'}
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenuFull,
    M_DrawOptions,
    60,37,
    0
};


enum
{
    endgame_no_mouse,
    messages_no_mouse,
    crosshair_opt_no_mouse,
    always_run_opt_no_mouse,
    //detail_no_mouse, // Details do nothing?
    scrnsize_no_mouse,
    option_empty1_no_mouse,
    soundvol_no_mouse,
    opt_end_no_mouse
} options_e_no_mouse;

menuitem_t OptionsMenuNoMouse[] =
{
    {1,"M_ENDGAM",  M_EndGame,'e'},
    {1,"M_MESSG",   M_ChangeMessages,'m'},
    {1,"TXT_CROS",  M_ChangeCrosshair,'c'},
    {1,"TXT_ARUN",  M_ChangeAlwaysRun,'r'},
    //{1,"M_DETAIL",  M_ChangeDetail,'g'}, // Details do nothing?
    {2,"M_SCRNSZ",  M_SizeDisplay,'s'},
    {-1,"",0},
    {1,"M_SVOL",    M_Sound,'s'}
};

menu_t  OptionsNoMouseDef =
{
    opt_end_no_mouse,
    &MainDef,
    OptionsMenuNoMouse,
    M_DrawOptions,
    60,37,
    0
};


enum
{
    endgame_no_sound,
    messages_no_sound,
    crosshair_opt_no_sound,
    always_run_opt_no_sound,
    //detail_no_sound, // Details do nothing?
    scrnsize_no_sound,
    option_empty1_no_sound,
    mouseoptions_no_sound,
    opt_end_no_sound
} options_e_no_sound;

menuitem_t OptionsMenuNoSound[] =
{
    {1,"M_ENDGAM",  M_EndGame,'e'},
    {1,"M_MESSG",   M_ChangeMessages,'m'},
    {1,"TXT_CROS",  M_ChangeCrosshair,'c'},
    {1,"TXT_ARUN",  M_ChangeAlwaysRun,'r'},
    //{1,"M_DETAIL",  M_ChangeDetail,'g'}, // Details do nothing?
    {2,"M_SCRNSZ",  M_SizeDisplay,'s'},
    {-1,"",0},
    {1,"TXT_MOPT",  M_MouseOptions,'f'}
};

menu_t OptionsNoSoundDef =
{
    opt_end_no_sound,
    &MainDef,
    OptionsMenuNoSound,
    M_DrawOptions,
    60,37,
    0
};


enum
{
    endgame_no_sound_no_mouse,
    messages_no_sound_no_mouse,
    crosshair_opt_no_sound_no_mouse,
    always_run_top_no_sound_no_mouse,
    //detail_no_sound_no_mouse, // Details do nothing?
    scrnsize_no_sound_no_mouse,
    option_empty1_no_sound_no_mouse,
    opt_end_no_sound_no_mouse
} options_e_no_sound_no_mouse;

menuitem_t OptionsMenuNoSoundNoMouse[] =
{
    {1,"M_ENDGAM",  M_EndGame,'e'},
    {1,"M_MESSG",   M_ChangeMessages,'m'},
    {1,"TXT_CROS",  M_ChangeCrosshair,'c'},
    {1,"TXT_ARUN",  M_ChangeAlwaysRun,'r'},
    //{1,"M_DETAIL",  M_ChangeDetail,'g'}, // Details do nothing?
    {2,"M_SCRNSZ",  M_SizeDisplay,'s'},
    {-1,"",0}
};

menu_t OptionsNoSoundNoMouseDef =
{
    opt_end_no_sound_no_mouse,
    &MainDef,
    OptionsMenuNoSoundNoMouse,
    M_DrawOptions,
    60,37,
    0
};


//
// MOUSE OPTIONS
//
enum
{
    mousemov,
    mousesens,
    mouse_option_empty1,
    mouse_opt_end
} mouseoptions_e;

menuitem_t MouseOptionsMenu[] =
{
    {1,"TXT_MMOV", M_MouseMove,'f'},
    {2,"M_MSENS", M_ChangeSensitivity,'m'},
    {-1,"",0},
};

menu_t  MouseOptionsDef =
{
    mouse_opt_end,
    &OptionsDef,
    MouseOptionsMenu,
    M_DrawMouseOptions,
    60,70,
    0
};


//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[] =
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};

//
// SOUND VOLUME MENU
//
menuitem_t* SoundMenu;

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenuFull[] =
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenuFull,
    M_DrawSound,
    80,64,
    0
};


enum
{
    music_vol_no_sfx,
    sfx_empty2_no_sfx,
    sound_end_no_sfx
} sound_e_no_sfx;

menuitem_t SoundMenuNoSFX[] =
{
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0}
};

menu_t  SoundNoSFXDef =
{
    sound_end_no_sfx,
    &OptionsDef,
    SoundMenuNoSFX,
    M_DrawSound,
    80,64,
    0
};


enum
{
    sfx_vol_no_music,
    sfx_empty1_no_music,
    sound_end_no_music
} sound_e_no_music;

menuitem_t SoundMenuNoMusic[] =
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0}
};

menu_t  SoundNoMusicDef =
{
    sound_end_no_music,
    &OptionsDef,
    SoundMenuNoMusic,
    M_DrawSound,
    80,64,
    0
};


//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadMenu[] =
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[] =
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};


//
// M_DrawCustomMenuText
//  Draw several segments of patches to make up new text
//
void M_DrawCustomMenuText(char* name, int x, int y)
{
    for (int i = 0; i < custom_texts_count; ++i)
    {
        menu_custom_text_t* custom_text = menu_custom_texts + i;
        if (doom_strcmp(custom_text->name, name) == 0)
        {
            menu_custom_text_seg_t* seg = custom_text->segs;
            while (seg->lump)
            {
                void* lump = W_CacheLumpName(seg->lump, PU_CACHE);
                V_DrawPatchRectDirect(x + seg->offx, y, 0, lump, seg->x, seg->w);
                ++seg;
            }
            break;
        }
    }
}


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    void* handle;
    int count;
    int i;
    char name[256];

    for (i = 0; i < load_end; i++)
    {
#if 0
        if (M_CheckParm("-cdrom"))
            //doom_sprintf(name, "c:\\doomdata\\" SAVEGAMENAME "%d.dsg", i);
        else
#endif
        {
            //doom_sprintf(name, SAVEGAMENAME"%d.dsg", i);
            doom_strcpy(name, SAVEGAMENAME);
            doom_concat(name, doom_itoa(i, 10));
            doom_strcpy(name, ".dsg");
        }

        handle = doom_open(name, "r");
        if (handle == 0)
        {
            doom_strcpy(&savegamestrings[i][0], EMPTYSTRING);
            LoadMenu[i].status = 0;
            continue;
        }
        count = doom_read(handle, &savegamestrings[i], SAVESTRINGSIZE);
        doom_close(handle);
        LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int i;

    V_DrawPatchDirect(72, 28, 0, W_CacheLumpName("M_LOADG", PU_CACHE));
    for (i = 0; i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x, LoadDef.y + LINEHEIGHT * i);
        M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * i, savegamestrings[i]);
    }
}


//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y)
{
    int i;

    V_DrawPatchDirect(x - 8, y + 7, 0, W_CacheLumpName("M_LSLEFT", PU_CACHE));

    for (i = 0; i < 24; i++)
    {
        V_DrawPatchDirect(x, y + 7, 0, W_CacheLumpName("M_LSCNTR", PU_CACHE));
        x += 8;
    }

    V_DrawPatchDirect(x, y + 7, 0, W_CacheLumpName("M_LSRGHT", PU_CACHE));
}


//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char name[256];

#if 0
    if (M_CheckParm("-cdrom"))
        //doom_sprintf(name, "c:\\doomdata\\"SAVEGAMENAME"%d.dsg", choice);
    else
#endif
    {
        //doom_sprintf(name, SAVEGAMENAME"%d.dsg", choice);
        doom_strcpy(name, SAVEGAMENAME);
        doom_concat(name, doom_itoa(choice, 10));
        doom_strcpy(name, ".dsg");
    }
    G_LoadGame(name);
    M_ClearMenus();
}


//
// Selected from DOOM menu
//
void M_LoadGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(LOADNET, 0, false);
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int i;

    V_DrawPatchDirect(72, 28, 0, W_CacheLumpName("M_SAVEG", PU_CACHE));
    for (i = 0; i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x, LoadDef.y + LINEHEIGHT * i);
        M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * i, savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[saveSlot]);
        M_WriteText(LoadDef.x + i, LoadDef.y + LINEHEIGHT * saveSlot, "_");
    }
}


//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame(slot, savegamestrings[slot]);
    M_ClearMenus();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
        quickSaveSlot = slot;
}


//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    doom_strcpy(saveOldString, savegamestrings[choice]);
    if (!doom_strcmp(savegamestrings[choice], EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = (int)doom_strlen(savegamestrings[choice]);
}


//
// Selected from DOOM menu
//
void M_SaveGame(int choice)
{
    if (!usergame)
    {
        M_StartMessage(SAVEDEAD, 0, false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}


//
// M_QuickSave
//
void M_QuickSaveResponse(int ch)
{
    if (ch == 'y')
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(0, sfx_swtchx);
    }
}


void M_QuickSave(void)
{
    if (!usergame)
    {
        S_StartSound(0, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;        // means to pick a slot now
        return;
    }
    //doom_sprintf(tempstring, QSPROMPT, savegamestrings[quickSaveSlot]);
    doom_strcpy(tempstring, QSPROMPT_1);
    doom_concat(tempstring, savegamestrings[quickSaveSlot]);
    doom_strcpy(tempstring, QSPROMPT_2);
    M_StartMessage(tempstring, M_QuickSaveResponse, true);
}


//
// M_QuickLoad
//
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y')
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(0, sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(QLOADNET, 0, false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(QSAVESPOT, 0, false);
        return;
    }
    //doom_sprintf(tempstring, QLPROMPT, savegamestrings[quickSaveSlot]);
    doom_strcpy(tempstring, QLPROMPT_1);
    doom_concat(tempstring, savegamestrings[quickSaveSlot]);
    doom_strcpy(tempstring, QLPROMPT_2);
    M_StartMessage(tempstring, M_QuickLoadResponse, true);
}


//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;
    switch (gamemode)
    {
        case commercial:
            V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP", PU_CACHE));
            break;
        case shareware:
        case registered:
        case retail:
            V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP1", PU_CACHE));
            break;
        default:
            break;
    }
    return;
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;
    switch (gamemode)
    {
        case retail:
        case commercial:
            // This hack keeps us from having to change menus.
            V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("CREDIT", PU_CACHE));
            break;
        case shareware:
        case registered:
            V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP2", PU_CACHE));
            break;
        default:
            break;
    }
    return;
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    V_DrawPatchDirect(60, 38, 0, W_CacheLumpName("M_SVOL", PU_CACHE));

    if (!(doom_flags & DOOM_FLAG_HIDE_SOUND_OPTIONS))
    {
        int offset = (doom_flags & DOOM_FLAG_HIDE_MUSIC_OPTIONS) ? sfx_vol_no_music : sfx_vol;
        M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (offset + 1),
                     16, snd_SfxVolume);
    }

    if (!(doom_flags & DOOM_FLAG_HIDE_MUSIC_OPTIONS))
    {
        int offset = (doom_flags & DOOM_FLAG_HIDE_SOUND_OPTIONS) ? music_vol_no_sfx : music_vol;
        M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (offset + 1),
                     16, snd_MusicVolume);
    }
}


void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}


void M_MouseOptions(int choice)
{
    M_SetupNextMenu(&MouseOptionsDef);
}


void M_SfxVol(int choice)
{
    switch (choice)
    {
        case 0:
            if (snd_SfxVolume)
                snd_SfxVolume--;
            break;
        case 1:
            if (snd_SfxVolume < 15)
                snd_SfxVolume++;
            break;
    }

    S_SetSfxVolume(snd_SfxVolume /* *8 */);
}


void M_MusicVol(int choice)
{
    switch (choice)
    {
        case 0:
            if (snd_MusicVolume)
                snd_MusicVolume--;
            break;
        case 1:
            if (snd_MusicVolume < 15)
                snd_MusicVolume++;
            break;
    }

    S_SetMusicVolume(snd_MusicVolume /* *8 */);
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    V_DrawPatchDirect(94, 2, 0, W_CacheLumpName("M_DOOM", PU_CACHE));
}


//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawPatchDirect(96, 14, 0, W_CacheLumpName("M_NEWG", PU_CACHE));
    V_DrawPatchDirect(54, 38, 0, W_CacheLumpName("M_SKILL", PU_CACHE));
}


void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(NEWGAME, 0, false);
        return;
    }

    if (gamemode == commercial)
        M_SetupNextMenu(&NewDef);
    else
        M_SetupNextMenu(&EpiDef);
}


//
// M_Episode
//
void M_DrawEpisode(void)
{
    V_DrawPatchDirect(54, 38, 0, W_CacheLumpName("M_EPISOD", PU_CACHE));
}

void M_VerifyNightmare(int ch)
{
    if (ch != 'y')
        return;

    G_DeferedInitNew(nightmare, epi + 1, 1);
    M_ClearMenus();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(NIGHTMARE, M_VerifyNightmare, true);
        return;
    }

    G_DeferedInitNew(choice, epi + 1, 1);
    M_ClearMenus();
}

void M_Episode(int choice)
{
    if ((gamemode == shareware)
        && choice)
    {
        M_StartMessage(SWSTRING, 0, false);
        M_SetupNextMenu(&ReadDef1);
        return;
    }

    // Yet another hack...
    if ((gamemode == registered)
        && (choice > 2))
    {
        doom_print(
                "M_Episode: 4th episode requires UltimateDOOM\n");
        choice = 0;
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}


//
// M_Options
//
void M_DrawOptions(void)
{
    V_DrawPatchDirect(108, 15, 0, W_CacheLumpName("M_OPTTTL", PU_CACHE));

    //V_DrawPatchDirect (OptionsDef.x + 175,OptionsDef.y+LINEHEIGHT*detail,0,
    //                W_CacheLumpName(detailNames[detailLevel],PU_CACHE)); // Details do nothing?

    V_DrawPatchDirect(OptionsDef.x + 120, OptionsDef.y + LINEHEIGHT * messages, 0,
                      W_CacheLumpName(msgNames[showMessages], PU_CACHE));

    extern int crosshair;
    V_DrawPatchDirect(OptionsDef.x + 131, OptionsDef.y + LINEHEIGHT * crosshair_opt, 0,
                      W_CacheLumpName(msgNames[crosshair], PU_CACHE));

    extern int always_run;
    V_DrawPatchDirect(OptionsDef.x + 147, OptionsDef.y + LINEHEIGHT * always_run_opt, 0,
                      W_CacheLumpName(msgNames[always_run], PU_CACHE));

    M_DrawThermo(OptionsDef.x, OptionsDef.y + LINEHEIGHT * (scrnsize + 1),
                 9, screenSize);
}


void M_DrawMouseOptions(void)
{
    M_DrawCustomMenuText("TXT_MOPT", 74, 45);

    V_DrawPatchDirect(MouseOptionsDef.x + 149, MouseOptionsDef.y + LINEHEIGHT * mousemov, 0,
                      W_CacheLumpName(msgNames[mousemove], PU_CACHE));

    M_DrawThermo(MouseOptionsDef.x, MouseOptionsDef.y + LINEHEIGHT * (mousesens + 1),
                 10, mouseSensitivity);
}


void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}


//
// Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
        players[consoleplayer].message = MSGOFF;
    else
        players[consoleplayer].message = MSGON;

    message_dontfuckwithme = true;
}


//
// Toggle crosshair on/off
//
void M_ChangeCrosshair(int choice)
{
    extern int crosshair;

    // warning: unused parameter `int choice'
    choice = 0;
    crosshair = 1 - crosshair;

    if (!crosshair)
        players[consoleplayer].message = CROSSOFF;
    else
        players[consoleplayer].message = CROSSON;

    message_dontfuckwithme = true;
}


//
// Toggle always-run on/off
//
void M_ChangeAlwaysRun(int choice)
{
    extern int always_run;

    // warning: unused parameter `int choice'
    choice = 0;
    always_run = 1 - always_run;

    if (!always_run)
        players[consoleplayer].message = ALWAYSRUNOFF;
    else
        players[consoleplayer].message = ALWAYSRUNON;

    message_dontfuckwithme = true;
}


//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
    if (ch != 'y')
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus();
    D_StartTitle();
}


void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
        S_StartSound(0, sfx_oof);
        return;
    }

    if (netgame)
    {
        M_StartMessage(NETEND, 0, false);
        return;
    }

    M_StartMessage(ENDGAME, M_EndGameResponse, true);
}


//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}


void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}


void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}


//
// M_QuitDOOM
//
void M_QuitResponse(int ch)
{
    if (ch != 'y')
        return;
    if (!netgame)
    {
        if (gamemode == commercial)
            S_StartSound(0, quitsounds2[(gametic >> 2) & 7]);
        else
            S_StartSound(0, quitsounds[(gametic >> 2) & 7]);
        I_WaitVBL(105);
    }
    I_Quit();
}


void M_QuitDOOM(int choice)
{
    // We pick index 0 which is language sensitive,
    //  or one at random, between 1 and maximum number.
    if (language != english)
    {
        //doom_sprintf(endstring, "%s\n\n"DOSY, endmsg[0]);
        doom_strcpy(endstring, endmsg[0]);
        doom_concat(endstring, "\n\n" DOSY);
    }
    else
    {
        //doom_sprintf(endstring, "%s\n\n" DOSY, endmsg[gametic % (NUM_QUITMESSAGES - 2) + 1]);
        doom_strcpy(endstring, endmsg[gametic % (NUM_QUITMESSAGES - 2) + 1]);
        doom_concat(endstring, "\n\n" DOSY);
    }

    M_StartMessage(endstring, M_QuitResponse, true);
}


void M_ChangeSensitivity(int choice)
{
    switch (choice)
    {
        case 0:
            if (mouseSensitivity)
                mouseSensitivity--;
            break;
        case 1:
            if (mouseSensitivity < 9)
                mouseSensitivity++;
            break;
    }
}


void M_MouseMove(int choice)
{
    choice = 0;
    mousemove = 1 - mousemove;

    return;
}


void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    // FIXME - does not work. Remove anyway?
    doom_print("M_ChangeDetail: low detail mode n.a.\n");
}


void M_SizeDisplay(int choice)
{
    switch (choice)
    {
        case 0:
            if (screenSize > 0)
            {
                screenblocks--;
                screenSize--;
            }
            break;
        case 1:
            if (screenSize < 8)
            {
                screenblocks++;
                screenSize++;
            }
            break;
    }

    R_SetViewSize(screenblocks, detailLevel);
}


//
// Menu Functions
//
void M_DrawThermo(int x, int y, int thermWidth, int thermDot)
{
    int xx;
    int i;

    xx = x;
    V_DrawPatchDirect(xx, y, 0, W_CacheLumpName("M_THERML", PU_CACHE));
    xx += 8;
    for (i = 0; i < thermWidth; i++)
    {
        V_DrawPatchDirect(xx, y, 0, W_CacheLumpName("M_THERMM", PU_CACHE));
        xx += 8;
    }
    V_DrawPatchDirect(xx, y, 0, W_CacheLumpName("M_THERMR", PU_CACHE));

    V_DrawPatchDirect((x + 8) + thermDot * 8, y,
                      0, W_CacheLumpName("M_THERMO", PU_CACHE));
}


void M_DrawEmptyCell(menu_t* menu, int item)
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0,
                      W_CacheLumpName("M_CELL1", PU_CACHE));
}


void M_DrawSelCell(menu_t* menu, int item)
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0,
                      W_CacheLumpName("M_CELL2", PU_CACHE));
}


void M_StartMessage(char* string, void* routine, doom_boolean input)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}


void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}


//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    int i;
    int w = 0;
    int c;

    for (i = 0; i < doom_strlen(string); i++)
    {
        c = doom_toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT(hu_font[c]->width);
    }

    return w;
}


//
// Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    int i;
    int h;
    int height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0; i < doom_strlen(string); i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// Write a string using the hu_font
//
void M_WriteText(int x, int y, char* string)
{
    int w;
    char* ch;
    int c;
    int cx;
    int cy;

    ch = string;
    cx = x;
    cy = y;

    while (1)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = doom_toupper(c) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        if (cx + w > SCREENWIDTH)
            break;
        V_DrawPatchDirect(cx, cy, 0, hu_font[c]);
        cx += w;
    }
}


//
// CONTROL PANEL
//

//
// M_Responder
//
doom_boolean M_Responder(event_t* ev)
{
    int             ch;
    int             i;
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

    ch = -1;

    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
        if (ev->data3 == -1)
        {
            ch = KEY_UPARROW;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 == 1)
        {
            ch = KEY_DOWNARROW;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 == -1)
        {
            ch = KEY_LEFTARROW;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 == 1)
        {
            ch = KEY_RIGHTARROW;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1 & 1)
        {
            ch = KEY_ENTER;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1 & 2)
        {
            ch = KEY_BACKSPACE;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty - 30)
            {
                ch = KEY_DOWNARROW;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty + 30)
            {
                ch = KEY_UPARROW;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;
            if (mousex < lastx - 30)
            {
                ch = KEY_LEFTARROW;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx + 30)
            {
                ch = KEY_RIGHTARROW;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1 & 1)
            {
                ch = KEY_ENTER;
                mousewait = I_GetTime() + 15;
            }

            if (ev->data1 & 2)
            {
                ch = KEY_BACKSPACE;
                mousewait = I_GetTime() + 15;
            }
        }
        else
            if (ev->type == ev_keydown)
            {
                ch = ev->data1;
            }
    }

    if (ch == -1)
        return false;


    // Save Game string input
    if (saveStringEnter)
    {
        switch (ch)
        {
            case KEY_BACKSPACE:
                if (saveCharIndex > 0)
                {
                    saveCharIndex--;
                    savegamestrings[saveSlot][saveCharIndex] = 0;
                }
                break;

            case KEY_ESCAPE:
                saveStringEnter = 0;
                doom_strcpy(&savegamestrings[saveSlot][0], saveOldString);
                break;

            case KEY_ENTER:
                saveStringEnter = 0;
                if (savegamestrings[saveSlot][0])
                    M_DoSave(saveSlot);
                break;

            default:
                ch = doom_toupper(ch);
                if (ch != 32)
                    if (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE)
                        break;
                if (ch >= 32 && ch <= 127 &&
                    saveCharIndex < SAVESTRINGSIZE - 1 &&
                    M_StringWidth(savegamestrings[saveSlot]) <
                    (SAVESTRINGSIZE - 2) * 8)
                {
                    savegamestrings[saveSlot][saveCharIndex++] = ch;
                    savegamestrings[saveSlot][saveCharIndex] = 0;
                }
                break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput == true &&
            !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE))
            return false;

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(ch);

        menuactive = false;
        S_StartSound(0, sfx_swtchx);
        return true;
    }

    if (devparm && ch == KEY_F1)
    {
        G_ScreenShot();
        return true;
    }


    // F-Keys
    if (!menuactive)
        switch (ch)
        {
            case KEY_MINUS:         // Screen size down
                if (automapactive || chat_on)
                    return false;
                M_SizeDisplay(0);
                S_StartSound(0, sfx_stnmov);
                return true;

            case KEY_EQUALS:        // Screen size up
                if (automapactive || chat_on)
                    return false;
                M_SizeDisplay(1);
                S_StartSound(0, sfx_stnmov);
                return true;

            case KEY_F1:            // Help key
                M_StartControlPanel();

                if (gamemode == retail)
                    currentMenu = &ReadDef2;
                else
                    currentMenu = &ReadDef1;

                itemOn = 0;
                S_StartSound(0, sfx_swtchn);
                return true;

            case KEY_F2:            // Save
                M_StartControlPanel();
                S_StartSound(0, sfx_swtchn);
                M_SaveGame(0);
                return true;

            case KEY_F3:            // Load
                M_StartControlPanel();
                S_StartSound(0, sfx_swtchn);
                M_LoadGame(0);
                return true;

            case KEY_F4:            // Sound Volume
                M_StartControlPanel();
                currentMenu = &SoundDef;
                itemOn = sfx_vol;
                S_StartSound(0, sfx_swtchn);
                return true;

            // case KEY_F5:            // Detail toggle
            //     M_ChangeDetail(0);
            //     S_StartSound(0, sfx_swtchn);
            //     return true;

            case KEY_F5:            // Crosshair toggle
                M_ChangeCrosshair(0);
                S_StartSound(0, sfx_swtchn);
                return true;

            case KEY_F6:            // Quicksave
                S_StartSound(0, sfx_swtchn);
                M_QuickSave();
                return true;

            case KEY_F7:            // End game
                S_StartSound(0, sfx_swtchn);
                M_EndGame(0);
                return true;

            case KEY_F8:            // Toggle messages
                M_ChangeMessages(0);
                S_StartSound(0, sfx_swtchn);
                return true;

            case KEY_F9:            // Quickload
                S_StartSound(0, sfx_swtchn);
                M_QuickLoad();
                return true;

            case KEY_F10:           // Quit DOOM
                S_StartSound(0, sfx_swtchn);
                M_QuitDOOM(0);
                return true;

            case KEY_F11:           // gamma toggle
                usegamma++;
                if (usegamma > 4)
                    usegamma = 0;
                players[consoleplayer].message = gammamsg[usegamma];
                I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
                return true;

        }


    // Pop-up menu?
    if (!menuactive)
    {
        if (ch == KEY_ESCAPE)
        {
            M_StartControlPanel();
            S_StartSound(0, sfx_swtchn);
            return true;
        }
        return false;
    }


    // Keys usable within menu
    switch (ch)
    {
        case KEY_DOWNARROW:
            do
            {
                if (itemOn + 1 > currentMenu->numitems - 1)
                    itemOn = 0;
                else itemOn++;
                S_StartSound(0, sfx_pstop);
            } while (currentMenu->menuitems[itemOn].status == -1);
            return true;

        case KEY_UPARROW:
            do
            {
                if (!itemOn)
                    itemOn = currentMenu->numitems - 1;
                else itemOn--;
                S_StartSound(0, sfx_pstop);
            } while (currentMenu->menuitems[itemOn].status == -1);
            return true;

        case KEY_LEFTARROW:
            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
            {
                S_StartSound(0, sfx_stnmov);
                currentMenu->menuitems[itemOn].routine(0);
            }
            return true;

        case KEY_RIGHTARROW:
            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
            {
                S_StartSound(0, sfx_stnmov);
                currentMenu->menuitems[itemOn].routine(1);
            }
            return true;

        case KEY_ENTER:
            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status)
            {
                currentMenu->lastOn = itemOn;
                if (currentMenu->menuitems[itemOn].status == 2)
                {
                    currentMenu->menuitems[itemOn].routine(1);      // right arrow
                    S_StartSound(0, sfx_stnmov);
                }
                else
                {
                    currentMenu->menuitems[itemOn].routine(itemOn);
                    S_StartSound(0, sfx_pistol);
                }
            }
            return true;

        case KEY_ESCAPE:
            currentMenu->lastOn = itemOn;
            M_ClearMenus();
            S_StartSound(0, sfx_swtchx);
            return true;

        case KEY_BACKSPACE:
            currentMenu->lastOn = itemOn;
            if (currentMenu->prevMenu)
            {
                currentMenu = currentMenu->prevMenu;
                itemOn = currentMenu->lastOn;
                S_StartSound(0, sfx_swtchn);
            }
            return true;

        default:
            for (i = itemOn + 1; i < currentMenu->numitems; i++)
                if (currentMenu->menuitems[i].alphaKey == ch)
                {
                    itemOn = i;
                    S_StartSound(0, sfx_pstop);
                    return true;
                }
            for (i = 0; i <= itemOn; i++)
                if (currentMenu->menuitems[i].alphaKey == ch)
                {
                    itemOn = i;
                    S_StartSound(0, sfx_pstop);
                    return true;
                }
            break;

    }

    return false;
}


//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
    static short x;
    static short y;
    short i;
    short max;
    char string[40];
    int start;

    inhelpscreens = false;


    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = 100 - M_StringHeight(messageString) / 2;
        while (*(messageString + start))
        {
            for (i = 0; i < doom_strlen(messageString + start); i++)
                if (*(messageString + start + i) == '\n')
                {
                    doom_memset(string, 0, 40);
                    doom_strncpy(string, messageString + start, i);
                    start += i + 1;
                    break;
                }

            if (i == doom_strlen(messageString + start))
            {
                doom_strcpy(string, messageString + start);
                start += i;
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }
        return;
    }

    if (!menuactive)
        return;

    // Darken background so the menu is more readable.
    if (doom_flags & DOOM_FLAG_MENU_DARKEN_BG)
    {
        extern byte* screens[5];
        extern unsigned char screen_palette[256 * 3];
        extern lighttable_t* colormaps;
        for (int j = 0, len = SCREENWIDTH * SCREENHEIGHT; j < len; ++j)
        {
            byte color = screens[0][j];
            color = colormaps[color + (20 * 256)];
            screens[0][j] = color;
        }
    }

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i = 0; i < max; i++)
    {
        menuitem_t* menuitem = currentMenu->menuitems + i;
        if (menuitem->name[0])
        {
            if (doom_strncmp(menuitem->name, "TXT_", 4) == 0)
            {
                M_DrawCustomMenuText(menuitem->name, x, y);
            }
            else
            {
                V_DrawPatchDirect(x, y, 0, W_CacheLumpName(menuitem->name, PU_CACHE));
            }
        }
        y += LINEHEIGHT;
    }


    // DRAW SKULL
    V_DrawPatchDirect(x + SKULLXOFF, currentMenu->y - 5 + itemOn * LINEHEIGHT, 0,
                      W_CacheLumpName(skullName[whichSkull], PU_CACHE));

}


//
// M_ClearMenus
//
void M_ClearMenus(void)
{
    menuactive = 0;
}


//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t* menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
void M_Ticker(void)
{
    if (--skullAnimCounter <= 0)
    {
        whichSkull ^= 1;
        skullAnimCounter = 8;
    }
}


//
// M_Init
//
void M_Init(void)
{
    doom_boolean hide_mouse = (doom_flags & DOOM_FLAG_HIDE_MOUSE_OPTIONS) ? true : false;
    doom_boolean hide_sound = ((doom_flags & DOOM_FLAG_HIDE_MUSIC_OPTIONS) && (doom_flags & DOOM_FLAG_HIDE_SOUND_OPTIONS)) ? true : false;

    OptionsMenu = OptionsMenuFull;
    if (hide_mouse && !hide_sound)
    {
        OptionsMenu = OptionsMenuNoMouse;
        doom_memcpy(&OptionsDef, &OptionsNoMouseDef, sizeof(OptionsDef));
    }
    else if (!hide_mouse && hide_sound)
    {
        OptionsMenu = OptionsMenuNoSound;
        doom_memcpy(&OptionsDef, &OptionsNoSoundDef, sizeof(OptionsDef));
    }
    else if (hide_mouse && hide_sound)
    {
        OptionsMenu = OptionsMenuNoSoundNoMouse;
        doom_memcpy(&OptionsDef, &OptionsNoSoundNoMouseDef, sizeof(OptionsDef));
    }

    SoundMenu = SoundMenuFull;
    if (doom_flags & DOOM_FLAG_HIDE_MUSIC_OPTIONS)
    {
        SoundMenu = SoundMenuNoMusic;
        doom_memcpy(&SoundDef, &SoundNoMusicDef, sizeof(SoundDef));
    }
    else if (doom_flags & DOOM_FLAG_HIDE_SOUND_OPTIONS)
    {
        SoundMenu = SoundMenuNoSFX;
        doom_memcpy(&SoundDef, &SoundNoSFXDef, sizeof(SoundDef));
    }

    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = 0;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    switch (gamemode)
    {
        case commercial:
            // This is used because DOOM 2 had only one HELP
            //  page. I use CREDIT as second page now, but
            //  kept this hack for educational purposes.
            MainMenu[readthis] = MainMenu[quitdoom];
            MainDef.numitems--;
            MainDef.y += 8;
            NewDef.prevMenu = &MainDef;
            ReadDef1.routine = M_DrawReadThis1;
            ReadDef1.x = 330;
            ReadDef1.y = 165;
            ReadMenu1[0].routine = M_FinishReadThis;
            break;
        case shareware:
            // Episode 2 and 3 are handled,
            //  branching to an ad screen.
        case registered:
            // We need to remove the fourth episode.
            EpiDef.numitems--;
            break;
        case retail:
            // We are fine.
        default:
            break;
    }
}
