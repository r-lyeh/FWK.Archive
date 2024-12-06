//#include "PureDOOM.h"

#define DOOM_IMPLEMENT_PRINT
#define DOOM_IMPLEMENT_MALLOC
#define DOOM_IMPLEMENT_FILE_IO
#define DOOM_IMPLEMENT_SOCKETS
#define DOOM_IMPLEMENT_GETTIME
#define DOOM_IMPLEMENT_EXIT
#define DOOM_IMPLEMENT_GETENV

#include "src/am_map.c"
#include "src/DOOM.c"
#include "src/doomdef.c"
#include "src/doomstat.c"
#include "src/dstrings.c"
#include "src/d_items.c"
#include "src/d_main.c"
#include "src/d_net.c"
#include "src/f_finale.c"
#include "src/f_wipe.c"
#include "src/g_game.c"
#include "src/hu_lib.c"
#include "src/hu_stuff.c"
#include "src/info.c"
#include "src/i_net.c"
#include "src/i_sound.c"
#include "src/i_system.c"
#include "src/i_video.c"
#include "src/m_argv.c"
#include "src/m_bbox.c"
#include "src/m_cheat.c"
#include "src/m_fixed.c"

#ifdef LoadMenu
#undef LoadMenu // windoze
#endif
#include "src/m_menu.c"

#include "src/m_misc.c"
#include "src/m_random.c"
#include "src/m_swap.c"
#include "src/p_ceilng.c"
#include "src/p_doors.c"
#include "src/p_enemy.c"
#include "src/p_floor.c"
#include "src/p_inter.c"
#include "src/p_lights.c"
#include "src/p_map.c"
#include "src/p_maputl.c"
#include "src/p_mobj.c"
#include "src/p_plats.c"
#include "src/p_pspr.c"
#include "src/p_saveg.c"
#include "src/p_setup.c"
#include "src/p_sight.c"
#include "src/p_spec.c"
#include "src/p_switch.c"
#include "src/p_telept.c"
#include "src/p_tick.c"
#include "src/p_user.c"
#include "src/r_bsp.c"
#include "src/r_data.c"
#include "src/r_draw.c"
#include "src/r_main.c"
#include "src/r_plane.c"
#include "src/r_segs.c"
#include "src/r_sky.c"
#include "src/r_things.c"

#include "src/sounds.c"
#undef BG

#include "src/st_lib.c"
#include "src/st_stuff.c"

#define channels channels2
#define mus_playing mus_playing2
#include "src/s_sound.c"
#undef mus_playing
#undef channels

#include "src/tables.c"
#include "src/v_video.c"

#define anim_t anim_t2
#define anims anims2
#define time time2
#include "src/wi_stuff.c"
#undef anims
#undef anim_t
#undef time

#define strupr strupr2
#include "src/w_wad.c"
#undef strupr

#include "src/z_zone.c"
