// tiled loader
// - rlyeh, public domain.

#include "fwk.h"
#define tiled_t tiled_t2
#define tiled tiled2
#define tiled_render tiled_render2
#define tiled_ui tiled_ui2

typedef struct tiled_t {
    char *map_name;
    unsigned first_gid, tilew, tileh, w, h;

    bool parallax;
    vec3 position;
    array(bool) visible;
    array(tilemap_t) layers;
    array(tileset_t) sets;
    array(char*) names;
} tiled_t;

tiled_t tiled(const char *file_tmx) {
    tiled_t zero = {0}, ti = zero;

    // read file and parse json
    if( !xml_push(file_tmx) ) return zero;

    // sanity checks
    bool supported = !strcmp(xml_string("/map/@orientation"), "orthogonal") && !strcmp(xml_string("/map/@renderorder"), "right-down");
    if( !supported ) return xml_pop(), zero;

    // tileset
    const char *file_tsx = xml_string("/map/tileset/@source");
    if( !xml_push(vfs_read(file_tsx)) ) return zero;
        const char *set_src = xml_string("/tileset/image/@source");
        int set_w = xml_int("/tileset/@tilewidth");
        int set_h = xml_int("/tileset/@tileheight");
        int set_c = xml_int("/tileset/@columns");
        int set_r = xml_int("/tileset/@tilecount") / set_c;
        tileset_t set = tileset(texture(set_src,0), set_w, set_h, set_c, set_r );
    xml_pop();

    // actual parsing
    ti.w = xml_int("/map/@width");
    ti.h = xml_int("/map/@height");
    ti.tilew = xml_int("/map/@tilewidth");
    ti.tileh = xml_int("/map/@tileheight");
    ti.first_gid = xml_int("/map/tileset/@firstgid");
    ti.map_name = STRDUP( xml_string("/map/tileset/@source") ); // @leak

    for(int l = 0, layers = xml_count("/map/layer"); l < layers; ++l ) {
        if( strcmp(xml_string("/map/layer[%d]/data/@encoding",l), "base64") || strcmp(xml_string("/map/layer[%d]/data/@compression",l), "zlib") ) {
            PRINTF("Warning: layer encoding not supported: '%s' -> layer '%s'\n", file_tmx, *array_back(ti.names));
            continue;
        }

        int cols = xml_int("/map/layer[%d]/@width",l);
        int rows = xml_int("/map/layer[%d]/@height",l);

        tilemap_t tm = tilemap("", ' ', '\n');
        tm.blank_chr = ~0u; //ti.first_gid - 1;
        tm.cols = cols;
        tm.rows = rows;
        array_resize(tm.map, tm.cols * tm.rows);
        memset(tm.map, 0xFF, tm.cols * tm.rows * sizeof(int));

        for( int c = 0, chunks = xml_count("/map/layer[%d]/data/chunk", l); c <= chunks; ++c ) {
            int cw, ch;
            int cx, cy;
            array(char) b64 = 0;

            if( !chunks ) { // non-infinite mode
                b64 = xml_blob("/map/layer[%d]/data/$",l);
                cw = tm.cols, ch = tm.rows;
                cx = 0, cy = 0;
            } else { // infinite mode
                b64 = xml_blob("/map/layer[%d]/data/chunk[%d]/$",l,c);
                cw = xml_int("/map/layer[%d]/data/chunk[%d]/@width",l,c), ch = xml_int("/map/layer[%d]/data/chunk[%d]/@height",l,c); // 20x20
                cx = xml_int("/map/layer[%d]/data/chunk[%d]/@x",l,c), cy = xml_int("/map/layer[%d]/data/chunk[%d]/@y",l,c); // (-16,-32)
                cx = abs(cx), cy = abs(cy);
            }

            int outlen = cw * ch * 4;
            static __thread int *out = 0; out = (int *)REALLOC( 0, outlen + zexcess(COMPRESS_ZLIB) ); // @leak
            if( zdecode( out, outlen, b64, array_count(b64), COMPRESS_ZLIB ) > 0 ) {
                for( int y = 0, p = 0; y < ch; ++y ) {
                    for( int x = 0; x < cw; ++x, ++p ) {
                        if( out[p] >= ti.first_gid ) {
                            int offset = (x + cx) + (y + cy) * tm.cols;
                            if( offset >= 0 && offset < (cw * ch) )
                            tm.map[ offset ] = out[ p ] - ti.first_gid;
                        }
                    }
                }
            }
            else {
                PRINTF("Warning: bad zlib stream: '%s' -> layer #%d -> chunk #%d\n", file_tmx, l, c);
            }

            array_free(b64);
        }

        array_push(ti.layers, tm);
        array_push(ti.names, STRDUP(xml_string("/map/layer[%d]/@name",l)));
        array_push(ti.visible, true);
        array_push(ti.sets, set);
    }

    xml_pop();
    return ti;
}

void tiled_render(tiled_t tmx) {
    for( unsigned i = 0, end = array_count(tmx.layers); i < end; ++i ) {
        tmx.layers[i].position = camera_get_active()->position;
        if( tmx.parallax ) tmx.layers[i].position.x /= (3+i), tmx.layers[i].position.y /= (5+i);
        if( tmx.visible[i] ) tilemap_render(tmx.layers[i], tmx.sets[i]);
    }
}

void tiled_ui(tiled_t *p) {
    tiled_t t = *p;
    ui_label2("Loaded map", t.map_name ? t.map_name : "(none)");
    ui_label2("Map dimensions", va("%dx%d", t.w, t.h));
    ui_label2("Tile dimensions", va("%dx%d", t.tilew, t.tileh));
    ui_separator();
    ui_bool("Parallax", &p->parallax);
    ui_separator();
    ui_label2("Layers", va("%d", array_count(t.layers)));
    for( int i = 0; i < array_count(t.layers); ++i ) {
        if( ui_label2_toolbar(va("- %s (%dx%d)", t.names[i], t.layers[i].cols, t.layers[i].rows ), t.visible[i] ? "\xee\xa3\xb4" : "\xee\xa3\xb5") > 0 ) { // ICON_MD_VISIBILITY / ICON_MD_VISIBILITY_OFF
            p->visible[i] ^= true;
        }
    }
    ui_separator();
    if( ui_collapse(va("Sets: %d", array_count(t.layers)), va("%p", p))) {
        for( int i = 0; i < array_count(t.layers); ++i ) {
            if( ui_collapse(va("%d", i+1), va("%p%d",p,i)) ) {
                p->sets[i].selected = tileset_ui( t.sets[i] );
                ui_collapse_end();
            }
        }
        ui_collapse_end();
    }
}

int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    cam.position = vec3(0,0,3); // at(0,0) zoom(x3)
    camera_enable(&cam);

    tiled_t tmx = {0}; // = tiled(vfs_read("castle.tmx"));

    while( window_swap() ) {
        // camera panning (x,y) & zooming (z)
        if( !ui_hover() && !ui_active() ) {
            if( input(MOUSE_L) ) cam.position.x += input_diff(MOUSE_X);
            if( input(MOUSE_L) ) cam.position.y += input_diff(MOUSE_Y);
            cam.position.z += input_diff(MOUSE_W) * 0.1;
        }

        // render
        tiled_render(tmx);

        // ui
        if( ui_panel("Tiled", 0) ) {
            if( ui_button("load") ) {
                tmx = tiled(vfs_read("castle.tmx"));
            } 
            ui_separator();
            ui_float("Zoom in", &cam.position.z);
            tiled_ui(&tmx);
            ui_panel_end();
        }
    }
}
