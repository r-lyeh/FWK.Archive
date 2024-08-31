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
// DESCRIPTION:
//
//    
//-----------------------------------------------------------------------------

#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"


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

//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
