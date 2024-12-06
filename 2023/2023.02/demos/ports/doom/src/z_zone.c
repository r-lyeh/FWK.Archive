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
//        Zone Memory Allocation. Neat.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "z_zone.h"
#include "i_system.h"
#include "doomdef.h"


//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
// and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
// because it will get overwritten automatically if needed.
//


#define ZONEID 0x1d4a11
#define MINFRAGMENT 64


typedef struct
{
    // total bytes malloced, including header
    int size;

    // start / end cap for linked list
    memblock_t blocklist;

    memblock_t* rover;
} memzone_t;


memzone_t* mainzone;


//
// Z_ClearZone
//
void Z_ClearZone(memzone_t* zone)
{
    memblock_t* block;

    // set the entire zone to one free block
    zone->blocklist.next =
        zone->blocklist.prev =
        block = (memblock_t*)((byte*)zone + sizeof(memzone_t));

    zone->blocklist.user = (void*)zone;
    zone->blocklist.tag = PU_STATIC;
    zone->rover = block;

    block->prev = block->next = &zone->blocklist;

    // 0 indicates a free block.
    block->user = 0;

    block->size = zone->size - sizeof(memzone_t);
}


//
// Z_Init
//
void Z_Init(void)
{
    memblock_t* block;
    int size;

    mainzone = (memzone_t*)I_ZoneBase(&size);
    mainzone->size = size;

    // set the entire zone to one free block
    mainzone->blocklist.next =
        mainzone->blocklist.prev =
        block = (memblock_t*)((byte*)mainzone + sizeof(memzone_t));

    mainzone->blocklist.user = (void*)mainzone;
    mainzone->blocklist.tag = PU_STATIC;
    mainzone->rover = block;

    block->prev = block->next = &mainzone->blocklist;

    // 0 indicates a free block.
    block->user = 0;

    block->size = mainzone->size - sizeof(memzone_t);
}


//
// Z_Free
//
void Z_Free(void* ptr)
{
    memblock_t* block;
    memblock_t* other;

    block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
        I_Error("Error: Z_Free: freed a pointer without ZONEID");

    if (block->user > (void**)0x100)
    {
        // smaller values are not pointers
        // Note: OS-dependend?

        // clear the user's mark
        *block->user = 0;
    }

    // mark as free
    block->user = 0;
    block->tag = 0;
    block->id = 0;

    other = block->prev;

    if (!other->user)
    {
        // merge with previous free block
        other->size += block->size;
        other->next = block->next;
        other->next->prev = other;

        if (block == mainzone->rover)
            mainzone->rover = other;

        block = other;
    }

    other = block->next;
    if (!other->user)
    {
        // merge the next free block onto the end
        block->size += other->size;
        block->next = other->next;
        block->next->prev = block;

        if (other == mainzone->rover)
            mainzone->rover = block;
    }
}


//
// Z_Malloc
// You can pass a 0 user if the tag is < PU_PURGELEVEL.
//
void* Z_Malloc(int size, int tag, void* user)
{
    int extra;
    memblock_t* start;
    memblock_t* rover;
    memblock_t* newblock;
    memblock_t* base;

    size = (size + 3) & ~3;

    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += sizeof(memblock_t);

    // if there is a free block behind the rover,
    //  back up over them
    base = mainzone->rover;

    if (!base->prev->user)
        base = base->prev;

    rover = base;
    start = base->prev;

    do
    {
        if (rover == start)
        {
            // scanned all the way around the list
            //I_Error("Error: Z_Malloc: failed on allocation of %i bytes", size);
            doom_strcpy(error_buf, "Error: Z_Malloc: failed on allocation of ");
            doom_concat(error_buf, doom_itoa(size, 10));
            doom_concat(error_buf, " bytes");
            I_Error(error_buf);
        }

        if (rover->user)
        {
            if (rover->tag < PU_PURGELEVEL)
            {
                // hit a block that can't be purged,
                //  so move base past it
                base = rover = rover->next;
            }
            else
            {
                // free the rover block (adding the size to base)

                // the rover can be the base block
                base = base->prev;
                Z_Free((byte*)rover + sizeof(memblock_t));
                base = base->next;
                rover = base->next;
            }
        }
        else
            rover = rover->next;
    } while (base->user || base->size < size);


    // found a block big enough
    extra = base->size - size;

    if (extra > MINFRAGMENT)
    {
        // there will be a free fragment after the allocated block
        newblock = (memblock_t*)((byte*)base + size);
        newblock->size = extra;

        // 0 indicates free block.
        newblock->user = 0;
        newblock->tag = 0;
        newblock->prev = base;
        newblock->next = base->next;
        newblock->next->prev = newblock;

        base->next = newblock;
        base->size = size;
    }

    if (user)
    {
        // mark as an in use block
        base->user = user;
        *(void**)user = (void*)((byte*)base + sizeof(memblock_t));
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            I_Error("Error: Z_Malloc: an owner is required for purgable blocks");

        // mark as in use, but unowned        
        base->user = (void*)2;
    }
    base->tag = tag;

    // next allocation will start looking here
    mainzone->rover = base->next;

    base->id = ZONEID;

    return (void*)((byte*)base + sizeof(memblock_t));
}


//
// Z_FreeTags
//
void Z_FreeTags(int lowtag, int hightag)
{
    memblock_t* block;
    memblock_t* next;

    for (block = mainzone->blocklist.next;
         block != &mainzone->blocklist;
         block = next)
    {
        // get link before freeing
        next = block->next;

        // free block?
        if (!block->user)
            continue;

        if (block->tag >= lowtag && block->tag <= hightag)
            Z_Free((byte*)block + sizeof(memblock_t));
    }
}


//
// Z_DumpHeap
// Note: TFileDumpHeap( stdout ) ?
//
void Z_DumpHeap(int lowtag, int hightag)
{
    memblock_t* block;

    //doom_print("zone size: %i  location: %p\n",
    //       mainzone->size, mainzone);
    doom_print("zone size: ");
    doom_print(doom_itoa(mainzone->size, 10));
    doom_print("  location: ");
    doom_print(doom_ptoa(mainzone));
    doom_print("\n");

    //doom_print("tag range: %i to %i\n",
    //       lowtag, hightag);
    doom_print("tag range: ");
    doom_print(doom_itoa(lowtag, 10));
    doom_print(" to ");
    doom_print(doom_itoa(hightag, 10));
    doom_print("\n");

    for (block = mainzone->blocklist.next; ; block = block->next)
    {
        if (block->tag >= lowtag && block->tag <= hightag)
        {
            //doom_print("block:%p    size:%7i    user:%p    tag:%3i\n", block, block->size, block->user, block->tag);
            doom_print("block");
            doom_print(doom_ptoa(block));
            doom_print("    size:");
            doom_print(doom_itoa(block->size, 10));
            doom_print("    user:");
            doom_print(doom_ptoa(block->user));
            doom_print("    tag:");
            doom_print(doom_itoa(block->tag, 10));
            doom_print("\n");
        }

        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte*)block + block->size != (byte*)block->next)
            doom_print("ERROR: block size does not touch the next block\n");

        if (block->next->prev != block)
            doom_print("ERROR: next block doesn't have proper back link\n");

        if (!block->user && !block->next->user)
            doom_print("ERROR: two consecutive free blocks\n");
    }
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap(void* f)
{
    memblock_t* block;

    //fprintf(f, "zone size: %i  location: %p\n", mainzone->size, mainzone);
    doom_fprint(f, "zone size: ");
    doom_fprint(f, doom_itoa(mainzone->size, 10));
    doom_fprint(f, "  location: ");
    doom_fprint(f, doom_ptoa(mainzone));
    doom_fprint(f, "\n");

    for (block = mainzone->blocklist.next; ; block = block->next)
    {
        //fprintf(f, "block:%p    size:%7i    user:%p    tag:%3i\n",
        //        block, block->size, block->user, block->tag);
        doom_fprint(f, "block:");
        doom_fprint(f, doom_ptoa(block));
        doom_fprint(f, "    size:");
        doom_fprint(f, doom_itoa(block->size, 10)); // Supposed to alignt 7, this is not worth the trouble
        doom_fprint(f, "    user:");
        doom_fprint(f, doom_ptoa(block->user));
        doom_fprint(f, "    tag:");
        doom_fprint(f, doom_itoa(block->tag, 10));
        doom_fprint(f, "\n");


        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte*)block + block->size != (byte*)block->next)
            doom_fprint(f, "ERROR: block size does not touch the next block\n");

        if (block->next->prev != block)
            doom_fprint(f, "ERROR: next block doesn't have proper back link\n");

        if (!block->user && !block->next->user)
            doom_fprint(f, "ERROR: two consecutive free blocks\n");
    }
}


//
// Z_CheckHeap
//
void Z_CheckHeap(void)
{
    memblock_t* block;

    for (block = mainzone->blocklist.next; ; block = block->next)
    {
        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte*)block + block->size != (byte*)block->next)
            I_Error("Error: Z_CheckHeap: block size does not touch the next block\n");

        if (block->next->prev != block)
            I_Error("Error: Z_CheckHeap: next block doesn't have proper back link\n");

        if (!block->user && !block->next->user)
            I_Error("Error: Z_CheckHeap: two consecutive free blocks\n");
    }
}


//
// Z_ChangeTag
//
void Z_ChangeTag2(void* ptr, int tag)
{
    memblock_t* block;

    block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
        I_Error("Error: Z_ChangeTag: freed a pointer without ZONEID");

    if (tag >= PU_PURGELEVEL && (unsigned long long)block->user < 0x100)
        I_Error("Error: Z_ChangeTag: an owner is required for purgable blocks");

    block->tag = tag;
}


//
// Z_FreeMemory
//
int Z_FreeMemory(void)
{
    memblock_t* block;
    int free;

    free = 0;

    for (block = mainzone->blocklist.next;
         block != &mainzone->blocklist;
         block = block->next)
    {
        if (!block->user || block->tag >= PU_PURGELEVEL)
            free += block->size;
    }
    return free;
}
