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
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "d_net.h"
#include "g_game.h"
#include "i_system.h"


int mb_used = 6 * (sizeof(void*) / 4);
ticcmd_t emptycmd;


extern doom_boolean demorecording;


void I_Tactile(int on, int off, int total)
{
}


ticcmd_t* I_BaseTiccmd(void)
{
    return &emptycmd;
}


int I_GetHeapSize(void)
{
    return mb_used * 1024 * 1024;
}


byte* I_ZoneBase(int* size)
{
    *size = mb_used * 1024 * 1024;
    return (byte*)doom_malloc(*size);
}


//
// I_GetTime
// returns time in 1/70th second tics
//
int I_GetTime(void)
{
    int sec, usec;
    int newtics;
    static int basetime = 0;

    doom_gettime(&sec, &usec);
    if (!basetime)
        basetime = sec;
    newtics = (sec - basetime) * TICRATE + usec * TICRATE / 1000000;
    return newtics;
}


//
// I_Init
//
void I_Init(void)
{
    I_InitSound();
}


//
// I_Quit
//
void I_Quit(void)
{
    D_QuitNetGame();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults();
    I_ShutdownGraphics();
    doom_exit(0);
}


void I_WaitVBL(int count)
{
#if 0 // [pd] Never sleep in main thread
#ifdef SGI
    sginap(1);
#else
#ifdef SUN
    sleep(0);
#else
    usleep(count * (1000000 / 70));
#endif
#endif
#endif
}


void I_BeginRead(void)
{
}


void I_EndRead(void)
{
}


byte* I_AllocLow(int length)
{
    byte* mem;

    mem = (byte*)doom_malloc(length);
    doom_memset(mem, 0, length);
    return mem;
}


//
// I_Error
//
void I_Error(char* error)
{
    // Message first.
    if (error) doom_print(error);
    doom_print("\n");

    // Shutdown. Here might be other errors.
    if (demorecording)
        G_CheckDemoStatus();

    D_QuitNetGame();
    I_ShutdownGraphics();

    doom_exit(-1);
}
