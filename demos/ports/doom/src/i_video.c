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
//        DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"



#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"
#include "doomdef.h"


#define POINTER_WARP_COUNTDOWN 1


// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
doom_boolean grabMouse;
int doPointerWarp = POINTER_WARP_COUNTDOWN;

unsigned char screen_palette[256 * 3];

doom_boolean mousemoved = false;
doom_boolean shmFinished;


// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int multiply = 1;
static int lastmousex = 0;
static int lastmousey = 0;


void I_ShutdownGraphics(void)
{
}


//
// I_StartFrame
//
void I_StartFrame(void)
{
}


void I_GetEvent(void)
{
}


//
// I_StartTic
//
void I_StartTic(void)
{
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
    // what is this?
}


//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    static int lasttic;
    int tics;
    int i;

    // draws little dots on the bottom of the screen
    if (devparm)
    {
        i = I_GetTime();
        tics = i - lasttic;
        lasttic = i;
        if (tics > 20) tics = 20;

        for (i = 0; i < tics * 2; i += 2)
            screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0xff;
        for (; i < 20 * 2; i += 2)
            screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0x0;

    }
}


//
// I_ReadScreen
//
void I_ReadScreen(byte* scr)
{
    doom_memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette(byte* palette)
{
    doom_memcpy(screen_palette, palette, 256 * 3);
}


void I_InitGraphics(void)
{
    screens[0] = (unsigned char*)doom_malloc(SCREENWIDTH * SCREENHEIGHT);
}
