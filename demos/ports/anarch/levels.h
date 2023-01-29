/**
  @file levels.h

  This file contains game levels and related code.

  by Miloslav Ciz (drummyfish), 2019

  Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
  plus a waiver of all other intellectual property. The goal of this work is to
  be and remain completely in the public domain forever, available for any use
  whatsoever.
*/

#ifndef _SFG_LEVELS_H
#define _SFG_LEVELS_H

#define SFG_MAP_SIZE 64
#define SFG_TILE_DICTIONARY_SIZE 64

/**
  Defines a single game map tile. The format is following:

    MSB aaabbbbb cccddddd LSB

   aaa:   ceiling texture index (from texture available on the map), 111
          means completely transparent texture
   bbbbb: ceiling height (1111 meaning no ceiling) ABOVE the floor
   ccc:   floor texture index, 111 means completely transparent texture
   ddddd: floor height
*/
typedef uint16_t SFG_TileDefinition;

#define SFG_TILE_CEILING_MAX_HEIGHT 31
#define SFG_TILE_TEXTURE_TRANSPARENT 7

typedef SFG_TileDefinition SFG_TileDictionary[SFG_TILE_DICTIONARY_SIZE];

/// helper macros for SFG_TileDefinition
#define SFG_TD(floorH, ceilH, floorT, ceilT)\
  ((floorH & 0x001f) |\
   ((floorT & 0x0007) << 5) |\
   ((ceilH & 0x001f) << 8) |\
   ((ceilT & 0x0007) << 13))

#define SFG_TILE_FLOOR_HEIGHT(tile) (tile & 0x1f)
#define SFG_TILE_FLOOR_TEXTURE(tile) ((tile & 0xe0) >> 5)
#define SFG_TILE_CEILING_HEIGHT(tile) ((tile & 0x1f00) >> 8)
#define SFG_TILE_CEILING_TEXTURE(tile) ((tile & 0xe000) >> 13)

#define SFG_OUTSIDE_TILE SFG_TD(63,0,7,7)

/**
  Game map represented as a 2D array. Array item has this format:

    MSB aabbbbbb LSB

    aa:     type of square, possible values:
      00:   normal
      01:   moving floor (elevator), moves from floor height to ceililing height
            (there is no ceiling above)
      10:   moving ceiling, moves from ceiling height to floor height
      11:   door
    bbbbbb: index into tile dictionary
*/
typedef uint8_t SFG_MapArray[SFG_MAP_SIZE * SFG_MAP_SIZE];

#define SFG_TILE_PROPERTY_MASK 0xc0
#define SFG_TILE_PROPERTY_NORMAL 0x00
#define SFG_TILE_PROPERTY_ELEVATOR 0x40
#define SFG_TILE_PROPERTY_SQUEEZER 0x80
#define SFG_TILE_PROPERTY_DOOR 0xc0

/**
  Serves to place elements (items, enemies etc.) into the game level.
*/
typedef struct
{
  uint8_t type;
  uint8_t coords[2];
} SFG_LevelElement;

#define SFG_MAX_LEVEL_ELEMENTS 128

/*
  Definitions of level element type. These values must leave the highest bit
  unused because that will be used by the game engine, so the values must be
  lower than 128.
*/
#define SFG_LEVEL_ELEMENT_NONE 0
#define SFG_LEVEL_ELEMENT_BARREL 0x01
#define SFG_LEVEL_ELEMENT_HEALTH 0x02
#define SFG_LEVEL_ELEMENT_BULLETS 0x03
#define SFG_LEVEL_ELEMENT_ROCKETS 0x04
#define SFG_LEVEL_ELEMENT_PLASMA 0x05
#define SFG_LEVEL_ELEMENT_TREE 0x06
#define SFG_LEVEL_ELEMENT_FINISH 0x07
#define SFG_LEVEL_ELEMENT_TELEPORTER 0x08
#define SFG_LEVEL_ELEMENT_TERMINAL 0x09
#define SFG_LEVEL_ELEMENT_COLUMN 0x0a
#define SFG_LEVEL_ELEMENT_RUIN 0x0b
#define SFG_LEVEL_ELEMENT_LAMP 0x0c
#define SFG_LEVEL_ELEMENT_CARD0 0x0d ///< Access card, unlocks doors with lock.
#define SFG_LEVEL_ELEMENT_CARD1 0x0e
#define SFG_LEVEL_ELEMENT_CARD2 0x0f
#define SFG_LEVEL_ELEMENT_LOCK0 0x10 /**< Special level element that must be
                                     placed on a tile with door. This door is
                                     then locked until taking the corresponding
                                     access card. */
#define SFG_LEVEL_ELEMENT_LOCK1 0x11
#define SFG_LEVEL_ELEMENT_LOCK2 0x12
#define SFG_LEVEL_ELEMENT_BLOCKER 0x13 ///< invisible wall

#define SFG_LEVEL_ELEMENT_MONSTER_SPIDER 0x20
#define SFG_LEVEL_ELEMENT_MONSTER_DESTROYER 0x21
#define SFG_LEVEL_ELEMENT_MONSTER_WARRIOR 0x22
#define SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT 0x23
#define SFG_LEVEL_ELEMENT_MONSTER_ENDER 0x24
#define SFG_LEVEL_ELEMENT_MONSTER_TURRET 0x25
#define SFG_LEVEL_ELEMENT_MONSTER_EXPLODER 0x26

#define SFG_MONSTERS_TOTAL 7

#define SFG_MONSTER_TYPE_TO_INDEX(monsterType) \
  ((monsterType) - SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

#define SFG_MONSTER_INDEX_TO_TYPE(monsterIndex) \
  ((monsterIndex) + SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

#define SFG_LEVEL_ELEMENT_TYPE_IS_MOSTER(t) \
  ((t) >= SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

typedef struct
{
  SFG_MapArray mapArray;
  SFG_TileDictionary tileDictionary;
  uint8_t textureIndices[7]; /**< Says which textures are used on the map. There
                             can be at most 7 because of 3bit indexing (one
                             value is reserved for special transparent
                             texture). */
  uint8_t doorTextureIndex; /**< Index (global, NOT from textureIndices) of a
                             texture used for door. */
  uint8_t floorColor;
  uint8_t ceilingColor;
  uint8_t playerStart[3];   /**< Player starting location: square X, square Y,
                                 direction (fourths of RCL_Unit). */
  uint8_t backgroundImage;  ///< Index of level background image.
  SFG_LevelElement elements[SFG_MAX_LEVEL_ELEMENTS];
} SFG_Level;

static inline SFG_TileDefinition SFG_getMapTile
(
  const SFG_Level *level,
  int16_t x,
  int16_t y,
  uint8_t *properties
)
{
  if (x >= 0 && x < SFG_MAP_SIZE && y >= 0 && y < SFG_MAP_SIZE)
  {
    uint8_t tile = level->mapArray[y * SFG_MAP_SIZE + x];

    *properties = tile & 0xc0;
    return level->tileDictionary[tile & 0x3f];
  }

  *properties = SFG_TILE_PROPERTY_NORMAL;
  return SFG_OUTSIDE_TILE;
}

#define SFG_NUMBER_OF_LEVELS 10

/*
  NOTE: Initially the levels were stored sequentially in one big array, but that
  caused some issues with Arduino's PROGMEM, so now we store each level in a
  separate variable and eventually create an array of pointers to these.

  Levels can be designed using bitmap editor and converted to the struct format
  with provided tools.
*/

SFG_PROGRAM_MEMORY SFG_Level SFG_level0 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (37 | SFG_TILE_PROPERTY_ELEVATOR)
    #define BB (24 | SFG_TILE_PROPERTY_ELEVATOR)
    #define CC (16 | SFG_TILE_PROPERTY_DOOR)
    #define DD (18 | SFG_TILE_PROPERTY_ELEVATOR)
    #define EE (32 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,27,23,23,23,23,23,23,23,23,23,27,23,23,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*0 */ 
/*1 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,27,27,23,1 ,29,29,29,27,27,27,29,29,29,1 ,23,27,27,29,29,29,29,29,29,29,29,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*1 */ 
/*2 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,1 ,1 ,1 ,1 ,o ,o ,o ,o ,23,23,29,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,29,30,30,30,29,30,30,30,27,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*2 */ 
/*3 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,23,29,1 ,1 ,1 ,6 ,6 ,1 ,1 ,1 ,1 ,1 ,6 ,6 ,1 ,1 ,1 ,28,30,30,30,30,30,30,30,27,o ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*3 */ 
/*4 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,11,11,11,o ,o ,38,38,38,o ,23,29,1 ,1 ,o ,26,o ,o ,o ,o ,o ,o ,o ,26,o ,1 ,1 ,29,30,30,30,29,30,30,30,27,o ,5 ,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*4 */ 
/*5 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,11,11,11,o ,o ,38,38,38,o ,23,23,1 ,1 ,o ,26,o ,o ,o ,o ,o ,o ,o ,26,o ,1 ,1 ,23,29,29,29,29,30,30,30,27,o ,5 ,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*5 */ 
/*6 */ o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,11,11,11,o ,o ,38,38,38,o ,23,23,29,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,29,23,o ,o ,o ,29,30,30,30,27,o ,5 ,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*6 */ 
/*7 */ o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,11,11,11,o ,o ,38,38,38,o ,23,23,23,6 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,6 ,23,23,o ,o ,o ,29,29,29,29,29,o ,5 ,15,15,15,15,15,5 ,5 ,5 ,o ,o ,o ,o ,o ,o ,o , /*7 */ 
/*8 */ o ,o ,o ,o ,3 ,3 ,3 ,23,23,23,23,23,o ,o ,o ,o ,o ,o ,38,o ,o ,23,23,29,6 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,6 ,23,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,5 ,15,15,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o , /*8 */ 
/*9 */ o ,o ,o ,o ,3 ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,38,o ,23,23,23,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,29,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,5 ,15,15,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o , /*9 */ 
/*10*/ o ,o ,o ,o ,3 ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,38,38,38,38,38,AA,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,23,o ,o ,o ,o ,o ,5 ,5 ,5 ,5 ,5 ,15,15,15,5 ,5 ,15,5 ,5 ,o ,o ,o ,o ,o ,o ,o , /*10*/ 
/*11*/ o ,o ,o ,o ,3 ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,o ,o ,o ,3 ,3 ,3 ,3 ,23,23,27,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,27,o ,o ,o ,o ,o ,5 ,15,15,15,15,15,15,15,5 ,5 ,5 ,5 ,o ,o ,o ,o ,o ,o ,o ,o , /*11*/ 
/*12*/ o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,31,31,o ,o ,o ,31,31,o ,o ,23,23,27,27,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,6 ,6 ,27,o ,o ,o ,o ,o ,5 ,15,15,15,15,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*12*/ 
/*13*/ 22,22,22,22,22,39,39,39,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,26,26,o ,o ,o ,o ,o ,26,26,25,25,BB,BB,27,o ,o ,o ,o ,o ,5 ,15,15,15,15,15,15,15,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,14, /*13*/ 
/*14*/ 21,21,21,21,21,21,21,21,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,25,25,19,19,5 ,5 ,5 ,o ,o ,o ,5 ,15,15,15,5 ,5 ,5 ,5 ,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,14, /*14*/ 
/*15*/ 21,21,21,21,21,21,21,21,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,5 ,19,19,19,19,19,5 ,5 ,5 ,5 ,5 ,15,15,15,5 ,5 ,6 ,6 ,5 ,5 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,14,14, /*15*/ 
/*16*/ 21,21,21,21,21,21,21,21,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,8 ,8 ,8 ,36,o ,o ,o ,5 ,19,19,19,19,19,5 ,o ,o ,o ,19,15,15,15,5 ,o ,o ,o ,o ,3 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,14,14, /*16*/ 
/*17*/ 22,22,22,22,22,21,21,21,22,31,31,o ,o ,31,31,o ,o ,o ,31,31,o ,o ,o ,31,31,o ,12,9 ,9 ,9 ,12,o ,31,31,5 ,19,19,19,19,19,5 ,o ,o ,o ,5 ,17,17,17,5 ,26,o ,o ,26,3 ,o ,o ,o ,o ,o ,o ,o ,o ,14,14, /*17*/ 
/*18*/ o ,o ,o ,o ,22,20,20,20,39,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,10,10,10,12,2 ,2 ,2 ,2 ,5 ,5 ,6 ,CC,6 ,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*18*/ 
/*19*/ o ,o ,o ,o ,22,21,21,21,39,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,10,10,10,12,2 ,2 ,2 ,2 ,5 ,5 ,19,19,19,5 ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,8 ,8 ,26,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*19*/ 
/*20*/ o ,o ,o ,o ,39,20,20,20,39,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,10,10,10,12,2 ,2 ,2 ,2 ,6 ,5 ,15,15,15,19,11,10,9 ,8 ,o ,o ,o ,o ,8 ,8 ,8 ,26,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*20*/ 
/*21*/ o ,o ,o ,o ,39,21,21,21,39,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,10,10,10,12,2 ,2 ,2 ,2 ,6 ,5 ,15,15,15,19,11,10,9 ,8 ,o ,o ,o ,o ,8 ,8 ,8 ,26,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*21*/ 
/*22*/ o ,o ,o ,o ,22,20,20,20,22,DD,o ,o ,o ,o ,o ,o ,26,26,26,26,26,o ,o ,o ,o ,o ,12,9 ,9 ,9 ,12,o ,2 ,2 ,2 ,6 ,5 ,15,15,15,19,11,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*22*/ 
/*23*/ o ,o ,o ,o ,22,21,21,21,22,12,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,8 ,8 ,8 ,36,o ,2 ,2 ,2 ,5 ,5 ,15,15,15,19,11,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*23*/ 
/*24*/ o ,o ,o ,o ,39,21,21,21,21,11,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,6 ,5 ,19,19,19,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*24*/ 
/*25*/ o ,o ,o ,o ,39,21,21,21,21,11,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,6 ,5 ,15,15,15,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,14,14, /*25*/ 
/*26*/ o ,o ,o ,o ,22,21,21,21,22,12,o ,o ,o ,o ,o ,o ,8 ,8 ,o ,o ,o ,8 ,8 ,o ,o ,o ,8 ,8 ,o ,o ,o ,o ,DD,2 ,2 ,6 ,5 ,15,15,15,5 ,o ,o ,o ,5 ,17,17,17,5 ,26,o ,o ,26,3 ,o ,o ,o ,o ,o ,o ,o ,o ,14,14, /*26*/ 
/*27*/ o ,o ,o ,o ,22,20,20,20,22,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,31,31,2 ,2 ,2 ,5 ,5 ,15,15,15,5 ,o ,o ,o ,5 ,15,15,15,5 ,o ,o ,o ,o ,3 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,14,14, /*27*/ 
/*28*/ o ,o ,o ,o ,39,21,21,21,39,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 ,2 ,5 ,15,15,15,15,5 ,5 ,5 ,5 ,5 ,19,19,19,5 ,5 ,6 ,6 ,5 ,5 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,14,14, /*28*/ 
/*29*/ o ,o ,o ,o ,39,20,20,20,39,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 ,2 ,5 ,5 ,15,15,15,15,15,15,15,19,15,15,15,5 ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,14,14, /*29*/ 
/*30*/ o ,o ,o ,o ,22,21,21,21,39,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 ,2 ,5 ,15,15,15,15,15,15,15,15,19,15,15,15,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,14, /*30*/ 
/*31*/ o ,o ,o ,o ,22,20,20,20,39,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 ,2 ,5 ,5 ,15,15,15,15,15,15,15,19,15,15,15,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,14, /*31*/ 
/*32*/ o ,o ,o ,o ,39,21,21,21,22,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 ,2 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,19,19,19,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,14, /*32*/ 
/*33*/ o ,o ,o ,o ,39,21,21,21,22,40,40,40,40,40,40,40,40,40,31,35,31,o ,o ,31,35,31,o ,o ,31,35,2 ,2 ,2 ,2 ,2 ,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,15,15,15,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 , /*33*/ 
/*34*/ o ,o ,o ,o ,22,21,21,21,21,11,10,9 ,40,40,40,40,40,40,31,35,31,o ,o ,31,35,31,o ,o ,31,35,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,5 ,1 ,1 ,1 ,5 ,15,15,15,5 ,o ,14,14,14,14,14,14,14,o ,o ,o ,o ,o ,o ,14, /*34*/ 
/*35*/ o ,o ,o ,o ,22,21,21,21,21,11,10,9 ,40,40,40,40,40,40,31,35,o ,o ,o ,o ,35,o ,o ,o ,o ,35,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,6 ,1 ,1 ,1 ,5 ,15,15,15,5 ,o ,14,14,14,14,14,14,14,o ,o ,o ,o ,o ,o ,14, /*35*/ 
/*36*/ o ,o ,o ,o ,22,21,21,21,21,11,10,9 ,40,40,40,40,40,40,40,35,o ,o ,o ,o ,35,o ,o ,o ,o ,35,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,6 ,1 ,1 ,1 ,5 ,15,15,15,5 ,6 ,14,14,14,14,14,14,14,3 ,3 ,3 ,3 ,o ,o ,14, /*36*/ 
/*37*/ o ,o ,o ,o ,22,39,39,39,22,40,40,40,40,40,40,40,40,40,40,35,35,o ,o ,35,35,35,o ,o ,35,35,31,31,31,31,6 ,6 ,6 ,6 ,2 ,2 ,6 ,1 ,1 ,1 ,1 ,6 ,CC,6 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,14,14, /*37*/ 
/*38*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,10,34,10,34,40,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,6 ,2 ,2 ,5 ,1 ,1 ,1 ,11,11,11,11,11,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,14,14, /*38*/ 
/*39*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,10,34,10,34,40,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,6 ,2 ,2 ,6 ,1 ,1 ,1 ,11,11,11,11,11,o ,o ,o ,o ,12,o ,o ,o ,o ,o ,o ,3 ,o ,14,14, /*39*/ 
/*40*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,35,35,35,35,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,6 ,2 ,2 ,6 ,1 ,1 ,1 ,11,11,11,11,11,11,11,11,11,12,11,10,9 ,8 ,o ,o ,3 ,o ,14,14, /*40*/ 
/*41*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,31,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,6 ,2 ,2 ,6 ,1 ,1 ,1 ,11,11,11,11,11,11,11,11,11,12,11,10,9 ,8 ,o ,o ,3 ,o ,14,14, /*41*/ 
/*42*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,5 ,1 ,1 ,6 ,2 ,2 ,5 ,1 ,1 ,1 ,11,11,11,11,11,11,11,11,11,12,11,10,9 ,8 ,o ,o ,3 ,o ,o ,1 , /*42*/ 
/*43*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,31,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,26,2 ,2 ,6 ,1 ,1 ,1 ,11,11,11,11,11,11,4 ,o ,o ,12,o ,o ,o ,o ,o ,o ,3 ,o ,o ,1 , /*43*/ 
/*44*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,35,35,35,35,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,40,40,o ,o ,o ,o ,o ,o ,o ,o ,26,2 ,2 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,1 , /*44*/ 
/*45*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,3 ,3 ,3 ,3 ,3 ,40,40,o ,o ,o ,o ,o ,o ,o ,o ,26,2 ,2 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,1 , /*45*/ 
/*46*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,40,40,o ,o ,o ,o ,o ,o ,o ,o ,31,2 ,2 ,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,5 ,1 ,1 ,1 ,1 ,1 ,o ,o ,1 , /*46*/ 
/*47*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,29,o ,o ,o ,o ,o ,31,2 ,2 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,1 , /*47*/ 
/*48*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,23,o ,o ,o ,o ,o ,31,2 ,2 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,o ,o ,8 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,1 , /*48*/ 
/*49*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,23,23,29,o ,o ,o ,26,2 ,2 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*49*/ 
/*50*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,11,11,11,o ,o ,o ,o ,23,23,23,23,23,23,23,23,o ,o ,o ,26,2 ,2 ,5 ,5 ,6 ,1 ,1 ,1 ,6 ,1 ,1 ,1 ,5 ,8 ,o ,o ,8 ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*50*/ 
/*51*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,8 ,9 ,10,11,11,11,10,9 ,8 ,o ,23,23,23,23,23,27,23,29,o ,o ,o ,26,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,6 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*51*/ 
/*52*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,8 ,9 ,10,11,11,11,10,9 ,8 ,o ,23,23,23,27,1 ,1 ,9 ,9 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*52*/ 
/*53*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,27,1 ,27,15,15,15,27,1 ,27,23,23,23,23,27,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*53*/ 
/*54*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,27,15,15,15,27,23,23,23,23,23,33,EE,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*54*/ 
/*55*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,27,15,15,15,27,23,23,23,23,23,23,27,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,8 ,8 ,8 ,8 ,8 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*55*/ 
/*56*/ 1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,15,15,15,15,15,15,15,15,15,23,23,23,23,27,1 ,1 ,9 ,9 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,o ,o ,o ,8 ,8 ,8 ,8 ,8 ,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*56*/ 
/*57*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,23,15,15,15,15,15,15,15,15,15,23,23,23,23,23,23,27,23,29,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,DD,o ,o ,o ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,9 ,10,11,11,3 ,1 ,1 , /*57*/ 
/*58*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,23,15,15,23,23,23,23,23,15,15,23,23,23,23,23,23,23,23,23,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,1 ,1 ,1 ,11,3 ,1 ,1 , /*58*/ 
/*59*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,23,15,15,23,23,23,23,23,15,15,23,23,23,23,23,23,23,23,29,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,8 ,8 ,8 ,8 ,1 ,11,3 ,1 ,1 , /*59*/ 
/*60*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,23,15,15,23,23,23,23,23,15,15,23,23,23,23,23,23,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,8 ,8 ,8 ,8 ,8 ,8 ,1 ,1 ,1 , /*60*/ 
/*61*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,23,15,15,23,23,23,23,23,15,15,23,23,23,23,23,23,29,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,8 ,8 ,8 ,8 ,8 ,1 ,1 ,1 , /*61*/ 
/*62*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,8 ,1 ,1 ,1 , /*62*/ 
/*63*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,3 ,3 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,8 ,o ,o ,o   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef o
    },
    {        // tileDictionary
      SFG_TD( 7,31,1,0),SFG_TD(26,31,0,0),SFG_TD( 0,31,0,0),SFG_TD(14,31,2,0), // 0 
      SFG_TD(11,31,2,0),SFG_TD(26,31,2,0),SFG_TD(26,31,3,0),SFG_TD( 7,11,0,0), // 4 
      SFG_TD( 8,31,4,0),SFG_TD( 9,31,4,0),SFG_TD(10,31,4,0),SFG_TD(11,31,4,0), // 8 
      SFG_TD(11,31,2,0),SFG_TD( 7,11,4,0),SFG_TD(30,31,0,0),SFG_TD(11, 7,2,0), // 12 
      SFG_TD(15, 0,4,3),SFG_TD(13, 2,2,2),SFG_TD( 0, 7,5,0),SFG_TD(11, 5,2,2), // 16 
      SFG_TD(11,31,2,0),SFG_TD(11, 7,2,7),SFG_TD(18, 0,2,7),SFG_TD( 7, 0,0,0), // 20 
      SFG_TD(11,15,5,0),SFG_TD(30,31,2,0),SFG_TD(12,31,6,0),SFG_TD( 7, 0,3,3), // 24 
      SFG_TD(26, 4,0,0),SFG_TD(31, 0,2,0),SFG_TD(26, 5,0,0),SFG_TD(10,31,6,0), // 28 
      SFG_TD(11, 0,1,0),SFG_TD( 7, 4,1,0),SFG_TD(10,31,2,0),SFG_TD(14,31,4,0), // 32 
      SFG_TD(14,31,2,0),SFG_TD( 3,23,5,0),SFG_TD( 3, 4,1,0),SFG_TD(18,31,4,0), // 36 
      SFG_TD( 8,31,2,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0), // 40 
      SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0), // 44 
      SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0), // 48 
      SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0), // 52 
      SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0), // 56 
      SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0),SFG_TD( 7,11,0,0)  // 60 
      },                    // tileDictionary
    {1 ,3 ,12,14,5 ,11,6 }, // textureIndices
    13,                     // doorTextureIndex
    20,                     // floorColor
    3,                      // ceilingColor
    {52, 55, 80 },          // player start: x, y, direction
    0,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_BULLETS, {31,2}},{SFG_LEVEL_ELEMENT_BULLETS, {29,2}},
      {SFG_LEVEL_ELEMENT_BULLETS, {35,4}},{SFG_LEVEL_ELEMENT_BULLETS, {25,4}},
      {SFG_LEVEL_ELEMENT_BULLETS, {51,5}},{SFG_LEVEL_ELEMENT_ROCKETS, {50,5}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {44,5}},{SFG_LEVEL_ELEMENT_TELEPORTER, {18,5}},
      {SFG_LEVEL_ELEMENT_BULLETS, {51,6}},{SFG_LEVEL_ELEMENT_ROCKETS, {50,6}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {54,10}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {36,13}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {29,13}},{SFG_LEVEL_ELEMENT_TERMINAL, {35,15}},
      {SFG_LEVEL_ELEMENT_LAMP, {32,15}},{SFG_LEVEL_ELEMENT_RUIN, {19,15}},
      {SFG_LEVEL_ELEMENT_BULLETS, {49,16}},{SFG_LEVEL_ELEMENT_RUIN, {18,16}},
      {SFG_LEVEL_ELEMENT_RUIN, {19,17}},{SFG_LEVEL_ELEMENT_LOCK0, {38,18}},
      {SFG_LEVEL_ELEMENT_CARD0, {50,20}},{SFG_LEVEL_ELEMENT_RUIN, {6,21}},
      {SFG_LEVEL_ELEMENT_RUIN, {5,21}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {46,22}},
      {SFG_LEVEL_ELEMENT_RUIN, {7,22}},{SFG_LEVEL_ELEMENT_LAMP, {9,23}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {16,24}},{SFG_LEVEL_ELEMENT_LAMP, {31,25}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {16,25}},{SFG_LEVEL_ELEMENT_LAMP, {9,26}},
      {SFG_LEVEL_ELEMENT_HEALTH, {49,27}},{SFG_LEVEL_ELEMENT_BULLETS, {47,27}},
      {SFG_LEVEL_ELEMENT_BULLETS, {45,27}},{SFG_LEVEL_ELEMENT_LAMP, {36,28}},
      {SFG_LEVEL_ELEMENT_LAMP, {36,30}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {34,32}},
      {SFG_LEVEL_ELEMENT_HEALTH, {30,33}},{SFG_LEVEL_ELEMENT_BULLETS, {27,33}},
      {SFG_LEVEL_ELEMENT_BULLETS, {22,33}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {21,36}},
      {SFG_LEVEL_ELEMENT_TREE, {53,39}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {46,39}},
      {SFG_LEVEL_ELEMENT_TREE, {15,39}},{SFG_LEVEL_ELEMENT_TREE, {13,39}},
      {SFG_LEVEL_ELEMENT_BULLETS, {44,41}},{SFG_LEVEL_ELEMENT_TREE, {53,43}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {32,43}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {31,43}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {30,43}},{SFG_LEVEL_ELEMENT_RUIN, {21,45}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,45}},{SFG_LEVEL_ELEMENT_RUIN, {19,45}},
      {SFG_LEVEL_ELEMENT_LAMP, {54,48}},{SFG_LEVEL_ELEMENT_BARREL, {53,48}},
      {SFG_LEVEL_ELEMENT_BARREL, {52,48}},{SFG_LEVEL_ELEMENT_LAMP, {51,48}},
      {SFG_LEVEL_ELEMENT_LAMP, {54,50}},{SFG_LEVEL_ELEMENT_LAMP, {51,50}},
      {SFG_LEVEL_ELEMENT_RUIN, {40,51}},{SFG_LEVEL_ELEMENT_TREE, {32,52}},
      {SFG_LEVEL_ELEMENT_FINISH, {28,54}},{SFG_LEVEL_ELEMENT_BULLETS, {49,55}},
      {SFG_LEVEL_ELEMENT_BULLETS, {48,55}},{SFG_LEVEL_ELEMENT_BULLETS, {49,56}},
      {SFG_LEVEL_ELEMENT_BULLETS, {48,56}},{SFG_LEVEL_ELEMENT_TREE, {32,56}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {58,57}},{SFG_LEVEL_ELEMENT_RUIN, {48,59}},
      {SFG_LEVEL_ELEMENT_RUIN, {48,60}},{SFG_LEVEL_ELEMENT_RUIN, {51,61}},
      {SFG_LEVEL_ELEMENT_RUIN, {50,61}},{SFG_LEVEL_ELEMENT_RUIN, {45,61}},
      {SFG_LEVEL_ELEMENT_RUIN, {44,61}},{SFG_LEVEL_ELEMENT_RUIN, {43,61}},
      {SFG_LEVEL_ELEMENT_LAMP, {42,61}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level1 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (21 | SFG_TILE_PROPERTY_DOOR)
    #define BB (23 | SFG_TILE_PROPERTY_ELEVATOR)
    #define CC (3 | SFG_TILE_PROPERTY_SQUEEZER)
    #define DD (31 | SFG_TILE_PROPERTY_ELEVATOR)
    #define EE (1 | SFG_TILE_PROPERTY_DOOR)
    #define FF (22 | SFG_TILE_PROPERTY_DOOR)
    #define GG (50 | SFG_TILE_PROPERTY_ELEVATOR)
    #define HH (49 | SFG_TILE_PROPERTY_ELEVATOR)
    #define II (48 | SFG_TILE_PROPERTY_ELEVATOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,6 ,7 ,7 ,7 ,7 ,6 ,7 ,7 ,7 ,7 ,6 ,6 ,6 ,6 ,6 ,7 ,7 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,12,6 ,6 ,6 ,12,6 ,6 ,6 ,12,6 ,6 ,6 ,12,6 ,6 ,6 ,12,6 ,6 , /*0 */ 
/*1 */ 4 ,4 ,4 ,4 ,5 ,5 ,5 ,4 ,4 ,4 ,4 ,4 ,6 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,21,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,7 ,9 ,9 ,9 ,9 ,6 ,5 ,5 ,5 ,5 ,5 ,21,6 ,5 ,5 ,5 ,57,57,5 ,5 ,5 ,13,16,16,16,16,16,16,16,16,16,16,6 , /*1 */ 
/*2 */ 4 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,6 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,21,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,7 ,9 ,9 ,9 ,9 ,6 ,28,28,28,5 ,5 ,21,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,16,16,16,16,16,16,16,16,16,16,16,15, /*2 */ 
/*3 */ 4 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,6 ,54,54,54,54,4 ,8 ,8 ,8 ,8 ,21,8 ,8 ,4 ,54,54,54,54,54,6 ,9 ,9 ,9 ,9 ,6 ,21,21,28,5 ,5 ,21,21,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,16,16,16,16,16,16,16,16,16,16,16,6 , /*3 */ 
/*4 */ 4 ,5 ,5 ,5 ,9 ,9 ,9 ,5 ,5 ,5 ,4 ,4 ,4 ,8 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,21,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,6 ,6 ,6 ,6 ,6 ,6 ,21,21,28,5 ,5 ,21,39,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,13,16,16,16,16,16,16,16,16,16,16,6 , /*4 */ 
/*5 */ 4 ,5 ,5 ,5 ,9 ,9 ,9 ,5 ,5 ,5 ,8 ,8 ,AA,8 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,21,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,6 ,16,16,16,16,16,21,21,28,5 ,5 ,21,13,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 , /*5 */ 
/*6 */ 4 ,5 ,5 ,5 ,9 ,9 ,9 ,5 ,5 ,5 ,4 ,4 ,4 ,8 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,21,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,6 ,16,16,16,16,16,63,21,28,5 ,5 ,21,39,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,57,6 , /*6 */ 
/*7 */ 9 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,6 ,8 ,8 ,8 ,8 ,4 ,21,21,21,21,21,AA,21,4 ,8 ,8 ,8 ,8 ,8 ,6 ,16,16,16,16,16,63,21,28,5 ,5 ,21,21,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,57,6 , /*7 */ 
/*8 */ 9 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,6 ,8 ,8 ,21,21,4 ,54,54,54,54,54,54,54,4 ,21,21,21,BB,BB,27,16,16,16,16,16,21,21,28,5 ,5 ,21,39,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 , /*8 */ 
/*9 */ 10,10,4 ,4 ,5 ,5 ,5 ,4 ,4 ,4 ,4 ,4 ,6 ,8 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,8 ,8 ,54,8 ,8 ,21,BB,BB,27,16,16,17,17,17,21,21,28,5 ,5 ,21,13,5 ,5 ,5 ,5 ,5 ,5 ,4 ,5 ,5 ,5 ,5 ,53,53,53,53,5 ,5 ,5 ,56,6 , /*9 */ 
/*10*/ 10,10,10,9 ,4 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,6 ,8 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,8 ,8 ,54,8 ,8 ,21,BB,BB,27,16,16,18,18,18,21,21,28,5 ,5 ,21,21,5 ,5 ,5 ,5 ,5 ,5 ,20,5 ,5 ,5 ,5 ,53,53,53,53,5 ,5 ,5 ,57,6 , /*10*/ 
/*11*/ 10,10,10,9 ,9 ,9 ,9 ,1 ,6 ,6 ,15,6 ,6 ,6 ,6 ,6 ,6 ,6 ,7 ,7 ,7 ,7 ,7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,19,19,19,21,21,28,5 ,5 ,21,21,59,5 ,5 ,5 ,5 ,5 ,21,5 ,5 ,5 ,5 ,53,53,53,53,1 ,13,13,13,1 , /*11*/ 
/*12*/ 10,10,10,9 ,9 ,9 ,9 ,1 ,12,16,16,16,16,16,16,12,5 ,56,28,28,28,28,28,60,58,58,5 ,5 ,5 ,5 ,5 ,5 ,21,21,20,20,20,21,21,28,5 ,5 ,21,21,5 ,5 ,59,5 ,5 ,5 ,21,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,1 ,o ,o ,o ,1 , /*12*/ 
/*13*/ 9 ,11,9 ,9 ,9 ,9 ,11,1 ,6 ,16,16,16,16,16,16,16,5 ,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,21,28,28,28,28,28,28,28,28,28,4 ,21,21,21,21,20,20,20,21,1 ,1 ,1 ,1 ,2 ,4 ,1 ,1 ,4 ,3 ,3 ,4 ,1 , /*13*/ 
/*14*/ 9 ,11,9 ,9 ,9 ,9 ,11,1 ,6 ,16,16,16,16,16,16,16,5 ,25,5 ,5 ,5 ,5 ,5 ,5 ,56,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,21,21,21,21,o ,CC,CC,CC,o ,21,1 ,4 ,o ,o ,o ,o ,o ,o ,14,14,o ,1 , /*14*/ 
/*15*/ 9 ,11,11,11,9 ,9 ,9 ,1 ,12,16,16,16,16,16,16,12,5 ,25,5 ,5 ,5 ,5 ,5 ,59,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,21,21,21,21,21,28,28,28,21,21,1 ,3 ,o ,o ,o ,o ,o ,o ,14,14,o ,1 , /*15*/ 
/*16*/ 10,10,11,11,9 ,9 ,9 ,1 ,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,25,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,60,29,5 ,5 ,21,21,53,53,53,21,21,1 ,3 ,o ,o ,o ,o ,o ,o ,14,14,o ,1 , /*16*/ 
/*17*/ 10,10,9 ,9 ,9 ,9 ,9 ,1 ,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,21,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,60,29,5 ,5 ,53,53,53,53,53,53,53,1 ,3 ,o ,o ,o ,o ,o ,o ,14,14,o ,1 , /*17*/ 
/*18*/ 10,10,9 ,9 ,9 ,9 ,9 ,1 ,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,58,58,5 ,5 ,5 ,5 ,21,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,29,5 ,5 ,53,53,53,53,53,53,53,1 ,3 ,o ,o ,o ,o ,o ,o ,14,14,o ,1 , /*18*/ 
/*19*/ 10,9 ,9 ,9 ,9 ,9 ,9 ,4 ,21,21,20,20,21,6 ,6 ,58,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,5 ,6 ,6 ,21,21,21,21,21,24,24,24,24,24,24,24,24,5 ,21,5 ,5 ,53,53,53,53,53,53,53,1 ,3 ,o ,o ,o ,o ,13,o ,14,14,o ,1 , /*19*/ 
/*20*/ 10,10,10,9 ,9 ,9 ,9 ,8 ,8 ,8 ,20,20,8 ,6 ,6 ,58,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,31,30,30,31,31,31,31,31,31,31,30,31,31,6 ,6 ,5 ,5 ,53,53,53,53,53,53,53,1 ,3 ,o ,o ,o ,o ,1 ,o ,14,14,o ,1 , /*20*/ 
/*21*/ 9 ,10,10,9 ,9 ,9 ,9 ,8 ,8 ,8 ,20,20,8 ,6 ,6 ,5 ,5 ,59,5 ,5 ,5 ,5 ,5 ,5 ,5 ,56,6 ,6 ,31,31,31,31,31,31,31,31,31,31,30,31,31,6 ,DD,8 ,8 ,53,53,53,53,53,53,53,1 ,3 ,3 ,3 ,3 ,3 ,1 ,3 ,3 ,3 ,3 ,1 , /*21*/ 
/*22*/ 11,11,9 ,9 ,9 ,9 ,9 ,4 ,8 ,8 ,20,20,8 ,6 ,6 ,58,58,5 ,5 ,5 ,5 ,5 ,5 ,56,5 ,59,6 ,6 ,31,30,30,30,31,31,31,31,31,31,30,31,31,6 ,31,8 ,8 ,53,53,53,53,53,53,53,1 ,o ,o ,o ,o ,3 ,1 ,o ,14,14,o ,1 , /*22*/ 
/*23*/ 11,11,9 ,9 ,9 ,9 ,9 ,21,8 ,8 ,20,20,8 ,8 ,8 ,58,58,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,57,6 ,6 ,31,30,31,30,30,31,31,31,31,31,30,31,31,31,31,8 ,8 ,53,53,53,53,53,53,53,1 ,o ,o ,o ,o ,3 ,1 ,o ,14,14,o ,1 , /*23*/ 
/*24*/ 9 ,10,10,10,9 ,9 ,9 ,21,8 ,8 ,20,20,8 ,8 ,8 ,62,58,5 ,5 ,5 ,5 ,57,61,5 ,61,57,6 ,6 ,31,30,31,31,30,30,30,30,30,30,30,31,31,6 ,6 ,6 ,6 ,1 ,1 ,1 ,1 ,EE,1 ,1 ,1 ,o ,o ,o ,o ,3 ,1 ,o ,14,14,o ,1 , /*24*/ 
/*25*/ 9 ,10,10,10,9 ,9 ,9 ,21,8 ,5 ,5 ,5 ,8 ,6 ,6 ,57,61,56,5 ,5 ,5 ,57,5 ,5 ,56,58,6 ,6 ,31,30,31,31,31,31,31,31,31,31,31,31,31,6 ,58,o ,o ,o ,1 ,o ,o ,o ,o ,o ,61,o ,o ,o ,o ,3 ,1 ,1 ,19,19,1 ,1 , /*25*/ 
/*26*/ 9 ,10,10,10,9 ,9 ,9 ,21,8 ,5 ,5 ,5 ,8 ,6 ,12,57,5 ,5 ,5 ,5 ,5 ,5 ,56,5 ,5 ,5 ,12,6 ,31,30,31,31,31,31,31,31,31,31,31,31,31,6 ,58,o ,o ,o ,2 ,o ,o ,o ,o ,o ,EE,o ,o ,o ,o ,3 ,1 ,1 ,18,18,1 ,1 , /*26*/ 
/*27*/ 9 ,11,11,9 ,9 ,9 ,9 ,20,5 ,5 ,5 ,5 ,8 ,6 ,6 ,61,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,58,5 ,6 ,6 ,31,31,31,31,31,31,31,31,31,31,31,31,31,6 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,3 ,61,3 ,3 ,3 ,3 ,4 ,1 ,1 ,17,17,1 ,1 , /*27*/ 
/*28*/ 9 ,11,11,9 ,9 ,9 ,9 ,21,8 ,8 ,8 ,8 ,8 ,21,6 ,58,5 ,5 ,5 ,5 ,5 ,5 ,5 ,56,5 ,5 ,6 ,6 ,31,30,31,31,31,31,31,31,31,31,31,31,31,6 ,58,o ,o ,o ,1 ,o ,o ,o ,o ,3 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,16,16,16,16,1 , /*28*/ 
/*29*/ 1 ,1 ,1 ,1 ,1 ,1 ,20,20,20,20,20,20,20,21,6 ,58,5 ,5 ,5 ,5 ,5 ,5 ,5 ,58,5 ,56,6 ,6 ,31,30,31,30,30,30,30,30,30,30,31,31,31,6 ,58,o ,o ,o ,2 ,o ,o ,o ,o ,3 ,EE,5 ,5 ,5 ,5 ,5 ,1 ,16,16,16,16,1 , /*29*/ 
/*30*/ 1 ,58,58,57,57,57,21,21,21,21,20,20,20,21,6 ,56,5 ,5 ,5 ,5 ,5 ,5 ,5 ,57,5 ,60,6 ,6 ,31,30,30,30,31,31,31,31,31,30,30,31,31,6 ,58,o ,o ,o ,1 ,o ,o ,o ,o ,3 ,1 ,o ,o ,o ,5 ,5 ,1 ,16,16,16,16,1 , /*30*/ 
/*31*/ 1 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,1 ,21,20,20,20,21,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,5 ,6 ,6 ,31,31,31,31,31,31,31,31,31,31,30,31,31,6 ,58,o ,o ,1 ,1 ,o ,o ,o ,o ,3 ,1 ,o ,o ,o ,5 ,5 ,1 ,16,16,16,16,1 , /*31*/ 
/*32*/ 1 ,5 ,58,58,5 ,1 ,5 ,57,57,21,20,20,20,21,21,21,21,21,20,20,20,20,21,4 ,7 ,7 ,6 ,6 ,31,31,31,31,31,31,31,31,31,31,30,31,31,6 ,58,o ,o ,o ,EE,o ,o ,o ,o ,3 ,EE,o ,o ,o ,5 ,5 ,1 ,16,16,16,16,1 , /*32*/ 
/*33*/ 1 ,5 ,1 ,5 ,5 ,5 ,5 ,5 ,1 ,57,5 ,5 ,5 ,5 ,5 ,12,13,13,53,53,53,53,21,21,13,13,6 ,6 ,6 ,6 ,6 ,6 ,31,31,30,30,30,30,30,31,31,6 ,58,o ,o ,o ,1 ,o ,o ,o ,o ,3 ,1 ,5 ,5 ,5 ,5 ,5 ,1 ,16,16,16,16,1 , /*33*/ 
/*34*/ 1 ,58,5 ,5 ,1 ,1 ,5 ,5 ,5 ,57,5 ,5 ,5 ,5 ,5 ,13,13,13,53,53,53,53,53,53,13,13,13,13,53,53,53,53,24,24,24,24,6 ,6 ,6 ,6 ,6 ,6 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,3 ,1 ,1 ,1 ,1 ,16,16,1 ,16,16,16,16,15, /*34*/ 
/*35*/ 1 ,5 ,5 ,1 ,5 ,5 ,5 ,57,5 ,57,5 ,5 ,5 ,5 ,5 ,13,13,13,53,53,53,53,53,53,13,5 ,5 ,13,53,53,53,53,53,53,53,53,6 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,o ,o ,o ,o ,3 ,2 ,o ,o ,1 ,16,16,1 ,16,16,16,16,21, /*35*/ 
/*36*/ 1 ,5 ,1 ,1 ,5 ,57,1 ,5 ,1 ,1 ,1 ,5 ,5 ,5 ,5 ,12,13,13,53,53,53,53,53,5 ,5 ,5 ,5 ,5 ,5 ,53,53,53,13,13,13,13,13,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,3 ,1 ,o ,o ,21,16,16,16,16,21,27,27,21, /*36*/ 
/*37*/ 1 ,19,18,1 ,5 ,1 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,53,13,13,53,53,53,53,13,5 ,5 ,5 ,5 ,5 ,5 ,13,13,13,13,13,4 ,61,1 ,2 ,1 ,1 ,2 ,1 ,1 ,2 ,1 ,61,4 ,o ,o ,o ,o ,3 ,1 ,o ,o ,21,16,16,16,16,21,BB,BB,21, /*37*/ 
/*38*/ 1 ,57,17,1 ,5 ,1 ,5 ,57,57,5 ,58,5 ,5 ,5 ,5 ,53,53,13,13,13,13,13,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,13,13,13,1 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,3 ,4 ,o ,o ,21,21,21,21,21,21,8 ,8 ,21, /*38*/ 
/*39*/ 1 ,16,16,1 ,5 ,5 ,5 ,58,5 ,5 ,58,5 ,5 ,5 ,5 ,12,53,53,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,57,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,1 ,o ,o ,1 ,o ,o ,21,7 ,7 ,54,54,4 , /*39*/ 
/*40*/ 1 ,16,1 ,1 ,1 ,1 ,1 ,5 ,5 ,58,62,5 ,5 ,5 ,5 ,53,53,53,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,EE,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,1 ,1 ,1 ,1 ,o ,o ,21,7 ,8 ,8 ,54,21, /*40*/ 
/*41*/ 1 ,16,1 ,16,16,16,16,1 ,5 ,5 ,62,5 ,5 ,5 ,5 ,53,53,53,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,EE,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,2 ,o ,o ,o ,o ,o ,21,7 ,8 ,8 ,54,21, /*41*/ 
/*42*/ 1 ,16,16,16,16,16,1 ,1 ,58,5 ,62,59,5 ,5 ,5 ,12,53,53,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,57,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,1 ,o ,o ,o ,o ,o ,21,7 ,8 ,8 ,54,21, /*42*/ 
/*43*/ 1 ,58,5 ,1 ,5 ,57,5 ,5 ,5 ,5 ,1 ,5 ,5 ,5 ,5 ,53,53,13,13,13,13,13,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,13,13,13,1 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,1 ,o ,o ,21,21,21,21,7 ,8 ,8 ,54,21, /*43*/ 
/*44*/ 1 ,5 ,5 ,1 ,5 ,5 ,5 ,1 ,1 ,5 ,1 ,5 ,5 ,5 ,5 ,53,13,13,13,13,13,13,13,5 ,5 ,5 ,5 ,5 ,5 ,13,13,1 ,1 ,1 ,4 ,1 ,1 ,2 ,1 ,1 ,2 ,1 ,1 ,57,EE,57,1 ,1 ,1 ,EE,1 ,4 ,1 ,o ,o ,1 ,7 ,7 ,7 ,7 ,54,54,54,4 , /*44*/ 
/*45*/ 1 ,5 ,5 ,5 ,57,1 ,1 ,5 ,58,5 ,1 ,5 ,56,5 ,5 ,12,13,13,13,13,13,13,13,13,5 ,5 ,5 ,5 ,13,13,13,1 ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,22,22,22,FF,22,22,22,1 ,o ,1 ,1 ,1 ,o ,o ,1 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*45*/ 
/*46*/ 1 ,5 ,57,5 ,5 ,5 ,1 ,5 ,5 ,5 ,58,21,21,21,21,21,21,21,21,21,13,13,13,13,13,5 ,5 ,13,13,53,53,1 ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,22,o ,o ,o ,o ,o ,22,1 ,o ,o ,o ,1 ,o ,o ,1 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*46*/ 
/*47*/ 1 ,58,57,1 ,1 ,5 ,58,58,1 ,1 ,58,21,5 ,5 ,5 ,5 ,56,5 ,5 ,21,13,13,13,13,13,5 ,5 ,13,13,53,53,EE,o ,o ,55,1 ,1 ,1 ,1 ,o ,o ,22,o ,o ,o ,o ,o ,22,1 ,o ,o ,o ,1 ,o ,o ,1 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*47*/ 
/*48*/ 6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,53,53,53,EE,53,53,53,53,53,53,53,1 ,o ,o ,55,1 ,o ,o ,o ,o ,o ,22,o ,o ,o ,o ,o ,22,4 ,5 ,5 ,5 ,4 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,8 ,8 ,54,1 , /*48*/ 
/*49*/ 7 ,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,53,53,53,1 ,53,53,53,53,53,53,53,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,22,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*49*/ 
/*50*/ 7 ,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,20,5 ,5 ,5 ,5 ,5 ,5 ,5 ,21,13,13,13,13,13,13,13,13,53,53,53,1 ,1 ,22,22,22,22,22,22,22,22,4 ,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*50*/ 
/*51*/ 6 ,5 ,5 ,57,58,5 ,1 ,1 ,58,58,58,21,59,5 ,5 ,5 ,5 ,5 ,5 ,21,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,28,28,28,1 ,4 ,GG,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,8 ,54,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,54,1 , /*51*/ 
/*52*/ 6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,28,28,28,28,28,28,28,1 ,28,28,28,1 ,1 ,22,3 ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,8 ,54,54,54,54,54,54,54,54,54,54,54,4 , /*52*/ 
/*53*/ 11,11,11,11,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,28,28,28,28,28,28,28,1 ,28,28,28,1 ,1 ,22,3 ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,1 ,4 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*53*/ 
/*54*/ 11,11,11,11,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,28,28,28,28,28,28,28,1 ,28,28,28,1 ,4 ,HH,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,22,1 ,8 ,8 ,1 ,22,22,22,22,22,22,22,22,22,22,22,22, /*54*/ 
/*55*/ 11,11,10,10,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,28,28,28,28,28,28,28,1 ,28,28,28,1 ,1 ,22,22,22,22,22,22,22,22,4 ,22,GG,HH,II,22,22,1 ,8 ,8 ,1 ,22,22,22,22,22,63,6 ,6 ,6 ,6 ,6 ,6 , /*55*/ 
/*56*/ 6 ,10,10,10,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,o ,o ,o ,o ,1 ,26,26,1 ,28,28,28,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,63,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,1 ,22,5 ,5 ,56,5 ,5 ,5 ,56,6 ,6 ,6 ,6 , /*56*/ 
/*57*/ 6 ,10,10,10,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,o ,o ,o ,o ,26,28,28,28,28,28,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,63,54,54,8 ,54,54,54,54,54,54,54,54,54,1 ,22,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 , /*57*/ 
/*58*/ 6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,o ,o ,o ,o ,26,28,28,28,28,28,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,1 ,28,28,28,28,28,28,28,28,28,28,12,12,12,22,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,6 ,6 , /*58*/ 
/*59*/ 6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,o ,o ,o ,o ,26,28,28,28,28,28,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,EE,28,28,28,28,28,28,28,28,28,28,28,FF,FF,FF,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,3 ,31,13, /*59*/ 
/*60*/ 6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,6 ,o ,o ,o ,o ,EE,28,28,28,28,28,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,1 ,28,28,28,28,28,28,28,28,28,28,12,12,12,22,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,6 ,6 , /*60*/ 
/*61*/ 6 ,6 ,6 ,6 ,6 ,11,11,11,11,9 ,9 ,9 ,9 ,9 ,9 ,9 ,11,11,11,6 ,o ,o ,o ,o ,26,28,28,28,28,28,28,5 ,5 ,5 ,5 ,5 ,5 ,5 ,63,54,54,8 ,54,54,54,54,54,54,54,54,54,1 ,22,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 , /*61*/ 
/*62*/ 9 ,9 ,9 ,9 ,9 ,11,11,11,11,10,10,10,10,10,10,10,11,11,11,6 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,26,26,26,1 ,63,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,1 ,22,5 ,5 ,5 ,5 ,5 ,5 ,59,6 ,6 ,6 ,6 , /*62*/ 
/*63*/ 9 ,9 ,9 ,9 ,9 ,11,11,11,11,10,10,10,10,10,10,10,11,11,11,6 ,28,28,28,28,22,22,22,22,22,22,22,22,22,22,6 ,9 ,9 ,9 ,6 ,22,22,4 ,22,II,GG,HH,22,22,22,22,22,22,22,22,22,22,22,63,6 ,6 ,6 ,6 ,6 ,6   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef GG
    #undef HH
    #undef II
    #undef o
    },
    {        // tileDictionary
      SFG_TD(14, 7,0,0),SFG_TD(18, 0,0,1),SFG_TD(18, 0,2,1),SFG_TD(14, 6,0,4), // 0 
      SFG_TD(14, 0,4,4),SFG_TD(14,20,4,4),SFG_TD(29, 0,3,4),SFG_TD(27, 2,3,4), // 4 
      SFG_TD(25, 6,4,4),SFG_TD(14,31,0,0),SFG_TD(26,31,1,0),SFG_TD(30,31,0,0), // 8 
      SFG_TD(31, 0,2,2),SFG_TD(14, 0,6,6),SFG_TD(14, 8,0,0),SFG_TD(22, 0,2,3), // 12 
      SFG_TD(18, 8,6,0),SFG_TD(17, 9,6,0),SFG_TD(16,10,6,0),SFG_TD(15,11,6,0), // 16 
      SFG_TD(14, 7,3,3),SFG_TD(29, 0,3,3),SFG_TD(18, 0,5,2),SFG_TD(18, 7,4,0), // 20 
      SFG_TD(14,20,3,4),SFG_TD(13,21,0,4),SFG_TD(16, 2,0,1),SFG_TD(18, 4,6,3), // 24 
      SFG_TD(14,17,0,4),SFG_TD(18, 8,3,3),SFG_TD(13,21,0,0),SFG_TD( 4,30,0,0), // 28 
      SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0), // 32 
      SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(30, 0,3,6), // 36 
      SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0), // 40 
      SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(14, 8,0,0), // 44 
      SFG_TD(29,17,5,5),SFG_TD(29,18,5,5),SFG_TD(29,19,5,5),SFG_TD(14, 8,0,0), // 48 
      SFG_TD(14, 8,0,0),SFG_TD(14, 7,0,6),SFG_TD(25, 5,4,4),SFG_TD(16, 5,1,0), // 52 
      SFG_TD(18,16,1,0),SFG_TD(22,12,1,0),SFG_TD(26, 8,0,0),SFG_TD(18,16,0,0), // 56 
      SFG_TD(17,17,1,0),SFG_TD(21,13,0,0),SFG_TD(25, 9,1,0),SFG_TD(14, 0,5,5)  // 60 
      },                    // tileDictionary
    {0 ,4 ,7 ,5 ,11,15,3 }, // textureIndices
    7,                      // doorTextureIndex
    3,                      // floorColor
    18,                     // ceilingColor
    {62, 2 , 144},          // player start: x, y, direction
    1,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_BULLETS, {28,1}},{SFG_LEVEL_ELEMENT_BULLETS, {27,1}},
      {SFG_LEVEL_ELEMENT_BULLETS, {56,2}},{SFG_LEVEL_ELEMENT_HEALTH, {21,4}},
      {SFG_LEVEL_ELEMENT_LOCK1, {12,5}},{SFG_LEVEL_ELEMENT_CARD2, {10,5}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {5,7}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {45,8}},
      {SFG_LEVEL_ELEMENT_BARREL, {45,9}},{SFG_LEVEL_ELEMENT_RUIN, {40,9}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {20,9}},{SFG_LEVEL_ELEMENT_RUIN, {41,10}},
      {SFG_LEVEL_ELEMENT_RUIN, {39,10}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {20,10}},
      {SFG_LEVEL_ELEMENT_HEALTH, {3,10}},{SFG_LEVEL_ELEMENT_RUIN, {39,11}},
      {SFG_LEVEL_ELEMENT_HEALTH, {61,12}},{SFG_LEVEL_ELEMENT_BULLETS, {58,12}},
      {SFG_LEVEL_ELEMENT_BULLETS, {57,12}},{SFG_LEVEL_ELEMENT_HEALTH, {11,12}},
      {SFG_LEVEL_ELEMENT_BULLETS, {41,15}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {23,15}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {36,16}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {18,16}},
      {SFG_LEVEL_ELEMENT_BULLETS, {53,17}},{SFG_LEVEL_ELEMENT_BULLETS, {53,18}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {14,18}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {24,19}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {7,20}},{SFG_LEVEL_ELEMENT_BULLETS, {7,21}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {61,23}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {42,23}},
      {SFG_LEVEL_ELEMENT_BARREL, {23,23}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {61,26}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {37,26}},{SFG_LEVEL_ELEMENT_HEALTH, {43,28}},
      {SFG_LEVEL_ELEMENT_BULLETS, {16,28}},{SFG_LEVEL_ELEMENT_RUIN, {53,29}},
      {SFG_LEVEL_ELEMENT_BULLETS, {16,29}},{SFG_LEVEL_ELEMENT_HEALTH, {57,31}},
      {SFG_LEVEL_ELEMENT_RUIN, {44,31}},{SFG_LEVEL_ELEMENT_BULLETS, {43,32}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {43,39}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {41,39}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {26,40}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {41,41}},
      {SFG_LEVEL_ELEMENT_HEALTH, {36,45}},{SFG_LEVEL_ELEMENT_BULLETS, {35,45}},
      {SFG_LEVEL_ELEMENT_TREE, {32,45}},{SFG_LEVEL_ELEMENT_BULLETS, {7,45}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {37,46}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {43,47}},
      {SFG_LEVEL_ELEMENT_LOCK1, {23,48}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {44,50}},
      {SFG_LEVEL_ELEMENT_COLUMN, {29,50}},{SFG_LEVEL_ELEMENT_HEALTH, {39,52}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {34,52}},{SFG_LEVEL_ELEMENT_BULLETS, {39,53}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {34,53}},{SFG_LEVEL_ELEMENT_TELEPORTER, {46,54}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {35,54}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {9,54}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {25,55}},{SFG_LEVEL_ELEMENT_BARREL, {18,55}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {11,55}},{SFG_LEVEL_ELEMENT_TERMINAL, {57,56}},
      {SFG_LEVEL_ELEMENT_COLUMN, {23,56}},{SFG_LEVEL_ELEMENT_COLUMN, {20,56}},
      {SFG_LEVEL_ELEMENT_HEALTH, {4,56}},{SFG_LEVEL_ELEMENT_TREE, {37,57}},
      {SFG_LEVEL_ELEMENT_TREE, {32,57}},{SFG_LEVEL_ELEMENT_BULLETS, {20,58}},
      {SFG_LEVEL_ELEMENT_CARD1, {8,58}},{SFG_LEVEL_ELEMENT_FINISH, {62,59}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {58,59}},{SFG_LEVEL_ELEMENT_LOCK2, {52,59}},
      {SFG_LEVEL_ELEMENT_CARD0, {41,59}},{SFG_LEVEL_ELEMENT_LOCK0, {38,59}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {28,59}},{SFG_LEVEL_ELEMENT_BULLETS, {20,60}},
      {SFG_LEVEL_ELEMENT_TREE, {37,61}},{SFG_LEVEL_ELEMENT_TREE, {32,61}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {57,62}},{SFG_LEVEL_ELEMENT_ROCKETS, {49,62}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level2 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (20 | SFG_TILE_PROPERTY_DOOR)
    #define BB (32 | SFG_TILE_PROPERTY_DOOR)
    #define CC (22 | SFG_TILE_PROPERTY_ELEVATOR)
    #define DD (40 | SFG_TILE_PROPERTY_SQUEEZER)
    #define EE (44 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 23,23,23,23,23,23,23,23,23,20,4 ,4 ,4 ,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 , /*0 */ 
/*1 */ 23,22,22,22,22,22,22,22,22,16,16,4 ,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,20,16,16,16,16,16,20,4 ,2 ,2 ,2 ,48,3 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,2 ,o ,o ,48,4 ,4 ,4 ,4 ,4 ,4 ,4 , /*1 */ 
/*2 */ 23,22,22,22,22,22,22,22,22,o ,o ,3 ,o ,o ,18,18,18,18,18,18,18,o ,o ,46,o ,o ,o ,o ,o ,16,20,21,19,19,19,19,4 ,2 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,48,4 ,3 ,o ,5 ,5 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 , /*2 */ 
/*3 */ 23,22,22,22,22,22,22,22,22,17,17,3 ,17,17,18,18,18,18,18,18,18,17,17,47,17,17,17,17,o ,16,AA,19,19,19,19,19,2 ,o ,o ,5 ,5 ,o ,o ,3 ,48,4 ,5 ,5 ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,6 ,7 ,6 ,4 ,4 ,4 , /*3 */ 
/*4 */ 23,22,22,22,22,22,22,22,22,o ,3 ,o ,o ,o ,18,18,18,18,18,18,18,o ,o ,46,o ,o ,o ,17,o ,16,20,19,19,19,19,19,3 ,o ,o ,o ,o ,2 ,4 ,4 ,4 ,4 ,2 ,o ,3 ,3 ,4 ,o ,2 ,4 ,4 ,4 ,4 ,4 ,10,11,11,4 ,4 ,4 , /*4 */ 
/*5 */ 23,22,22,22,22,22,22,22,22,16,4 ,4 ,16,16,16,16,16,16,16,16,16,16,16,16,16,16,o ,17,o ,16,20,16,16,16,16,16,20,20,20,20,20,20,20,20,20,4 ,4 ,4 ,4 ,4 ,4 ,8 ,4 ,4 ,4 ,4 ,9 ,9 ,10,4 ,11,12,4 ,4 , /*5 */ 
/*6 */ 23,22,22,22,22,22,23,23,23,20,4 ,4 ,4 ,20,20,20,20,20,20,20,20,20,20,20,20,16,o ,17,o ,16,20,20,20,AA,20,20,20,19,19,19,19,19,19,19,20,4 ,4 ,4 ,4 ,4 ,9 ,9 ,4 ,4 ,4 ,4 ,9 ,4 ,4 ,4 ,4 ,12,4 ,4 , /*6 */ 
/*7 */ 23,22,22,22,22,22,23,23,23,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,16,o ,17,o ,16,20,20,20,19,19,19,19,19,19,19,19,19,19,19,20,4 ,4 ,4 ,4 ,10,10,9 ,10,11,4 ,8 ,8 ,4 ,4 ,13,13,12,4 ,4 , /*7 */ 
/*8 */ 23,22,22,22,22,22,23,23,23,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,4 ,3 ,18,18,18,16,20,20,20,20,20,20,20,20,20,24,24,29,29,29,20,4 ,4 ,4 ,11,11,4 ,4 ,4 ,12,12,8 ,4 ,4 ,13,13,4 ,4 ,4 ,4 , /*8 */ 
/*9 */ 23,22,22,22,22,22,23,23,23,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,4 ,4 ,18,18,18,16,20,29,29,29,29,29,29,29,20,25,25,29,29,29,20,4 ,4 ,12,12,4 ,4 ,4 ,4 ,4 ,4 ,8 ,4 ,4 ,13,4 ,4 ,4 ,4 ,4 , /*9 */ 
/*10*/ 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,20,20,20,4 ,3 ,18,18,18,3 ,20,29,29,29,29,29,29,29,20,26,26,29,20,29,20,4 ,4 ,12,12,4 ,4 ,15,9 ,9 ,8 ,8 ,13,13,13,13,15,4 ,4 ,4 , /*10*/ 
/*11*/ 30,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,41,o ,o ,20,16,18,18,18,16,20,29,29,34,34,34,29,29,20,27,27,29,20,29,20,4 ,4 ,13,13,4 ,4 ,15,8 ,8 ,8 ,8 ,13,13,13,13,13,15,4 ,4 , /*11*/ 
/*12*/ 30,40,41,41,41,41,41,40,40,40,40,40,41,41,41,41,41,41,40,40,40,41,o ,o ,20,16,18,18,18,16,20,29,29,34,34,34,29,29,20,28,28,29,20,29,20,4 ,4 ,13,13,13,4 ,13,8 ,8 ,8 ,8 ,13,13,4 ,13,13,13,4 ,4 , /*12*/ 
/*13*/ 30,40,41,40,40,40,41,41,41,41,40,40,41,40,40,40,40,41,40,40,40,41,o ,o ,31,16,18,18,18,16,20,29,29,29,29,29,29,29,20,29,29,29,20,29,20,4 ,4 ,4 ,4 ,13,4 ,8 ,8 ,8 ,8 ,8 ,13,13,4 ,4 ,13,13,6 ,4 , /*13*/ 
/*14*/ 30,40,41,40,40,40,40,40,40,41,40,40,41,40,40,41,41,41,40,40,40,41,o ,o ,32,16,18,18,18,16,20,29,29,29,29,29,29,29,20,29,29,29,20,29,20,4 ,4 ,4 ,4 ,13,13,8 ,8 ,8 ,8 ,8 ,12,13,4 ,6 ,13,13,7 ,4 , /*14*/ 
/*15*/ 30,40,41,40,40,40,40,40,40,41,41,41,41,40,40,41,40,40,40,40,42,41,o ,36,31,16,o ,17,o ,16,20,20,20,29,29,29,20,20,20,32,32,32,32,29,20,20,4 ,4 ,4 ,13,13,8 ,8 ,8 ,8 ,8 ,8 ,13,4 ,7 ,12,12,6 ,4 , /*15*/ 
/*16*/ 30,40,41,40,40,40,40,40,40,40,40,40,40,40,40,41,40,40,40,40,43,41,o ,35,20,16,o ,17,o ,16,20,19,19,28,28,28,19,19,20,29,29,29,29,29,29,20,4 ,4 ,4 ,13,13,8 ,8 ,8 ,8 ,8 ,8 ,13,13,6 ,11,11,4 ,4 , /*16*/ 
/*17*/ 30,40,41,41,41,41,41,41,40,40,40,40,40,40,40,41,40,40,41,41,41,41,o ,16,AA,16,o ,17,o ,16,20,19,19,27,27,27,19,19,20,29,30,31,31,30,29,31,4 ,4 ,4 ,13,13,12,8 ,8 ,8 ,8 ,12,13,13,4 ,11,10,10,4 , /*17*/ 
/*18*/ 30,40,40,40,40,40,40,41,40,40,40,40,40,41,40,41,40,40,41,40,43,41,o ,35,20,16,o ,17,o ,16,20,19,19,26,26,26,19,19,20,29,31,31,31,31,29,30,4 ,4 ,4 ,4 ,13,13,13,13,10,13,13,13,4 ,4 ,4 ,10,9 ,4 , /*18*/ 
/*19*/ 30,40,40,40,40,40,40,41,40,40,41,40,40,41,40,41,40,40,41,40,42,41,o ,36,31,16,o ,17,o ,16,20,19,19,25,25,25,19,19,20,29,31,31,31,31,29,30,4 ,4 ,15,15,15,13,13,13,13,13,13,4 ,4 ,4 ,4 ,9 ,9 ,4 , /*19*/ 
/*20*/ 30,41,41,41,41,40,40,41,41,41,41,40,40,41,40,41,41,41,41,40,40,41,o ,o ,32,16,o ,17,o ,16,20,19,19,24,24,24,19,19,20,29,30,31,31,30,29,31,4 ,4 ,15,4 ,15,4 ,4 ,14,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 ,8 ,4 , /*20*/ 
/*21*/ 30,40,40,40,40,40,40,40,40,40,41,40,40,41,40,40,40,40,40,40,40,41,o ,o ,31,16,o ,17,o ,16,20,19,19,19,19,19,19,19,20,29,29,29,29,29,29,20,4 ,4 ,15,4 ,11,11,11,12,4 ,4 ,o ,o ,3 ,4 ,8 ,8 ,4 ,4 , /*21*/ 
/*22*/ 30,40,40,40,40,40,40,40,40,40,41,40,40,41,40,40,40,40,40,40,40,41,o ,o ,20,16,o ,17,o ,16,16,19,19,19,19,25,21,19,20,24,25,26,27,28,28,20,4 ,4 ,15,15,4 ,11,4 ,4 ,4 ,o ,5 ,5 ,o ,1 ,o ,4 ,4 ,4 , /*22*/ 
/*23*/ 30,40,41,41,41,41,41,41,40,40,41,40,30,30,30,30,30,30,30,30,30,20,20,20,20,16,o ,17,o ,16,16,19,19,19,19,25,34,19,20,19,19,19,19,19,19,20,4 ,4 ,4 ,15,15,8 ,33,4 ,4 ,48,o ,o ,3 ,4 ,4 ,4 ,4 ,4 , /*23*/ 
/*24*/ 30,40,41,40,40,40,40,41,41,40,41,40,30,31,31,31,31,31,31,31,31,31,31,31,20,16,o ,17,o ,16,16,19,19,19,19,19,19,19,20,19,19,19,19,19,19,20,4 ,4 ,4 ,4 ,4 ,14,14,4 ,4 ,2 ,3 ,6 ,4 ,4 ,4 ,4 ,4 ,4 , /*24*/ 
/*25*/ 30,40,41,40,40,40,40,40,41,41,41,40,30,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,31,31,16,o ,17,o ,16,16,19,19,19,19,19,19,21,20,19,19,19,19,19,19,20,4 ,4 ,4 ,14,14,14,14,3 ,3 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 , /*25*/ 
/*26*/ 30,40,41,41,41,41,40,40,40,40,40,40,30,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,31,32,16,o ,17,o ,16,16,19,19,19,19,19,19,19,20,19,19,19,19,19,19,20,20,4 ,14,14,14,14,4 ,4 ,2 ,3 ,4 ,2 ,3 ,3 ,2 ,4 ,4 ,4 , /*26*/ 
/*27*/ 30,40,40,40,40,41,41,40,40,40,40,40,30,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,31,31,16,o ,17,o ,16,16,19,19,19,19,19,19,19,20,21,19,19,19,19,19,19,20,4 ,14,14,14,4 ,4 ,3 ,o ,o ,3 ,o ,o ,o ,o ,3 ,4 ,4 , /*27*/ 
/*28*/ 30,40,40,40,40,40,41,40,40,40,40,40,30,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,31,20,16,o ,17,o ,16,16,19,19,19,19,25,34,19,20,19,19,21,19,19,19,19,20,4 ,14,14,14,4 ,3 ,o ,o ,o ,o ,o ,5 ,5 ,5 ,1 ,4 ,4 , /*28*/ 
/*29*/ 30,40,40,40,40,40,41,40,40,40,40,40,30,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,31,20,16,o ,17,o ,16,16,19,19,19,19,25,21,19,20,20,20,20,20,20,16,16,20,4 ,14,14,14,8 ,o ,o ,2 ,o ,o ,o ,o ,o ,5 ,o ,3 ,4 , /*29*/ 
/*30*/ 30,40,30,40,40,41,41,41,40,40,30,40,30,31,o ,o ,o ,47,31,6 ,36,36,36,31,20,16,o ,17,o ,16,20,19,19,19,19,19,19,19,20,29,29,29,20,20,16,16,32,4 ,14,14,8 ,8 ,o ,3 ,3 ,3 ,o ,o ,4 ,3 ,o ,5 ,o ,4 , /*30*/ 
/*31*/ 30,43,30,41,41,41,41,41,41,41,30,43,30,31,o ,o ,o ,o ,46,o ,35,35,35,31,31,16,o ,17,o ,16,20,19,19,24,24,24,19,19,20,29,29,29,20,20,16,16,32,4 ,4 ,4 ,8 ,o ,4 ,4 ,4 ,4 ,o ,4 ,4 ,4 ,o ,5 ,o ,4 , /*31*/ 
/*32*/ 30,42,30,40,40,30,30,30,40,40,30,42,30,31,o ,o ,o ,47,46,o ,16,16,16,31,32,16,o ,17,o ,16,20,19,19,25,25,25,19,19,20,29,29,29,20,20,16,16,20,20,20,20,3 ,o ,4 ,4 ,4 ,4 ,2 ,4 ,4 ,4 ,o ,5 ,o ,4 , /*32*/ 
/*33*/ 30,40,40,40,40,30,30,30,40,40,40,40,30,31,4 ,3 ,3 ,31,31,31,32,BB,32,32,31,16,o ,17,o ,16,20,19,19,26,26,26,19,19,20,29,29,29,20,20,19,19,19,19,19,20,3 ,o ,o ,4 ,4 ,3 ,3 ,4 ,4 ,4 ,3 ,o ,1 ,4 , /*33*/ 
/*34*/ 30,30,30,30,30,30,30,30,30,30,30,30,30,4 ,4 ,o ,o ,4 ,4 ,32,16,16,16,16,20,16,o ,17,o ,16,20,19,19,27,27,27,19,19,20,29,29,29,20,20,19,19,19,19,19,20,6 ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,3 ,2 ,48,4 ,4 , /*34*/ 
/*35*/ 30,30,30,30,30,30,30,30,30,30,30,30,30,4 ,2 ,o ,o ,2 ,4 ,32,16,16,16,16,20,16,o ,17,o ,16,20,19,19,28,28,28,19,19,20,29,29,29,20,20,19,19,19,19,19,20,4 ,o ,5 ,o ,1 ,4 ,4 ,4 ,1 ,3 ,4 ,4 ,4 ,4 , /*35*/ 
/*36*/ 30,30,30,30,30,30,30,30,30,30,30,30,30,4 ,4 ,o ,o ,o ,4 ,32,16,16,16,16,20,16,o ,17,o ,16,16,20,20,29,29,29,20,20,20,29,29,29,20,34,19,25,25,25,19,34,4 ,3 ,o ,5 ,o ,3 ,3 ,1 ,2 ,2 ,4 ,4 ,4 ,4 , /*36*/ 
/*37*/ 30,30,30,30,30,30,30,30,30,30,30,30,30,32,32,4 ,3 ,3 ,4 ,32,16,16,16,16,31,16,o ,17,o ,16,20,29,29,29,29,29,29,29,29,29,29,29,20,34,19,25,25,25,19,34,4 ,4 ,3 ,o ,2 ,3 ,3 ,1 ,o ,o ,3 ,4 ,4 ,4 , /*37*/ 
/*38*/ 30,40,40,40,40,40,40,40,40,40,42,43,41,o ,o ,o ,o ,36,35,32,16,16,16,16,32,16,o ,17,o ,16,20,29,29,29,29,29,29,29,29,29,29,29,20,34,19,25,25,25,19,34,4 ,4 ,4 ,4 ,3 ,3 ,4 ,4 ,o ,5 ,o ,3 ,6 ,6 , /*38*/ 
/*39*/ 30,40,41,40,40,40,40,40,40,40,42,43,41,o ,o ,o ,o ,o ,16,BB,16,16,16,16,31,16,o ,17,o ,16,20,20,20,34,34,34,20,20,20,20,20,20,20,20,19,19,19,19,19,20,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,o ,5 ,5 ,o ,o ,7 , /*39*/ 
/*40*/ 30,40,40,40,40,40,40,40,40,40,40,40,41,o ,o ,o ,o ,36,35,32,16,16,16,16,20,16,o ,17,o ,16,34,20,20,20,20,20,20,34,34,34,20,20,20,20,20,20,20,20,20,20,4 ,4 ,4 ,4 ,4 ,4 ,4 ,2 ,o ,5 ,o ,3 ,6 ,6 , /*40*/ 
/*41*/ 30,40,40,40,40,40,40,40,40,40,40,40,30,32,32,38,38,38,32,32,16,16,16,16,20,16,o ,17,o ,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,20,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,2 ,o ,o ,o ,o ,4 ,4 ,4 , /*41*/ 
/*42*/ 30,40,40,40,40,40,40,40,40,40,40,40,30,o ,o ,o ,o ,o ,o ,32,16,16,34,AA,20,16,o ,17,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,16,34,20,7 ,7 ,7 ,7 ,7 ,4 ,4 ,1 ,5 ,o ,o ,3 ,4 ,4 ,4 , /*42*/ 
/*43*/ 30,40,40,40,40,40,40,40,40,40,40,40,30,o ,o ,o ,o ,o ,o ,32,16,16,34,16,16,16,o ,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,o ,16,16,31,7 ,7 ,o ,o ,7 ,7 ,4 ,1 ,5 ,o ,2 ,3 ,4 ,4 ,4 , /*43*/ 
/*44*/ 30,40,40,40,42,43,41,43,42,40,40,40,41,o ,o ,o ,o ,o ,o ,32,32,32,34,34,20,16,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,17,o ,16,34,20,7 ,o ,o ,o ,o ,7 ,4 ,3 ,5 ,5 ,o ,2 ,4 ,4 ,4 , /*44*/ 
/*45*/ 30,30,30,30,30,30,41,30,30,30,30,30,30,o ,30,o ,30,o ,30,32,32,32,20,20,20,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,o ,17,o ,16,20,7 ,7 ,o ,o ,o ,o ,7 ,4 ,4 ,o ,5 ,5 ,o ,4 ,4 ,4 , /*45*/ 
/*46*/ 44,44,44,44,44,34,46,34,44,44,44,44,44,32,32,32,32,32,32,32,32,32,20,20,20,20,20,31,32,31,20,20,31,32,31,20,20,31,16,31,20,20,16,o ,17,o ,16,20,7 ,o ,o ,o ,o ,7 ,7 ,4 ,4 ,3 ,o ,o ,o ,o ,4 ,4 , /*46*/ 
/*47*/ 44,46,46,46,39,o ,o ,o ,39,46,46,46,44,44,44,44,44,44,44,20,20,20,20,30,32,32,16,32,32,30,20,20,20,20,20,20,20,32,32,32,20,20,16,o ,17,o ,16,20,7 ,o ,o ,o ,o ,7 ,4 ,4 ,1 ,3 ,3 ,o ,o ,o ,3 ,4 , /*47*/ 
/*48*/ 44,46,46,46,39,o ,o ,o ,39,46,46,46,44,CC,CC,CC,CC,CC,44,20,20,16,16,16,16,16,16,16,16,16,16,16,20,20,20,20,20,20,20,20,20,20,16,o ,17,o ,16,20,7 ,7 ,o ,o ,7 ,7 ,4 ,1 ,1 ,3 ,3 ,4 ,o ,5 ,o ,4 , /*48*/ 
/*49*/ 44,46,46,46,39,o ,o ,o ,39,46,46,46,44,CC,DD,DD,DD,CC,44,20,31,16,16,16,16,16,16,16,16,16,16,16,31,16,16,16,16,16,16,16,16,20,16,o ,17,o ,16,20,7 ,7 ,7 ,o ,7 ,6 ,4 ,1 ,3 ,3 ,3 ,4 ,o ,5 ,o ,4 , /*49*/ 
/*50*/ 44,46,46,46,39,o ,o ,o ,39,46,46,46,44,CC,DD,DD,DD,CC,44,20,31,16,16,16,16,16,16,16,16,16,16,16,31,16,16,16,16,16,16,16,16,AA,16,o ,17,o ,16,20,4 ,4 ,6 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,o ,3 ,4 , /*50*/ 
/*51*/ 44,44,EE,44,44,o ,o ,o ,44,44,EE,44,44,CC,DD,DD,DD,CC,44,20,20,16,16,16,16,16,16,16,16,16,16,16,20,20,20,20,16,16,20,20,20,20,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,6 ,6 ,6 ,4 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,4 ,4 , /*51*/ 
/*52*/ 44,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,44,CC,CC,CC,CC,CC,44,20,20,16,16,16,20,20,20,20,20,16,16,16,20,20,20,20,16,16,20,16,16,AA,16,o ,17,3 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,3 ,3 ,o ,o ,3 ,1 ,3 ,4 ,4 ,4 ,4 , /*52*/ 
/*53*/ 44,44,44,44,34,o ,o ,o ,34,44,44,44,44,36,36,36,36,36,44,35,35,16,16,16,35,35,37,35,35,16,16,16,35,35,37,20,16,16,20,16,16,20,16,o ,17,o ,16,20,4 ,4 ,4 ,4 ,3 ,2 ,o ,o ,o ,o ,o ,1 ,3 ,4 ,4 ,4 , /*53*/ 
/*54*/ 4 ,4 ,4 ,4 ,44,o ,o ,o ,44,36,46,46,46,46,46,46,46,46,44,36,36,o ,o ,o ,36,36,37,36,36,o ,o ,o ,36,36,37,20,16,16,20,16,16,20,16,o ,17,o ,16,20,4 ,4 ,3 ,48,2 ,o ,o ,o ,5 ,5 ,o ,o ,1 ,4 ,4 ,4 , /*54*/ 
/*55*/ 4 ,4 ,4 ,4 ,44,o ,o ,o ,44,36,46,o ,o ,o ,o ,o ,o ,o ,44,o ,o ,o ,o ,o ,o ,o ,37,46,o ,o ,o ,o ,o ,46,37,20,16,16,34,16,16,20,16,o ,17,o ,16,20,5 ,5 ,o ,o ,o ,o ,5 ,5 ,5 ,5 ,5 ,o ,1 ,1 ,4 ,4 , /*55*/ 
/*56*/ 4 ,4 ,4 ,44,44,o ,o ,o ,44,36,46,45,45,o ,45,45,o ,o ,44,o ,o ,o ,o ,o ,o ,o ,37,46,o ,o ,o ,o ,o ,46,37,20,16,16,16,16,16,20,16,o ,17,o ,3 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 , /*56*/ 
/*57*/ 4 ,4 ,4 ,o ,o ,o ,o ,o ,o ,o ,46,45,45,o ,45,45,o ,o ,o ,o ,o ,o ,o ,o ,o ,30,37,46,o ,36,36,36,o ,46,37,20,16,16,16,16,16,20,16,o ,17,o ,o ,3 ,o ,5 ,5 ,5 ,o ,o ,o ,2 ,3 ,48,1 ,o ,5 ,5 ,5 ,6 , /*57*/ 
/*58*/ 4 ,4 ,4 ,o ,o ,o ,o ,o ,o ,o ,46,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,46,37,46,o ,36,36,36,o ,46,37,20,16,16,16,16,16,20,16,o ,17,o ,o ,3 ,2 ,o ,o ,o ,o ,2 ,2 ,4 ,4 ,4 ,4 ,1 ,o ,5 ,5 ,6 , /*58*/ 
/*59*/ 4 ,4 ,4 ,4 ,2 ,o ,o ,o ,o ,o ,46,45,45,o ,45,45,o ,o ,o ,o ,o ,o ,o ,o ,o ,46,37,46,o ,36,36,36,o ,46,37,20,31,31,31,31,31,20,16,o ,17,o ,3 ,4 ,2 ,2 ,o ,o ,2 ,2 ,4 ,4 ,4 ,4 ,4 ,3 ,1 ,o ,o ,4 , /*59*/ 
/*60*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,3 ,o ,o ,46,45,45,o ,45,45,o ,o ,o ,o ,30,46,46,46,46,46,37,46,o ,o ,o ,o ,o ,46,37,20,20,20,20,20,20,20,16,o ,17,o ,16,20,2 ,2 ,2 ,2 ,2 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,3 ,3 ,4 ,4 , /*60*/ 
/*61*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,3 ,46,o ,o ,o ,o ,o ,o ,o ,37,37,37,37,37,37,37,37,37,30,o ,o ,o ,o ,o ,30,37,20,20,20,20,20,20,20,16,o ,17,22,16,20,2 ,2 ,2 ,3 ,4 ,3 ,3 ,4 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,4 , /*61*/ 
/*62*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,2 ,2 ,o ,o ,3 ,3 ,37,4 ,4 ,4 ,4 ,4 ,4 ,4 ,32,32,32,32,CC,32,32,32,32,20,20,20,20,20,20,20,4 ,3 ,17,2 ,3 ,20,3 ,3 ,4 ,4 ,4 ,4 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 , /*62*/ 
/*63*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,32,32,32,32,32,32,32,32,32,20,20,20,20,20,20,20,20,4 ,4 ,4 ,4 ,20,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef o
    },
    {        // tileDictionary
      SFG_TD(14,10,0,0),SFG_TD(16, 8,0,0),SFG_TD(14, 8,0,0),SFG_TD(16, 6,0,0), // 0 
      SFG_TD(14, 0,0,0),SFG_TD(13,12,0,0),SFG_TD(21, 0,1,1),SFG_TD(14, 0,2,2), // 4 
      SFG_TD(14,24,0,0),SFG_TD(16,22,0,0),SFG_TD(18,20,0,0),SFG_TD(20,18,0,0), // 8 
      SFG_TD(22,16,0,0),SFG_TD(24,14,0,0),SFG_TD(24, 4,0,0),SFG_TD(31, 7,0,0), // 12 
      SFG_TD(17, 5,1,1),SFG_TD(15, 9,3,0),SFG_TD(23, 1,3,0),SFG_TD(17,10,1,1), // 16 
      SFG_TD(21, 0,1,4),SFG_TD(22, 5,5,1),SFG_TD(14,31,3,0),SFG_TD(26, 0,1,7), // 20 
      SFG_TD(18, 9,1,1),SFG_TD(19, 8,1,1),SFG_TD(20, 7,1,1),SFG_TD(21, 6,1,1), // 24 
      SFG_TD(22, 5,1,1),SFG_TD(23, 4,1,1),SFG_TD(21, 0,3,3),SFG_TD(21, 0,5,5), // 28 
      SFG_TD(21, 0,6,6),SFG_TD(14, 4,0,0),SFG_TD(21, 0,4,4),SFG_TD(16, 6,1,1), // 32 
      SFG_TD(15, 7,1,1),SFG_TD(21, 0,5,1),SFG_TD(17, 2,6,6),SFG_TD(16, 3,1,1), // 36 
      SFG_TD( 9,15,1,3),SFG_TD(14, 4,3,3),SFG_TD(11,13,3,3),SFG_TD(13,11,3,3), // 40 
      SFG_TD(18, 0,1,1),SFG_TD(15, 9,5,1),SFG_TD(14, 9,0,3),SFG_TD(15, 8,3,3), // 44 
      SFG_TD(16, 6,1,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 48 
      SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 52 
      SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 56 
      SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0)  // 60 
      },                    // tileDictionary
    {9 ,3 ,6 ,11,12,2 ,5 }, // textureIndices
    13,                     // doorTextureIndex
    18,                     // floorColor
    26,                     // ceilingColor
    {44, 61, 64 },          // player start: x, y, direction
    0,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_BULLETS, {55,2}},{SFG_LEVEL_ELEMENT_ROCKETS, {12,2}},
      {SFG_LEVEL_ELEMENT_RUIN, {11,2}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {25,3}},
      {SFG_LEVEL_ELEMENT_RUIN, {11,3}},{SFG_LEVEL_ELEMENT_RUIN, {10,4}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {51,5}},{SFG_LEVEL_ELEMENT_BARREL, {32,5}},
      {SFG_LEVEL_ELEMENT_LAMP, {31,5}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {61,6}},
      {SFG_LEVEL_ELEMENT_HEALTH, {26,7}},{SFG_LEVEL_ELEMENT_BARREL, {55,10}},
      {SFG_LEVEL_ELEMENT_RUIN, {29,10}},{SFG_LEVEL_ELEMENT_BULLETS, {23,11}},
      {SFG_LEVEL_ELEMENT_BULLETS, {22,11}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {13,11}},
      {SFG_LEVEL_ELEMENT_BULLETS, {54,12}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {43,13}},
      {SFG_LEVEL_ELEMENT_LAMP, {50,14}},{SFG_LEVEL_ELEMENT_RUIN, {34,14}},
      {SFG_LEVEL_ELEMENT_RUIN, {33,14}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {50,15}},
      {SFG_LEVEL_ELEMENT_RUIN, {35,15}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {10,15}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {11,16}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {55,18}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {50,20}},{SFG_LEVEL_ELEMENT_LAMP, {31,20}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {5,21}},{SFG_LEVEL_ELEMENT_TELEPORTER, {51,23}},
      {SFG_LEVEL_ELEMENT_LAMP, {25,23}},{SFG_LEVEL_ELEMENT_TREE, {36,24}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {32,24}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {34,25}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {25,25}},{SFG_LEVEL_ELEMENT_BULLETS, {18,25}},
      {SFG_LEVEL_ELEMENT_BARREL, {55,26}},{SFG_LEVEL_ELEMENT_BARREL, {54,26}},
      {SFG_LEVEL_ELEMENT_PLASMA, {48,26}},{SFG_LEVEL_ELEMENT_LAMP, {39,26}},
      {SFG_LEVEL_ELEMENT_HEALTH, {37,26}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {32,26}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {17,26}},{SFG_LEVEL_ELEMENT_PLASMA, {48,27}},
      {SFG_LEVEL_ELEMENT_TREE, {36,27}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {20,27}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {19,27}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {9,27}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {48,28}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {17,28}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {18,29}},{SFG_LEVEL_ELEMENT_LAMP, {31,31}},
      {SFG_LEVEL_ELEMENT_HEALTH, {50,32}},{SFG_LEVEL_ELEMENT_BARREL, {16,32}},
      {SFG_LEVEL_ELEMENT_BARREL, {15,32}},{SFG_LEVEL_ELEMENT_HEALTH, {55,33}},
      {SFG_LEVEL_ELEMENT_BARREL, {25,33}},{SFG_LEVEL_ELEMENT_BULLETS, {10,33}},
      {SFG_LEVEL_ELEMENT_HEALTH, {9,33}},{SFG_LEVEL_ELEMENT_CARD1, {3,33}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {56,36}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {44,36}},
      {SFG_LEVEL_ELEMENT_LAMP, {30,36}},{SFG_LEVEL_ELEMENT_CARD0, {46,37}},
      {SFG_LEVEL_ELEMENT_RUIN, {36,37}},{SFG_LEVEL_ELEMENT_RUIN, {35,38}},
      {SFG_LEVEL_ELEMENT_HEALTH, {31,38}},{SFG_LEVEL_ELEMENT_RUIN, {18,39}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {2,39}},{SFG_LEVEL_ELEMENT_RUIN, {18,40}},
      {SFG_LEVEL_ELEMENT_BARREL, {33,41}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {23,41}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {37,42}},{SFG_LEVEL_ELEMENT_LOCK1, {23,42}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {9,42}},{SFG_LEVEL_ELEMENT_BLOCKER, {8,42}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {7,42}},{SFG_LEVEL_ELEMENT_BLOCKER, {6,42}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {5,42}},{SFG_LEVEL_ELEMENT_BLOCKER, {4,42}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {3,42}},{SFG_LEVEL_ELEMENT_TERMINAL, {47,43}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {35,43}},{SFG_LEVEL_ELEMENT_BULLETS, {21,43}},
      {SFG_LEVEL_ELEMENT_HEALTH, {20,43}},{SFG_LEVEL_ELEMENT_ROCKETS, {16,43}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {52,44}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {51,44}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {37,44}},{SFG_LEVEL_ELEMENT_LAMP, {38,46}},
      {SFG_LEVEL_ELEMENT_LAMP, {26,47}},{SFG_LEVEL_ELEMENT_HEALTH, {61,49}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {57,49}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {10,49}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {2,49}},{SFG_LEVEL_ELEMENT_BARREL, {1,49}},
      {SFG_LEVEL_ELEMENT_RUIN, {44,50}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {35,50}},
      {SFG_LEVEL_ELEMENT_HEALTH, {33,50}},{SFG_LEVEL_ELEMENT_PLASMA, {15,50}},
      {SFG_LEVEL_ELEMENT_RUIN, {45,51}},{SFG_LEVEL_ELEMENT_RUIN, {43,51}},
      {SFG_LEVEL_ELEMENT_RUIN, {42,51}},{SFG_LEVEL_ELEMENT_LOCK0, {41,52}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {39,52}},{SFG_LEVEL_ELEMENT_RUIN, {9,52}},
      {SFG_LEVEL_ELEMENT_LAMP, {58,55}},{SFG_LEVEL_ELEMENT_BULLETS, {53,56}},
      {SFG_LEVEL_ELEMENT_BULLETS, {52,56}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {39,56}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {30,56}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {40,57}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {22,57}},{SFG_LEVEL_ELEMENT_RUIN, {46,60}},
      {SFG_LEVEL_ELEMENT_FINISH, {30,62}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level3 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (5 | SFG_TILE_PROPERTY_SQUEEZER)
    #define BB (19 | SFG_TILE_PROPERTY_DOOR)
    #define CC (24 | SFG_TILE_PROPERTY_DOOR)
    #define DD (56 | SFG_TILE_PROPERTY_ELEVATOR)
    #define EE (7 | SFG_TILE_PROPERTY_ELEVATOR)
    #define FF (8 | SFG_TILE_PROPERTY_DOOR)
    #define GG (63 | SFG_TILE_PROPERTY_DOOR)
    #define HH (47 | SFG_TILE_PROPERTY_SQUEEZER)
    #define II (4 | SFG_TILE_PROPERTY_SQUEEZER)
    #define JJ (51 | SFG_TILE_PROPERTY_ELEVATOR)
    #define KK (17 | SFG_TILE_PROPERTY_ELEVATOR)
    #define LL (50 | SFG_TILE_PROPERTY_ELEVATOR)
    #define MM (1 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 8 ,8 ,8 ,8 ,8 ,24,24,24,8 ,8 ,24,24,24,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,21,8 ,8 ,8 ,8 ,8 ,8 ,8 ,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,8 , /*0 */ 
/*1 */ 8 ,5 ,5 ,5 ,5 ,24,5 ,24,9 ,9 ,24,5 ,24,5 ,5 ,5 ,5 ,5 ,5 ,27,27,27,59,59,21,59,59,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*1 */ 
/*2 */ 8 ,5 ,24,24,24,9 ,5 ,9 ,9 ,9 ,9 ,5 ,9 ,9 ,5 ,5 ,5 ,5 ,5 ,27,27,27,59,59,21,59,59,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*2 */ 
/*3 */ 8 ,5 ,24,5 ,5 ,AA,AA,AA,AA,9 ,AA,AA,AA,9 ,5 ,5 ,5 ,5 ,5 ,27,27,27,59,5 ,5 ,5 ,59,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,32,32,32,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*3 */ 
/*4 */ 8 ,5 ,24,5 ,9 ,AA,AA,9 ,AA,AA,AA,AA,AA,9 ,5 ,5 ,5 ,5 ,57,9 ,59,59,59,5 ,5 ,5 ,59,59,59,9 ,9 ,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*4 */ 
/*5 */ 63,57,24,5 ,9 ,AA,AA,9 ,5 ,AA,AA,AA,9 ,9 ,5 ,5 ,5 ,5 ,57,59,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,57,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*5 */ 
/*6 */ 8 ,57,9 ,AA,AA,AA,AA,9 ,24,9 ,9 ,9 ,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,59,59,59,59,59,59,59,5 ,59,57,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,32,32,32,32,32,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*6 */ 
/*7 */ 8 ,57,9 ,AA,AA,AA,9 ,24,5 ,AA,AA,9 ,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,59,9 ,27,27,27,9 ,59,5 ,59,57,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*7 */ 
/*8 */ 53,57,9 ,AA,AA,AA,9 ,24,5 ,AA,AA,21,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,59,27,5 ,5 ,5 ,27,59,5 ,59,57,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*8 */ 
/*9 */ 8 ,57,24,5 ,24,5 ,24,24,5 ,AA,AA,21,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,59,27,5 ,5 ,5 ,27,59,5 ,59,57,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,32,32,32,32,32,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*9 */ 
/*10*/ 8 ,57,24,5 ,24,5 ,24,24,5 ,AA,AA,9 ,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,19,27,5 ,5 ,5 ,27,19,5 ,59,57,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*10*/ 
/*11*/ 63,57,9 ,9 ,9 ,AA,9 ,9 ,24,AA,AA,9 ,5 ,5 ,5 ,5 ,5 ,5 ,57,59,5 ,BB,27,5 ,5 ,5 ,27,BB,5 ,59,57,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,32,32,32,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*11*/ 
/*12*/ 8 ,57,9 ,AA,AA,AA,AA,AA,9 ,AA,AA,9 ,9 ,9 ,9 ,27,27,27,9 ,58,59,58,9 ,27,27,27,9 ,58,59,58,9 ,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,32,32,32,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*12*/ 
/*13*/ 8 ,57,9 ,9 ,AA,AA,AA,AA,9 ,AA,AA,AA,AA,9 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*13*/ 
/*14*/ 53,57,24,5 ,AA,AA,9 ,9 ,9 ,9 ,9 ,AA,AA,9 ,27,27,27,27,27,27,27,27,9 ,27,27,27,9 ,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*14*/ 
/*15*/ 8 ,57,24,5 ,AA,AA,5 ,24,5 ,AA,AA,AA,AA,9 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*15*/ 
/*16*/ 8 ,57,24,5 ,AA,AA,5 ,24,5 ,AA,AA,AA,9 ,9 ,27,27,27,27,27,27,27,27,9 ,27,9 ,27,9 ,27,27,27,27,27,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*16*/ 
/*17*/ 63,57,24,5 ,AA,AA,9 ,9 ,9 ,AA,AA,AA,9 ,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*17*/ 
/*18*/ 8 ,5 ,9 ,9 ,AA,AA,AA,AA,AA,AA,AA,AA,9 ,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,6 ,6 ,24,8 , /*18*/ 
/*19*/ 8 ,5 ,9 ,9 ,AA,9 ,AA,AA,AA,AA,5 ,5 ,24,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,23,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,8 , /*19*/ 
/*20*/ 53,5 ,9 ,9 ,AA,9 ,AA,AA,AA,AA,5 ,5 ,24,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,24,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,25,24,8 , /*20*/ 
/*21*/ 53,5 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,24,24,9 ,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,24,24,24,24,24,24,24,24,24,24,24,CC,24,24,24,24,24,24,24,24,24,24,42,24,8 , /*21*/ 
/*22*/ 8 ,5 ,19,59,59,59,63,63,63,59,59,59,19,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,6 ,24,24,42,24,8 , /*22*/ 
/*23*/ 8 ,5 ,BB,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,59,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,24,48,48,48,48,48,24,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,42,42,24,8 , /*23*/ 
/*24*/ 8 ,59,19,59,59,59,19,21,19,59,59,59,19,57,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,5 ,5 ,5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,24,24,24,8 , /*24*/ 
/*25*/ 53,27,27,27,27,27,27,27,27,27,27,27,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,24,48,48,48,48,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,26,6 ,24,24,24,6 ,6 ,6 ,24,24,24,6 ,6 ,6 ,24,24,24,6 ,26,6 ,6 ,8 ,8 , /*25*/ 
/*26*/ 8 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,23,5 ,5 ,5 ,48,5 ,6 ,6 ,6 ,6 ,6 ,6 ,24,26,26,24,6 ,24,24,24,6 ,6 ,6 ,24,6 ,24,6 ,6 ,6 ,24,24,24,6 ,26,6 ,6 ,DD,8 , /*26*/ 
/*27*/ 8 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,23,5 ,48,5 ,5 ,5 ,23,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,8 ,8 , /*27*/ 
/*28*/ 8 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,19,57,57,57,57,57,57,57,57,57,5 ,5 ,5 ,5 ,23,5 ,48,48,48,48,23,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,24,6 ,6 ,DD,8 , /*28*/ 
/*29*/ 8 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,19,59,59,59,58,58,59,59,59,19,5 ,5 ,5 ,5 ,23,5 ,5 ,5 ,5 ,5 ,23,6 ,6 ,6 ,6 ,6 ,26,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,8 ,8 , /*29*/ 
/*30*/ 8 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,BB,27,27,27,27,27,27,27,27,BB,5 ,5 ,5 ,5 ,24,48,48,48,63,11,22,63,20,CC,20,20,20,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,8 ,8 , /*30*/ 
/*31*/ 53,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,27,19,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,19,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,22,EE,EE,12,6 ,6 ,20,20,20,6 ,6 ,6 ,23,6 ,6 ,6 ,26,6 ,6 ,6 ,23,6 ,6 ,6 ,23,6 ,6 ,6 ,23,6 ,6 ,6 ,8 ,8 , /*31*/ 
/*32*/ 8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,14,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,10,EE,EE,22,47,47,18,4 ,4 ,18,47,47,47,47,47,47,63,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,33,8 , /*32*/ 
/*33*/ 8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,14,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,22,FF,63,47,47,18,4 ,4 ,18,47,47,47,47,47,47,63,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,33,8 , /*33*/ 
/*34*/ 63,o ,o ,o ,o ,63,63,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,55,8 , /*34*/ 
/*35*/ 8 ,o ,o ,o ,63,63,63,63,o ,o ,o ,o ,o ,3 ,3 ,3 ,61,61,63,24,63,61,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,18,13,18,4 ,55,DD, /*35*/ 
/*36*/ 8 ,o ,o ,o ,63,63,63,63,o ,o ,o ,o ,o ,3 ,3 ,3 ,61,63,62,62,62,63,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,13,13,13,4 ,55,DD, /*36*/ 
/*37*/ 63,o ,o ,o ,o ,63,63,o ,o ,o ,o ,o ,o ,62,62,62,63,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,46,46,46,46,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,13,13,13,4 ,55,8 , /*37*/ 
/*38*/ 8 ,o ,54,54,54,o ,o ,54,54,54,o ,o ,o ,62,62,62,GG,62,62,62,62,62,24,o ,o ,o ,o ,o ,o ,o ,8 ,53,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,46,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,18,13,13,13,18,18,4 ,53,8 , /*38*/ 
/*39*/ 8 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,15,15,62,62,62,63,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,46,46,16,16,16,16,16,16,16,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,13,13,13,13,18,4 ,4 ,55,8 , /*39*/ 
/*40*/ 8 ,3 ,JJ,JJ,JJ,3 ,3 ,JJ,JJ,JJ,3 ,14,14,3 ,3 ,3 ,61,63,62,62,62,63,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,46,4 ,16,HH,16,II,16,HH,16,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,13,13,13,4 ,4 ,4 ,4 ,55,8 , /*40*/ 
/*41*/ 63,3 ,JJ,JJ,JJ,3 ,3 ,JJ,JJ,JJ,3 ,3 ,3 ,3 ,3 ,3 ,61,61,63,24,63,61,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,46,46,4 ,4 ,47,4 ,47,4 ,47,4 ,4 ,8 ,18,4 ,4 ,4 ,4 ,4 ,13,13,13,4 ,4 ,4 ,4 ,55,8 , /*41*/ 
/*42*/ 8 ,3 ,JJ,JJ,JJ,3 ,3 ,JJ,JJ,JJ,3 ,14,14,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,47,4 ,47,4 ,47,4 ,4 ,13,45,4 ,4 ,4 ,4 ,4 ,18,13,18,4 ,4 ,4 ,4 ,55,8 , /*42*/ 
/*43*/ 8 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,15,15,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,o ,o ,8 ,53,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,45,4 ,45,4 ,45,4 ,4 ,13,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,53,8 , /*43*/ 
/*44*/ 8 ,o ,54,54,54,o ,o ,54,54,54,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,61,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,47,47,47,47,47,4 ,4 ,18,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*44*/ 
/*45*/ 8 ,o ,49,49,49,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,61,61,61,61,61,24,24,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,19,4 ,4 ,4 ,46,46,46,46,47,4 ,4 ,18,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*45*/ 
/*46*/ 63,o ,49,49,49,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,61,61,61,61,61,24,24,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,46,46,46,47,4 ,4 ,13,45,4 ,4 ,46,46,4 ,4 ,4 ,4 ,4 ,46,46,4 ,8 ,8 , /*46*/ 
/*47*/ 8 ,o ,49,49,49,o ,o ,o ,15,15,15,o ,o ,15,15,15,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,46,46,46,47,4 ,4 ,13,45,4 ,4 ,46,46,4 ,4 ,4 ,4 ,4 ,46,46,4 ,8 ,8 , /*47*/ 
/*48*/ 8 ,o ,49,49,49,o ,o ,o ,14,14,14,o ,o ,14,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,53,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,47,47,47,47,47,4 ,4 ,18,18,4 ,4 ,4 ,4 ,4 ,18,13,18,4 ,4 ,4 ,4 ,8 ,8 , /*48*/ 
/*49*/ 8 ,o ,49,49,49,o ,o ,o ,3 ,3 ,3 ,o ,o ,3 ,3 ,3 ,o ,o ,54,o ,o ,o ,54,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,16,13,16,4 ,4 ,4 ,4 ,8 ,8 , /*49*/ 
/*50*/ 8 ,o ,49,49,49,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,16,KK,16,4 ,4 ,4 ,4 ,8 ,8 , /*50*/ 
/*51*/ 8 ,o ,24,LL,24,o ,o ,54,3 ,54,o ,o ,o ,o ,54,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,47,47,47,47,47,4 ,4 ,18,18,4 ,4 ,4 ,4 ,4 ,16,16,16,4 ,4 ,4 ,4 ,DD,8 , /*51*/ 
/*52*/ 63,o ,2 ,2 ,2 ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,3 ,3 ,49,61,61,49,3 ,3 ,3 ,3 ,54,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,18,4 ,4 ,4 ,4 ,4 ,4 ,4 ,47,4 ,4 ,13,45,4 ,4 ,4 ,4 ,4 ,18,13,18,4 ,4 ,4 ,4 ,8 ,8 , /*52*/ 
/*53*/ 8 ,o ,2 ,2 ,2 ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,60,61,61,61,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,8 ,53,4 ,4 ,4 ,4 ,4 ,18,4 ,4 ,4 ,4 ,4 ,4 ,4 ,47,4 ,4 ,13,45,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*53*/ 
/*54*/ 8 ,15,2 ,2 ,2 ,o ,o ,54,3 ,54,o ,o ,o ,o ,o ,o ,o ,60,61,61,61,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,47,47,47,4 ,47,4 ,4 ,18,45,4 ,19,46,46,46,46,46,46,46,46,46,46,19,8 , /*54*/ 
/*55*/ 8 ,14,2 ,2 ,2 ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,3 ,3 ,49,61,61,49,3 ,3 ,3 ,3 ,54,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,45,4 ,45,4 ,47,4 ,4 ,18,45,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*55*/ 
/*56*/ 8 ,3 ,2 ,2 ,2 ,2 ,8 ,24,3 ,61,61,o ,o ,o ,54,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,18,18,4 ,4 ,4 ,47,4 ,47,47,47,4 ,4 ,13,45,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*56*/ 
/*57*/ 8 ,3 ,2 ,2 ,2 ,2 ,2 ,8 ,3 ,61,61,o ,o ,o ,54,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,47,4 ,4 ,4 ,4 ,4 ,4 ,13,45,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*57*/ 
/*58*/ 63,14,2 ,2 ,2 ,2 ,2 ,2 ,MM,61,61,o ,o ,o ,o ,3 ,o ,o ,54,o ,o ,o ,54,o ,o ,o ,63,63,o ,o ,8 ,53,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,18,HH,18,4 ,4 ,4 ,4 ,4 ,8 ,18,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*58*/ 
/*59*/ 8 ,15,2 ,2 ,2 ,2 ,2 ,8 ,3 ,61,61,o ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,63,63,63,o ,o ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,16,16,16,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,8 , /*59*/ 
/*60*/ 8 ,o ,2 ,2 ,2 ,2 ,8 ,24,3 ,61,61,54,o ,54,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,63,63,63,o ,o ,o ,4 ,4 ,4 ,4 ,19,46,46,46,19,46,19,46,46,46,46,46,46,46,46,19,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,53,8 , /*60*/ 
/*61*/ 8 ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,63,o ,o ,o ,o ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,34,8 , /*61*/ 
/*62*/ 8 ,o ,o ,o ,o ,o ,o ,o ,o ,54,o ,54,o ,54,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,4 ,46,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,34,8 , /*62*/ 
/*63*/ 8 ,8 ,8 ,DD,8 ,DD,8 ,8 ,8 ,8 ,53,53,53,8 ,8 ,8 ,53,8 ,34,34,34,8 ,53,8 ,34,34,34,8 ,53,8 ,19,19,33,33,33,53,19,8 ,8 ,8 ,8 ,8 ,53,34,34,53,8 ,8 ,8 ,8 ,53,34,34,53,8 ,8 ,8 ,8 ,8 ,53,34,34,34,8   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef GG
    #undef HH
    #undef II
    #undef JJ
    #undef KK
    #undef LL
    #undef MM
    #undef o
    },
    {        // tileDictionary
      SFG_TD(10,20,0,0),SFG_TD( 4,26,0,0),SFG_TD( 0, 4,0,0),SFG_TD( 4,26,0,0), // 0 
      SFG_TD( 0,30,2,2),SFG_TD(17,13,0,2),SFG_TD(24, 6,0,1),SFG_TD( 0,24,1,0), // 4 
      SFG_TD( 4, 0,1,0),SFG_TD(19, 0,4,1),SFG_TD(10, 5,0,0),SFG_TD(17, 5,0,0), // 8 
      SFG_TD(24, 5,0,0),SFG_TD( 8,22,3,0),SFG_TD( 6,24,0,0),SFG_TD( 8,22,0,0), // 12 
      SFG_TD(11,19,1,0),SFG_TD( 0,10,0,0),SFG_TD( 8,22,0,0),SFG_TD(21, 0,4,4), // 16 
      SFG_TD(24, 0,0,5),SFG_TD(17, 5,0,1),SFG_TD( 0, 0,0,0),SFG_TD(24, 6,5,5), // 20 
      SFG_TD(28, 0,5,1),SFG_TD(24, 8,0,1),SFG_TD(26, 3,5,1),SFG_TD(17,10,0,5), // 24 
      SFG_TD(13,15,0,0),SFG_TD(13,15,0,0),SFG_TD(13,15,0,0),SFG_TD(13,15,0,0), // 28 
      SFG_TD(24, 8,0,1),SFG_TD( 8, 0,6,0),SFG_TD(18, 0,0,6),SFG_TD(13,15,0,0), // 32 
      SFG_TD(13,15,0,0),SFG_TD(13,15,0,0),SFG_TD(13,15,0,0),SFG_TD(13,15,0,0), // 36 
      SFG_TD(13,15,0,0),SFG_TD(13,15,0,0),SFG_TD(28, 4,5,1),SFG_TD(13,15,0,0), // 40 
      SFG_TD(13,15,0,0),SFG_TD( 2,28,4,2),SFG_TD( 0,13,0,4),SFG_TD( 2,28,2,2), // 44 
      SFG_TD(22, 8,5,2),SFG_TD(15,15,0,0),SFG_TD( 0,15,0,0),SFG_TD(15,11,5,0), // 48 
      SFG_TD(13,15,0,0),SFG_TD( 0, 0,1,1),SFG_TD(10,20,1,0),SFG_TD(22, 8,0,0), // 52 
      SFG_TD(31,24,4,4),SFG_TD(19,11,6,2),SFG_TD( 0, 0,3,3),SFG_TD(19, 3,4,3), // 56 
      SFG_TD(12,18,5,0),SFG_TD(14,16,5,0),SFG_TD(17,13,1,1),SFG_TD(21, 0,2,2)  // 60 
      },                    // tileDictionary
    {14,3 ,11,15,2 ,12,10}, // textureIndices
    13,                     // doorTextureIndex
    50,                     // floorColor
    186,                     // ceilingColor
    {56, 50, 64 },          // player start: x, y, direction
    1,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_FINISH, {24,0}},{SFG_LEVEL_ELEMENT_BULLETS, {29,1}},
      {SFG_LEVEL_ELEMENT_BULLETS, {28,1}},{SFG_LEVEL_ELEMENT_CARD0, {11,1}},
      {SFG_LEVEL_ELEMENT_PLASMA, {4,1}},{SFG_LEVEL_ELEMENT_ROCKETS, {3,1}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {35,2}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {60,3}},
      {SFG_LEVEL_ELEMENT_CARD2, {40,6}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {51,7}},
      {SFG_LEVEL_ELEMENT_LAMP, {12,7}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {36,9}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {47,10}},{SFG_LEVEL_ELEMENT_LAMP, {12,10}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {1,10}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {42,11}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {34,11}},{SFG_LEVEL_ELEMENT_LOCK2, {27,11}},
      {SFG_LEVEL_ELEMENT_LOCK2, {21,11}},{SFG_LEVEL_ELEMENT_BARREL, {5,11}},
      {SFG_LEVEL_ELEMENT_COLUMN, {37,12}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {35,12}},
      {SFG_LEVEL_ELEMENT_COLUMN, {33,12}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {55,13}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {1,13}},{SFG_LEVEL_ELEMENT_HEALTH, {3,15}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {50,17}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {53,20}},
      {SFG_LEVEL_ELEMENT_BARREL, {18,20}},{SFG_LEVEL_ELEMENT_LOCK2, {50,21}},
      {SFG_LEVEL_ELEMENT_BARREL, {19,21}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {18,21}},
      {SFG_LEVEL_ELEMENT_BARREL, {17,21}},{SFG_LEVEL_ELEMENT_BARREL, {18,22}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {60,23}},{SFG_LEVEL_ELEMENT_HEALTH, {39,23}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {39,24}},{SFG_LEVEL_ELEMENT_ROCKETS, {61,25}},
      {SFG_LEVEL_ELEMENT_HEALTH, {60,25}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {50,26}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {29,27}},{SFG_LEVEL_ELEMENT_HEALTH, {5,28}},
      {SFG_LEVEL_ELEMENT_LAMP, {22,29}},{SFG_LEVEL_ELEMENT_LOCK1, {35,30}},
      {SFG_LEVEL_ELEMENT_COLUMN, {56,31}},{SFG_LEVEL_ELEMENT_COLUMN, {50,31}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {54,32}},{SFG_LEVEL_ELEMENT_BULLETS, {53,32}},
      {SFG_LEVEL_ELEMENT_BULLETS, {38,32}},{SFG_LEVEL_ELEMENT_PLASMA, {37,32}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {32,32}},{SFG_LEVEL_ELEMENT_LAMP, {33,34}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {33,35}},{SFG_LEVEL_ELEMENT_PLASMA, {9,35}},
      {SFG_LEVEL_ELEMENT_HEALTH, {23,36}},{SFG_LEVEL_ELEMENT_TERMINAL, {19,36}},
      {SFG_LEVEL_ELEMENT_PLASMA, {9,36}},{SFG_LEVEL_ELEMENT_CARD1, {21,38}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {19,38}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {57,39}},
      {SFG_LEVEL_ELEMENT_PLASMA, {56,40}},{SFG_LEVEL_ELEMENT_ROCKETS, {56,41}},
      {SFG_LEVEL_ELEMENT_BARREL, {37,48}},{SFG_LEVEL_ELEMENT_BARREL, {36,48}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {44,49}},{SFG_LEVEL_ELEMENT_HEALTH, {36,52}},
      {SFG_LEVEL_ELEMENT_BULLETS, {36,53}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {18,53}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {11,53}},{SFG_LEVEL_ELEMENT_BARREL, {3,53}},
      {SFG_LEVEL_ELEMENT_BULLETS, {58,57}},{SFG_LEVEL_ELEMENT_BULLETS, {56,57}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {44,57}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {3,57}},
      {SFG_LEVEL_ELEMENT_BARREL, {51,58}},{SFG_LEVEL_ELEMENT_BARREL, {50,58}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {39,58}},{SFG_LEVEL_ELEMENT_LOCK0, {8,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {5,58}},{SFG_LEVEL_ELEMENT_BARREL, {51,59}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {3,59}},{SFG_LEVEL_ELEMENT_BULLETS, {20,61}},
      {SFG_LEVEL_ELEMENT_HEALTH, {3,61}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level

  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level4 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (5 | SFG_TILE_PROPERTY_DOOR)
    #define BB (10 | SFG_TILE_PROPERTY_DOOR)
    #define CC (6 | SFG_TILE_PROPERTY_DOOR)
    #define DD (8 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,60,60,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, /*0 */ 
/*1 */ 62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,o ,o ,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, /*1 */ 
/*2 */ 62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,o ,o ,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,18,18,18,18,18,18,18,18,18,17,18,18,18,17,18,18,18,18,18,18, /*2 */ 
/*3 */ 62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,o ,o ,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,17,18,18,18,17,18,18,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,17,18,18,18,18, /*3 */ 
/*4 */ 62,62,62,62,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,62,62,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,17,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,2 ,2 ,2 ,2 ,18, /*4 */ 
/*5 */ o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,AA,7 ,7 ,7 ,7 ,7 ,7 ,7 ,2 ,2 ,2 ,2 ,18, /*5 */ 
/*6 */ o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,18,2 ,2 ,2 ,17,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,2 ,2 ,2 ,2 ,18, /*6 */ 
/*7 */ o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,18,18,18,18,17,18,18,18,17,18,18,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,17,2 ,2 ,2 ,17, /*7 */ 
/*8 */ o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,18,2 ,2 ,2 ,18, /*8 */ 
/*9 */ o ,o ,o ,o ,63,63,63,61,61,61,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,18,2 ,2 ,2 ,18, /*9 */ 
/*10*/ o ,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,17,2 ,2 ,2 ,17, /*10*/ 
/*11*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,60,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,18,7 ,7 ,11,11,11,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,18,2 ,2 ,2 ,18, /*11*/ 
/*12*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,59,59,59,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,18,7 ,7 ,11,11,11,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,18,2 ,2 ,2 ,18, /*12*/ 
/*13*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,59,59,59,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,18,7 ,7 ,11,11,11,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,17,2 ,2 ,2 ,17, /*13*/ 
/*14*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,59,59,59,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,18,18,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,18,2 ,2 ,2 ,18, /*14*/ 
/*15*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,62,1 ,18,18,18,17,18,18,18,17,18,18,18,17,18,18,18,17,18,18,18,2 ,2 ,2 ,18, /*15*/ 
/*16*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,62,62,62,62,62,62,1 ,17,17,17,17,17,17,17,17,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,18,2 ,2 ,2 ,18, /*16*/ 
/*17*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,62,62,62,62,62,62,62,62,1 ,17,17,17,17,17,2 ,2 ,17,1 ,1 ,55,1 ,1 ,1 ,1 ,1 ,55,1 ,1 ,17,2 ,2 ,2 ,17, /*17*/ 
/*18*/ 62,62,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,62,62,62,62,62,62,62,62,1 ,17,17,17,17,17,2 ,2 ,2 ,2 ,2 ,2 ,25,1 ,1 ,1 ,25,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 , /*18*/ 
/*19*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,62,62,62,62,62,62,62,62,1 ,17,17,17,17,17,2 ,2 ,2 ,2 ,2 ,2 ,25,25,25,25,25,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 , /*19*/ 
/*20*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,62,62,o ,o ,o ,o ,o ,62,1 ,17,17,17,17,17,2 ,2 ,17,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 , /*20*/ 
/*21*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,60,60,60,62,62,o ,o ,o ,o ,o ,62,1 ,17,17,17,17,17,17,17,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 , /*21*/ 
/*22*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,62,62,60,60,60,60,60,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 , /*22*/ 
/*23*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,55,10,BB,BB,BB,10,55,o ,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,60,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 , /*23*/ 
/*24*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,10,10,36,36,36,36,36,10,10,o ,o ,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,60,o ,62,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,1 ,1 , /*24*/ 
/*25*/ 60,o ,o ,o ,63,o ,o ,o ,o ,o ,o ,o ,10,36,36,36,36,36,36,36,36,36,10,o ,o ,o ,o ,59,o ,o ,o ,o ,o ,o ,o ,o ,o ,60,o ,62,1 ,1 ,1 ,1 ,1 ,51,51,1 ,1 ,1 ,1 ,2 ,3 ,2 ,3 ,2 ,1 ,1 ,1 ,1 ,51,51,1 ,1 , /*25*/ 
/*26*/ 10,49,49,10,10,10,10,10,10,10,10,10,10,36,36,36,36,36,36,36,36,36,10,10,10,10,10,10,10,10,10,10,49,49,10,o ,o ,60,o ,62,62,1 ,1 ,1 ,1 ,51,51,1 ,1 ,1 ,1 ,2 ,CC,2 ,CC,2 ,1 ,1 ,1 ,1 ,51,51,1 ,1 , /*26*/ 
/*27*/ 10,33,33,33,45,46,46,46,46,46,46,46,46,36,36,36,36,36,36,36,36,36,46,46,46,46,46,46,46,46,45,33,33,33,10,o ,o ,60,o ,62,62,62,1 ,1 ,1 ,51,51,51,51,51,51,2 ,2 ,2 ,2 ,2 ,51,51,51,51,51,51,1 ,1 , /*27*/ 
/*28*/ 10,33,33,33,45,46,46,46,46,46,46,46,46,36,36,36,36,36,36,36,36,36,46,46,46,46,46,46,46,46,45,33,33,33,10,o ,o ,60,o ,62,62,62,62,1 ,1 ,51,51,51,51,51,51,2 ,2 ,2 ,2 ,2 ,51,51,51,51,51,51,1 ,1 , /*28*/ 
/*29*/ 10,33,33,33,45,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,45,33,33,33,10,60,60,60,60,8 ,62,62,62,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*29*/ 
/*30*/ 10,33,33,33,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,33,33,33,10,o ,o ,o ,o ,o ,o ,21,21,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*30*/ 
/*31*/ 10,34,34,34,10,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,10,34,34,34,10,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,62,62,62,1 ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*31*/ 
/*32*/ 49,35,35,35,10,35,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,35,10,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,1 ,1 ,4 ,2 ,2 ,2 ,4 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*32*/ 
/*33*/ 49,35,35,35,10,35,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,35,10,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*33*/ 
/*34*/ 49,35,35,35,10,43,42,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,42,43,10,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,62,62,62,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*34*/ 
/*35*/ 10,35,35,35,DD,35,34,33,32,40,32,40,32,40,32,40,32,40,32,40,32,40,32,40,32,40,32,33,34,35,DD,35,35,35,10,o ,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*35*/ 
/*36*/ 10,10,10,10,10,43,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,43,10,10,10,10,10,o ,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,17,17,17,17,17,17, /*36*/ 
/*37*/ 10,9 ,9 ,9 ,9 ,9 ,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,9 ,9 ,9 ,9 ,9 ,10,o ,o ,60,60,60,60,60,60,62,1 ,1 ,1 ,25,25,25,25,25,1 ,2 ,1 ,1 ,1 ,1 ,17,30,30,30,17,17, /*37*/ 
/*38*/ 10,9 ,9 ,9 ,9 ,9 ,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,9 ,9 ,9 ,9 ,9 ,10,o ,o ,60,o ,o ,o ,o ,o ,62,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,29,30,30,30,17,17, /*38*/ 
/*39*/ 10,9 ,9 ,9 ,9 ,9 ,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,9 ,9 ,9 ,9 ,9 ,10,o ,o ,60,o ,o ,o ,o ,o ,62,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,29,30,30,30,17,17, /*39*/ 
/*40*/ 10,BB,10,10,8 ,43,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,43,8 ,10,10,BB,10,o ,o ,60,62,62,8 ,8 ,62,62,1 ,1 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,17,30,30,30,17,17, /*40*/ 
/*41*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,32,33,34,35,8 ,9 ,9 ,9 ,10,o ,o ,60,62,1 ,1 ,1 ,1 ,1 ,17,1 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,17,17,30,17,17,17, /*41*/ 
/*42*/ 10,9 ,9 ,9 ,8 ,43,42,41,40,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,40,41,42,43,8 ,9 ,9 ,9 ,10,o ,o ,60,62,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,17,30,30,30,17,17, /*42*/ 
/*43*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,32,33,34,35,8 ,9 ,9 ,9 ,10,o ,o ,60,62,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,17,30,30,30,17,17, /*43*/ 
/*44*/ 10,9 ,9 ,9 ,8 ,43,42,41,40,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,40,41,42,43,8 ,9 ,9 ,9 ,10,62,62,62,62,1 ,2 ,2 ,2 ,1 ,17,1 ,1 ,1 ,1 ,1 ,1 ,1 ,4 ,2 ,4 ,1 ,1 ,1 ,17,30,30,30,17,17, /*44*/ 
/*45*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,35,8 ,9 ,9 ,9 ,10,62,62,1 ,1 ,1 ,29,29,29,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,17,17,17,17,17,17, /*45*/ 
/*46*/ 10,9 ,9 ,9 ,9 ,9 ,36,36,36,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,36,36,36,9 ,9 ,9 ,9 ,9 ,10,62,62,1 ,1 ,30,30,30,30,30,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*46*/ 
/*47*/ BB,9 ,9 ,9 ,9 ,9 ,36,36,36,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,36,36,36,9 ,9 ,9 ,9 ,9 ,BB,62,62,1 ,31,31,31,31,31,31,31,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*47*/ 
/*48*/ 10,9 ,9 ,9 ,9 ,9 ,36,36,36,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,36,36,36,9 ,9 ,9 ,9 ,9 ,10,62,62,1 ,31,31,31,31,31,31,31,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*48*/ 
/*49*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,35,8 ,9 ,9 ,9 ,10,62,62,1 ,31,31,31,31,31,31,31,1 ,1 ,1 ,1 ,4 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,4 ,1 ,1 ,1 ,1 ,1 ,1 , /*49*/ 
/*50*/ 10,9 ,9 ,9 ,8 ,43,42,41,40,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,40,41,42,43,8 ,9 ,9 ,9 ,10,62,62,1 ,1 ,31,31,31,31,31,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 , /*50*/ 
/*51*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,32,33,34,35,8 ,9 ,9 ,9 ,10,62,62,1 ,1 ,31,31,31,31,31,1 ,1 ,17,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,17,1 ,1 ,1 , /*51*/ 
/*52*/ 10,9 ,9 ,9 ,8 ,43,42,41,40,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,40,41,42,43,8 ,9 ,9 ,9 ,10,62,62,1 ,1 ,1 ,31,31,31,1 ,1 ,1 ,3 ,1 ,1 ,4 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,4 ,1 ,1 ,3 ,1 ,1 ,1 , /*52*/ 
/*53*/ 10,9 ,9 ,9 ,8 ,35,34,33,32,36,22,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,36,32,33,34,35,8 ,9 ,9 ,9 ,12,o ,62,62,62,1 ,31,31,31,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 , /*53*/ 
/*54*/ 10,BB,10,10,8 ,43,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,43,8 ,10,10,BB,10,o ,o ,21,8 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 , /*54*/ 
/*55*/ 10,9 ,9 ,9 ,9 ,9 ,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,9 ,9 ,9 ,9 ,9 ,10,o ,o ,21,8 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,29,29,2 ,2 ,2 ,29,29,2 ,2 ,2 ,2 ,1 ,1 ,1 , /*55*/ 
/*56*/ 10,9 ,9 ,9 ,9 ,9 ,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,9 ,9 ,9 ,9 ,9 ,10,o ,o ,o ,8 ,62,62,62,62,1 ,1 ,1 ,4 ,2 ,2 ,30,30,30,2 ,2 ,2 ,30,30,30,2 ,2 ,4 ,1 ,1 ,1 , /*56*/ 
/*57*/ 10,9 ,9 ,9 ,9 ,9 ,34,33,32,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,32,33,34,9 ,9 ,9 ,9 ,9 ,10,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,2 ,30,30,30,2 ,2 ,2 ,30,30,30,2 ,1 ,1 ,1 ,1 ,1 , /*57*/ 
/*58*/ 10,10,10,10,10,43,42,41,40,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,36,40,41,42,43,10,10,10,10,10,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,2 ,30,30,30,2 ,2 ,2 ,30,30,30,2 ,1 ,1 ,1 ,1 ,1 , /*58*/ 
/*59*/ 10,35,35,35,DD,35,34,33,32,40,32,40,40,40,32,40,32,40,32,40,32,40,40,40,32,40,32,33,34,35,DD,35,35,35,10,60,60,60,60,60,60,60,62,1 ,1 ,1 ,1 ,1 ,1 ,30,30,30,2 ,2 ,2 ,30,30,30,1 ,1 ,1 ,1 ,1 ,1 , /*59*/ 
/*60*/ 49,35,35,35,8 ,43,42,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,33,41,42,43,8 ,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*60*/ 
/*61*/ 49,35,35,35,8 ,35,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,42,34,35,8 ,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*61*/ 
/*62*/ 49,35,35,35,8 ,35,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,43,35,35,8 ,35,35,35,49,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*62*/ 
/*63*/ 10,35,35,35,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,35,35,35,10,o ,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,17,BB,17,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef o
    },
    {        // tileDictionary
      SFG_TD(19,31,1,0),SFG_TD(23, 0,4,4),SFG_TD(19, 7,4,4),SFG_TD(22, 4,6,0), // 0 
      SFG_TD(19, 3,0,3),SFG_TD(23, 3,3,4),SFG_TD(24, 5,4,4),SFG_TD(19,12,4,4), // 4 
      SFG_TD(31, 0,0,1),SFG_TD(19, 6,1,1),SFG_TD(23, 0,1,1),SFG_TD(20,11,1,4), // 8 
      SFG_TD(19, 4,1,1),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0), // 12 
      SFG_TD(19,15,0,0),SFG_TD(23, 0,1,1),SFG_TD(23, 0,2,4),SFG_TD(19,15,0,0), // 16 
      SFG_TD(19,15,0,0),SFG_TD(23,31,2,0),SFG_TD(21,31,2,0),SFG_TD(27,31,1,0), // 20 
      SFG_TD(19,15,0,0),SFG_TD(22, 3,4,4),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0), // 24 
      SFG_TD(19,15,0,0),SFG_TD(18, 8,4,4),SFG_TD(17, 9,4,4),SFG_TD(16,10,4,4), // 28 
      SFG_TD(21,20,0,0),SFG_TD(23,18,0,0),SFG_TD(25,16,0,0),SFG_TD(27,14,0,0), // 32 
      SFG_TD(19,22,0,0),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0), // 36 
      SFG_TD(21,20,2,0),SFG_TD(23,18,2,0),SFG_TD(25,16,2,0),SFG_TD(27,14,2,0), // 40 
      SFG_TD(19,15,0,0),SFG_TD(21,10,0,1),SFG_TD(19,12,1,1),SFG_TD(19,15,0,0), // 44 
      SFG_TD(19,15,0,0),SFG_TD(29, 3,1,1),SFG_TD(19,15,0,0),SFG_TD(19, 6,4,4), // 48 
      SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(31,31,3,0), // 52 
      SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(19,15,0,0),SFG_TD(21,31,1,0), // 56 
      SFG_TD(25,31,4,0),SFG_TD(22,31,2,0),SFG_TD(19, 0,5,5),SFG_TD(27,31,1,0)  // 60 
      },                    // tileDictionary
    {3 ,12,0 ,8 ,5 ,1 ,15}, // textureIndices
    7,                      // doorTextureIndex
    202,                     // floorColor
    66,                     // ceilingColor
    {53, 61, 64 },          // player start: x, y, direction
    2,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_LAMP, {58,3}},{SFG_LEVEL_ELEMENT_COLUMN, {51,4}},
      {SFG_LEVEL_ELEMENT_LOCK2, {51,5}},{SFG_LEVEL_ELEMENT_FINISH, {44,5}},
      {SFG_LEVEL_ELEMENT_COLUMN, {51,6}},{SFG_LEVEL_ELEMENT_LAMP, {58,7}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {9,9}},{SFG_LEVEL_ELEMENT_BLOCKER, {8,9}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {7,9}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {51,10}},
      {SFG_LEVEL_ELEMENT_LAMP, {6,11}},{SFG_LEVEL_ELEMENT_TELEPORTER, {44,12}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {58,13}},{SFG_LEVEL_ELEMENT_TELEPORTER, {23,13}},
      {SFG_LEVEL_ELEMENT_HEALTH, {17,17}},{SFG_LEVEL_ELEMENT_BULLETS, {15,17}},
      {SFG_LEVEL_ELEMENT_BULLETS, {45,18}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {17,18}},
      {SFG_LEVEL_ELEMENT_HEALTH, {55,19}},{SFG_LEVEL_ELEMENT_BULLETS, {53,19}},
      {SFG_LEVEL_ELEMENT_HEALTH, {51,19}},{SFG_LEVEL_ELEMENT_BULLETS, {45,19}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {56,21}},{SFG_LEVEL_ELEMENT_LAMP, {20,22}},
      {SFG_LEVEL_ELEMENT_LAMP, {14,22}},{SFG_LEVEL_ELEMENT_RUIN, {26,24}},
      {SFG_LEVEL_ELEMENT_HEALTH, {8,24}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {18,25}},
      {SFG_LEVEL_ELEMENT_BULLETS, {5,25}},{SFG_LEVEL_ELEMENT_ROCKETS, {54,26}},
      {SFG_LEVEL_ELEMENT_PLASMA, {52,26}},{SFG_LEVEL_ELEMENT_HEALTH, {36,26}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {16,26}},{SFG_LEVEL_ELEMENT_TREE, {18,29}},
      {SFG_LEVEL_ELEMENT_TREE, {16,29}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {53,30}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {29,35}},{SFG_LEVEL_ELEMENT_ROCKETS, {51,37}},
      {SFG_LEVEL_ELEMENT_HEALTH, {49,37}},{SFG_LEVEL_ELEMENT_COLUMN, {19,37}},
      {SFG_LEVEL_ELEMENT_COLUMN, {15,37}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {60,38}},
      {SFG_LEVEL_ELEMENT_BULLETS, {30,38}},{SFG_LEVEL_ELEMENT_BARREL, {21,38}},
      {SFG_LEVEL_ELEMENT_BARREL, {13,38}},{SFG_LEVEL_ELEMENT_HEALTH, {4,38}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {20,40}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {13,40}},
      {SFG_LEVEL_ELEMENT_HEALTH, {21,41}},{SFG_LEVEL_ELEMENT_PLASMA, {13,41}},
      {SFG_LEVEL_ELEMENT_PLASMA, {60,43}},{SFG_LEVEL_ELEMENT_BARREL, {17,46}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {12,46}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {42,47}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {40,47}},{SFG_LEVEL_ELEMENT_LOCK0, {34,47}},
      {SFG_LEVEL_ELEMENT_BULLETS, {23,47}},{SFG_LEVEL_ELEMENT_BARREL, {18,47}},
      {SFG_LEVEL_ELEMENT_BARREL, {16,47}},{SFG_LEVEL_ELEMENT_ROCKETS, {11,47}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {3,47}},{SFG_LEVEL_ELEMENT_LOCK0, {0,47}},
      {SFG_LEVEL_ELEMENT_BARREL, {41,48}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {32,48}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {21,48}},{SFG_LEVEL_ELEMENT_BARREL, {17,48}},
      {SFG_LEVEL_ELEMENT_BARREL, {42,49}},{SFG_LEVEL_ELEMENT_BARREL, {40,49}},
      {SFG_LEVEL_ELEMENT_PLASMA, {42,50}},{SFG_LEVEL_ELEMENT_PLASMA, {40,50}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {43,51}},{SFG_LEVEL_ELEMENT_PLASMA, {41,51}},
      {SFG_LEVEL_ELEMENT_PLASMA, {39,51}},{SFG_LEVEL_ELEMENT_PLASMA, {60,52}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {46,52}},{SFG_LEVEL_ELEMENT_RUIN, {33,52}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {41,53}},{SFG_LEVEL_ELEMENT_HEALTH, {40,53}},
      {SFG_LEVEL_ELEMENT_PLASMA, {21,53}},{SFG_LEVEL_ELEMENT_HEALTH, {13,53}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {21,54}},{SFG_LEVEL_ELEMENT_MONSTER_ENDER, {17,55}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {13,55}},{SFG_LEVEL_ELEMENT_HEALTH, {30,56}},
      {SFG_LEVEL_ELEMENT_BARREL, {21,56}},{SFG_LEVEL_ELEMENT_BARREL, {13,56}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {4,56}},{SFG_LEVEL_ELEMENT_HEALTH, {41,57}},
      {SFG_LEVEL_ELEMENT_COLUMN, {19,57}},{SFG_LEVEL_ELEMENT_COLUMN, {15,57}},
      {SFG_LEVEL_ELEMENT_RUIN, {29,59}},{SFG_LEVEL_ELEMENT_LOCK0, {4,59}},
      {SFG_LEVEL_ELEMENT_LOCK1, {53,63}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level5 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (63 | SFG_TILE_PROPERTY_ELEVATOR)
    #define BB (32 | SFG_TILE_PROPERTY_DOOR)
    #define CC (30 | SFG_TILE_PROPERTY_DOOR)
    #define DD (59 | SFG_TILE_PROPERTY_ELEVATOR)
    #define EE (53 | SFG_TILE_PROPERTY_DOOR)
    #define FF (44 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,1 ,1 ,1 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,7 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,5 ,2 ,2 ,2 ,2 , /*0 */ 
/*1 */ 22,22,22,22,22,22,22,22,o ,o ,o ,22,22,22,22,22,22,22,22,22,22,22,22,22,22,1 ,1 ,1 ,1 ,1 ,1 ,1 ,35,31,o ,o ,31,35,7 ,40,40,40,40,40,40,40,40,40,26,35,31,o ,o ,31,35,31,o ,o ,31,35,2 ,2 ,2 ,2 , /*1 */ 
/*2 */ 23,23,23,23,23,23,22,22,o ,o ,o ,22,22,22,22,22,22,22,22,22,22,22,22,22,22,1 ,1 ,1 ,1 ,1 ,1 ,1 ,35,31,o ,o ,31,35,7 ,11,10,40,40,40,40,40,40,40,26,35,31,o ,o ,31,35,31,o ,o ,31,35,2 ,2 ,2 ,2 , /*2 */ 
/*3 */ 23,23,23,23,23,23,22,22,o ,o ,o ,o ,o ,o ,o ,o ,3 ,o ,o ,o ,o ,55,55,55,60,60,60,1 ,1 ,1 ,1 ,1 ,35,o ,o ,o ,o ,35,7 ,11,10,40,40,40,40,40,40,40,26,35,o ,o ,o ,o ,35,o ,o ,o ,o ,35,2 ,2 ,2 ,2 , /*3 */ 
/*4 */ 23,23,23,23,23,23,22,22,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,o ,56,56,55,55,60,60,1 ,1 ,1 ,1 ,1 ,35,o ,o ,o ,o ,35,7 ,11,10,40,40,40,40,40,40,40,40,35,o ,o ,o ,o ,35,o ,o ,o ,o ,35,2 ,2 ,2 ,2 , /*4 */ 
/*5 */ 23,23,23,23,23,23,55,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,56,56,5 ,1 ,1 ,5 ,35,35,35,35,35,35,o ,o ,35,35,7 ,40,40,40,40,40,40,40,40,40,40,35,35,o ,o ,35,35,35,o ,o ,35,35,31,31,31,31, /*5 */ 
/*6 */ 23,23,23,23,23,23,61,61,55,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,2 ,2 ,2 ,61,1 ,1 ,1 ,1 ,31,31,o ,o ,35,o ,o ,o ,o ,o ,7 ,7 ,7 ,7 ,10,34,10,34,40,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*6 */ 
/*7 */ 23,23,23,23,23,23,2 ,2 ,56,55,55,55,55,o ,o ,o ,o ,55,55,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,7 ,10,34,10,34,40,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*7 */ 
/*8 */ 23,o ,o ,2 ,2 ,2 ,2 ,2 ,2 ,56,55,55,55,55,55,55,55,55,61,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,7 ,35,35,35,35,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*8 */ 
/*9 */ 23,o ,o ,56,56,2 ,2 ,2 ,2 ,61,61,61,61,55,55,55,61,61,61,2 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,31,31,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,7 ,31,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*9 */ 
/*10*/ 23,o ,o ,o ,56,56,2 ,2 ,2 ,61,55,55,55,o ,o ,o ,o ,55,55,61,2 ,2 ,2 ,61,5 ,1 ,1 ,5 ,35,35,35,35,35,o ,o ,o ,o ,o ,o ,o ,o ,7 ,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o , /*10*/ 
/*11*/ 23,o ,o ,23,23,23,2 ,2 ,2 ,55,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,56,56,o ,61,2 ,2 ,2 ,2 ,2 ,56,21,21,21,31,o ,o ,o ,o ,o ,o ,o ,7 ,31,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,o ,o ,o ,o , /*11*/ 
/*12*/ 23,o ,o ,23,23,23,2 ,2 ,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,2 ,2 ,2 ,61,o ,21,21,21,31,o ,o ,o ,o ,o ,o ,o ,7 ,35,35,35,35,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,40,40,o ,o ,o ,o ,o , /*12*/ 
/*13*/ 23,o ,o ,23,23,23,61,2 ,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,56,o ,o ,o ,21,21,21,16,o ,o ,o ,o ,o ,o ,o ,7 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,o ,o ,o ,3 ,3 ,3 ,3 ,3 ,40,40,o ,o ,o ,o ,o , /*13*/ 
/*14*/ 23,23,23,23,23,23,56,56,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,21,21,21,31,o ,o ,o ,o ,o ,o ,o ,7 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,40,40,o ,o ,o ,o ,o , /*14*/ 
/*15*/ 23,23,22,22,22,22,22,22,22,22,37,16,57,57,26,57,57,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,21,21,21,31,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,29,o ,o , /*15*/ 
/*16*/ 23,23,22,22,22,22,22,22,22,22,22,12,57,26,26,26,26,21,21,21,21,21,21,o ,o ,o ,o ,22,22,22,21,21,21,16,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,23,o ,o , /*16*/ 
/*17*/ 23,23,AA,o ,o ,22,22,22,22,22,22,17,13,26,26,26,26,21,21,21,21,21,21,o ,o ,o ,o ,22,22,22,21,21,21,31,o ,o ,o ,o ,o ,o ,o ,7 ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,23,23,23,23,23,23,29, /*17*/ 
/*18*/ 23,23,10,o ,o ,22,22,22,22,22,22,12,57,26,26,26,26,21,21,21,21,21,21,o ,o ,o ,o ,22,22,22,21,21,21,31,o ,o ,o ,o ,o ,16,16,7 ,3 ,o ,o ,o ,o ,o ,o ,11,11,11,o ,o ,o ,o ,23,23,23,23,23,23,23,23, /*18*/ 
/*19*/ 23,23,8 ,o ,o ,22,22,22,22,22,22,16,57,26,26,26,57,o ,16,o ,o ,16,o ,o ,o ,o ,o ,16,31,31,16,31,31,16,o ,o ,o ,o ,o ,12,12,7 ,3 ,o ,o ,o ,8 ,9 ,10,11,11,11,10,9 ,8 ,o ,23,23,23,23,23,27,23,29, /*19*/ 
/*20*/ 23,23,o ,o ,o ,22,22,22,22,22,22,o ,57,57,57,57,57,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,17,17,7 ,3 ,3 ,3 ,o ,8 ,9 ,10,11,11,11,10,9 ,8 ,o ,23,23,23,27,1 ,1 ,9 ,9 , /*20*/ 
/*21*/ 23,23,o ,22,22,22,22,22,22,22,37,o ,o ,62,o ,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,13,13,7 ,7 ,7 ,7 ,23,27,1 ,27,15,15,15,27,1 ,27,23,23,23,23,27,o ,o ,o ,o , /*21*/ 
/*22*/ 30,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,54,13,13,13,13,13,23,23,23,27,15,15,15,27,23,23,23,23,23,33,BB,o ,o ,o ,o , /*22*/ 
/*23*/ 30,52,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,61,61,2 ,2 ,2 ,2 ,13,o ,o ,13,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,61,61,54,13,13,13,13,23,23,23,27,15,15,15,27,27,27,27,27,27,27,27,o ,o ,o ,o , /*23*/ 
/*24*/ 30,52,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,o ,o ,12,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,54,13,13,13,23,23,23,27,15,15,15,15,15,15,15,15,27,27,27,1 ,1 ,9 ,9 , /*24*/ 
/*25*/ 30,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,56,54,13,13,23,23,23,27,15,15,15,15,15,15,15,15,27,27,23,23,27,23,29, /*25*/ 
/*26*/ 30,52,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,12,o ,o ,12,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,56,54,13,23,23,23,27,27,27,27,27,27,27,15,15,27,27,23,23,23,23,23, /*26*/ 
/*27*/ 30,52,2 ,2 ,2 ,61,61,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,13,o ,o ,13,2 ,2 ,2 ,61,61,61,61,61,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,56,54,13,13,13,30,13,13,22,22,22,22,27,15,15,27,27,22,22,22,22,22, /*27*/ 
/*28*/ 3 ,o ,o ,o ,o ,o ,o ,o ,4 ,2 ,4 ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,54,13,13,13,CC,13,13,22,22,22,22,27,15,15,27,27,22,22,22,22,22, /*28*/ 
/*29*/ 3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,2 ,4 ,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,54,13,13,13,30,13,13,22,22,22,22,27,27,27,27,27,22,22,22,22,22, /*29*/ 
/*30*/ 3 ,2 ,DD,DD,DD,4 ,4 ,2 ,2 ,2 ,4 ,o ,o ,o ,35,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,54,13,13,13,30,30,30,23,23,23,23,23,23,23,23,23,23,23,23,o ,o , /*30*/ 
/*31*/ 3 ,2 ,DD,DD,DD,4 ,4 ,2 ,4 ,4 ,4 ,o ,o ,o ,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,2 ,54,13,13,13,13,22,23,23,20,20,20,20,20,20,20,20,23,23,o ,o , /*31*/ 
/*32*/ 3 ,2 ,DD,DD,DD,4 ,4 ,2 ,4 ,4 ,4 ,37,36,36,36,36,49,o ,o ,o ,o ,o ,31,31,16,16,31,31,o ,o ,o ,31,31,31,31,o ,o ,o ,o ,61,2 ,2 ,2 ,2 ,61,54,13,13,13,22,23,23,12,12,12,12,12,12,12,12,23,23,o ,o , /*32*/ 
/*33*/ 3 ,2 ,4 ,4 ,4 ,4 ,4 ,2 ,2 ,2 ,4 ,36,58,58,58,58,37,o ,o ,o ,o ,37,22,22,22,22,22,22,37,31,37,22,22,22,22,49,55,55,55,61,2 ,2 ,2 ,2 ,2 ,54,17,17,17,22,23,23,12,12,12,12,12,12,12,12,23,23,o ,o , /*33*/ 
/*34*/ 3 ,2 ,4 ,2 ,2 ,2 ,4 ,2 ,2 ,2 ,4 ,36,58,58,58,58,58,o ,o ,o ,o ,22,22,22,22,22,22,22,37,21,37,22,22,22,22,22,o ,o ,o ,o ,56,2 ,2 ,2 ,2 ,54,12,12,12,22,23,23,20,20,20,12,12,20,20,20,23,23,o ,o , /*34*/ 
/*35*/ 3 ,2 ,4 ,2 ,4 ,2 ,16,4 ,4 ,2 ,4 ,36,58,58,58,58,37,o ,o ,o ,o ,22,22,22,22,22,22,22,37,21,37,22,22,22,22,49,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,54,16,16,16,30,23,23,23,23,20,12,12,20,23,23,23,23,o ,o , /*35*/ 
/*36*/ 3 ,2 ,2 ,2 ,4 ,2 ,12,2 ,2 ,2 ,4 ,36,19,58,19,19,36,o ,o ,o ,o ,22,22,22,22,22,22,22,22,13,13,13,13,17,12,16,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,27,23,23,23,27,EE,EE,27,23,23,23,27,o ,o , /*36*/ 
/*37*/ 3 ,2 ,2 ,2 ,4 ,2 ,17,2 ,4 ,4 ,4 ,36,58,58,58,58,36,o ,o ,o ,o ,49,22,22,22,22,22,22,22,13,13,13,13,17,12,16,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,3 ,o ,22,22,17,12,12,17,22,22,60,3 ,3 ,o , /*37*/ 
/*38*/ 3 ,2 ,2 ,2 ,4 ,2 ,2 ,2 ,2 ,2 ,4 ,36,58,58,58,58,36,o ,o ,o ,o ,22,22,22,22,37,37,37,49,13,13,13,13,17,12,16,o ,o ,o ,o ,60,2 ,2 ,2 ,2 ,o ,o ,o ,62,30,3 ,o ,o ,12,12,12,60,60,12,o ,o ,3 ,3 ,o , /*38*/ 
/*39*/ 3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,37,58,58,58,36,49,o ,o ,o ,o ,22,22,22,22,36,13,13,13,13,13,13,37,22,22,37,o ,o ,o ,o ,o ,2 ,2 ,2 ,61,o ,o ,o ,o ,30,3 ,o ,o ,60,12,o ,o ,60,12,o ,o ,3 ,3 ,o , /*39*/ 
/*40*/ 3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,FF,58,58,58,36,o ,o ,o ,o ,o ,22,22,22,22,36,13,13,13,13,13,13,22,22,22,22,o ,o ,o ,o ,o ,2 ,2 ,2 ,61,o ,o ,o ,62,30,3 ,o ,o ,16,16,o ,o ,16,16,o ,o ,3 ,3 ,o , /*40*/ 
/*41*/ 3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,37,58,58,58,36,o ,21,21,21,21,22,22,22,22,37,37,37,22,13,13,13,22,22,22,49,o ,o ,o ,o ,60,2 ,2 ,2 ,61,o ,o ,o ,o ,30,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,o , /*41*/ 
/*42*/ 7 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,37,36,36,36,37,o ,21,21,21,21,37,37,22,22,49,49,22,22,13,13,13,37,22,22,21,o ,o ,o ,o ,o ,2 ,2 ,2 ,56,o ,o ,o ,o ,30,3 ,31,o ,o ,o ,o ,o ,o ,o ,o ,31,3 ,3 ,o , /*42*/ 
/*43*/ 7 ,55,55,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,16,12,17,13,13,13,13,13,13,13,13,13,13,13,22,22,22,21,62,o ,o ,o ,o ,2 ,2 ,2 ,56,o ,o ,o ,o ,30,3 ,60,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,o , /*43*/ 
/*44*/ 7 ,55,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,16,12,17,13,13,13,13,13,13,13,13,13,13,13,22,22,22,21,o ,o ,o ,o ,o ,13,12,12,13,o ,o ,o ,o ,30,3 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,o , /*44*/ 
/*45*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,o ,o ,16,12,17,13,13,13,13,13,13,13,13,13,13,37,22,22,37,21,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,30,3 ,31,o ,o ,o ,o ,o ,o ,55,o ,31,3 ,3 ,28, /*45*/ 
/*46*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,21,21,21,21,21,22,22,49,30,30,13,13,13,13,13,13,13,13,13,49,22,37,21,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,30,3 ,o ,o ,o ,o ,o ,o ,o ,55,o ,o ,3 ,3 ,28, /*46*/ 
/*47*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,21,21,21,21,21,22,22,22,13,30,13,13,13,13,13,13,13,13,13,49,22,37,21,62,o ,o ,o ,o ,13,12,12,13,o ,o ,o ,o ,30,3 ,o ,o ,o ,o ,o ,o ,o ,55,55,o ,3 ,3 ,28, /*47*/ 
/*48*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,21,21,21,21,21,22,22,22,13,13,13,13,13,13,13,13,13,13,37,22,22,37,21,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,3 ,3 ,3 ,3 ,o ,o ,o ,o ,3 ,3 ,3 ,3 ,3 ,28, /*48*/ 
/*49*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,55,22,22,22,21,21,22,22,22,13,30,13,13,13,13,13,13,13,13,13,22,22,22,21,62,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,60,60,o ,3 ,o ,o ,o ,o ,3 ,o ,o ,o ,3 ,28, /*49*/ 
/*50*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,61,22,22,22,21,21,22,22,37,13,30,30,30,30,30,30,13,13,13,13,22,22,22,21,o ,o ,o ,o ,60,2 ,2 ,2 ,2 ,o ,o ,o ,62,30,o ,o ,o ,3 ,3 ,o ,o ,3 ,3 ,o ,o ,o ,37,37, /*50*/ 
/*51*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56,2 ,2 ,36,22,21,21,22,22,49,13,13,13,13,13,13,30,13,13,13,13,37,22,22,49,o ,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,22, /*51*/ 
/*52*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,55,2 ,61,36,22,22,22,22,22,49,49,37,22,22,22,22,22,22,49,13,13,22,22,22,22,16,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,62,30,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,22, /*52*/ 
/*53*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,13,13,22,22,22,22,12,o ,o ,o ,60,2 ,2 ,2 ,2 ,o ,o ,o ,o ,30,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,22,22, /*53*/ 
/*54*/ 7 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,37,37,37,37,37,37,37,37,22,22,22,49,22,49,22,22,49,13,13,37,22,22,37,17,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,21,21,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,60,22,22, /*54*/ 
/*55*/ 7 ,o ,o ,8 ,9 ,10,10,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,16,12,17,13,o ,o ,o ,o ,o ,o ,o ,o ,35,13,13,13,13,13,13,13,o ,o ,o ,o ,2 ,2 ,2 ,2 ,o ,o ,o ,21,21,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,7 ,7 ,22,22, /*55*/ 
/*56*/ 22,22,23,23,23,23,EE,23,23,23,23,22,22,37,37,37,37,37,37,37,37,22,o ,o ,o ,o ,o ,o ,62,35,13,13,13,13,13,13,13,o ,o ,o ,o ,52,52,2 ,52,o ,o ,o ,21,21,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,7 ,o ,o ,o , /*56*/ 
/*57*/ 22,22,23,20,20,14,14,14,20,23,23,22,22,22,22,22,22,22,23,23,23,22,8 ,o ,o ,o ,o ,o ,o ,35,17,17,12,16,o ,o ,o ,o ,o ,o ,o ,DD,2 ,2 ,2 ,o ,o ,o ,21,21,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,7 ,o ,o ,o , /*57*/ 
/*58*/ 20,20,20,14,14,14,14,14,20,20,20,19,19,19,19,19,19,19,AA,20,20,22,9 ,o ,o ,o ,o ,o ,6 ,35,17,17,12,16,o ,o ,o ,o ,o ,o ,o ,DD,2 ,2 ,2 ,o ,o ,o ,21,21,o ,30,30,30,o ,o ,o ,o ,21,21,21,o ,o ,21, /*58*/ 
/*59*/ 20,14,14,14,14,20,20,20,20,14,14,14,14,14,14,14,14,14,14,14,14,23,10,o ,o ,o ,o ,o ,21,21,17,17,37,37,37,37,23,23,23,23,23,3 ,3 ,3 ,3 ,o ,o ,o ,21,21,7 ,21,21,21,o ,o ,o ,56,21,21,21,2 ,o ,21, /*59*/ 
/*60*/ 20,14,14,14,14,14,14,14,20,14,14,14,14,14,14,14,14,14,14,14,14,EE,10,o ,o ,o ,o ,o ,21,20,EE,EE,20,20,20,20,20,20,14,14,14,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,2 ,2 ,2 ,2 ,o ,21, /*60*/ 
/*61*/ 20,14,14,14,14,14,14,14,EE,14,14,14,14,14,20,20,14,14,14,14,14,23,9 ,o ,o ,o ,o ,o ,21,20,14,14,14,EE,14,14,14,14,14,14,14,10,9 ,8 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61,2 ,2 ,2 ,2 ,61,o ,21, /*61*/ 
/*62*/ 20,14,14,14,14,14,14,14,20,14,14,14,14,14,20,20,20,20,20,20,20,23,8 ,o ,o ,o ,6 ,o ,21,20,14,14,14,20,14,14,14,14,14,20,23,3 ,3 ,3 ,3 ,21,21,21,23,23,23,23,23,23,55,56,61,2 ,2 ,2 ,2 ,o ,o ,21, /*62*/ 
/*63*/ 20,20,50,50,20,50,50,20,20,20,50,20,50,20,20,20,20,20,20,20,20,23,7 ,7 ,7 ,7 ,7 ,7 ,23,20,20,20,20,20,20,20,20,20,20,20,23,28,28,28,28,21,21,21,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23  /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef o
    },
    {        // tileDictionary
      SFG_TD(10,31,1,0),SFG_TD(29,31,0,0),SFG_TD( 0,31,0,0),SFG_TD(17,31,2,0), // 0 
      SFG_TD(10,31,2,0),SFG_TD(29,31,2,0),SFG_TD(14,31,5,0),SFG_TD(21,31,2,0), // 4 
      SFG_TD(11,31,4,0),SFG_TD(12,31,4,0),SFG_TD(13,31,4,0),SFG_TD(14,31,4,0), // 8 
      SFG_TD(12,31,2,0),SFG_TD(14,31,2,0),SFG_TD(13, 5,2,0),SFG_TD(14, 7,2,0), // 12 
      SFG_TD(11,31,2,0),SFG_TD(13,31,2,0),SFG_TD(10, 7,0,0),SFG_TD(30,31,2,0), // 16 
      SFG_TD(30, 0,2,0),SFG_TD(26,31,0,0),SFG_TD(30,31,0,0),SFG_TD(10, 0,0,0), // 20 
      SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0),SFG_TD(15,31,6,0),SFG_TD(10, 0,3,3), // 24 
      SFG_TD(24,31,6,0),SFG_TD(31, 0,2,0),SFG_TD(18,31,6,0),SFG_TD(13,31,6,0), // 28 
      SFG_TD(14, 0,1,0),SFG_TD(10, 7,1,0),SFG_TD(13,31,2,0),SFG_TD(17,31,4,0), // 32 
      SFG_TD(30,31,4,0),SFG_TD(30,31,3,0),SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0), // 36 
      SFG_TD(11,31,2,0),SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0), // 40 
      SFG_TD(14, 0,1,0),SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0),SFG_TD(10, 7,0,0), // 44 
      SFG_TD(10, 7,0,0),SFG_TD(30,31,2,0),SFG_TD(15, 2,2,2),SFG_TD(13, 5,2,0), // 48 
      SFG_TD( 9,31,5,0),SFG_TD(17, 0,3,3),SFG_TD(14,31,1,0),SFG_TD( 9,31,1,0), // 52 
      SFG_TD( 6,31,1,0),SFG_TD(18,31,0,0),SFG_TD(10,11,3,3),SFG_TD( 0,10,5,0), // 56 
      SFG_TD(12,31,1,0),SFG_TD( 3,31,1,0),SFG_TD(12,31,5,0),SFG_TD(13,17,5,0)  // 60 
      },                    // tileDictionary
    {1 ,9 ,12,14,5 ,0 ,6 }, // textureIndices
    7,                      // doorTextureIndex
    20,                     // floorColor
    3,                      // ceilingColor
    {50, 24, 64 },          // player start: x, y, direction
    0,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_BULLETS, {35,1}},{SFG_LEVEL_ELEMENT_BULLETS, {34,1}},
      {SFG_LEVEL_ELEMENT_RUIN, {6,5}},{SFG_LEVEL_ELEMENT_HEALTH, {30,6}},
      {SFG_LEVEL_ELEMENT_RUIN, {23,7}},{SFG_LEVEL_ELEMENT_HEALTH, {23,8}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {37,9}},{SFG_LEVEL_ELEMENT_RUIN, {4,9}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,10}},{SFG_LEVEL_ELEMENT_CARD2, {5,10}},
      {SFG_LEVEL_ELEMENT_HEALTH, {26,11}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {22,11}},
      {SFG_LEVEL_ELEMENT_RUIN, {51,13}},{SFG_LEVEL_ELEMENT_RUIN, {50,13}},
      {SFG_LEVEL_ELEMENT_RUIN, {49,13}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {21,13}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {19,13}},{SFG_LEVEL_ELEMENT_ROCKETS, {55,14}},
      {SFG_LEVEL_ELEMENT_BULLETS, {54,14}},{SFG_LEVEL_ELEMENT_BULLETS, {9,16}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {16,17}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {7,17}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {6,18}},{SFG_LEVEL_ELEMENT_LAMP, {43,19}},
      {SFG_LEVEL_ELEMENT_LAMP, {21,19}},{SFG_LEVEL_ELEMENT_LAMP, {18,19}},
      {SFG_LEVEL_ELEMENT_CARD1, {9,20}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {8,20}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {50,21}},{SFG_LEVEL_ELEMENT_LAMP, {5,22}},
      {SFG_LEVEL_ELEMENT_TREE, {4,22}},{SFG_LEVEL_ELEMENT_RUIN, {34,23}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {53,24}},{SFG_LEVEL_ELEMENT_BLOCKER, {53,25}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {18,25}},{SFG_LEVEL_ELEMENT_RUIN, {1,25}},
      {SFG_LEVEL_ELEMENT_PLASMA, {49,27}},{SFG_LEVEL_ELEMENT_ROCKETS, {48,27}},
      {SFG_LEVEL_ELEMENT_PLASMA, {34,27}},{SFG_LEVEL_ELEMENT_BULLETS, {49,28}},
      {SFG_LEVEL_ELEMENT_LOCK2, {47,28}},{SFG_LEVEL_ELEMENT_LAMP, {15,28}},
      {SFG_LEVEL_ELEMENT_RUIN, {13,28}},{SFG_LEVEL_ELEMENT_PLASMA, {49,29}},
      {SFG_LEVEL_ELEMENT_HEALTH, {48,29}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {45,29}},
      {SFG_LEVEL_ELEMENT_LAMP, {35,32}},{SFG_LEVEL_ELEMENT_LAMP, {21,32}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {7,32}},{SFG_LEVEL_ELEMENT_FINISH, {56,34}},
      {SFG_LEVEL_ELEMENT_FINISH, {55,34}},{SFG_LEVEL_ELEMENT_PLASMA, {24,34}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {56,35}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {55,35}},
      {SFG_LEVEL_ELEMENT_BULLETS, {24,35}},{SFG_LEVEL_ELEMENT_BARREL, {20,35}},
      {SFG_LEVEL_ELEMENT_PLASMA, {9,35}},{SFG_LEVEL_ELEMENT_BULLETS, {5,35}},
      {SFG_LEVEL_ELEMENT_COLUMN, {57,37}},{SFG_LEVEL_ELEMENT_COLUMN, {54,37}},
      {SFG_LEVEL_ELEMENT_BULLETS, {15,37}},{SFG_LEVEL_ELEMENT_HEALTH, {60,38}},
      {SFG_LEVEL_ELEMENT_RUIN, {44,38}},{SFG_LEVEL_ELEMENT_ROCKETS, {26,39}},
      {SFG_LEVEL_ELEMENT_CARD0, {34,40}},{SFG_LEVEL_ELEMENT_LOCK1, {11,40}},
      {SFG_LEVEL_ELEMENT_PLASMA, {19,41}},{SFG_LEVEL_ELEMENT_TERMINAL, {14,41}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {55,42}},{SFG_LEVEL_ELEMENT_TREE, {36,42}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {19,42}},{SFG_LEVEL_ELEMENT_HEALTH, {60,44}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {56,44}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {54,44}},
      {SFG_LEVEL_ELEMENT_TREE, {48,44}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {33,44}},
      {SFG_LEVEL_ELEMENT_BULLETS, {48,45}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {27,45}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {26,45}},{SFG_LEVEL_ELEMENT_COLUMN, {15,45}},
      {SFG_LEVEL_ELEMENT_TREE, {48,46}},{SFG_LEVEL_ELEMENT_LAMP, {1,47}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {15,48}},{SFG_LEVEL_ELEMENT_ROCKETS, {14,48}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {7,48}},{SFG_LEVEL_ELEMENT_HEALTH, {61,49}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {6,49}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {3,49}},
      {SFG_LEVEL_ELEMENT_TREE, {36,50}},{SFG_LEVEL_ELEMENT_HEALTH, {5,50}},
      {SFG_LEVEL_ELEMENT_LAMP, {58,51}},{SFG_LEVEL_ELEMENT_LAMP, {53,51}},
      {SFG_LEVEL_ELEMENT_HEALTH, {26,51}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {12,51}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {7,51}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {26,53}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {5,53}},{SFG_LEVEL_ELEMENT_LAMP, {1,53}},
      {SFG_LEVEL_ELEMENT_PLASMA, {58,55}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {55,55}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {50,55}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,55}},
      {SFG_LEVEL_ELEMENT_BARREL, {13,55}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,56}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {25,56}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,57}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {24,57}},{SFG_LEVEL_ELEMENT_RUIN, {47,58}},
      {SFG_LEVEL_ELEMENT_RUIN, {46,58}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {23,58}},{SFG_LEVEL_ELEMENT_BULLETS, {21,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {18,58}},{SFG_LEVEL_ELEMENT_RUIN, {46,59}},
      {SFG_LEVEL_ELEMENT_RUIN, {45,59}},{SFG_LEVEL_ELEMENT_LOCK0, {31,60}},
      {SFG_LEVEL_ELEMENT_LOCK0, {30,60}},{SFG_LEVEL_ELEMENT_TREE, {7,60}},
      {SFG_LEVEL_ELEMENT_LOCK1, {33,61}},{SFG_LEVEL_ELEMENT_TREE, {16,61}},
      {SFG_LEVEL_ELEMENT_TREE, {13,61}},{SFG_LEVEL_ELEMENT_LOCK0, {8,61}},
      {SFG_LEVEL_ELEMENT_BARREL, {27,62}},{SFG_LEVEL_ELEMENT_HEALTH, {25,62}},
      {SFG_LEVEL_ELEMENT_TREE, {1,62}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level6 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (17 | SFG_TILE_PROPERTY_DOOR)
    #define BB (33 | SFG_TILE_PROPERTY_DOOR)
    #define CC (16 | SFG_TILE_PROPERTY_ELEVATOR)
    #define DD (1 | SFG_TILE_PROPERTY_DOOR)
    #define EE (41 | SFG_TILE_PROPERTY_ELEVATOR)
    #define FF (55 | SFG_TILE_PROPERTY_DOOR)
    #define GG (56 | SFG_TILE_PROPERTY_ELEVATOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 1 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,1 ,1 ,1 ,1 ,5 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,33,50,50,33,33,33,50,5 ,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,o ,o ,o ,o ,o , /*0 */ 
/*1 */ 47,14,14,1 ,13,13,13,13,13,13,13,13,13,13,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,31,31,50,31,31,31,50,5 ,19,19,19,5 ,19,19,19,5 ,19,19,19,5 ,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*1 */ 
/*2 */ 47,14,14,15,13,13,13,13,13,13,13,13,13,13,1 ,5 ,5 ,33,50,50,50,33,5 ,5 ,5 ,33,50,50,50,33,5 ,5 ,31,31,50,31,31,31,50,50,50,33,50,50,50,33,50,50,50,33,50,50,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*2 */ 
/*3 */ 63,14,14,15,13,13,13,13,13,13,13,13,13,13,1 ,5 ,31,31,31,50,31,31,31,5 ,31,31,31,50,31,31,31,5 ,31,31,50,31,31,31,50,5 ,31,31,31,50,31,31,31,5 ,31,31,31,31,19,19,19,19,19,19,20,o ,o ,o ,o ,o , /*3 */ 
/*4 */ 63,14,14,15,13,13,13,35,35,35,35,13,13,13,1 ,1 ,31,31,34,63,34,31,31,1 ,31,31,34,63,34,31,31,1 ,31,31,50,31,31,31,50,5 ,31,31,34,63,34,31,31,5 ,31,31,31,31,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*4 */ 
/*5 */ 1 ,1 ,AA,1 ,13,13,13,35,13,13,35,13,13,13,1 ,5 ,31,31,31,50,31,31,31,5 ,31,31,31,50,31,31,31,5 ,31,31,50,31,31,31,50,50,31,31,31,50,31,31,31,50,31,31,31,50,5 ,1 ,19,19,19,19,20,o ,o ,o ,o ,o , /*5 */ 
/*6 */ 1 ,1 ,CC,1 ,13,13,13,35,35,35,35,13,13,13,1 ,5 ,31,31,34,63,34,31,31,5 ,31,31,34,63,34,31,31,5 ,31,31,BB,31,31,31,50,5 ,31,31,34,63,34,31,31,5 ,31,31,34,63,5 ,24,19,19,19,19,20,o ,o ,o ,o ,o , /*6 */ 
/*7 */ o ,1 ,DD,1 ,13,13,13,13,13,13,13,13,13,13,1 ,1 ,31,31,34,63,34,31,31,1 ,31,31,34,63,34,31,31,1 ,31,31,50,31,31,31,50,50,31,31,34,63,34,31,31,50,31,31,34,63,5 ,24,19,19,19,19,20,o ,o ,o ,o ,o , /*7 */ 
/*8 */ o ,1 ,18,18,13,13,13,13,13,13,13,13,13,13,1 ,5 ,31,31,31,50,31,31,31,5 ,31,31,31,50,31,31,31,5 ,31,50,31,31,31,31,31,50,31,31,31,50,31,31,31,50,31,31,31,50,5 ,24,19,19,19,19,20,o ,o ,o ,o ,o , /*8 */ 
/*9 */ o ,1 ,13,13,13,13,13,13,13,13,13,13,13,13,1 ,50,50,BB,50,63,50,BB,50,63,50,BB,50,63,50,BB,50,63,50,31,31,31,31,31,31,31,50,BB,50,63,50,BB,50,63,50,BB,50,63,5 ,1 ,19,19,19,19,20,o ,o ,o ,o ,o , /*9 */ 
/*10*/ o ,1 ,13,13,13,13,13,13,13,13,13,13,13,13,1 ,50,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,49,49,49,31,31,31,31,31,31,31,31,31,31,31,31,31,50,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*10*/ 
/*11*/ 1 ,1 ,13,13,13,13,13,13,13,13,13,13,13,13,1 ,50,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,45,49,45,31,31,31,31,31,31,31,31,31,31,31,31,31,50,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*11*/ 
/*12*/ 1 ,14,13,13,13,13,13,13,13,13,13,13,13,13,1 ,50,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,45,45,45,31,31,31,31,31,31,31,31,31,31,31,31,31,50,5 ,19,19,19,19,19,20,o ,o ,o ,o ,o , /*12*/ 
/*13*/ 1 ,14,13,13,13,13,13,13,13,13,13,13,35,13,1 ,50,50,BB,50,63,50,BB,50,63,50,BB,50,63,50,BB,50,63,50,31,31,31,31,31,31,31,50,BB,50,63,50,BB,50,63,50,BB,50,63,1 ,24,24,1 ,5 ,5 ,1 ,1 ,1 ,1 ,o ,o , /*13*/ 
/*14*/ 1 ,14,13,13,13,13,13,13,13,13,13,13,35,18,1 ,5 ,31,31,31,50,31,31,31,5 ,31,31,31,50,31,31,31,5 ,31,50,31,31,31,31,31,50,31,31,31,50,31,31,31,5 ,31,31,31,50,1 ,40,40,1 ,3 ,3 ,1 ,40,40,1 ,o ,o , /*14*/ 
/*15*/ 1 ,14,1 ,1 ,1 ,5 ,1 ,3 ,3 ,3 ,3 ,1 ,5 ,DD,5 ,1 ,31,31,34,63,34,31,31,5 ,31,31,34,63,34,31,31,5 ,31,31,50,31,31,31,50,50,31,31,34,63,34,31,31,5 ,31,31,34,63,1 ,40,13,13,3 ,3 ,13,13,40,1 ,1 ,20, /*15*/ 
/*16*/ 1 ,14,14,1 ,o ,o ,1 ,24,24,24,24,1 ,14,CC,14,1 ,31,31,34,63,34,31,31,1 ,31,31,34,63,34,31,31,1 ,31,31,50,31,31,31,50,5 ,31,31,34,63,34,31,31,50,31,31,34,63,1 ,40,13,13,3 ,3 ,13,13,40,5 ,19,20, /*16*/ 
/*17*/ 1 ,14,14,1 ,o ,o ,1 ,24,24,24,24,1 ,14,14,14,1 ,31,31,31,50,31,31,31,1 ,31,31,31,50,31,31,31,5 ,31,31,50,31,31,31,50,50,31,31,31,50,31,31,31,50,31,31,31,50,1 ,40,40,1 ,3 ,3 ,1 ,40,40,40,19,20, /*17*/ 
/*18*/ 1 ,14,14,1 ,o ,o ,1 ,1 ,24,24,1 ,1 ,14,14,14,1 ,31,31,34,63,34,31,31,5 ,31,31,34,63,34,31,31,5 ,31,31,50,31,31,31,50,5 ,31,31,31,31,34,31,31,5 ,31,31,34,63,1 ,40,13,13,3 ,3 ,13,13,40,40,19,20, /*18*/ 
/*19*/ 1 ,14,14,1 ,1 ,1 ,1 ,24,24,24,24,1 ,14,14,14,1 ,31,31,31,50,31,31,31,5 ,31,31,31,50,31,31,31,BB,31,31,50,31,31,31,50,5 ,31,31,31,31,31,31,31,5 ,31,31,31,50,1 ,40,13,13,3 ,3 ,13,13,40,5 ,19,20, /*19*/ 
/*20*/ 1 ,14,14,1 ,14,14,1 ,13,13,13,13,1 ,14,14,14,1 ,5 ,5 ,36,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,31,31,31,50,50,50,50,50,50,50,50,50,50,50,50,50,50,1 ,40,40,1 ,3 ,3 ,1 ,40,40,1 ,1 ,20, /*20*/ 
/*21*/ 1 ,14,14,1 ,14,14,14,14,13,13,14,14,14,14,14,1 ,4 ,4 ,36,4 ,4 ,4 ,50,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,50,1 ,1 ,1 ,1 ,1 ,1 ,1 ,63,63,63,63,63,63,1 ,40,13,13,3 ,3 ,13,13,40,1 ,o ,o , /*21*/ 
/*22*/ 1 ,14,14,14,14,14,1 ,13,13,13,13,1 ,14,14,14,1 ,4 ,4 ,36,36,36,4 ,50,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,50,1 ,2 ,2 ,DD,2 ,o ,24,o ,24,o ,24,24,24,24,40,13,13,3 ,3 ,13,13,40,1 ,o ,o , /*22*/ 
/*23*/ 1 ,14,14,14,14,14,1 ,13,13,13,13,1 ,37,1 ,37,1 ,4 ,4 ,4 ,4 ,36,4 ,50,31,31,31,31,30,30,32,30,30,31,31,31,50,50,50,50,1 ,2 ,2 ,1 ,2 ,o ,24,o ,24,o ,24,24,24,24,40,40,1 ,3 ,3 ,1 ,40,40,1 ,o ,o , /*23*/ 
/*24*/ 1 ,1 ,1 ,1 ,14,14,14,13,13,13,13,24,24,24,24,24,13,13,13,4 ,36,4 ,63,31,31,31,31,29,29,32,29,29,31,31,31,1 ,o ,o ,2 ,1 ,2 ,2 ,1 ,1 ,1 ,1 ,63,63,63,63,63,63,1 ,40,40,40,3 ,3 ,40,40,40,1 ,o ,o , /*24*/ 
/*25*/ o ,o ,o ,1 ,14,14,14,13,13,13,13,24,24,24,24,24,13,13,13,36,36,4 ,63,31,31,31,31,28,28,32,28,28,31,31,31,1 ,o ,o ,2 ,DD,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,40,40,40,3 ,3 ,40,40,40,1 ,o ,o , /*25*/ 
/*26*/ o ,o ,o ,1 ,14,14,14,13,13,13,13,24,24,24,24,24,13,13,13,4 ,4 ,4 ,63,31,31,31,31,27,27,32,27,27,31,31,31,1 ,40,40,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,5 ,5 ,1 ,1 ,1 ,1 ,o ,o , /*26*/ 
/*27*/ o ,o ,o ,1 ,4 ,4 ,4 ,36,4 ,4 ,36,1 ,37,1 ,37,1 ,24,24,24,1 ,1 ,1 ,1 ,31,31,31,31,26,26,26,26,26,31,31,31,1 ,40,40,1 ,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55, /*27*/ 
/*28*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,36,36,36,36,36,4 ,1 ,1 ,37,24,24,24,37,37,37,1 ,31,31,31,32,25,25,25,25,25,32,31,31,1 ,40,40,1 ,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,55, /*28*/ 
/*29*/ 46,58,58,58,58,58,4 ,4 ,4 ,4 ,4 ,31,4 ,1 ,1 ,1 ,40,40,40,40,40,24,24,24,24,24,24,24,24,24,24,24,24,24,24,5 ,40,40,1 ,55,o ,1 ,1 ,1 ,o ,1 ,1 ,1 ,o ,1 ,1 ,o ,1 ,1 ,o ,1 ,1 ,1 ,o ,1 ,1 ,o ,o ,55, /*29*/ 
/*30*/ 46,58,58,58,58,58,58,58,58,29,30,31,4 ,1 ,1 ,1 ,40,40,40,40,40,24,24,24,24,24,24,24,24,24,24,24,24,24,24,40,40,40,1 ,55,o ,o ,1 ,o ,o ,o ,1 ,1 ,o ,o ,1 ,o ,o ,1 ,o ,o ,1 ,1 ,o ,o ,1 ,o ,o ,55, /*30*/ 
/*31*/ 46,58,58,58,58,58,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,1 ,1 ,40,40,40,40,40,24,24,24,24,24,24,24,24,24,24,24,24,24,24,40,40,40,1 ,55,o ,o ,1 ,o ,o ,1 ,1 ,1 ,o ,o ,1 ,o ,1 ,1 ,o ,1 ,1 ,1 ,o ,o ,1 ,1 ,o ,55, /*31*/ 
/*32*/ 4 ,4 ,4 ,4 ,4 ,4 ,4 ,39,39,39,39,39,39,1 ,1 ,1 ,40,40,40,40,40,24,24,24,24,24,24,24,24,24,24,24,24,24,24,5 ,40,40,1 ,55,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,55, /*32*/ 
/*33*/ 39,39,39,39,39,39,39,39,o ,o ,o ,o ,o ,3 ,3 ,3 ,40,40,40,1 ,1 ,31,31,31,31,31,24,31,31,31,31,31,31,31,24,31,31,1 ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55, /*33*/ 
/*34*/ 60,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,40,40,40,1 ,5 ,31,31,31,31,31,24,31,31,31,31,31,31,31,24,31,31,1 ,1 ,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,63, /*34*/ 
/*35*/ 60,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,40,40,40,1 ,63,31,31,24,24,24,24,63,1 ,1 ,1 ,24,24,24,24,31,31,1 ,1 ,39,39,39,52,52,52,52,52,52,24,24,26,EE,EE,EE,24,24,24,52,52,52,52,52,52,7 , /*35*/ 
/*36*/ 60,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,39,o ,61,1 ,1 ,1 ,1 ,63,31,31,24,24,24,24,63,1 ,1 ,1 ,24,24,24,24,31,31,1 ,1 ,39,39,39,39,39,52,52,52,52,24,24,24,EE,EE,EE,24,24,24,52,52,52,52,52,52,63, /*36*/ 
/*37*/ 39,39,5 ,o ,o ,5 ,5 ,o ,5 ,5 ,o ,o ,o ,39,o ,61,1 ,1 ,1 ,1 ,63,31,31,24,24,24,24,63,1 ,1 ,1 ,24,24,24,24,31,31,1 ,1 ,1 ,1 ,1 ,1 ,39,52,52,52,39,24,24,26,EE,EE,EE,26,24,24,52,52,52,52,52,52,7 , /*37*/ 
/*38*/ o ,o ,39,o ,o ,39,o ,o ,o ,39,o ,o ,o ,39,o ,61,1 ,1 ,1 ,1 ,5 ,31,31,31,31,31,24,31,31,31,1 ,24,24,24,24,31,31,31,31,30,29,28,1 ,39,52,52,52,39,24,24,26,26,24,24,5 ,24,24,4 ,39,39,39,39,39,63, /*38*/ 
/*39*/ o ,o ,39,o ,o ,5 ,o ,o ,o ,5 ,39,39,39,39,o ,61,1 ,1 ,1 ,1 ,1 ,31,31,31,31,31,24,31,31,31,1 ,24,24,24,24,31,31,31,31,30,29,28,1 ,39,39,39,o ,39,24,24,24,24,24,24,5 ,24,24,o ,4 ,o ,o ,o ,o ,o , /*39*/ 
/*40*/ o ,o ,39,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,39,o ,61,61,61,61,61,61,24,24,24,24,24,24,1 ,30,30,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,25,26,27,1 ,o ,o ,39,o ,39,24,24,24,24,24,24,5 ,24,24,o ,o ,4 ,o ,o ,o ,o , /*40*/ 
/*41*/ o ,o ,39,o ,o ,5 ,o ,o ,o ,o ,o ,o ,o ,39,o ,o ,o ,o ,o ,o ,o ,24,24,24,24,24,24,1 ,29,29,1 ,1 ,1 ,63,63,63,1 ,o ,o ,25,26,27,1 ,o ,o ,o ,o ,39,4 ,4 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,o ,o ,o ,4 ,o ,o ,o , /*41*/ 
/*42*/ o ,o ,39,o ,o ,39,o ,o ,o ,o ,o ,o ,o ,39,o ,61,61,61,61,61,61,24,24,24,24,24,24,63,28,28,27,26,25,24,24,24,24,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,53,o ,51,38,38,38,4 ,o ,o ,o ,4 ,o ,o ,o ,4 ,o ,o , /*42*/ 
/*43*/ o ,o ,39,o ,o ,5 ,5 ,o ,5 ,39,5 ,o ,o ,39,o ,61,51,51,51,51,61,24,24,24,24,24,24,63,28,28,27,26,25,24,24,24,24,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,53,o ,51,38,38,38,o ,o ,4 ,o ,4 ,4 ,o ,o ,o ,4 ,o , /*43*/ 
/*44*/ 39,39,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,39,o ,5 ,52,51,51,51,61,24,24,24,24,24,24,63,63,63,1 ,1 ,1 ,24,24,24,1 ,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,53,o ,51,38,38,38,o ,4 ,1 ,o ,4 ,1 ,4 ,o ,o ,o ,4 , /*44*/ 
/*45*/ 1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,39,o ,o ,o ,51,51,51,5 ,61,61,61,o ,o ,61,61,51,51,51,51,12,24,24,24,12,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,53,o ,51,38,38,38,o ,4 ,4 ,o ,o ,o ,o ,4 ,o ,o ,60, /*45*/ 
/*46*/ 1 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,39,39,39,39,o ,5 ,52,51,51,51,61,3 ,3 ,3 ,3 ,3 ,o ,52,51,51,51,51,1 ,24,24,24,1 ,o ,o ,o ,o ,o ,1 ,59,53,53,53,53,o ,51,38,38,38,o ,o ,4 ,o ,o ,o ,o ,4 ,o ,o ,4 , /*46*/ 
/*47*/ 1 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,DD,2 ,o ,o ,o ,61,51,51,51,51,61,3 ,3 ,3 ,3 ,3 ,o ,52,51,51,51,51,12,24,24,24,12,o ,o ,o ,o ,o ,1 ,59,59,59,59,59,o ,51,38,38,38,4 ,o ,1 ,o ,4 ,1 ,4 ,4 ,o ,o ,60, /*47*/ 
/*48*/ 1 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,61,o ,o ,o ,o ,61,61,61,61,61,61,3 ,3 ,61,3 ,3 ,61,52,52,52,52,1 ,1 ,o ,o ,o ,1 ,o ,o ,o ,o ,o ,1 ,4 ,4 ,59,4 ,4 ,o ,51,38,38,38,4 ,o ,4 ,o ,o ,o ,o ,4 ,o ,o ,4 , /*48*/ 
/*49*/ 60,o ,o ,63,48,48,48,48,48,48,63,63,63,63,63,63,5 ,24,5 ,6 ,6 ,3 ,3 ,3 ,3 ,3 ,6 ,6 ,6 ,48,48,1 ,1 ,1 ,2 ,1 ,1 ,o ,o ,o ,o ,o ,1 ,4 ,57,59,57,4 ,o ,51,38,38,38,o ,o ,4 ,o ,o ,o ,o ,4 ,o ,o ,60, /*49*/ 
/*50*/ 60,o ,o ,63,6 ,6 ,6 ,6 ,6 ,6 ,25,26,27,28,29,30,30,24,24,6 ,6 ,3 ,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,1 ,1 ,1 ,DD,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,4 ,4 ,59,4 ,4 ,o ,51,38,38,38,o ,4 ,1 ,o ,4 ,1 ,4 ,4 ,o ,o ,4 , /*50*/ 
/*51*/ 60,o ,o ,63,6 ,6 ,6 ,6 ,6 ,6 ,25,26,27,28,29,30,30,24,24,6 ,6 ,3 ,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,1 ,1 ,o ,2 ,2 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,4 ,57,59,57,4 ,o ,51,38,38,38,o ,o ,4 ,o ,o ,o ,o ,5 ,o ,o ,60, /*51*/ 
/*52*/ 60,o ,o ,63,6 ,6 ,6 ,6 ,6 ,6 ,25,26,27,28,29,30,5 ,6 ,5 ,1 ,63,63,63,63,1 ,1 ,6 ,6 ,6 ,5 ,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,4 ,4 ,4 ,59,4 ,4 ,o ,51,38,38,38,4 ,o ,4 ,o ,o ,o ,o ,40,24,24,4 , /*52*/ 
/*53*/ 60,o ,o ,63,6 ,6 ,6 ,6 ,6 ,6 ,25,26,27,28,29,30,5 ,6 ,6 ,48,6 ,6 ,6 ,6 ,6 ,5 ,6 ,6 ,6 ,5 ,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,4 ,24,24,24,24,4 ,o ,51,38,38,38,4 ,24,4 ,o ,o ,o ,o ,40,24,24,4 , /*53*/ 
/*54*/ 1 ,o ,o ,1 ,6 ,6 ,6 ,6 ,6 ,6 ,25,26,27,28,29,30,5 ,6 ,6 ,48,6 ,6 ,6 ,6 ,6 ,5 ,6 ,6 ,6 ,2 ,1 ,2 ,o ,3 ,3 ,3 ,o ,2 ,1 ,2 ,o ,o ,o ,24,24,24,62,4 ,o ,51,38,38,38,4 ,24,4 ,o ,o ,o ,o ,5 ,24,24,4 , /*54*/ 
/*55*/ 5 ,25,25,5 ,25,25,5 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,5 ,6 ,6 ,48,6 ,6 ,6 ,6 ,6 ,5 ,6 ,6 ,6 ,2 ,DD,2 ,o ,3 ,3 ,3 ,o ,2 ,DD,2 ,o ,3 ,3 ,24,24,24,24,4 ,4 ,4 ,5 ,5 ,5 ,4 ,54,4 ,4 ,4 ,1 ,1 ,4 ,54,24,4 , /*55*/ 
/*56*/ 1 ,26,26,1 ,26,26,1 ,31,31,31,31,31,31,31,31,1 ,5 ,6 ,6 ,48,48,48,48,48,6 ,5 ,6 ,6 ,6 ,2 ,1 ,2 ,o ,3 ,3 ,3 ,o ,2 ,1 ,2 ,o ,o ,o ,24,24,24,62,4 ,53,53,53,53,53,4 ,FF,4 ,53,53,4 ,4 ,4 ,FF,4 ,4 , /*56*/ 
/*57*/ 5 ,27,27,5 ,27,27,5 ,31,31,31,31,31,31,31,31,1 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,6 ,6 ,6 ,5 ,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,4 ,24,24,24,24,4 ,4 ,4 ,4 ,62,24,24,54,24,24,62,4 ,40,40,54,40,4 , /*57*/ 
/*58*/ 1 ,28,28,1 ,28,28,1 ,31,31,31,31,31,31,31,31,1 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,1 ,6 ,6 ,6 ,5 ,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,58,58,58,4 ,58,4 ,4 ,4 ,58,58,4 ,24,24,24,24,24,24,24,4 ,24,24,24,24,4 , /*58*/ 
/*59*/ 5 ,28,28,5 ,28,28,5 ,30,30,50,50,5 ,BB,5 ,50,50,5 ,o ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,o ,o ,o ,o ,o ,1 ,1 ,4 ,58,4 ,58,4 ,58,4 ,58,58,58,58,4 ,24,24,24,24,24,24,24,4 ,24,24,24,24,4 , /*59*/ 
/*60*/ 1 ,28,28,1 ,28,28,1 ,29,29,50,31,34,31,31,31,50,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,1 ,1 ,1 ,o ,1 ,1 ,1 ,4 ,4 ,58,4 ,58,58,58,4 ,58,58,4 ,58,4 ,24,24,24,24,24,24,24,58,24,24,24,24,4 , /*60*/ 
/*61*/ 1 ,28,28,28,28,28,28,28,28,50,31,34,31,31,31,50,5 ,o ,1 ,o ,o ,o ,o ,o ,o ,o ,3 ,3 ,3 ,o ,o ,2 ,2 ,1 ,o ,1 ,4 ,4 ,58,58,58,4 ,4 ,4 ,4 ,4 ,58,58,4 ,58,4 ,4 ,24,24,24,24,54,54,4 ,24,24,24,24,4 , /*61*/ 
/*62*/ 1 ,28,28,28,28,28,28,28,28,50,31,31,31,31,31,50,5 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,2 ,1 ,o ,1 ,4 ,4 ,58,4 ,58,58,58,58,58,58,58,58,4 ,58,58,58,GG,24,24,24,54,54,4 ,40,40,40,40,4 , /*62*/ 
/*63*/ 1 ,1 ,1 ,37,1 ,1 ,37,1 ,1 ,50,50,33,50,33,50,50,5 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,1 ,7 ,7 ,7 ,7 ,7 ,7 ,1 ,1 ,DD,1 ,4 ,4 ,46,4 ,4 ,57,4 ,57,4 ,57,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,60,60,4 ,4   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef GG
    #undef o
    },
    {        // tileDictionary
      SFG_TD( 8, 6,1,1),SFG_TD(13, 0,0,0),SFG_TD( 9, 5,0,0),SFG_TD( 8, 8,0,0), // 0 
      SFG_TD(13, 0,2,2),SFG_TD(13, 0,3,3),SFG_TD( 8, 9,0,0),SFG_TD(11, 3,4,0), // 4 
      SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0), // 8 
      SFG_TD(10, 2,0,0),SFG_TD( 8,21,0,0),SFG_TD(23, 6,0,0),SFG_TD(25, 2,0,0), // 12 
      SFG_TD( 9,14,2,0),SFG_TD(27, 0,0,0),SFG_TD( 9,20,0,0),SFG_TD( 8,31,1,1), // 16 
      SFG_TD(11, 0,0,7),SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0), // 20 
      SFG_TD( 8,13,0,0),SFG_TD( 9,12,0,0),SFG_TD(10,11,0,0),SFG_TD(11,10,0,0), // 24 
      SFG_TD(12, 9,0,0),SFG_TD(13, 8,0,0),SFG_TD(14, 7,0,0),SFG_TD(15, 6,0,0), // 28 
      SFG_TD(18, 3,3,0),SFG_TD(19, 0,6,4),SFG_TD(17, 4,4,0),SFG_TD(10,19,1,0), // 32 
      SFG_TD(17, 4,2,2),SFG_TD(13, 0,1,1),SFG_TD( 4, 9,1,1),SFG_TD(11, 0,1,0), // 36 
      SFG_TD( 8, 6,1,0),SFG_TD( 7, 1,1,0),SFG_TD(10, 5,0,0),SFG_TD(10, 5,0,0), // 40 
      SFG_TD(10, 5,0,0),SFG_TD(18, 4,3,0),SFG_TD(17, 0,6,0),SFG_TD(27, 0,6,0), // 44 
      SFG_TD(10, 3,4,4),SFG_TD(15, 7,0,0),SFG_TD(19, 0,0,4),SFG_TD( 6, 8,1,1), // 48 
      SFG_TD( 7, 7,1,1),SFG_TD(11, 0,2,5),SFG_TD( 9,12,0,0),SFG_TD(13, 0,5,5), // 52 
      SFG_TD( 8, 5,2,2),SFG_TD( 5, 0,5,5),SFG_TD(13, 4,2,2),SFG_TD( 8, 7,2,2), // 56 
      SFG_TD(12, 0,6,0),SFG_TD(14, 0,0,1),SFG_TD(11,10,5,0),SFG_TD( 0, 0,4,4)  // 60 
      },                    // tileDictionary
    {12,3 ,11,14,4 ,8 ,10}, // textureIndices
    7,                      // doorTextureIndex
    58,                     // floorColor
    2,                      // ceilingColor
    {34, 59, 64 },          // player start: x, y, direction
    0,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_PLASMA, {33,1}},{SFG_LEVEL_ELEMENT_HEALTH, {8,1}},
      {SFG_LEVEL_ELEMENT_CARD1, {2,1}},{SFG_LEVEL_ELEMENT_HEALTH, {1,1}},
      {SFG_LEVEL_ELEMENT_RUIN, {52,3}},{SFG_LEVEL_ELEMENT_BULLETS, {20,3}},
      {SFG_LEVEL_ELEMENT_RUIN, {51,4}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {41,7}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {36,8}},{SFG_LEVEL_ELEMENT_PLASMA, {7,8}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {8,9}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {6,9}},
      {SFG_LEVEL_ELEMENT_HEALTH, {53,10}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {10,10}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {4,10}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {40,11}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {36,11}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {32,11}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {7,11}},{SFG_LEVEL_ELEMENT_ROCKETS, {1,12}},
      {SFG_LEVEL_ELEMENT_LOCK0, {17,13}},{SFG_LEVEL_ELEMENT_PLASMA, {1,13}},
      {SFG_LEVEL_ELEMENT_TREE, {57,14}},{SFG_LEVEL_ELEMENT_TREE, {56,14}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {17,14}},{SFG_LEVEL_ELEMENT_HEALTH, {33,15}},
      {SFG_LEVEL_ELEMENT_CARD0, {62,17}},{SFG_LEVEL_ELEMENT_BULLETS, {46,17}},
      {SFG_LEVEL_ELEMENT_RUIN, {43,18}},{SFG_LEVEL_ELEMENT_ROCKETS, {20,18}},
      {SFG_LEVEL_ELEMENT_PLASMA, {17,18}},{SFG_LEVEL_ELEMENT_LOCK0, {42,22}},
      {SFG_LEVEL_ELEMENT_COLUMN, {29,23}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {8,23}},
      {SFG_LEVEL_ELEMENT_HEALTH, {20,24}},{SFG_LEVEL_ELEMENT_TERMINAL, {41,25}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {14,25}},{SFG_LEVEL_ELEMENT_COLUMN, {29,26}},
      {SFG_LEVEL_ELEMENT_LAMP, {32,28}},{SFG_LEVEL_ELEMENT_LAMP, {26,28}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,29}},{SFG_LEVEL_ELEMENT_HEALTH, {61,30}},
      {SFG_LEVEL_ELEMENT_PLASMA, {59,30}},{SFG_LEVEL_ELEMENT_HEALTH, {55,30}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {52,30}},{SFG_LEVEL_ELEMENT_PLASMA, {45,30}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,30}},{SFG_LEVEL_ELEMENT_TELEPORTER, {2,30}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,31}},{SFG_LEVEL_ELEMENT_RUIN, {19,32}},
      {SFG_LEVEL_ELEMENT_RUIN, {20,33}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {22,34}},
      {SFG_LEVEL_ELEMENT_BULLETS, {62,35}},{SFG_LEVEL_ELEMENT_ROCKETS, {61,35}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {42,35}},{SFG_LEVEL_ELEMENT_BULLETS, {23,35}},
      {SFG_LEVEL_ELEMENT_TREE, {21,35}},{SFG_LEVEL_ELEMENT_HEALTH, {62,36}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {59,36}},{SFG_LEVEL_ELEMENT_TERMINAL, {50,36}},
      {SFG_LEVEL_ELEMENT_RUIN, {14,36}},{SFG_LEVEL_ELEMENT_ROCKETS, {23,37}},
      {SFG_LEVEL_ELEMENT_TREE, {21,37}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {14,37}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {12,38}},{SFG_LEVEL_ELEMENT_ROCKETS, {11,38}},
      {SFG_LEVEL_ELEMENT_PLASMA, {10,38}},{SFG_LEVEL_ELEMENT_LAMP, {24,40}},
      {SFG_LEVEL_ELEMENT_LAMP, {22,40}},{SFG_LEVEL_ELEMENT_BARREL, {45,41}},
      {SFG_LEVEL_ELEMENT_BARREL, {15,41}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {7,41}},
      {SFG_LEVEL_ELEMENT_BARREL, {46,42}},{SFG_LEVEL_ELEMENT_BARREL, {44,42}},
      {SFG_LEVEL_ELEMENT_BARREL, {43,42}},{SFG_LEVEL_ELEMENT_RUIN, {36,42}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {33,42}},{SFG_LEVEL_ELEMENT_BARREL, {54,43}},
      {SFG_LEVEL_ELEMENT_RUIN, {36,43}},{SFG_LEVEL_ELEMENT_BULLETS, {19,43}},
      {SFG_LEVEL_ELEMENT_TREE, {26,44}},{SFG_LEVEL_ELEMENT_TREE, {21,44}},
      {SFG_LEVEL_ELEMENT_BULLETS, {46,45}},{SFG_LEVEL_ELEMENT_BULLETS, {45,45}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {35,45}},{SFG_LEVEL_ELEMENT_ROCKETS, {19,45}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {58,46}},{SFG_LEVEL_ELEMENT_BARREL, {54,46}},
      {SFG_LEVEL_ELEMENT_PLASMA, {41,46}},{SFG_LEVEL_ELEMENT_HEALTH, {37,46}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {14,46}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {51,47}},
      {SFG_LEVEL_ELEMENT_BULLETS, {19,47}},{SFG_LEVEL_ELEMENT_RUIN, {14,47}},
      {SFG_LEVEL_ELEMENT_BULLETS, {59,48}},{SFG_LEVEL_ELEMENT_LAMP, {40,48}},
      {SFG_LEVEL_ELEMENT_LAMP, {38,48}},{SFG_LEVEL_ELEMENT_PLASMA, {59,49}},
      {SFG_LEVEL_ELEMENT_BARREL, {53,49}},{SFG_LEVEL_ELEMENT_FINISH, {39,49}},
      {SFG_LEVEL_ELEMENT_TREE, {17,49}},{SFG_LEVEL_ELEMENT_LOCK2, {34,50}},
      {SFG_LEVEL_ELEMENT_BARREL, {54,51}},{SFG_LEVEL_ELEMENT_COLUMN, {36,52}},
      {SFG_LEVEL_ELEMENT_COLUMN, {32,52}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {6,52}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {51,54}},{SFG_LEVEL_ELEMENT_TREE, {46,54}},
      {SFG_LEVEL_ELEMENT_COLUMN, {42,54}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {22,54}},
      {SFG_LEVEL_ELEMENT_BULLETS, {19,54}},{SFG_LEVEL_ELEMENT_TREE, {62,55}},
      {SFG_LEVEL_ELEMENT_LOCK1, {38,55}},{SFG_LEVEL_ELEMENT_TREE, {46,56}},
      {SFG_LEVEL_ELEMENT_COLUMN, {42,56}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {56,59}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {51,59}},{SFG_LEVEL_ELEMENT_HEALTH, {14,60}},
      {SFG_LEVEL_ELEMENT_TERMINAL, {10,60}},{SFG_LEVEL_ELEMENT_HEALTH, {61,61}},
      {SFG_LEVEL_ELEMENT_HEALTH, {32,61}},{SFG_LEVEL_ELEMENT_ROCKETS, {17,61}},
      {SFG_LEVEL_ELEMENT_CARD2, {57,62}},{SFG_LEVEL_ELEMENT_HEALTH, {38,62}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {34,62}},{SFG_LEVEL_ELEMENT_BULLETS, {32,62}},
      {SFG_LEVEL_ELEMENT_LAMP, {25,62}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level7 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (31 | SFG_TILE_PROPERTY_DOOR)
    #define BB (34 | SFG_TILE_PROPERTY_ELEVATOR)
    #define CC (29 | SFG_TILE_PROPERTY_SQUEEZER)
    #define DD (30 | SFG_TILE_PROPERTY_DOOR)
    #define EE (39 | SFG_TILE_PROPERTY_ELEVATOR)
    #define FF (28 | SFG_TILE_PROPERTY_DOOR)
    #define GG (44 | SFG_TILE_PROPERTY_ELEVATOR)
    #define HH (8 | SFG_TILE_PROPERTY_DOOR)
    #define II (1 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,32,32,1 ,11,11,11,11,11,11,11,11,11,11,11,27,25,25,25,1 ,31,31,8 ,31,8 ,31,31, /*0 */ 
/*1 */ 12,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,1 ,23,23,1 ,22,22,22,22,22,22,22,22,22,22,22,27,24,o ,24,1 ,31,31,41,41,41,31,31, /*1 */ 
/*2 */ 12,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,8 ,1 ,23,23,27,22,22,15,15,15,15,15,15,15,15,21,27,o ,o ,o ,1 ,8 ,41,41,41,41,41,8 , /*2 */ 
/*3 */ 12,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,8 ,1 ,23,23,27,23,22,15,20,20,20,20,20,20,20,20,27,o ,o ,o ,1 ,31,41,41,41,41,41,31, /*3 */ 
/*4 */ 12,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,8 ,1 ,23,23,35,23,22,15,20,22,22,22,22,22,22,19,27,o ,o ,o ,1 ,8 ,23,23,23,23,23,8 , /*4 */ 
/*5 */ 12,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,4 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,4 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,8 ,1 ,23,23,27,23,22,15,20,22,18,18,18,18,18,18,28,o ,o ,o ,1 ,31,23,23,23,23,23,31, /*5 */ 
/*6 */ 12,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,3 ,4 ,3 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,3 ,4 ,3 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,8 ,1 ,23,23,27,22,22,15,20,22,18,20,20,20,20,17,28,o ,o ,o ,1 ,8 ,23,23,23,23,23,8 , /*6 */ 
/*7 */ 12,o ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,3 ,4 ,3 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,2 ,3 ,4 ,3 ,2 ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,1 ,23,23,1 ,22,22,21,20,19,18,17,o ,o ,o ,o ,16,o ,o ,o ,1 ,31,31,23,23,23,31,31, /*7 */ 
/*8 */ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,2 ,3 ,4 ,3 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,2 ,3 ,4 ,3 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,23,1 ,22,22,15,20,22,18,20,o ,o ,24,o ,28,o ,o ,o ,1 ,27,27,27,AA,27,27,27, /*8 */ 
/*9 */ 1 ,11,17,17,17,17,17,17,1 ,o ,2 ,3 ,4 ,3 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,17,23,23,23,23,23,23,1 ,26,1 ,26,1 ,1 ,1 ,1 ,1 ,23,23,1 ,22,22,15,20,22,18,20,37,o ,24,o ,28,o ,o ,o ,1 ,1 ,23,23,23,23,23,1 , /*9 */ 
/*10*/ 1 ,25,17,CC,17,17,CC,17,1 ,o ,2 ,2 ,2 ,2 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,18,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,BB,BB,1 ,1 ,23,23,1 ,22,22,15,20,22,18,20,37,o ,24,o ,28,o ,o ,o ,42,32,23,23,23,23,23,33, /*10*/ 
/*11*/ 1 ,25,17,17,17,17,17,17,1 ,23,23,23,o ,23,23,23,1 ,1 ,1 ,1 ,1 ,1 ,19,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,BB,BB,23,DD,23,23,1 ,22,22,15,20,22,18,20,o ,o ,24,o ,28,o ,o ,o ,o ,32,23,23,23,23,23,32, /*11*/ 
/*12*/ 1 ,25,17,CC,17,17,CC,17,17,o ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,20,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,BB,BB,1 ,1 ,23,23,1 ,22,22,21,20,19,18,17,o ,o ,o ,o ,16,o ,o ,o ,24,32,23,23,23,23,23,33, /*12*/ 
/*13*/ 1 ,25,17,17,17,17,17,17,17,o ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,21,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,1 ,1 ,1 ,1 ,23,23,27,22,22,15,20,22,18,20,20,20,20,17,28,o ,o ,o ,o ,32,23,23,23,23,23,1 , /*13*/ 
/*14*/ 1 ,25,17,17,17,17,17,17,17,o ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,22,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,1 ,1 ,1 ,1 ,23,23,27,23,22,15,20,22,18,18,18,18,18,18,28,o ,o ,o ,o ,32,23,23,1 ,23,23,32, /*14*/ 
/*15*/ 1 ,25,17,17,17,17,17,17,17,o ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,1 ,23,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,1 ,1 ,1 ,1 ,23,23,35,23,22,15,20,22,22,22,22,22,22,19,27,o ,o ,o ,24,32,41,41,41,41,41,32, /*15*/ 
/*16*/ 1 ,25,17,CC,17,17,CC,17,1 ,10,10,10,10,10,10,10,10,10,10,10,10,1 ,1 ,1 ,26,1 ,26,20,20,20,26,1 ,26,1 ,1 ,1 ,1 ,1 ,23,23,27,23,22,15,20,20,20,20,20,20,20,20,27,o ,o ,o ,o ,32,41,41,41,41,41,1 , /*16*/ 
/*17*/ 1 ,25,17,17,17,17,17,17,1 ,17,18,19,20,22,23,23,22,20,19,18,17,9 ,9 ,1 ,17,37,o ,o ,o ,o ,o ,o ,37,1 ,1 ,1 ,23,23,23,23,27,22,22,15,15,15,15,15,15,15,15,21,27,o ,o ,o ,o ,32,41,41,41,41,41,33, /*17*/ 
/*18*/ 1 ,25,17,CC,17,17,CC,17,1 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,17,37,o ,o ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,1 ,22,22,22,22,22,22,22,22,22,22,22,27,o ,o ,o ,24,32,41,41,41,41,41,32, /*18*/ 
/*19*/ 1 ,11,17,17,17,17,17,17,1 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,1 ,37,o ,o ,o ,o ,o ,o ,37,1 ,1 ,1 ,23,23,23,23,1 ,1 ,1 ,27,27,27,27,27,27,27,27,27,27,o ,o ,o ,o ,32,41,41,41,41,41,33, /*19*/ 
/*20*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,37,o ,o ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,22,21,20,19,18,17,o ,o ,o ,o ,32,41,41,41,41,41,1 , /*20*/ 
/*21*/ 5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,37,o ,o ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,22,21,20,19,18,17,o ,o ,o ,1 ,1 ,40,1 ,DD,DD,1 ,1 , /*21*/ 
/*22*/ 5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,1 ,37,37,37,37,37,37,37,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,22,21,20,19,18,17,o ,o ,o ,1 ,1 ,40,1 ,23,23,1 ,1 , /*22*/ 
/*23*/ 5 ,5 ,5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,8 ,8 ,8 ,8 ,9 ,9 ,9 ,9 ,9 ,1 ,FF,1 ,1 ,o ,o ,o ,o ,37,1 ,1 ,1 ,23,23,23,23,1 ,23,23,23,23,38,38,38,38,38,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,40,EE,23,23,EE,1 , /*23*/ 
/*24*/ 5 ,6 ,6 ,6 ,10,6 ,6 ,10,6 ,6 ,10,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,CC,23,23,CC,1 , /*24*/ 
/*25*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,37,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,1 ,o ,24,o ,o ,o ,24,o ,1 ,EE,23,23,EE,1 , /*25*/ 
/*26*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,1 ,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,CC,23,23,CC,1 , /*26*/ 
/*27*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,37,37,37,37,37,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,28,28,28,28,28,1 ,o ,o ,o ,o ,o ,o ,o ,1 ,EE,23,23,EE,1 , /*27*/ 
/*28*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,37,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,1 ,1 ,1 ,o ,37,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,17,18,18,18,17,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,23,23,1 ,1 , /*28*/ 
/*29*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,6 ,6 ,6 ,1 ,o ,37,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,18,18,18,18,18,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,DD,DD,1 ,1 , /*29*/ 
/*30*/ 7 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,6 ,6 ,6 ,FF,o ,37,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,27,23,23,27,27, /*30*/ 
/*31*/ 5 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,5 ,8 ,10,6 ,37,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,6 ,6 ,6 ,1 ,o ,37,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,27,23,23,27,27, /*31*/ 
/*32*/ 5 ,5 ,6 ,6 ,5 ,5 ,6 ,6 ,6 ,5 ,5 ,5 ,5 ,5 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,1 ,1 ,1 ,o ,37,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,31,23,23,31,27, /*32*/ 
/*33*/ 1 ,o ,o ,o ,o ,5 ,6 ,6 ,6 ,5 ,8 ,8 ,8 ,8 ,8 ,10,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,42,o ,o ,o ,37,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,27,31,23,23,31,27, /*33*/ 
/*34*/ 1 ,o ,o ,o ,o ,5 ,6 ,6 ,6 ,5 ,8 ,10,10,10,10,10,6 ,37,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,27,27,27,23,23,27,27, /*34*/ 
/*35*/ 1 ,FF,1 ,1 ,1 ,5 ,6 ,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,28,24,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,27,27,27,27,23,23,27,27, /*35*/ 
/*36*/ GG,45,GG,GG,5 ,5 ,6 ,6 ,6 ,5 ,8 ,10,6 ,6 ,6 ,6 ,6 ,6 ,6 ,9 ,9 ,9 ,9 ,9 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,37,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,23,23,23,23,23,23,23,23,1 , /*36*/ 
/*37*/ GG,45,45,GG,5 ,6 ,6 ,6 ,6 ,5 ,8 ,8 ,1 ,1 ,8 ,HH,8 ,1 ,1 ,1 ,1 ,DD,1 ,1 ,1 ,FF,1 ,1 ,o ,o ,o ,o ,28,1 ,28,24,o ,o ,o ,o ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,32,23,23,23,23,23,23,23,23,32, /*37*/ 
/*38*/ GG,45,45,GG,5 ,6 ,6 ,6 ,5 ,10,o ,o ,o ,o ,o ,37,o ,19,19,19,23,36,36,o ,o ,o ,o ,o ,o ,o ,o ,o ,28,1 ,1 ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,32,23,23,27,27,27,27,23,23,32, /*38*/ 
/*39*/ GG,45,GG,5 ,6 ,6 ,6 ,6 ,5 ,10,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,36,36,o ,o ,o ,o ,o ,o ,o ,o ,28,28,1 ,1 ,o ,o ,o ,o ,o ,1 ,1 ,1 ,28,24,o ,o ,o ,o ,o ,o ,o ,o ,o ,32,23,23,31,27,27,31,23,23,32, /*39*/ 
/*40*/ GG,GG,5 ,5 ,6 ,6 ,6 ,5 ,10,10,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,36,36,o ,o ,o ,o ,o ,o ,o ,28,28,28,1 ,1 ,1 ,1 ,16,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,32,23,23,27,27,27,27,23,23,32, /*40*/ 
/*41*/ 5 ,5 ,6 ,6 ,6 ,6 ,5 ,10,10,10,o ,o ,o ,o ,o ,37,o ,19,19,19,23,36,36,o ,o ,o ,27,43,43,43,28,27,27,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,24,28,24,o ,o ,o ,o ,o ,o ,o ,o ,o ,32,23,23,23,23,23,23,23,23,32, /*41*/ 
/*42*/ 5 ,5 ,6 ,6 ,6 ,5 ,10,10,10,10,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,27,o ,o ,o ,37,10,27,1 ,1 ,42,o ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,23,23,23,23,23,23,23,23,1 , /*42*/ 
/*43*/ 5 ,6 ,6 ,6 ,6 ,5 ,10,10,10,10,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,36,36,o ,o ,o ,28,o ,o ,o ,o ,17,28,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,24,28,24,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,DD,1 ,1 ,1 ,1 , /*43*/ 
/*44*/ 5 ,6 ,6 ,6 ,5 ,10,10,10,10,10,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,36,36,36,o ,o ,o ,FF,o ,o ,o ,o ,17,28,1 ,1 ,42,o ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,23,23,23,23,23,23,23,33, /*44*/ 
/*45*/ 5 ,6 ,6 ,6 ,5 ,10,10,10,10,10,o ,o ,37,37,37,37,o ,o ,o ,o ,36,36,36,o ,o ,o ,28,o ,o ,o ,o ,17,28,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,1 ,1 ,16,16,1 ,1 ,o ,o ,1 ,1 ,23,23,23,23,23,23,23,32, /*45*/ 
/*46*/ 5 ,6 ,6 ,6 ,5 ,10,10,10,10,8 ,o ,o ,37,o ,o ,1 ,1 ,1 ,1 ,1 ,36,36,36,o ,o ,o ,27,o ,o ,o ,37,10,27,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,37,37,1 ,24,o ,o ,24,1 ,37,37,1 ,1 ,23,23,38,38,38,23,23,33, /*46*/ 
/*47*/ 1 ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,37,o ,o ,1 ,23,23,23,23,36,36,36,18,o ,18,27,27,28,28,28,27,27,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,24,o ,o ,24,1 ,1 ,1 ,1 ,1 ,23,23,38,38,38,23,23,1 , /*47*/ 
/*48*/ 1 ,o ,o ,o ,o ,o ,o ,37,o ,o ,o ,o ,37,1 ,1 ,1 ,23,23,23,23,36,36,36,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,24,15,15,15,15,15,15,15,15,15,15,15,15,33, /*48*/ 
/*49*/ 1 ,o ,o ,o ,o ,o ,o ,37,37,37,37,37,37,1 ,1 ,1 ,23,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,15,15,15,15,15,15,15,15,15,15,15,15,32, /*49*/ 
/*50*/ 1 ,1 ,o ,o ,o ,o ,o ,o ,o ,37,1 ,1 ,1 ,1 ,42,1 ,23,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,15,15,15,15,15,15,15,15,15,15,15,15,15,33, /*50*/ 
/*51*/ 1 ,1 ,o ,o ,o ,o ,o ,o ,o ,37,FF,o ,o ,o ,o ,o ,23,23,1 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,1 ,6 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,15,15,15,15,15,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , /*51*/ 
/*52*/ 1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,37,1 ,o ,o ,o ,o ,o ,23,23,1 ,8 ,5 ,10,5 ,10,5 ,10,5 ,10,5 ,8 ,1 ,6 ,o ,o ,o ,o ,17,18,19,20,21,22,23,15,15,15,15,15,15,15,15,15,1 ,12,12,12,12,12,12,12,12,12,12,12, /*52*/ 
/*53*/ 1 ,1 ,1 ,1 ,1 ,II,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,23,23,1 ,8 ,10,o ,o ,o ,o ,o ,o ,o ,10,8 ,1 ,6 ,o ,o ,o ,o ,17,18,19,20,21,22,23,15,15,15,15,15,15,15,15,1 ,12,12,12,12,13,13,13,13,12,12,12,12, /*53*/ 
/*54*/ 1 ,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,1 ,17,18,19,20,22,23,23,1 ,8 ,5 ,o ,o ,o ,o ,o ,o ,o ,5 ,8 ,1 ,6 ,o ,o ,o ,o ,17,18,19,20,21,22,23,15,15,15,15,15,15,15,15,1 ,12,12,13,13,13,13,13,13,13,13,12,12, /*54*/ 
/*55*/ 1 ,1 ,o ,o ,2 ,2 ,2 ,o ,o ,1 ,1 ,17,18,19,20,22,23,23,1 ,8 ,10,o ,o ,o ,o ,o ,o ,o ,10,8 ,1 ,6 ,o ,o ,o ,o ,17,18,19,20,21,22,23,15,15,15,15,15,15,15,15,1 ,12,12,13,13,14,13,13,14,13,13,12,12, /*55*/ 
/*56*/ 1 ,1 ,o ,2 ,3 ,3 ,3 ,2 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,5 ,o ,o ,o ,o ,o ,o ,o ,5 ,8 ,1 ,1 ,o ,o ,o ,o ,o ,o ,27,15,15,15,15,15,15,15,15,15,15,1 ,1 ,1 ,12,13,13,13,14,13,13,14,13,13,13,12, /*56*/ 
/*57*/ 1 ,o ,2 ,3 ,4 ,4 ,4 ,3 ,2 ,o ,1 ,o ,2 ,3 ,4 ,4 ,3 ,2 ,o ,1 ,1 ,10,10,10,10,10,10,10,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,15,23,23,23,23,23,23,23,23,23,23,23,23,1 ,12,13,13,13,14,13,13,14,13,13,13,12, /*57*/ 
/*58*/ 1 ,o ,2 ,3 ,4 ,4 ,4 ,3 ,2 ,o ,II,o ,o ,o ,o ,o ,o ,o ,o ,II,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,24,15,23,23,23,23,23,23,23,23,23,23,22,22,1 ,12,13,13,13,13,13,13,13,13,13,13,12, /*58*/ 
/*59*/ 1 ,o ,2 ,3 ,4 ,4 ,4 ,3 ,2 ,o ,1 ,o ,2 ,3 ,4 ,4 ,3 ,2 ,o ,1 ,1 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,1 ,o ,o ,o ,o ,o ,15,23,38,23,23,38,23,23,23,23,23,22,38,1 ,12,13,13,14,13,13,13,13,14,13,13,12, /*59*/ 
/*60*/ 1 ,1 ,o ,2 ,3 ,3 ,3 ,2 ,o ,1 ,1 ,11,11,11,11,11,11,11,11,1 ,1 ,1 ,1 ,1 ,FF,1 ,6 ,6 ,6 ,6 ,6 ,6 ,FF,o ,o ,o ,o ,24,15,23,23,23,23,23,23,23,23,23,23,22,22,1 ,12,12,13,13,14,14,14,14,13,13,12,12, /*60*/ 
/*61*/ 1 ,1 ,o ,o ,2 ,2 ,2 ,o ,o ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,o ,o ,o ,1 ,o ,o ,o ,o ,o ,o ,1 ,o ,o ,o ,o ,o ,15,23,38,23,23,38,23,23,23,23,23,22,38,1 ,12,12,13,13,13,13,13,13,13,13,12,12, /*61*/ 
/*62*/ 1 ,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,o ,o ,o ,1 ,o ,42,o ,42,o ,o ,1 ,o ,o ,o ,o ,o ,27,23,23,23,23,23,23,23,23,23,23,22,22,1 ,12,12,12,12,13,13,13,13,12,12,12,12, /*62*/ 
/*63*/ 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,25,25,25,25,25,1 ,1 ,1 ,33,32,33,1 ,1 ,33,32,33,1 ,1 ,1 ,12,12,12,12,12,12,12,12,12,12,12,12  /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef GG
    #undef HH
    #undef II
    #undef o
    },
    {        // tileDictionary
      SFG_TD(15,14,0,0),SFG_TD(19, 0,0,0),SFG_TD(13,18,0,0),SFG_TD(11,22,0,0), // 0 
      SFG_TD( 9,26,0,0),SFG_TD(19, 0,1,2),SFG_TD(15, 9,0,0),SFG_TD(19, 0,1,3), // 4 
      SFG_TD(19, 0,4,4),SFG_TD(22, 7,4,4),SFG_TD(16, 7,4,4),SFG_TD(31, 0,3,3), // 8 
      SFG_TD(26, 0,3,7),SFG_TD(15,31,0,0),SFG_TD(18,31,0,0),SFG_TD(23, 6,5,5), // 12 
      SFG_TD(15, 5,0,0),SFG_TD(16,13,5,5),SFG_TD(17,12,5,5),SFG_TD(18,11,5,5), // 16 
      SFG_TD(19,10,5,5),SFG_TD(20, 9,5,5),SFG_TD(21, 8,5,5),SFG_TD(22, 7,5,0), // 20 
      SFG_TD(17,12,6,5),SFG_TD(23, 0,0,3),SFG_TD(19, 0,2,2),SFG_TD(31, 0,6,6), // 24 
      SFG_TD(19, 0,6,0),SFG_TD(17, 6,4,4),SFG_TD(26, 0,5,0),SFG_TD(26, 0,1,1), // 28 
      SFG_TD(25, 3,0,0),SFG_TD(26, 1,0,0),SFG_TD(22,31,6,6),SFG_TD(22, 5,5,6), // 32 
      SFG_TD(22, 7,0,0),SFG_TD(15,13,0,4),SFG_TD(24, 5,6,5),SFG_TD(20,14,4,4), // 36 
      SFG_TD(31, 8,0,0),SFG_TD(22,13,5,0),SFG_TD(21, 8,2,5),SFG_TD(15, 6,0,0), // 40 
      SFG_TD(31,30,4,4),SFG_TD(15, 4,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0), // 44 
      SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0), // 48 
      SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0), // 52 
      SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0), // 56 
      SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0),SFG_TD(15,13,0,0)  // 60 
      },                    // tileDictionary
    {12,8 ,2 ,10,11,14,4 }, // textureIndices
    13,                     // doorTextureIndex
    4,                      // floorColor
    66,                     // ceilingColor
    {34, 11, 0  },          // player start: x, y, direction
    1,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_FINISH, {60,1}},{SFG_LEVEL_ELEMENT_BULLETS, {54,1}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {20,3}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {24,4}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {18,5}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {8,5}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {44,6}},{SFG_LEVEL_ELEMENT_COLUMN, {61,9}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {60,9}},{SFG_LEVEL_ELEMENT_COLUMN, {59,9}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {51,9}},{SFG_LEVEL_ELEMENT_CARD2, {50,9}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {46,9}},{SFG_LEVEL_ELEMENT_BULLETS, {41,9}},
      {SFG_LEVEL_ELEMENT_PLASMA, {41,10}},{SFG_LEVEL_ELEMENT_BLOCKER, {28,10}},
      {SFG_LEVEL_ELEMENT_HEALTH, {50,11}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {38,11}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {31,11}},{SFG_LEVEL_ELEMENT_BLOCKER, {30,11}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {26,11}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,12}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {28,12}},{SFG_LEVEL_ELEMENT_BLOCKER, {26,12}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {31,13}},{SFG_LEVEL_ELEMENT_BLOCKER, {27,13}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {23,13}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {48,14}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {44,14}},{SFG_LEVEL_ELEMENT_BLOCKER, {31,14}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {29,14}},{SFG_LEVEL_ELEMENT_BLOCKER, {26,14}},
      {SFG_LEVEL_ELEMENT_BARREL, {20,14}},{SFG_LEVEL_ELEMENT_TERMINAL, {3,14}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {31,15}},{SFG_LEVEL_ELEMENT_BLOCKER, {29,15}},
      {SFG_LEVEL_ELEMENT_CARD0, {28,15}},{SFG_LEVEL_ELEMENT_BLOCKER, {27,15}},
      {SFG_LEVEL_ELEMENT_BLOCKER, {26,15}},{SFG_LEVEL_ELEMENT_BULLETS, {4,19}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {35,20}},{SFG_LEVEL_ELEMENT_PLASMA, {35,21}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {35,22}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {62,23}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {10,24}},{SFG_LEVEL_ELEMENT_PLASMA, {7,24}},
      {SFG_LEVEL_ELEMENT_BULLETS, {4,24}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {62,25}},
      {SFG_LEVEL_ELEMENT_TREE, {56,25}},{SFG_LEVEL_ELEMENT_TREE, {52,25}},
      {SFG_LEVEL_ELEMENT_HEALTH, {49,25}},{SFG_LEVEL_ELEMENT_CARD1, {17,25}},
      {SFG_LEVEL_ELEMENT_HEALTH, {15,25}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {59,27}},
      {SFG_LEVEL_ELEMENT_BARREL, {28,27}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {14,27}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {10,27}},{SFG_LEVEL_ELEMENT_BULLETS, {43,28}},
      {SFG_LEVEL_ELEMENT_BULLETS, {42,28}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {5,28}},
      {SFG_LEVEL_ELEMENT_RUIN, {27,29}},{SFG_LEVEL_ELEMENT_RUIN, {26,30}},
      {SFG_LEVEL_ELEMENT_HEALTH, {24,30}},{SFG_LEVEL_ELEMENT_RUIN, {27,31}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {10,31}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {4,31}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {17,34}},{SFG_LEVEL_ELEMENT_HEALTH, {4,34}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {49,35}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {51,36}},
      {SFG_LEVEL_ELEMENT_LOCK1, {15,37}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {5,38}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {4,39}},{SFG_LEVEL_ELEMENT_BULLETS, {1,39}},
      {SFG_LEVEL_ELEMENT_COLUMN, {29,41}},{SFG_LEVEL_ELEMENT_COLUMN, {28,41}},
      {SFG_LEVEL_ELEMENT_COLUMN, {27,41}},{SFG_LEVEL_ELEMENT_ROCKETS, {8,41}},
      {SFG_LEVEL_ELEMENT_PLASMA, {31,42}},{SFG_LEVEL_ELEMENT_ROCKETS, {7,42}},
      {SFG_LEVEL_ELEMENT_LOCK0, {59,43}},{SFG_LEVEL_ELEMENT_HEALTH, {35,43}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {30,44}},{SFG_LEVEL_ELEMENT_TREE, {59,46}},
      {SFG_LEVEL_ELEMENT_PLASMA, {31,46}},{SFG_LEVEL_ELEMENT_BULLETS, {59,47}},
      {SFG_LEVEL_ELEMENT_TREE, {25,47}},{SFG_LEVEL_ELEMENT_TREE, {23,47}},
      {SFG_LEVEL_ELEMENT_BULLETS, {46,51}},{SFG_LEVEL_ELEMENT_BULLETS, {15,51}},
      {SFG_LEVEL_ELEMENT_LOCK1, {10,51}},{SFG_LEVEL_ELEMENT_TELEPORTER, {27,52}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {25,52}},{SFG_LEVEL_ELEMENT_TELEPORTER, {23,52}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {21,52}},{SFG_LEVEL_ELEMENT_ROCKETS, {15,52}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {28,53}},{SFG_LEVEL_ELEMENT_TELEPORTER, {20,53}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {47,54}},{SFG_LEVEL_ELEMENT_TERMINAL, {24,54}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {28,55}},{SFG_LEVEL_ELEMENT_TELEPORTER, {20,55}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {62,57}},{SFG_LEVEL_ELEMENT_PLASMA, {59,57}},
      {SFG_LEVEL_ELEMENT_PLASMA, {56,57}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {45,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {14,58}},{SFG_LEVEL_ELEMENT_BARREL, {12,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {5,58}},{SFG_LEVEL_ELEMENT_HEALTH, {1,58}},
      {SFG_LEVEL_ELEMENT_TREE, {50,59}},{SFG_LEVEL_ELEMENT_LOCK2, {32,60}},
      {SFG_LEVEL_ELEMENT_TREE, {50,61}},{SFG_LEVEL_ELEMENT_ROCKETS, {22,61}},
      {SFG_LEVEL_ELEMENT_PLASMA, {22,62}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level8 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (10 | SFG_TILE_PROPERTY_DOOR)
    #define BB (63 | SFG_TILE_PROPERTY_ELEVATOR)
    #define CC (11 | SFG_TILE_PROPERTY_DOOR)
    #define DD (13 | SFG_TILE_PROPERTY_DOOR)
    #define EE (7 | SFG_TILE_PROPERTY_DOOR)
    #define FF (60 | SFG_TILE_PROPERTY_DOOR)
    #define GG (12 | SFG_TILE_PROPERTY_ELEVATOR)
    #define HH (61 | SFG_TILE_PROPERTY_DOOR)
    #define II (0 | SFG_TILE_PROPERTY_DOOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 44,44,44,58,58,58,58,58,58,58,58,58,58,10,61,6 ,6 ,6 ,6 ,61,61,6 ,6 ,6 ,6 ,61,61,6 ,6 ,6 ,6 ,61,1 ,6 ,6 ,6 ,6 ,6 ,61,6 ,6 ,6 ,6 ,61,6 ,6 ,6 ,6 ,6 ,1 ,1 ,1 ,1 ,15,1 ,1 ,1 ,1 ,1 ,15,1 ,1 ,1 ,1 , /*0 */ 
/*1 */ 44,44,58,58,59,59,59,59,59,59,59,59,59,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,1 , /*1 */ 
/*2 */ 44,58,58,59,59,59,59,59,59,59,59,59,59,AA,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 , /*2 */ 
/*3 */ 58,58,59,59,59,59,59,59,59,59,59,59,59,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,11,11,11,15,4 ,4 ,4 ,5 ,5 ,4 ,4 ,4 ,5 ,5 ,4 ,4 ,4 ,15, /*3 */ 
/*4 */ 58,59,59,59,59,59,59,59,59,10,10,10,10,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,1 , /*4 */ 
/*5 */ 58,59,59,59,59,59,59,59,10,61,61,61,61,61,4 ,4 ,3 ,3 ,4 ,4 ,3 ,3 ,4 ,4 ,13,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,14,14,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,11,o ,o ,2 ,4 ,4 ,4 ,4 ,5 ,BB,BB,BB,5 ,4 ,4 ,4 ,4 ,1 , /*5 */ 
/*6 */ 58,59,59,59,59,59,59,57,57,54,53,52,51,50,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,13,13,13,13,13,13,13,15,15,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,CC,o ,o ,2 ,4 ,4 ,4 ,4 ,5 ,BB,BB,BB,5 ,4 ,4 ,4 ,4 ,1 , /*6 */ 
/*7 */ 58,59,59,59,59,59,57,57,55,54,53,52,51,50,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,13,13,13,o ,13,13,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,11,o ,o ,2 ,4 ,4 ,4 ,4 ,5 ,BB,BB,BB,5 ,4 ,4 ,4 ,4 ,1 , /*7 */ 
/*8 */ 58,59,59,59,59,10,57,55,55,54,53,52,51,50,4 ,4 ,4 ,4 ,3 ,3 ,4 ,4 ,4 ,4 ,DD,o ,o ,o ,o ,o ,o ,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,4 ,4 ,4 ,1 , /*8 */ 
/*9 */ 58,59,59,59,10,61,55,55,55,54,53,52,51,50,4 ,4 ,4 ,4 ,3 ,3 ,4 ,4 ,4 ,4 ,1 ,13,13,13,o ,15,o ,o ,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,11,11,11,15,4 ,4 ,4 ,5 ,5 ,4 ,4 ,4 ,5 ,5 ,4 ,4 ,4 ,15, /*9 */ 
/*10*/ 58,59,59,59,10,61,55,55,55,54,53,52,51,50,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,13,o ,o ,o ,o ,o ,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 , /*10*/ 
/*11*/ 58,59,59,59,10,61,55,55,55,54,53,52,51,50,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,13,o ,o ,15,o ,13,13,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,1 , /*11*/ 
/*12*/ 58,59,59,59,10,61,55,55,55,54,53,52,51,50,4 ,4 ,3 ,3 ,4 ,4 ,3 ,3 ,4 ,4 ,1 ,13,o ,o ,o ,o ,o ,o ,DD,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,15,1 ,2 ,2 ,2 ,1 ,15,1 ,1 ,1 ,1 , /*12*/ 
/*13*/ 10,10,AA,10,61,10,10,10,10,10,10,10,8 ,8 ,10,10,8 ,8 ,10,10,8 ,8 ,10,10,1 ,13,13,13,o ,13,13,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,62,62,1 , /*13*/ 
/*14*/ 61,o ,o ,o ,o ,10,10,10,10,10,10,o ,o ,o ,o ,o ,o ,o ,23,23,15,15,15,15,13,13,13,13,13,13,13,13,13,23,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,1 , /*14*/ 
/*15*/ 56,o ,o ,o ,o ,10,10,10,10,10,8 ,o ,o ,o ,o ,o ,o ,o ,23,23,15,15,15,15,15,15,15,15,15,15,13,13,13,23,o ,o ,o ,o ,o ,o ,61,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,61, /*15*/ 
/*16*/ 56,o ,o ,o ,o ,49,10,10,10,10,8 ,o ,o ,62,62,15,15,EE,15,15,15,15,15,o ,o ,o ,o ,o ,15,15,15,15,15,15,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,11,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*16*/ 
/*17*/ 56,o ,o ,o ,o ,o ,10,10,10,10,10,o ,o ,62,15,15,o ,o ,o ,15,15,15,15,o ,o ,o ,o ,o ,15,15,o ,o ,61,o ,o ,o ,o ,o ,o ,o ,8 ,62,o ,o ,o ,o ,o ,11,62,8 ,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*17*/ 
/*18*/ 56,o ,o ,o ,o ,o ,o ,49,10,10,10,49,49,15,15,o ,o ,o ,o ,o ,15,15,15,15,15,15,o ,o ,15,15,o ,o ,EE,o ,40,40,40,40,40,o ,8 ,62,o ,o ,o ,o ,o ,o ,62,8 ,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*18*/ 
/*19*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,o ,o ,15,o ,o ,o ,o ,o ,o ,o ,15,15,15,15,1 ,o ,o ,1 ,15,o ,o ,61,o ,40,40,40,40,40,o ,8 ,62,o ,o ,o ,o ,o ,o ,62,8 ,62,62,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61, /*19*/ 
/*20*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,62,8 ,o ,o ,15,o ,o ,15,o ,15,o ,o ,15,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,o ,40,40,40,40,40,o ,8 ,8 ,8 ,49,49,49,49,8 ,8 ,8 ,8 ,8 ,8 ,49,10,40,40,40,40,40,40,10,49,10, /*20*/ 
/*21*/ 56,o ,o ,o ,o ,o ,o ,o ,62,62,10,o ,o ,15,o ,o ,o ,o ,o ,o ,o ,15,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,o ,40,40,40,40,40,o ,o ,o ,3 ,2 ,2 ,2 ,2 ,3 ,1 ,1 ,1 ,1 ,1 ,o ,40,40,40,40,40,40,40,40,o ,10, /*21*/ 
/*22*/ 61,o ,o ,o ,o ,o ,o ,o ,62,62,10,o ,o ,15,15,o ,o ,o ,o ,o ,15,61,o ,o ,15,1 ,o ,o ,1 ,15,15,15,15,o ,o ,o ,o ,o ,o ,o ,o ,o ,3 ,2 ,2 ,2 ,2 ,3 ,1 ,1 ,1 ,1 ,1 ,o ,40,40,40,40,40,40,40,40,o ,10, /*22*/ 
/*23*/ 61,o ,o ,o ,o ,o ,o ,o ,o ,62,8 ,o ,o ,15,15,15,o ,o ,o ,15,o ,EE,o ,o ,15,15,o ,o ,15,15,15,15,8 ,62,62,10,10,1 ,1 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,40,40,40,40,40,40,40,40,o ,10, /*23*/ 
/*24*/ 61,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,49,49,15,15,15,o ,o ,o ,15,o ,61,o ,o ,15,15,o ,o ,o ,o ,o ,15,8 ,o ,o ,10,10,1 ,62,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,o ,40,40,40,40,40,40,40,40,o ,10, /*24*/ 
/*25*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,15,15,61,EE,61,15,o ,15,15,15,15,15,o ,o ,o ,o ,o ,15,8 ,o ,o ,o ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,o ,40,40,40,40,40,40,40,40,o ,10, /*25*/ 
/*26*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,15,23,23,23,23,23,23,23,15,15,15,15,15,15,15,61,EE,61,8 ,8 ,o ,o ,o ,AA,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,49,10,40,40,40,40,40,40,10,49,10, /*26*/ 
/*27*/ 56,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,24,8 ,49,49,49,49,o ,o ,o ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,62,1 ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61, /*27*/ 
/*28*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,55,55,8 ,55,55,55,55,55,55,55,55,55,55,55,55,55,62,o ,o ,o ,o ,o ,o ,10,10,1 ,62,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*28*/ 
/*29*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,4 ,4 ,4 ,55,55,FF,55,55,55,55,55,55,55,55,55,55,55,55,55,62,o ,o ,o ,o ,o ,o ,10,10,1 ,1 ,1 ,1 ,9 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,o ,o ,AA,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*29*/ 
/*30*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,4 ,50,50,54,54,8 ,55,4 ,4 ,4 ,4 ,4 ,50,51,52,53,54,55,8 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,47,47,46,46,GG,46,46,46,47,1 ,o ,o ,o ,o ,o ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,56, /*30*/ 
/*31*/ 56,o ,o ,o ,o ,o ,o ,o ,o ,4 ,51,51,53,53,8 ,55,4 ,4 ,4 ,4 ,4 ,50,51,52,53,54,55,1 ,1 ,1 ,53,53,52,52,52,51,51,47,47,46,46,46,46,46,46,47,47,1 ,o ,o ,62,1 ,10,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,61, /*31*/ 
/*32*/ 61,o ,o ,o ,o ,o ,11,o ,o ,4 ,52,52,52,52,8 ,55,4 ,4 ,4 ,4 ,4 ,50,51,52,53,54,55,1 ,1 ,1 ,53,53,52,52,52,51,51,47,47,47,47,47,47,47,47,47,47,47,1 ,1 ,3 ,3 ,1 ,1 ,7 ,13,7 ,1 ,1 ,7 ,13,7 ,1 ,1 , /*32*/ 
/*33*/ 61,EE,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,8 ,55,4 ,4 ,4 ,4 ,4 ,50,51,52,53,54,55,1 ,1 ,1 ,54,53,53,52,51,51,50,47,47,10,o ,o ,o ,o ,o ,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,o , /*33*/ 
/*34*/ 61,o ,o ,o ,o ,24,24,24,24,24,24,1 ,1 ,1 ,3 ,55,4 ,4 ,4 ,4 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,54,54,54,60,50,50,50,47,47,o ,o ,o ,o ,o ,o ,o ,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,o , /*34*/ 
/*35*/ 61,61,61,24,24,29,29,29,29,29,24,24,1 ,1 ,1 ,55,4 ,4 ,4 ,4 ,o ,o ,o ,o ,o ,o ,o ,1 ,15,1 ,54,54,54,1 ,50,50,50,47,47,o ,o ,o ,o ,o ,o ,o ,47,47,60,60,60,60,47,47,47,47,1 ,1 ,1 ,47,47,47,47,o , /*35*/ 
/*36*/ 24,24,24,29,29,29,29,29,29,29,29,24,24,1 ,1 ,55,4 ,4 ,4 ,4 ,o ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,55,1 ,55,55,55,1 ,4 ,4 ,4 ,47,47,10,o ,o ,o ,o ,o ,47,47,47,o ,o ,o ,60,47,47,47,47,23,23,23,47,47,47,47,o , /*36*/ 
/*37*/ 24,29,29,29,29,29,29,29,29,29,29,29,24,1 ,1 ,55,4 ,4 ,4 ,4 ,o ,50,50,50,50,50,50,1 ,55,1 ,55,55,55,1 ,4 ,4 ,4 ,47,47,47,47,47,47,47,47,47,47,o ,o ,o ,o ,60,47,47,47,47,23,23,23,47,47,47,47,o , /*37*/ 
/*38*/ 24,o ,29,29,29,29,29,29,29,29,29,29,24,1 ,1 ,55,4 ,4 ,4 ,4 ,o ,51,51,51,51,51,51,1 ,55,1 ,55,55,55,1 ,1 ,EE,1 ,1 ,47,47,47,47,47,47,47,1 ,o ,o ,o ,o ,o ,60,47,47,47,47,o ,1 ,o ,47,47,47,47,o , /*38*/ 
/*39*/ 24,o ,o ,o ,29,29,24,24,29,29,29,29,24,24,1 ,55,4 ,4 ,4 ,4 ,o ,52,52,52,52,52,52,1 ,55,HH,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,60,47,47,47,47,o ,1 ,o ,47,47,47,47,o , /*39*/ 
/*40*/ 24,o ,o ,o ,o ,o ,24,24,29,29,29,29,29,24,1 ,55,4 ,4 ,4 ,4 ,o ,53,53,53,53,53,53,1 ,55,1 ,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,3 ,47,47,47,47,47,47,II,II,II,47,47,47,47,o , /*40*/ 
/*41*/ 24,o ,o ,o ,o ,o ,24,24,29,29,29,29,29,24,1 ,1 ,o ,o ,o ,o ,o ,54,54,54,54,54,54,60,1 ,60,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,II,o ,o ,o ,o ,o ,47,47,47,47,47,o ,47,46,47,46,47,o , /*41*/ 
/*42*/ 24,28,28,o ,o ,o ,o ,24,29,29,29,29,29,24,1 ,1 ,1 ,EE,1 ,1 ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,o ,47,47,47,47,47,o ,47,47,47,47,47,o , /*42*/ 
/*43*/ 24,24,28,31,31,31,31,24,24,29,29,29,29,24,24,1 ,1 ,o ,o ,o ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,o ,47,47,47,47,47,o ,47,46,47,46,47,o , /*43*/ 
/*44*/ 24,24,28,28,30,30,30,24,24,29,29,29,29,29,24,24,1 ,o ,o ,o ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,o ,47,47,47,47,47,o ,47,47,47,47,47,o , /*44*/ 
/*45*/ 24,24,24,24,30,30,30,30,24,29,29,29,29,29,29,24,24,1 ,1 ,o ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,o ,15,1 ,1 ,1 ,7 ,EE,7 ,1 ,1 ,1 ,1 ,1 , /*45*/ 
/*46*/ 24,24,24,24,29,29,30,30,24,29,29,29,29,29,29,29,24,24,1 ,o ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,46,46,46,46,46,46,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,1 ,1 , /*46*/ 
/*47*/ 24,24,24,24,29,29,29,29,24,24,29,29,29,30,30,30,30,24,1 ,o ,1 ,55,55,55,55,55,55,55,55,55,55,55,55,1 ,o ,o ,o ,46,46,46,45,45,45,46,46,46,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,23,23,23,1 ,1 ,1 ,1 ,1 , /*47*/ 
/*48*/ 24,24,24,24,29,29,29,29,29,24,29,30,30,30,30,30,30,24,24,1 ,1 ,1 ,60,9 ,9 ,60,1 ,1 ,1 ,60,HH,60,1 ,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,1 ,1 ,1 ,1 ,1 , /*48*/ 
/*49*/ 24,24,24,29,29,29,29,29,29,24,30,30,30,30,30,30,30,30,24,24,8 ,46,46,GG,GG,46,46,1 ,55,55,55,55,55,1 ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,8 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 , /*49*/ 
/*50*/ 24,24,24,o ,o ,o ,o ,o ,o ,24,30,30,30,30,30,30,30,30,24,24,8 ,46,46,GG,GG,46,46,56,55,55,55,55,55,56,o ,o ,o ,8 ,o ,o ,o ,o ,o ,o ,o ,8 ,o ,o ,o ,o ,23,o ,o ,23,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 ,1 , /*50*/ 
/*51*/ 24,24,24,o ,o ,o ,o ,o ,o ,24,30,30,30,30,31,31,31,31,24,24,8 ,46,46,GG,GG,46,46,56,55,55,55,55,55,56,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,EE,o ,o ,23,o ,o ,o ,o ,o ,o ,1 ,1 ,1 ,1 , /*51*/ 
/*52*/ 24,24,24,24,o ,o ,o ,o ,o ,24,30,31,31,31,31,31,31,31,24,24,8 ,46,46,GG,GG,46,46,1 ,55,55,55,55,55,61,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,23,o ,o ,23,o ,o ,o ,o ,o ,o ,62,1 ,1 ,1 , /*52*/ 
/*53*/ 24,24,24,24,o ,o ,o ,o ,28,24,31,31,31,31,31,31,31,28,24,24,8 ,47,47,9 ,9 ,47,47,1 ,55,55,55,55,55,56,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,8 ,8 ,8 ,48,1 ,1 ,1 ,1 ,1 ,o ,o ,o ,o ,62,62,1 ,1 , /*53*/ 
/*54*/ 24,24,28,o ,o ,o ,o ,o ,24,24,31,31,31,31,31,31,31,28,24,24,8 ,47,47,9 ,9 ,47,47,1 ,55,55,55,55,55,56,o ,o ,o ,o ,11,o ,o ,o ,o ,o ,o ,o ,o ,8 ,48,48,4 ,50,51,54,55,1 ,o ,o ,o ,o ,o ,o ,o ,1 , /*54*/ 
/*55*/ 24,28,28,o ,o ,o ,o ,28,24,24,24,31,31,o ,o ,o ,o ,28,24,24,8 ,47,47,9 ,9 ,47,47,1 ,55,55,55,55,55,1 ,o ,1 ,1 ,1 ,1 ,1 ,1 ,EE,1 ,1 ,13,1 ,1 ,48,48,4 ,4 ,50,52,53,54,1 ,o ,o ,o ,o ,o ,o ,o ,6 , /*55*/ 
/*56*/ 24,28,o ,o ,o ,o ,o ,24,24,24,24,24,24,o ,o ,o ,o ,28,24,24,8 ,8 ,8 ,9 ,9 ,47,47,3 ,3 ,55,55,55,55,48,14,48,48,48,48,48,48,48,48,48,48,48,48,48,4 ,4 ,4 ,50,51,52,51,1 ,o ,o ,o ,o ,o ,o ,o ,6 , /*56*/ 
/*57*/ 24,o ,o ,o ,o ,o ,o ,24,28,o ,o ,28,24,o ,o ,o ,28,28,24,24,8 ,8 ,8 ,9 ,9 ,47,47,1 ,3 ,3 ,55,55,55,48,4 ,48,4 ,o ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,50,50,50,7 ,o ,o ,o ,o ,o ,o ,o ,6 , /*57*/ 
/*58*/ 24,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,28,24,24,8 ,9 ,9 ,9 ,9 ,9 ,47,47,1 ,1 ,3 ,1 ,1 ,1 ,48,4 ,48,4 ,48,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,7 ,o ,o ,o ,o ,o ,o ,o ,6 , /*58*/ 
/*59*/ 24,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,28,24,24,15,9 ,9 ,9 ,9 ,9 ,47,47,46,46,46,46,46,9 ,9 ,4 ,48,4 ,48,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,7 ,o ,o ,o ,o ,o ,o ,o ,6 , /*59*/ 
/*60*/ 24,24,o ,o ,o ,o ,o ,o ,o ,24,24,o ,o ,o ,o ,o ,24,24,24,8 ,9 ,9 ,9 ,47,47,47,47,47,47,47,47,47,9 ,3 ,o ,3 ,o ,3 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,1 ,o ,o ,o ,o ,o ,o ,o ,1 , /*60*/ 
/*61*/ 24,24,28,28,o ,o ,o ,28,28,24,24,24,28,o ,o ,28,24,24,24,15,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,48,4 ,4 ,4 ,48,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,53,53,1 ,1 ,1 ,1 ,EE,1 ,1 ,1 ,1 , /*61*/ 
/*62*/ 24,24,24,28,28,28,28,28,24,24,24,24,24,24,24,24,24,24,24,8 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,48,4 ,4 ,4 ,48,4 ,4 ,4 ,4 ,23,4 ,4 ,4 ,23,4 ,4 ,4 ,23,4 ,4 ,53,53,1 ,1 ,1 ,1 ,o ,o ,o ,o ,1 , /*62*/ 
/*63*/ 24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,8 ,56,56,56,56,8 ,6 ,6 ,6 ,3 ,3 ,6 ,6 ,6 ,48,61,61,61,61,61,6 ,6 ,6 ,8 ,6 ,6 ,6 ,8 ,6 ,6 ,6 ,8 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,o ,1   /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef FF
    #undef GG
    #undef HH
    #undef II
    #undef o
    },
    {        // tileDictionary
      SFG_TD(13,11,0,0),SFG_TD(14, 0,1,0),SFG_TD(13, 7,0,0),SFG_TD(17, 0,2,2), // 0 
      SFG_TD(13,22,0,0),SFG_TD(12,23,0,0),SFG_TD(17, 0,0,5),SFG_TD(17, 0,1,0), // 4 
      SFG_TD(20, 0,0,6),SFG_TD(20, 4,0,0),SFG_TD(17, 0,6,6),SFG_TD(17, 7,1,0), // 8 
      SFG_TD( 9,11,2,0),SFG_TD(17, 0,1,1),SFG_TD(15, 9,0,0),SFG_TD( 0, 0,4,4), // 12 
      SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0), // 16 
      SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13, 8,0,4), // 20 
      SFG_TD(17, 0,3,3),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0), // 24 
      SFG_TD(15, 7,3,3),SFG_TD(19, 5,3,3),SFG_TD(17, 7,3,3),SFG_TD(15, 9,3,3), // 28 
      SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0), // 32 
      SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0), // 36 
      SFG_TD(13,10,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0),SFG_TD(13,14,0,0), // 40 
      SFG_TD( 0, 0,7,7),SFG_TD(11,24,0,0),SFG_TD( 9,26,0,0),SFG_TD( 9, 4,0,0), // 44 
      SFG_TD(14, 0,4,0),SFG_TD(13, 7,0,6),SFG_TD(14,21,0,0),SFG_TD(15,20,0,0), // 48 
      SFG_TD(16,19,0,0),SFG_TD(17,18,0,0),SFG_TD(18,17,0,0),SFG_TD(19,16,0,0), // 52 
      SFG_TD(20, 2,0,0),SFG_TD(23, 2,6,6),SFG_TD(17,31,0,0),SFG_TD(13,31,0,0), // 56 
      SFG_TD(23, 0,6,6),SFG_TD(23, 0,0,0),SFG_TD(15, 9,3,0),SFG_TD(12,31,0,0)  // 60 
      },                    // tileDictionary
    {12,2 ,11,6 ,8 ,10,14}, // textureIndices
    7,                      // doorTextureIndex
    5,                      // floorColor
    4,                      // ceilingColor
    {59, 59, 64 },          // player start: x, y, direction
    2,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_CARD0, {33,3}},{SFG_LEVEL_ELEMENT_HEALTH, {31,3}},
      {SFG_LEVEL_ELEMENT_LAMP, {46,4}},{SFG_LEVEL_ELEMENT_FINISH, {56,6}},
      {SFG_LEVEL_ELEMENT_LOCK2, {46,6}},{SFG_LEVEL_ELEMENT_HEALTH, {6,6}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {28,7}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {17,7}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {11,7}},{SFG_LEVEL_ELEMENT_LAMP, {46,8}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {40,8}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {28,8}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {27,8}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {26,8}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {25,8}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {8,10}},
      {SFG_LEVEL_ELEMENT_CARD2, {7,10}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {12,11}},
      {SFG_LEVEL_ELEMENT_COLUMN, {57,12}},{SFG_LEVEL_ELEMENT_COLUMN, {56,12}},
      {SFG_LEVEL_ELEMENT_COLUMN, {55,12}},{SFG_LEVEL_ELEMENT_LOCK2, {32,12}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {37,13}},{SFG_LEVEL_ELEMENT_HEALTH, {28,13}},
      {SFG_LEVEL_ELEMENT_LOCK1, {2,13}},{SFG_LEVEL_ELEMENT_ROCKETS, {23,16}},
      {SFG_LEVEL_ELEMENT_BULLETS, {50,17}},{SFG_LEVEL_ELEMENT_PLASMA, {23,17}},
      {SFG_LEVEL_ELEMENT_BULLETS, {50,18}},{SFG_LEVEL_ELEMENT_PLASMA, {41,18}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {54,19}},{SFG_LEVEL_ELEMENT_ROCKETS, {37,19}},
      {SFG_LEVEL_ELEMENT_HEALTH, {36,19}},{SFG_LEVEL_ELEMENT_PLASMA, {37,20}},
      {SFG_LEVEL_ELEMENT_BULLETS, {36,20}},{SFG_LEVEL_ELEMENT_BULLETS, {19,20}},
      {SFG_LEVEL_ELEMENT_BULLETS, {15,20}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {27,21}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {9,21}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {7,21}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {45,22}},{SFG_LEVEL_ELEMENT_ROCKETS, {9,22}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {4,22}},{SFG_LEVEL_ELEMENT_PLASMA, {1,22}},
      {SFG_LEVEL_ELEMENT_TREE, {34,23}},{SFG_LEVEL_ELEMENT_TREE, {33,23}},
      {SFG_LEVEL_ELEMENT_PLASMA, {1,23}},{SFG_LEVEL_ELEMENT_PLASMA, {1,24}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {17,26}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {47,27}},
      {SFG_LEVEL_ELEMENT_PLASMA, {28,28}},{SFG_LEVEL_ELEMENT_TREE, {27,28}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {41,29}},{SFG_LEVEL_ELEMENT_HEALTH, {28,29}},
      {SFG_LEVEL_ELEMENT_TREE, {27,29}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {20,29}},
      {SFG_LEVEL_ELEMENT_RUIN, {1,32}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {49,33}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {49,34}},{SFG_LEVEL_ELEMENT_PLASMA, {38,34}},
      {SFG_LEVEL_ELEMENT_LAMP, {15,34}},{SFG_LEVEL_ELEMENT_HEALTH, {38,35}},
      {SFG_LEVEL_ELEMENT_HEALTH, {58,36}},{SFG_LEVEL_ELEMENT_ROCKETS, {56,36}},
      {SFG_LEVEL_ELEMENT_HEALTH, {28,36}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {15,36}},
      {SFG_LEVEL_ELEMENT_BULLETS, {49,37}},{SFG_LEVEL_ELEMENT_BULLETS, {49,38}},
      {SFG_LEVEL_ELEMENT_LOCK0, {35,38}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {53,39}},
      {SFG_LEVEL_ELEMENT_HEALTH, {50,40}},{SFG_LEVEL_ELEMENT_ROCKETS, {28,40}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {57,41}},{SFG_LEVEL_ELEMENT_RUIN, {17,41}},
      {SFG_LEVEL_ELEMENT_CARD1, {10,41}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {61,42}},
      {SFG_LEVEL_ELEMENT_MONSTER_TURRET, {53,42}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {44,42}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {38,42}},{SFG_LEVEL_ELEMENT_BARREL, {37,42}},
      {SFG_LEVEL_ELEMENT_LAMP, {12,43}},{SFG_LEVEL_ELEMENT_ROCKETS, {60,44}},
      {SFG_LEVEL_ELEMENT_NONE, {45,44}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {41,44}},
      {SFG_LEVEL_ELEMENT_BARREL, {37,44}},{SFG_LEVEL_ELEMENT_BARREL, {24,47}},
      {SFG_LEVEL_ELEMENT_BARREL, {23,47}},{SFG_LEVEL_ELEMENT_TELEPORTER, {4,47}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {32,49}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {28,49}},
      {SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {14,49}},{SFG_LEVEL_ELEMENT_ROCKETS, {51,50}},
      {SFG_LEVEL_ELEMENT_COLUMN, {50,50}},{SFG_LEVEL_ELEMENT_BULLETS, {11,50}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {7,50}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {6,50}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {5,50}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {4,50}},
      {SFG_LEVEL_ELEMENT_LOCK0, {50,51}},{SFG_LEVEL_ELEMENT_BULLETS, {11,51}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {57,52}},{SFG_LEVEL_ELEMENT_PLASMA, {51,52}},
      {SFG_LEVEL_ELEMENT_COLUMN, {50,52}},{SFG_LEVEL_ELEMENT_PLASMA, {28,52}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {28,53}},{SFG_LEVEL_ELEMENT_BULLETS, {62,54}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {61,54}},{SFG_LEVEL_ELEMENT_TELEPORTER, {54,54}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {3,54}},{SFG_LEVEL_ELEMENT_PLASMA, {56,55}},
      {SFG_LEVEL_ELEMENT_PLASMA, {41,55}},{SFG_LEVEL_ELEMENT_RUIN, {34,55}},
      {SFG_LEVEL_ELEMENT_HEALTH, {22,55}},{SFG_LEVEL_ELEMENT_PLASMA, {21,55}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {34,57}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {45,58}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {9,59}},{SFG_LEVEL_ELEMENT_LAMP, {2,59}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {56,60}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {41,60}},
      {SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {36,60}},{SFG_LEVEL_ELEMENT_HEALTH, {23,60}},
      {SFG_LEVEL_ELEMENT_HEALTH, {15,61}},{SFG_LEVEL_ELEMENT_ROCKETS, {8,61}},
      {SFG_LEVEL_ELEMENT_RUIN, {61,62}},{SFG_LEVEL_ELEMENT_ROCKETS, {60,62}},
      {SFG_LEVEL_ELEMENT_HEALTH, {59,62}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

SFG_PROGRAM_MEMORY SFG_Level SFG_level9 =
  {          // level
    {        // mapArray
    #define o 0
    #define AA (41 | SFG_TILE_PROPERTY_ELEVATOR)
    #define BB (53 | SFG_TILE_PROPERTY_ELEVATOR)
    #define CC (48 | SFG_TILE_PROPERTY_DOOR)
    #define DD (4 | SFG_TILE_PROPERTY_DOOR)
    #define EE (7 | SFG_TILE_PROPERTY_ELEVATOR)
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
/*0 */ 6 ,6 ,6 ,AA,AA,AA,6 ,6 ,6 ,6 ,21,21,21,21,4 ,4 ,4 ,4 ,6 ,6 ,6 ,6 ,AA,AA,AA,AA,62,62,62,62,62,62,62,62,62,62,62,62,62,63,63,58,58,58,58,58,58,58,58,58,63,63,63,58,58,58,58,58,58,58,58,58,63,63, /*0 */ 
/*1 */ 62,33,62,32,33,39,62,37,38,33,62,37,32,33,62,35,36,35,62,33,34,33,62,32,33,32,62,62,62,62,62,62,62,62,62,62,62,62,62,63,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58,63, /*1 */ 
/*2 */ 47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,62,62,62,BB,52,52,52,52,52,52,52,52,52,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58, /*2 */ 
/*3 */ 47,47,47,47,47,47,47,47,47,49,47,49,47,47,47,49,47,49,47,47,47,49,47,49,47,47,62,62,62,47,62,62,62,62,62,62,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58, /*3 */ 
/*4 */ 47,47,47,49,47,49,47,47,49,49,49,49,49,47,49,49,49,49,49,47,49,49,49,49,49,47,47,47,47,47,62,62,62,62,62,62,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58, /*4 */ 
/*5 */ 47,47,47,47,47,47,47,47,49,49,49,49,49,47,49,49,49,49,49,47,49,49,49,49,49,47,62,62,62,62,62,62,62,62,62,62,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58, /*5 */ 
/*6 */ 47,47,49,47,47,47,49,47,47,49,49,49,47,47,47,49,49,49,47,47,47,49,49,49,47,47,62,62,63,63,63,63,63,63,63,63,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58, /*6 */ 
/*7 */ 47,47,47,49,49,49,47,47,47,47,49,47,47,47,47,47,49,47,47,47,47,47,49,47,47,47,62,62,63,o ,o ,o ,o ,o ,o ,63,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58,58,58,58,58,58,58,58,58,58,o ,o ,58, /*7 */ 
/*8 */ 47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,62,62,57,o ,o ,o ,o ,o ,o ,57,62,62,62,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58,1 ,1 ,1 ,1 ,1 ,1 ,25,1 ,58,58,58,58, /*8 */ 
/*9 */ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,63,62,62,62,63,58,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,58,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,58,23,23,23, /*9 */ 
/*10*/ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,57,o ,o ,o ,o ,o ,o ,57,62,62,62,63,63,58,58,58,58,58,58,58,58,58,58,58,1 ,62,62,62,62,62,62,62,47,47,62,62, /*10*/ 
/*11*/ 47,62,62,62,62,62,63,63,57,63,63,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,63,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,63,57,63,63,62,62,62,47,62,62, /*11*/ 
/*12*/ 47,62,62,62,62,62,63,o ,o ,o ,63,63,57,63,57,63,57,63,57,63,57,63,57,63,57,63,57,63,63,o ,o ,o ,o ,o ,o ,63,63,57,63,57,63,57,63,57,63,57,63,57,63,57,63,57,63,63,o ,o ,o ,63,62,62,62,47,47,47, /*12*/ 
/*13*/ 47,62,62,62,62,62,57,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,57,62,62,62,62,62,CC, /*13*/ 
/*14*/ 47,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,62,62,62,62,62,47, /*14*/ 
/*15*/ 47,62,62,62,62,62,63,63,o ,o ,1 ,1 ,1 ,25,1 ,1 ,1 ,1 ,1 ,1 ,60,1 ,1 ,1 ,1 ,o ,8 ,9 ,10,11,12,1 ,1 ,12,11,10,9 ,8 ,o ,1 ,1 ,1 ,1 ,60,1 ,1 ,1 ,1 ,1 ,1 ,25,1 ,1 ,1 ,o ,o ,63,63,62,62,62,62,62,47, /*15*/ 
/*16*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,2 ,2 ,2 ,59,2 ,26,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,26,2 ,59,2 ,2 ,2 ,1 ,2 ,2 ,2 ,2 ,2 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*16*/ 
/*17*/ 47,62,62,62,62,62,62,23,o ,o ,1 ,2 ,3 ,3 ,3 ,21,1 ,21,27,15,15,15,3 ,2 ,2 ,2 ,59,2 ,59,2 ,59,2 ,2 ,59,2 ,59,2 ,59,2 ,2 ,2 ,3 ,3 ,3 ,3 ,27,21,1 ,21,3 ,3 ,3 ,2 ,1 ,o ,o ,23,62,62,62,62,62,62,47, /*17*/ 
/*18*/ 47,62,62,62,62,62,62,23,o ,o ,60,59,61,6 ,6 ,21,1 ,1 ,21,15,15,15,3 ,3 ,3 ,3 ,61,3 ,61,3 ,61,3 ,3 ,61,3 ,61,3 ,61,3 ,3 ,3 ,3 ,6 ,6 ,6 ,21,1 ,1 ,21,6 ,6 ,61,59,60,o ,o ,23,62,62,62,62,62,62,47, /*18*/ 
/*19*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,2 ,3 ,6 ,6 ,21,1 ,1 ,21,15,15,15,15,6 ,6 ,6 ,61,6 ,61,6 ,61,6 ,6 ,61,6 ,61,6 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,21,1 ,1 ,21,6 ,6 ,3 ,2 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*19*/ 
/*20*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,2 ,3 ,6 ,6 ,21,21,21,21,6 ,15,15,15,6 ,6 ,6 ,61,6 ,61,22,61,6 ,6 ,61,22,61,6 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,21,21,21,21,6 ,6 ,3 ,2 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*20*/ 
/*21*/ 47,62,62,62,62,62,62,63,o ,o ,25,26,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,15,15,6 ,6 ,61,6 ,61,6 ,61,6 ,6 ,61,6 ,61,6 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,26,25,o ,o ,63,62,62,62,62,62,62,47, /*21*/ 
/*22*/ 47,62,62,62,62,63,63,57,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,15,15,6 ,61,6 ,61,6 ,61,6 ,6 ,61,6 ,61,6 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,o ,o ,57,63,63,62,62,62,62,47, /*22*/ 
/*23*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,15,6 ,61,3 ,61,3 ,61,6 ,6 ,61,3 ,61,3 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*23*/ 
/*24*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,15,15,4 ,4 ,4 ,4 ,13,13,13,13,4 ,4 ,4 ,4 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*24*/ 
/*25*/ 47,62,62,62,62,63,o ,o ,o ,o ,60,2 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,4 ,4 ,4 ,13,13,13,5 ,5 ,13,13,13,4 ,4 ,4 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,60,o ,o ,o ,o ,63,62,62,62,62,47, /*25*/ 
/*26*/ 47,62,62,62,62,63,o ,o ,o ,o ,60,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,4 ,4 ,4 ,13,5 ,5 ,5 ,5 ,5 ,5 ,13,4 ,4 ,4 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,60,o ,o ,o ,o ,63,62,62,62,62,47, /*26*/ 
/*27*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,13,4 ,4 ,13,5 ,5 ,5 ,5 ,5 ,5 ,13,4 ,4 ,13,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*27*/ 
/*28*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,3 ,6 ,61,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,4 ,4 ,4 ,13,13,13,5 ,5 ,13,13,13,4 ,4 ,4 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,61,6 ,3 ,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*28*/ 
/*29*/ 47,62,62,62,62,63,63,57,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,4 ,4 ,4 ,4 ,4 ,13,DD,DD,13,4 ,4 ,4 ,4 ,4 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,o ,o ,57,63,63,62,62,62,62,47, /*29*/ 
/*30*/ 47,62,62,62,62,62,62,63,o ,o ,25,26,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,4 ,4 ,13,4 ,4 ,5 ,5 ,4 ,4 ,13,4 ,4 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,26,25,o ,o ,63,62,62,62,62,62,62,47, /*30*/ 
/*31*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,2 ,2 ,2 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,6 ,3 ,6 ,6 ,6 ,6 ,3 ,6 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,2 ,2 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*31*/ 
/*32*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,1 ,1 ,2 ,14,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,14,2 ,1 ,1 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*32*/ 
/*33*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,1 ,1 ,2 ,14,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,14,2 ,1 ,1 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*33*/ 
/*34*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,1 ,1 ,2 ,14,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,14,2 ,1 ,1 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*34*/ 
/*35*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,1 ,1 ,2 ,14,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,14,2 ,1 ,1 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*35*/ 
/*36*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,2 ,2 ,2 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,2 ,2 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*36*/ 
/*37*/ 47,62,62,62,62,62,62,57,o ,o ,25,2 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,2 ,25,o ,o ,57,62,62,62,62,62,62,47, /*37*/ 
/*38*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*38*/ 
/*39*/ 47,62,62,62,62,63,63,57,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,6 ,6 ,15,15,2 ,1 ,o ,o ,57,63,63,62,62,62,62,47, /*39*/ 
/*40*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,15,15,15,15,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*40*/ 
/*41*/ 47,62,62,62,62,63,o ,o ,o ,58,1 ,2 ,3 ,6 ,6 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,15,15,3 ,2 ,1 ,58,o ,o ,o ,63,62,62,62,62,47, /*41*/ 
/*42*/ 47,62,62,62,62,63,o ,o ,o ,1 ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,1 ,o ,o ,o ,63,62,62,62,62,47, /*42*/ 
/*43*/ 47,62,62,62,62,63,o ,o ,o ,1 ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,1 ,o ,o ,o ,63,62,62,62,62,47, /*43*/ 
/*44*/ 47,62,62,62,62,63,o ,o ,o ,58,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,58,o ,o ,o ,63,62,62,62,62,47, /*44*/ 
/*45*/ 47,62,62,62,62,63,o ,o ,o ,o ,1 ,2 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,2 ,1 ,o ,o ,o ,o ,63,62,62,62,62,47, /*45*/ 
/*46*/ 47,62,62,62,62,63,63,57,o ,o ,1 ,2 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,1 ,o ,o ,57,63,63,62,62,62,62,47, /*46*/ 
/*47*/ 47,62,62,62,62,62,62,63,o ,o ,60,59,61,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,61,59,60,o ,o ,63,62,62,62,62,62,62,47, /*47*/ 
/*48*/ 47,62,62,62,62,62,62,57,o ,o ,25,26,27,3 ,6 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,14,14,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,6 ,3 ,27,26,25,o ,o ,57,62,62,62,62,62,62,47, /*48*/ 
/*49*/ 47,62,62,62,62,62,62,63,o ,o ,60,59,61,3 ,6 ,6 ,6 ,3 ,6 ,6 ,61,6 ,6 ,61,6 ,6 ,6 ,6 ,20,5 ,5 ,5 ,5 ,5 ,5 ,20,6 ,6 ,6 ,6 ,61,6 ,6 ,61,6 ,6 ,3 ,6 ,6 ,6 ,3 ,61,59,60,o ,o ,63,62,62,62,62,62,62,47, /*49*/ 
/*50*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,2 ,3 ,3 ,6 ,6 ,6 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,20,5 ,5 ,5 ,5 ,5 ,5 ,20,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,6 ,6 ,6 ,3 ,3 ,2 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*50*/ 
/*51*/ 47,62,62,62,62,62,62,63,o ,o ,60,59,61,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,3 ,3 ,5 ,5 ,EE,EE,5 ,5 ,3 ,3 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,3 ,61,59,60,o ,o ,63,62,62,62,62,62,62,47, /*51*/ 
/*52*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,2 ,3 ,3 ,6 ,6 ,6 ,6 ,3 ,3 ,3 ,6 ,6 ,6 ,3 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,3 ,6 ,6 ,6 ,3 ,3 ,3 ,6 ,6 ,6 ,6 ,3 ,3 ,2 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*52*/ 
/*53*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,2 ,2 ,3 ,3 ,3 ,3 ,3 ,3 ,2 ,3 ,3 ,3 ,3 ,3 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,3 ,2 ,3 ,3 ,3 ,3 ,3 ,3 ,2 ,2 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*53*/ 
/*54*/ 47,62,62,62,62,62,62,57,o ,o ,1 ,2 ,2 ,2 ,59,2 ,59,2 ,2 ,2 ,2 ,2 ,26,2 ,2 ,2 ,1 ,16,17,18,19,2 ,2 ,19,18,17,16,1 ,2 ,2 ,2 ,26,2 ,2 ,2 ,2 ,2 ,59,2 ,59,2 ,2 ,2 ,1 ,o ,o ,57,62,62,62,62,62,62,47, /*54*/ 
/*55*/ 47,62,62,62,62,62,62,63,o ,o ,1 ,1 ,1 ,1 ,60,25,60,1 ,1 ,1 ,1 ,1 ,25,1 ,60,1 ,60,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,60,1 ,60,1 ,25,1 ,1 ,1 ,1 ,1 ,60,25,60,1 ,1 ,1 ,1 ,o ,o ,63,62,62,62,62,62,62,47, /*55*/ 
/*56*/ 47,62,62,62,62,62,62,57,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,57,57,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,57,62,62,62,62,62,62,47, /*56*/ 
/*57*/ 47,62,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,63,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,o ,63,62,62,62,62,62,62,47, /*57*/ 
/*58*/ 47,62,62,62,62,62,62,63,63,57,63,57,63,57,63,57,63,23,23,23,23,23,63,57,63,57,63,57,o ,o ,o ,o ,o ,o ,o ,o ,57,63,57,63,57,63,23,23,23,23,23,63,57,63,57,63,57,63,57,63,63,62,62,62,62,62,62,47, /*58*/ 
/*59*/ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,o ,o ,o ,o ,o ,o ,o ,o ,63,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,47, /*59*/ 
/*60*/ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,63,63,63,63,63,63,63,63,63,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,47, /*60*/ 
/*61*/ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,47, /*61*/ 
/*62*/ 47,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,47, /*62*/ 
/*63*/ 47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47  /*63*/ 
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
    #undef AA
    #undef BB
    #undef CC
    #undef DD
    #undef EE
    #undef o
    },
    {        // tileDictionary
      SFG_TD( 0,31,0,0),SFG_TD(12,31,0,0),SFG_TD(22,31,0,0),SFG_TD(31,31,0,0), // 0 
      SFG_TD(31, 0,0,0),SFG_TD(27,31,0,0),SFG_TD(29,31,4,0),SFG_TD(22, 5,1,0), // 4 
      SFG_TD( 2,31,0,0),SFG_TD( 4,31,0,0),SFG_TD( 6,31,0,0),SFG_TD( 8,31,0,0), // 8 
      SFG_TD(10,31,0,0),SFG_TD(31, 0,3,3),SFG_TD(31,31,3,0),SFG_TD(28,31,4,0), // 12 
      SFG_TD(14,31,0,0),SFG_TD(16,31,0,0),SFG_TD(18,31,0,0),SFG_TD(20,31,0,0), // 16 
      SFG_TD(29,31,0,0),SFG_TD(31,31,5,0),SFG_TD(31, 0,6,6),SFG_TD( 9,31,0,0), // 20 
      SFG_TD(14,15,0,0),SFG_TD(12,31,2,0),SFG_TD(22,31,2,0),SFG_TD(31,31,2,0), // 24 
      SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 28 
      SFG_TD(13, 4,7,7),SFG_TD( 1,16,7,7),SFG_TD( 9, 4,7,7),SFG_TD( 1,12,7,7), // 32 
      SFG_TD( 5,12,7,7),SFG_TD( 1,12,7,7),SFG_TD( 5, 4,7,7),SFG_TD( 1, 4,7,7), // 36 
      SFG_TD(14,15,0,0),SFG_TD(31,18,5,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 40 
      SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0),SFG_TD( 0,31,7,7), // 44 
      SFG_TD( 4,31,0,0),SFG_TD( 0,31,0,0),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 48 
      SFG_TD(31,31,7,7),SFG_TD( 0,31,0,7),SFG_TD(14,15,0,0),SFG_TD(14,15,0,0), // 52 
      SFG_TD(14,15,0,0),SFG_TD( 4,31,3,0),SFG_TD( 2,31,5,0),SFG_TD(23,31,1,0), // 56 
      SFG_TD(13,31,1,0),SFG_TD(30,31,1,0),SFG_TD(31, 0,7,7),SFG_TD( 4,31,0,0)  // 60 
      },                    // tileDictionary
    {14,11,1 ,8 ,9 ,5 ,13}, // textureIndices
    7,                      // doorTextureIndex
    74,                     // floorColor
    74,                     // ceilingColor
    {55, 13, 160},          // player start: x, y, direction
    2,                      // backgroundImage
    {                       // elements
      {SFG_LEVEL_ELEMENT_BULLETS, {55,2}},{SFG_LEVEL_ELEMENT_BULLETS, {53,2}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {47,2}},{SFG_LEVEL_ELEMENT_ROCKETS, {45,2}},
      {SFG_LEVEL_ELEMENT_HEALTH, {55,4}},{SFG_LEVEL_ELEMENT_HEALTH, {53,4}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {47,4}},{SFG_LEVEL_ELEMENT_ROCKETS, {45,4}},
      {SFG_LEVEL_ELEMENT_PLASMA, {48,6}},{SFG_LEVEL_ELEMENT_PLASMA, {48,7}},
      {SFG_LEVEL_ELEMENT_TELEPORTER, {43,7}},{SFG_LEVEL_ELEMENT_HEALTH, {33,8}},
      {SFG_LEVEL_ELEMENT_HEALTH, {30,8}},{SFG_LEVEL_ELEMENT_BULLETS, {33,10}},
      {SFG_LEVEL_ELEMENT_PLASMA, {30,10}},{SFG_LEVEL_ELEMENT_LOCK2, {63,13}},
      {SFG_LEVEL_ELEMENT_RUIN, {43,13}},{SFG_LEVEL_ELEMENT_RUIN, {43,14}},
      {SFG_LEVEL_ELEMENT_RUIN, {42,15}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {52,17}},
      {SFG_LEVEL_ELEMENT_BARREL, {25,17}},{SFG_LEVEL_ELEMENT_TELEPORTER, {46,19}},
      {SFG_LEVEL_ELEMENT_RUIN, {22,19}},{SFG_LEVEL_ELEMENT_HEALTH, {17,19}},
      {SFG_LEVEL_ELEMENT_BULLETS, {16,19}},{SFG_LEVEL_ELEMENT_HEALTH, {47,21}},
      {SFG_LEVEL_ELEMENT_PLASMA, {46,21}},{SFG_LEVEL_ELEMENT_PLASMA, {17,21}},
      {SFG_LEVEL_ELEMENT_HEALTH, {16,21}},{SFG_LEVEL_ELEMENT_ROCKETS, {47,22}},
      {SFG_LEVEL_ELEMENT_BULLETS, {46,22}},{SFG_LEVEL_ELEMENT_BULLETS, {17,22}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {16,22}},{SFG_LEVEL_ELEMENT_HEALTH, {32,23}},
      {SFG_LEVEL_ELEMENT_HEALTH, {31,23}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {10,23}},
      {SFG_LEVEL_ELEMENT_BARREL, {47,24}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {42,24}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {56,25}},{SFG_LEVEL_ELEMENT_BARREL, {47,25}},
      {SFG_LEVEL_ELEMENT_FINISH, {32,25}},{SFG_LEVEL_ELEMENT_FINISH, {31,25}},
      {SFG_LEVEL_ELEMENT_RUIN, {24,25}},{SFG_LEVEL_ELEMENT_ROCKETS, {7,25}},
      {SFG_LEVEL_ELEMENT_PLASMA, {56,26}},{SFG_LEVEL_ELEMENT_HEALTH, {42,26}},
      {SFG_LEVEL_ELEMENT_COLUMN, {33,26}},{SFG_LEVEL_ELEMENT_COLUMN, {30,26}},
      {SFG_LEVEL_ELEMENT_HEALTH, {21,26}},{SFG_LEVEL_ELEMENT_PLASMA, {7,26}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {56,27}},{SFG_LEVEL_ELEMENT_COLUMN, {33,27}},
      {SFG_LEVEL_ELEMENT_COLUMN, {30,27}},{SFG_LEVEL_ELEMENT_ROCKETS, {7,27}},
      {SFG_LEVEL_ELEMENT_HEALTH, {42,28}},{SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {38,28}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {26,28}},{SFG_LEVEL_ELEMENT_HEALTH, {21,28}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {39,29}},{SFG_LEVEL_ELEMENT_LOCK2, {32,29}},
      {SFG_LEVEL_ELEMENT_LOCK2, {31,29}},{SFG_LEVEL_ELEMENT_ROCKETS, {25,29}},
      {SFG_LEVEL_ELEMENT_BULLETS, {33,30}},{SFG_LEVEL_ELEMENT_BULLETS, {30,30}},
      {SFG_LEVEL_ELEMENT_MONSTER_ENDER, {40,32}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {31,32}},
      {SFG_LEVEL_ELEMENT_MONSTER_ENDER, {23,32}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {55,33}},
      {SFG_LEVEL_ELEMENT_HEALTH, {52,33}},{SFG_LEVEL_ELEMENT_HEALTH, {11,33}},
      {SFG_LEVEL_ELEMENT_HEALTH, {52,34}},{SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {45,34}},
      {SFG_LEVEL_ELEMENT_MONSTER_SPIDER, {19,34}},{SFG_LEVEL_ELEMENT_HEALTH, {11,34}},
      {SFG_LEVEL_ELEMENT_MONSTER_ENDER, {31,35}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {27,36}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {39,37}},{SFG_LEVEL_ELEMENT_MONSTER_WARRIOR, {35,37}},
      {SFG_LEVEL_ELEMENT_MONSTER_DESTROYER, {24,37}},{SFG_LEVEL_ELEMENT_PLASMA, {43,39}},
      {SFG_LEVEL_ELEMENT_PLASMA, {20,39}},{SFG_LEVEL_ELEMENT_HEALTH, {32,40}},
      {SFG_LEVEL_ELEMENT_HEALTH, {31,40}},{SFG_LEVEL_ELEMENT_BARREL, {43,41}},
      {SFG_LEVEL_ELEMENT_BARREL, {42,41}},{SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {38,41}},
      {SFG_LEVEL_ELEMENT_MONSTER_EXPLODER, {25,41}},{SFG_LEVEL_ELEMENT_BARREL, {20,41}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {56,42}},{SFG_LEVEL_ELEMENT_BARREL, {43,42}},
      {SFG_LEVEL_ELEMENT_BARREL, {42,42}},{SFG_LEVEL_ELEMENT_BARREL, {21,42}},
      {SFG_LEVEL_ELEMENT_BARREL, {19,42}},{SFG_LEVEL_ELEMENT_ROCKETS, {7,42}},
      {SFG_LEVEL_ELEMENT_PLASMA, {56,43}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {52,43}},
      {SFG_LEVEL_ELEMENT_BARREL, {32,43}},{SFG_LEVEL_ELEMENT_BARREL, {31,43}},
      {SFG_LEVEL_ELEMENT_BARREL, {30,43}},{SFG_LEVEL_ELEMENT_BARREL, {20,43}},
      {SFG_LEVEL_ELEMENT_PLASMA, {7,43}},{SFG_LEVEL_ELEMENT_HEALTH, {32,45}},
      {SFG_LEVEL_ELEMENT_HEALTH, {31,45}},{SFG_LEVEL_ELEMENT_ROCKETS, {49,50}},
      {SFG_LEVEL_ELEMENT_HEALTH, {48,50}},{SFG_LEVEL_ELEMENT_HEALTH, {15,50}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {14,50}},{SFG_LEVEL_ELEMENT_PLASMA, {49,51}},
      {SFG_LEVEL_ELEMENT_BULLETS, {48,51}},{SFG_LEVEL_ELEMENT_BULLETS, {15,51}},
      {SFG_LEVEL_ELEMENT_PLASMA, {14,51}},{SFG_LEVEL_ELEMENT_RUIN, {31,52}},
      {SFG_LEVEL_ELEMENT_RUIN, {36,53}},{SFG_LEVEL_ELEMENT_RUIN, {35,53}},
      {SFG_LEVEL_ELEMENT_RUIN, {34,53}},{SFG_LEVEL_ELEMENT_RUIN, {33,53}},
      {SFG_LEVEL_ELEMENT_RUIN, {32,53}},{SFG_LEVEL_ELEMENT_RUIN, {28,53}},
      {SFG_LEVEL_ELEMENT_RUIN, {27,53}},{SFG_LEVEL_ELEMENT_MONSTER_TURRET, {25,53}},
      {SFG_LEVEL_ELEMENT_BARREL, {18,54}},{SFG_LEVEL_ELEMENT_HEALTH, {55,57}},
      {SFG_LEVEL_ELEMENT_HEALTH, {8,57}},{SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT, {33,58}},
      {SFG_LEVEL_ELEMENT_ROCKETS, {32,59}},{SFG_LEVEL_ELEMENT_PLASMA, {31,59}},
      {SFG_LEVEL_ELEMENT_NONE, {0,0}},{SFG_LEVEL_ELEMENT_NONE, {0,0}}
    }, // elements
  } // level
  ;

static const SFG_Level * SFG_levels[SFG_NUMBER_OF_LEVELS] =
{
  &SFG_level0, &SFG_level1, &SFG_level2, &SFG_level3, &SFG_level4, &SFG_level5,
  &SFG_level6, &SFG_level7, &SFG_level8, &SFG_level9
};

#endif // guard

