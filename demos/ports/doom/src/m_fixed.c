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
//        Fixed point implementation.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "stdlib.h"
#include "doomtype.h"
#include "i_system.h"
#include "m_fixed.h"


fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return ((long long)a * (long long)b) >> FRACBITS;
}


//
// FixedDiv, C version.
//
fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((doom_abs(a) >> 14) >= doom_abs(b))
        return (a ^ b) < 0 ? DOOM_MININT : DOOM_MAXINT;
    return FixedDiv2(a, b);
}


fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
    double c;

    c = ((double)a) / ((double)b) * FRACUNIT;

    if (c >= 2147483648.0 || c < -2147483648.0)
        I_Error("Error: FixedDiv: divide by zero");
    return (fixed_t)c;
}
