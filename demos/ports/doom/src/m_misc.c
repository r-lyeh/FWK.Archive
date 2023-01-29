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
//
// $Log:$
//
// DESCRIPTION:
//        Main loop menu stuff.
//        Default Config File.
//        PCX Screenshots.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "doomdef.h"
#include "z_zone.h"
#include "m_swap.h"
#include "m_argv.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "doomstat.h" // State.
#include "dstrings.h" // Data.
#include "m_misc.h"


#ifndef O_BINARY
#define O_BINARY 0
#endif
#define STRING_VALUE 0xFFFF


//
// SCREEN SHOTS
//
typedef struct
{
    char manufacturer;
    char version;
    char encoding;
    char bits_per_pixel;

    unsigned short xmin;
    unsigned short ymin;
    unsigned short xmax;
    unsigned short ymax;

    unsigned short hres;
    unsigned short vres;

    unsigned char palette[48];

    char reserved;
    char color_planes;
    unsigned short bytes_per_line;
    unsigned short palette_type;

    char filler[58];
    unsigned char data; // unbounded
} pcx_t;


//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t* hu_font[HU_FONTSIZE];


//
// DEFAULTS
//
extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;

extern int key_strafeleft;
extern int key_straferight;

extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;
extern int mousemove;

extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;

extern int viewwidth;
extern int viewheight;

extern int mouseSensitivity;
extern int showMessages;

extern int detailLevel;

extern int screenblocks;

extern int showMessages;

// machine-independent sound params
extern int numChannels;

extern char* chat_macros[];

extern byte scantokey[128];


int usemouse;
int usejoystick;
int crosshair;
int always_run;


default_t defaults[] =
{
    {"mouse_sensitivity",&mouseSensitivity, 5},
    {"sfx_volume",&snd_SfxVolume, 8},
    {"music_volume",&snd_MusicVolume, 8},
    {"show_messages",&showMessages, 1},

    {"key_right",&key_right, KEY_RIGHTARROW},
    {"key_left",&key_left, KEY_LEFTARROW},
    {"key_up",&key_up, KEY_UPARROW},
    {"key_down",&key_down, KEY_DOWNARROW},
    {"key_strafeleft",&key_strafeleft, ','},
    {"key_straferight",&key_straferight, '.'},

    {"key_fire",&key_fire, KEY_RCTRL},
    {"key_use",&key_use, ' '},
    {"key_strafe",&key_strafe, KEY_RALT},
    {"key_speed",&key_speed, KEY_RSHIFT},

    {"use_mouse",&usemouse, 1},
    {"mouseb_fire",&mousebfire,0},
    {"mouseb_strafe",&mousebstrafe,1},
    {"mouseb_forward",&mousebforward,2},
    {"mouse_move",&mousemove,0},

    {"use_joystick",&usejoystick, 0},
    {"joyb_fire",&joybfire,0},
    {"joyb_strafe",&joybstrafe,1},
    {"joyb_use",&joybuse,3},
    {"joyb_speed",&joybspeed,2},

    {"screenblocks",&screenblocks, 9},
    {"detaillevel",&detailLevel, 0},
    {"crosshair",&crosshair, 0},
    {"always_run",&always_run, 0},

    {"snd_channels",&numChannels, 3},

    {"usegamma",&usegamma, 0},

    {"chatmacro0", 0, STRING_VALUE, 0, 0, &chat_macros[0], HUSTR_CHATMACRO0 },
    {"chatmacro1", 0, STRING_VALUE, 0, 0, &chat_macros[1], HUSTR_CHATMACRO1 },
    {"chatmacro2", 0, STRING_VALUE, 0, 0, &chat_macros[2], HUSTR_CHATMACRO2 },
    {"chatmacro3", 0, STRING_VALUE, 0, 0, &chat_macros[3], HUSTR_CHATMACRO3 },
    {"chatmacro4", 0, STRING_VALUE, 0, 0, &chat_macros[4], HUSTR_CHATMACRO4 },
    {"chatmacro5", 0, STRING_VALUE, 0, 0, &chat_macros[5], HUSTR_CHATMACRO5 },
    {"chatmacro6", 0, STRING_VALUE, 0, 0, &chat_macros[6], HUSTR_CHATMACRO6 },
    {"chatmacro7", 0, STRING_VALUE, 0, 0, &chat_macros[7], HUSTR_CHATMACRO7 },
    {"chatmacro8", 0, STRING_VALUE, 0, 0, &chat_macros[8], HUSTR_CHATMACRO8 },
    {"chatmacro9", 0, STRING_VALUE, 0, 0, &chat_macros[9], HUSTR_CHATMACRO9 }
};


int numdefaults = sizeof(defaults) / sizeof(default_t);;
char* defaultfile;


int M_DrawText(int x, int y, doom_boolean direct, char* string)
{
    int c;
    int w;

    while (*string)
    {
        c = doom_toupper(*string) - HU_FONTSTART;
        string++;
        if (c < 0 || c> HU_FONTSIZE)
        {
            x += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        if (x + w > SCREENWIDTH)
            break;
        if (direct)
            V_DrawPatchDirect(x, y, 0, hu_font[c]);
        else
            V_DrawPatch(x, y, 0, hu_font[c]);
        x += w;
    }

    return x;
}


//
// M_WriteFile
//
doom_boolean M_WriteFile(char const* name, void* source, int length)
{
    void* handle;
    int count;

    handle = doom_open(name, "wb");

    if (handle == 0)
        return false;

    count = doom_write(handle, source, length);
    doom_close(handle);

    if (count < length)
        return false;

    return true;
}


//
// M_ReadFile
//
int M_ReadFile(char const* name, byte** buffer)
{
    void* handle;
    int count, length;
    byte* buf;

    handle = doom_open(name, "rb");
    if (handle == 0)
    {
        //I_Error("Error: Couldn't read file %s", name);
        
        doom_strcpy(error_buf, "Error: Couldn't read file ");
        doom_concat(error_buf, name);
        I_Error(error_buf);
    }
    doom_seek(handle, 0, DOOM_SEEK_END);
    length = doom_tell(handle);
    doom_seek(handle, 0, DOOM_SEEK_SET);
    buf = Z_Malloc(length, PU_STATIC, 0);
    count = doom_read(handle, buf, length);
    doom_close(handle);

    if (count < length)
    {
        //I_Error("Error: Couldn't read file %s", name);
        
        doom_strcpy(error_buf, "Error: Couldn't read file ");
        doom_concat(error_buf, name);
        I_Error(error_buf);
    }

    *buffer = buf;
    return length;
}


//
// M_SaveDefaults
//
void M_SaveDefaults(void)
{
    int i;
    int v;
    void* f;

    f = doom_open(defaultfile, "w");
    if (!f)
        return; // can't write the file, but don't complain

    for (i = 0; i < numdefaults; i++)
    {
        if (defaults[i].defaultvalue > -0xfff
            && defaults[i].defaultvalue < 0xfff)
        {
            v = *defaults[i].location;
            //fprintf(f, "%s\t\t%i\n", defaults[i].name, v);
            doom_fprint(f, defaults[i].name);
            doom_fprint(f, "\t\t");
            doom_fprint(f, doom_itoa(v, 10));
            doom_fprint(f, "\n");
        }
        else
        {
            //fprintf(f, "%s\t\t\"%s\"\n", defaults[i].name,
            //        *(char**)(defaults[i].text_location));
            doom_fprint(f, defaults[i].name);
            doom_fprint(f, "\t\t\"");
            doom_fprint(f, *(char**)(defaults[i].text_location));
            doom_fprint(f, "\"\n");
        }
    }

    doom_close(f);
}


//
// M_LoadDefaults
//
void M_LoadDefaults(void)
{
    int i;
    int len;
    void* f;
    char def[80];
    char strparm[100];
    char* newstring;
    int parm;
    doom_boolean isstring;

    // set everything to base values
    // numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i = 0; i < numdefaults; i++)
    {
        if (defaults[i].defaultvalue == 0xFFFF)
            *defaults[i].text_location = defaults[i].default_text_value;
        else
            *defaults[i].location = (int)defaults[i].defaultvalue;
    }

    // check for a custom default file
    i = M_CheckParm("-config");
    if (i && i < myargc - 1)
    {
        defaultfile = myargv[i + 1];
        //doom_print("        default file: %s\n", defaultfile);
        doom_print("        default file: ");
        doom_print(defaultfile);
        doom_print("\n");
    }
    else
        defaultfile = basedefault;

    // read the file in, overriding any set defaults
    f = doom_open(defaultfile, "r");
    if (f)
    {
        while (!doom_eof(f))
        {
            // def
            int arg_read = 0;
            char c;
            for (i = 0; i < 79; ++i)
            {
                doom_read(f, &c, 1);
                if (c == ' ' || c == '\n' || c == '\t')
                {
                    if (i > 0) arg_read++;
                    break;
                }
                def[i] = c;
            }
            def[i] = '\0';

            // Ignore spaces
            if (c != '\n')
            {
                while (1)
                {
                    doom_read(f, &c, 1);
                    if (c != ' ' && c != '\t') break;
                }

                // strparam
                i = 0;
                if (c != '\n')
                {
                    for (; i < 260;)
                    {
                        strparm[i++] = c;
                        doom_read(f, &c, 1);
                        if (c == '\n')
                        {
                            if (i > 0) arg_read++;
                            break;
                        }
                    }
                }
                strparm[i] = '\0';
            }

            isstring = false;
            //if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
            if (arg_read == 2)
            {
                if (strparm[0] == '"')
                {
                    // get a string default
                    isstring = true;
                    len = (int)doom_strlen(strparm);
                    newstring = (char*)doom_malloc(len);
                    strparm[len - 1] = 0;
                    doom_strcpy(newstring, strparm + 1);
                }
                else if (strparm[0] == '0' && strparm[1] == 'x')
                {
                    //sscanf(strparm + 2, "%x", &parm);
                    parm = doom_atox(strparm + 2);
                }
                else
                {
                    //sscanf(strparm, "%i", &parm);
                    parm = doom_atoi(strparm);
                }
                for (i = 0; i < numdefaults; i++)
                    if (!doom_strcmp(def, defaults[i].name))
                    {
                        if (!isstring)
                            *defaults[i].location = parm;
                        else
                            *defaults[i].text_location = newstring;
                        break;
                    }
            }
        }

        doom_close(f);
    }
}


//
// WritePCXfile
//
void WritePCXfile(char* filename, byte* data, int width, int height, byte* palette)
{
    int i;
    int length;
    pcx_t* pcx;
    byte* pack;

    pcx = Z_Malloc(width * height * 2 + 1000, PU_STATIC, 0);

    pcx->manufacturer = 0x0a; // PCX id
    pcx->version = 5; // 256 color
    pcx->encoding = 1; // uncompressed
    pcx->bits_per_pixel = 8; // 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width - 1);
    pcx->ymax = SHORT(height - 1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    doom_memset(pcx->palette, 0, sizeof(pcx->palette));
    pcx->color_planes = 1; // chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2); // not a grey scale
    doom_memset(pcx->filler, 0, sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;

    for (i = 0; i < width * height; i++)
    {
        if ((*data & 0xc0) != 0xc0)
            *pack++ = *data++;
        else
        {
            *pack++ = 0xc1;
            *pack++ = *data++;
        }
    }

    // write the palette
    *pack++ = 0x0c;        // palette ID byte
    for (i = 0; i < 768; i++)
        *pack++ = *palette++;

    // write output file
    length = (int)(pack - (byte*)pcx);
    M_WriteFile(filename, pcx, length);

    Z_Free(pcx);
}


//
// M_ScreenShot
//
void M_ScreenShot(void)
{
    int i;
    byte* linear;
    char lbmname[12];
    void* f;

    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen(linear);

    // find a file name to save it to
    doom_strcpy(lbmname, "DOOM00.pcx");

    for (i = 0; i <= 99; i++)
    {
        lbmname[4] = i / 10 + '0';
        lbmname[5] = i % 10 + '0';
        if ((f = doom_open(lbmname, "wb")) == 0)
            break;        // file doesn't exist
        doom_close(f);
    }
    if (i == 100)
        I_Error("Error: M_ScreenShot: Couldn't create a PCX");

    // save the pcx file
    WritePCXfile(lbmname, linear,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName("PLAYPAL", PU_CACHE));

    players[consoleplayer].message = "screen shot";
}
