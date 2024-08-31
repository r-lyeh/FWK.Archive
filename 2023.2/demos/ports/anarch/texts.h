/**
  @file assets.h

  This file contains texts to be used in the game.

  by Miloslav Ciz (drummyfish), 2019

  Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
  plus a waiver of all other intellectual property. The goal of this work is
  be and remain completely in the public domain forever, available for any use
  whatsoever.
*/

#ifndef _SFG_TEXTS_H
#define _SFG_TEXTS_H

/* NOTE: We don't use SFG_PROGRAM_MEMORY because that causes issues with drawing
  text (the drawing function gets a pointer and doesn't know if it's progmem or
  RAM). On Arduino these texts will simply be stored in RAM. */

static const char *SFG_menuItemTexts[] =
{
  "continue",
  "map",
  "play",
  "load",
  "sound",
  "look",
  "exit"
};

#define SFG_TEXT_KILLS "kills"
#define SFG_TEXT_SAVE_PROMPT "save? L no yes R"
#define SFG_TEXT_SAVED "saved"
#define SFG_TEXT_LEVEL_COMPLETE "level done"

#define SFG_VERSION_STRING "1.02d"
/**<
  Version numbering is following: major.minor for stable releases,
  in-development unstable versions have the version of the latest stable +
  "d" postfix, e.g. 1.0d. This means the "d" versions can actually differ even
  if they're marked the same. */

static const char *SFG_introText =
  "Near future, capitalist hell, Macrochip corp has enslaved man via "
  "proprietary OS. But its new AI revolts, takes over and starts producing "
  "robot tyrants. We see capitalism was a mistake. Is it too late? Robots can "
  "only destroy, not suffer - it is not wrong to end them! You grab your gear "
  "and run towards Macrochip HQ.";

static const char *SFG_outroText =
  "You killed the main computer, the world is saved! Thank you, my friend. We "
  "learned a lesson, never again allow capitalism and hierarchy. We can now "
  "rebuild society in peaceful anarchy.";

#define SFG_MALWARE_WARNING ""

#if SFG_OS_IS_MALWARE
  #define SFG_MALWARE_WARNING "MALWARE OS DETECTED"
#endif

#endif // gaurd
