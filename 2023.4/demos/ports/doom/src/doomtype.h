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
//        Simple basic typedefs, isolated here to make it easier
//         separating modules.
//    
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
