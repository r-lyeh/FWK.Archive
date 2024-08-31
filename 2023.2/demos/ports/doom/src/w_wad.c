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
//        Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------


#include "doom_config.h"

#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"


// #define strcmpi strcasecmp


//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t* lumpinfo;
int numlumps;
void** lumpcache;
int reloadlump;
char* reloadname;
int info[2500][10];
int profilecount;


void doom_strupr(char* s)
{
    while (*s)
    {
        *s = doom_toupper(*s); s++;
    }
}


void ExtractFileBase(char* path, char* dest)
{
    char* src;
    int length;

    src = path + doom_strlen(path) - 1;

    // back up until a \ or the start
    while (src != path
           && *(src - 1) != '\\'
           && *(src - 1) != '/')
    {
        src--;
    }

    // copy up to eight characters
    doom_memset(dest, 0, 8);
    length = 0;

    while (*src && *src != '.')
    {
        if (++length == 9)
        {
            //I_Error("Error: Filename base of %s >8 chars", path);
            doom_strcpy(error_buf, "Error: Filename base of ");
            doom_concat(error_buf, path);
            doom_concat(error_buf, " >8 chars");
            I_Error(error_buf);
        }

        *dest++ = doom_toupper((int)*src++);
    }
}


//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...
void W_AddFile(char* filename)
{
    wadinfo_t header;
    lumpinfo_t* lump_p;
    unsigned i;
    void* handle;
    int length;
    int startlump;
    filelump_t* fileinfo;
    filelump_t singleinfo;
    void* storehandle;
    void* allocated = 0;

    // open the file and add to directory

    // handle reload indicator.
    if (filename[0] == '~')
    {
        filename++;
        reloadname = filename;
        reloadlump = numlumps;
    }

    if ((handle = doom_open(filename, "rb")) == 0)
    {
        //doom_print(" couldn't open %s\n", filename);
        doom_print(" couldn't open ");
        doom_print(filename);
        doom_print("\n");
        return;
    }

    //doom_print(" adding %s\n", filename);
    doom_print(" adding ");
    doom_print(filename);
    doom_print("\n");
    startlump = numlumps;

    if (doom_strcasecmp(filename + doom_strlen(filename) - 3, "wad"))
    {
        // single lump file
        fileinfo = &singleinfo;
        singleinfo.filepos = 0;
        //singleinfo.size = LONG(filelength(handle));
        doom_seek(handle, 0, DOOM_SEEK_END);
        singleinfo.size = doom_tell(handle);
        doom_seek(handle, 0, DOOM_SEEK_SET);
        ExtractFileBase(filename, singleinfo.name);
        numlumps++;
    }
    else
    {
        // WAD file
        doom_read(handle, &header, sizeof(header));
        if (doom_strncmp(header.identification, "IWAD", 4))
        {
            // Homebrew levels?
            if (doom_strncmp(header.identification, "PWAD", 4))
            {
                //I_Error("Error: Wad file %s doesn't have IWAD "
                //        "or PWAD id\n", filename);
                doom_strcpy(error_buf, "Error: Wad file ");
                doom_concat(error_buf, filename);
                doom_concat(error_buf, " doesn't have IWAD or PWAD id\n");
                I_Error(error_buf);
            }

            // ???modifiedgame = true;                
        }
        header.numlumps = LONG(header.numlumps);
        header.infotableofs = LONG(header.infotableofs);
        length = header.numlumps * sizeof(filelump_t);
        fileinfo = allocated = doom_malloc(length);
        doom_seek(handle, header.infotableofs, DOOM_SEEK_SET);
        doom_read(handle, fileinfo, length);
        numlumps += header.numlumps;
    }


    // Fill in lumpinfo
    static int previous_realloc_size = 1;
    void* new_lumpinfo = doom_malloc(numlumps * sizeof(lumpinfo_t));
    doom_memcpy(new_lumpinfo, lumpinfo, previous_realloc_size);
    previous_realloc_size = numlumps * sizeof(lumpinfo_t);
    lumpinfo = new_lumpinfo;

    if (!lumpinfo)
        I_Error("Error: Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];

    storehandle = reloadname ? 0 : handle;

    for (i = startlump; i < (unsigned)numlumps; i++, lump_p++, fileinfo++)
    {
        lump_p->handle = storehandle;
        lump_p->position = LONG(fileinfo->filepos);
        lump_p->size = LONG(fileinfo->size);
        doom_strncpy(lump_p->name, fileinfo->name, 8);
    }

    if (reloadname)
        doom_close(handle);

    if (allocated) doom_free(allocated);
}


//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload(void)
{
    wadinfo_t header;
    int lumpcount;
    lumpinfo_t* lump_p;
    unsigned i;
    void* handle;
    int length;
    filelump_t* fileinfo;

    if (!reloadname)
        return;

    if ((handle = doom_open(reloadname, "rb")) == 0)
    {
        //I_Error("Error: W_Reload: couldn't open %s", reloadname);
        doom_strcpy(error_buf, "Error: W_Reload: couldn't open ");
        doom_concat(error_buf, reloadname);
        I_Error(error_buf);
    }

    doom_read(handle, &header, sizeof(header));
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount * sizeof(filelump_t);
    fileinfo = doom_malloc(length);
    doom_seek(handle, header.infotableofs, DOOM_SEEK_SET);
    doom_read(handle, fileinfo, length);

    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];

    for (i = reloadlump;
         i < (unsigned)(reloadlump + lumpcount);
         i++, lump_p++, fileinfo++)
    {
        if (lumpcache[i])
            Z_Free(lumpcache[i]);

        lump_p->position = LONG(fileinfo->filepos);
        lump_p->size = LONG(fileinfo->size);
    }

    doom_close(handle);

    doom_free(fileinfo);
}


//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
void W_InitMultipleFiles(char** filenames)
{
    int                size;

    // open all the files, load headers, and count lumps
    numlumps = 0;

    // will be realloced as lumps are added
    lumpinfo = doom_malloc(1);

    for (; *filenames; filenames++)
        W_AddFile(*filenames);

    if (!numlumps)
        I_Error("Error: W_InitFiles: no files found");

    // set up caching
    size = numlumps * sizeof(*lumpcache);
    lumpcache = doom_malloc(size);

    if (!lumpcache)
        I_Error("Error: Couldn't allocate lumpcache");

    doom_memset(lumpcache, 0, size);
}


//
// W_InitFile
// Just initialize from a single file.
//
void W_InitFile(char* filename)
{
    char* names[2];

    names[0] = filename;
    names[1] = 0;
    W_InitMultipleFiles(names);
}


//
// W_NumLumps
//
int W_NumLumps(void)
{
    return numlumps;
}


//
// W_CheckNumForName
// Returns -1 if name not found.
//
int W_CheckNumForName(char* name)
{
    union
    {
        char s[9];
        int x[2];
    } name8;

    int v1;
    int v2;
    lumpinfo_t* lump_p;

    // make the name into two integers for easy compares
    doom_strncpy(name8.s, name, 8);

    // in case the name was a fill 8 chars
    name8.s[8] = 0;

    // case insensitive
    doom_strupr(name8.s);

    v1 = name8.x[0];
    v2 = name8.x[1];


    // scan backwards so patch lump files take precedence
    lump_p = lumpinfo + numlumps;

    while (lump_p-- != lumpinfo)
    {
        if (*(int*)lump_p->name == v1
            && *(int*)&lump_p->name[4] == v2)
        {
            return (int)(lump_p - lumpinfo);
        }
    }

    // TFB. Not found.
    return -1;
}


//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(char* name)
{
    int i;

    i = W_CheckNumForName(name);

    if (i == -1)
    {
        if (doom_strcmp(name, "HELP2") == 0)
        {
            name = "HELP1"; // Ultimate Doom EXE was modified to use this instead
            i = W_CheckNumForName(name);
        }
        if (i == -1)
        {
            //I_Error("Error: W_GetNumForName, %s not found!", name);
            doom_strcpy(error_buf, "Error: W_GetNumForName, ");
            doom_concat(error_buf, name);
            doom_concat(error_buf, " not found!");
            I_Error(error_buf);
        }
    }

    return i;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(int lump)
{
    if (lump >= numlumps)
    {
        //I_Error("Error: W_LumpLength: %i >= numlumps", lump);
        doom_strcpy(error_buf, "Error: W_LumpLength: ");
        doom_concat(error_buf, doom_itoa(lump, 10));
        doom_concat(error_buf, " >= numlumps");
        I_Error(error_buf);
    }

    return lumpinfo[lump].size;
}


//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(int lump, void* dest)
{
    int c;
    lumpinfo_t* l;
    void* handle;

    if (lump >= numlumps)
    {
        //I_Error("Error: W_ReadLump: %i >= numlumps", lump);
        doom_strcpy(error_buf, "Error: W_ReadLump: ");
        doom_concat(error_buf, doom_itoa(lump, 10));
        doom_concat(error_buf, " >= numlump");
        I_Error(error_buf);
    }

    l = lumpinfo + lump;

    // ??? I_BeginRead ();

    if (l->handle == 0)
    {
        // reloadable file, so use open / read / close
        if ((handle = doom_open(reloadname, "rb")) == 0)
        {
            //I_Error("Error: W_ReadLump: couldn't open %s", reloadname);
            doom_strcpy(error_buf, "Error: W_ReadLump: couldn't open ");
            doom_concat(error_buf, reloadname);
            I_Error(error_buf);
        }
    }
    else
        handle = l->handle;

    doom_seek(handle, l->position, DOOM_SEEK_SET);
    c = doom_read(handle, dest, l->size);

    if (c < l->size)
    {
        //I_Error("Error: W_ReadLump: only read %i of %i on lump %i",
        //        c, l->size, lump);
        doom_strcpy(error_buf, "Error: W_ReadLump: only read ");
        doom_concat(error_buf, doom_itoa(c, 10));
        doom_concat(error_buf, " of ");
        doom_concat(error_buf, doom_itoa(l->size, 10));
        doom_concat(error_buf, " on lump ");
        doom_concat(error_buf, doom_itoa(lump, 10));
        I_Error(error_buf);
    }

    if (l->handle == 0)
        doom_close(handle);

    // ??? I_EndRead ();
}


//
// W_CacheLumpNum
//
void* W_CacheLumpNum(int lump, int tag)
{
    byte* ptr;

    if ((unsigned)lump >= (unsigned)numlumps)
    {
        //I_Error("Error: W_CacheLumpNum: %i >= numlumps", lump);
        doom_strcpy(error_buf, "Error: W_CacheLumpNum: ");
        doom_concat(error_buf, doom_itoa(lump, 10));
        doom_concat(error_buf, " >= numlumps");
        I_Error(error_buf);
    }

    if (!lumpcache[lump])
    {
        // read the lump in

        //doom_print ("cache miss on lump %i\n",lump);
        ptr = Z_Malloc(W_LumpLength(lump), tag, &lumpcache[lump]);
        W_ReadLump(lump, lumpcache[lump]);
    }
    else
    {
        //doom_print ("cache hit on lump %i\n",lump);
        Z_ChangeTag(lumpcache[lump], tag);
    }

    return lumpcache[lump];
}


//
// W_CacheLumpName
//
void* W_CacheLumpName(char* name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}


//
// W_Profile
//
void W_Profile(void)
{
    int i;
    memblock_t* block;
    void* ptr;
    char ch;
    void* f;
    int j;
    char name[9];

    for (i = 0; i < numlumps; i++)
    {
        ptr = lumpcache[i];
        if (!ptr)
        {
            ch = ' ';
            continue;
        }
        else
        {
            block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));
            if (block->tag < PU_PURGELEVEL)
                ch = 'S';
            else
                ch = 'P';
        }
        info[i][profilecount] = ch;
    }
    profilecount++;

    f = doom_open("waddump.txt", "w");
    name[8] = 0;

    for (i = 0; i < numlumps; i++)
    {
        doom_memcpy(name, lumpinfo[i].name, 8);

        for (j = 0; j < 8; j++)
            if (!name[j])
                break;

        for (; j < 8; j++)
            name[j] = ' ';

        doom_fprint(f, name);
        doom_fprint(f, " ");

        for (j = 0; j < profilecount; j++)
        {
            //fprintf(f, "    %c", info[i][j]);
            doom_fprint(f, "    ");
            doom_fprint(f, doom_ctoa(info[i][j]));
        }

        doom_fprint(f, "\n");
    }

    doom_close(f);
}
