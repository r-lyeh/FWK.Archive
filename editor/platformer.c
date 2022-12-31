#include "fwk.h"
#define tile tile2
#define tilemap tilemap2
#define os_exit(rc, ...) exit((printf(__VA_ARGS__), rc))



array(texture_t) sprites = 0;
int sprite_load(const char *file) {
    texture_t sprite = texture(file, TEXTURE_RGBA);
    array_push(sprites, sprite);
    return array_count(sprites) - 1;
}
void sprite_draw(int spr, int x, int y, int ox, int oy) {
    assert(spr < array_count(sprites));
    vec3 pos = {x,y,0}, sheet = {0,1,1};
    vec3 offs = {ox,oy,0}, sca = {1,1,1};
    float rot = 0;
    offs = mul3(offs, sca);
    pos = mul3(pos, sca);
    sprite_sheet( sprites[spr], &sheet.x, &pos.x, rot, &offs.x, &sca.x, false, ~0u );
}


enum { FLOOR_Y = 480 };

map(int,int) tileset_chr2spr;

typedef struct tile {
    int x, y;
    int sprite;
    int floor:1;
//    int block_jump:1;
} tile;
typedef struct tilemap {
    array(tile) tiles;
    int tw, th; // tile-width, tile-height (in pixel units)
    int mw, mh; // map-width,   map-height (in tile units)
    char *mapdata;
} tilemap;

tilemap statics;
tilemap dynamics;

void tileset_new(const char *tile_path, int frame_from, int frame_end, const char *keys) {
    map_init( tileset_chr2spr, less_int, hash_int );
    for( int i = frame_from; i <= frame_end; ++i ) {
        int sprite = sprite_load(va(tile_path, i));
        map_insert(tileset_chr2spr, keys[(i - frame_from)], sprite);
    }
}
void tilemap_new(int tw, int th, int w, int h, const char *mapdata, const char *floorchr ) {
    statics.tw = tw;
    statics.th = th;
    statics.mw = w;
    statics.mh = h;
    statics.mapdata = strdup(mapdata);
    for( int y = 0; y < h; ++y ) {
        for( int x = 0; x < w; ++x ) {
            int chr = mapdata[ x + y * statics.mw ];
            int floor = !!strchr(floorchr, chr);
            int *found = map_find(tileset_chr2spr, chr);
            tile t = { x * tw, y * th, found ? *found : -1, floor };
            array_push(statics.tiles, t);
        }
    }
}
const tile *tilemap_at(int x, int y) {
    int cx = x / statics.tw, cy = y / statics.th;
    if( cx >= 0 && cx < statics.mw && cy >= 0 && cy < statics.mh ) {
        int chr = statics.mapdata[ cx + cy * statics.mw ];
        return &statics.tiles[cx + cy * statics.mw];
    }
    return 0;
}
void tilemap_draw(int ox, int oy) {
    array_foreach(statics.tiles, tile, t) {
        if( t.sprite >= 0 ) sprite_draw(t.sprite, ox + t.x, oy + t.y, 0, +32);
    }
    array_foreach(dynamics.tiles, tile, t) {
        if( t.sprite >= 0 ) sprite_draw(t.sprite, ox + t.x, oy + t.y, 0, +32);
    }
}

typedef struct player {
    float x, y, h, vx, vy;
    float px, py, dx, dy, ppx, ppy, ddx, ddy; // prevx, prevy (for dir estimation)
    int frame;
    int sprite[3];
    int key_left, key_right, key_jump;
    float t;
    int debug;
    int state;
} player;

player player_new(int x, int y, int keys[], const char *sprite_path) {
    player p1 = {0};
    p1.x = x;
    p1.y = y;
    p1.key_left = keys[0];
    p1.key_right = keys[1];
    p1.key_jump = keys[2];
    for( int i = 0; i < 3; ++i ) {
        p1.sprite[i] = sprite_load(va(sprite_path, i+1));
    }
    return p1;
}
void player_draw(player *p) {
    sprite_draw(p->sprite[p->frame], p->x, p->y, 0, 8+32);
}
void player_move(player *p, float dt) {
    enum { fall, ground, jump, };
    const char *table[] = { "fall", "ground", "jump" };

    if(p->debug) {
        const tile *t = tilemap_at(p->x-16,p->y-32);
        if(t && t->sprite >= 0) sprite_draw(t->sprite, 64, 128, 0, 0);
        window_title(
            va("st:%s y:%+6.3f dy:%+6.3f vy:%+6.3f", table[p->state], p->y, p->dy, p->vy)
        );
    }
    if( dt <= 0 ) {
        return;
    }

    // @todo: Per-second damping coefficient, and framerate-independent coeff
    // (http://lolengine.net/blog/2015/05/03/damping-with-delta-time)
    #define K(X) (X)

    if( input(p->key_left) ) p->vx = (-4);
    if( input(p->key_right) ) p->vx = (+4);
    if( input_down(p->key_jump) && !p->vy ) p->vy = (-22), p->state = jump;

    // animation
    p->t += dt * 1000;
    if( p->t > 96 ) {
        p->t = 0;
        ++p->frame; if( p->frame > 2 ) p->frame = 0;
    }
    if( abs(p->vx) < 0.1 ) p->frame = 0; // stand
    if( abs(p->vy) > 0.1 ) p->frame = 2; // fly

    // movement

    p->x += p->vx; (p->vx *= K(0.80f));

    if( p->state == jump || p->state == fall ) {
    p->y += p->vy; (p->vy *= K(0.95f));
    }

    // block
    const tile *t = tilemap_at(p->x-16,p->y-32);

    // gravity
    int top = p->y - 32;
    int bottom = p->y;
    int ground_y = t && t->floor ? t->y+32 : FLOOR_Y;
    if( p->state == jump || p->state == fall ) { p->y += K(10); if( p->dy > 0 ) p->state = fall; }
    if( p->state == fall && p->y >= FLOOR_Y ) { p->y = FLOOR_Y, p->vy = 0; p->state = ground; }
    if( p->state == fall && top < (ground_y-8) && bottom > (ground_y-8) && p->dy > 0 ) { p->y = ground_y, p->vy = 0; p->state = ground; }
    if( p->state == ground && (t && t->floor ? 0 : 1) ) p->state = fall;

#if 0
    tigrLine(window, p->x-64, top, p->x+64, top, tigrRGB(0xFF, 0xFF, 0xFF));
    tigrLine(window, p->x-64, bottom, p->x+64, bottom, tigrRGB(0xFF, 0xFF, 0xFF));
    tigrLine(window, 0, ground_y, window->w, ground_y, tigrRGB(0xFF, 0xFF, 0xFF));
#endif

    //

    p->dx = p->x - p->px;
    p->dy = p->y - p->py;

    p->px = p->x;
    p->py = p->y;
}


int main() {
    window_create(75., 0);

    vfs_mount("abstract-platformer-370-assets.zip"); // zipfile or directory/with/trailing/slash/

//  tileset_new("PNG/Tiles/Yellow tiles/tileYellow_%02d.png", 1, 27, "fgh#bcdaijklmnopqrstuvwxyze" );
//  tileset_new("PNG/Tiles/Brown tiles/tileBrown_%02d.png", 1, 27, "bcdaefghijklmnopqrstuvwxyz#" );
//  tileset_new("PNG/Tiles/Green tiles/tileGreen_%02d.png", 1, 27, "gh#bcdaijklmnopqrstuvwxyzef" );
    tileset_new("PNG/Tiles/Blue tiles/tileBlue_%02d.png", 1, 27, "gh#bcdaijklmnopqrstuvwxyzef" );
#if 0
    tilemap_new(64+1,64+1,9,3,
        "abcdefghi"
        "jklmnopqr"
        "stuvwxyz#"
    );
#else
    tilemap_new(64,64,10,7,
        "          "
        "          "
        "          "
        "          "
        "      bd  "
        "    bcd#  "
        "  yz####  ", "yzbcd"
    );
#endif

//    camera_t cam = camera();
//    camera_enable(&cam);
//    camera_get_active()->position.z = 10;

    int p1_keys[] = { KEY_A, KEY_D, KEY_W };
    int p2_keys[] = { KEY_J, KEY_L, KEY_I };

    player p1 = player_new( 20, 100, p1_keys, "PNG/Players/Player Grey/playerGrey_walk%d.png");
    p1.debug = 1;
    player p2 = player_new( 80, 100, p2_keys, "PNG/Players/Player Red/playerRed_walk%d.png");

    while( window_swap(NULL) ) {
        if( input(KEY_ESC) ) break;

        fullscreen_rgb_quad( sprites[16], 1.2f );
        tilemap_draw(50, 95); // FLOOR_Y-FLOOR_Y/4);

        static double now = 0;
        float dt = (time_ms() - now);
        now = time_ms();
        // if( input_read(TK_LCONTROL) ) dt /= 2;
        dt /= 1000.f;

        player_move(&p1, dt);
        player_move(&p2, dt);
        player_draw(&p1);
        player_draw(&p2);
    }
}
