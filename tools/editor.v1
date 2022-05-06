// ## editor
// editor = tree of nodes. world, levels and objects are nodes, and even widgets are also nodes.
// you can perform actions on some or all of these nodes, with or without descendants, from any top-bottom or bottom-top directions.
// these actions include load/save, reset, undo/redo, play/render, toggle vis:on/off/alpha logic:on/off/other ddraw:on/off log:on/off, etc.
// and that's all.
//
// ## organization: world as a filesystem
// anything can be serialized into disk. any object, any entity, any property or any widget can be serialized into disk.
// groups of them as well. the whole world state can be serialized into disk as a filesystem snapshot:
// - entities are folders. you can attach nodes on nodes (ie, create folders inside folders).
// - systems are dlls/scripts. you can modify them on the fly and they should reload.
// - components are data files. each component is a file. some components may be pure datas (ie, raw textures) but some others can be human-readable and editable.
//   inside of that, every line is a JSON property that you can tweak, modify or inspect.
//
// ## replication: diffing zips
// the whole world/filesystem will be compressed into a zipfile and delivered to the network when sharding/replicating in a network scenario.
// clients will be diffing/patching their filesystems on every received packet. there will be 3 operations to support internally that
// will reflect what the E/C/S world is doing behind the curtains:
// - added files/folders : when creating entities/components/systems
// - deleted files/folders : when removing entities/components/systems
// - modifying files/folders : when altering entities/components/systems
//
// ## communication: osc messages
// any living entity in the game, or within the editor, can be inspected, debugged or tweaked from external tools.
// in order to achieve that, an opensoundserver is listening on a binding IP and you can send UDP packets to every node in the world.
//
// ## widgets escalate from bottom
// there are only a few basic supplied widgets.
// and they correlate to C types: bool, u/int 8/16/32/64, float/double, strings and enums.
// structs are covered by reflecting and editing all members separately.
// optionally, you can extend some basic types to have better visualization widgets.
// ie, you could alias x4 float widgets together into a new shiny vec4 widget that is more compact, fancy and convenient to use.
// then you can also alias that very same vec4 into a color picker for example; or maybe convert that vec3 into a position gizmo.
// then maybe alias x2 color pickers and create a gradient widget. and so on...

#include "fwk.c"
#include "3rd_fa.h"


int editor_ui_bits(const char *label, uint8_t *enabled) {
    int clicked = 0;
    uint8_t copy = *enabled;

    // @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive
    nk_layout_row_dynamic(ui_ctx, 1, 1);
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    // actual widget: label + 8 checkboxes
    enum { HEIGHT = 18, BITS = 8, SPAN = 118 }; // bits widget below needs at least 118px wide
    nk_layout_row_begin(ui_ctx, NK_STATIC, HEIGHT, 1+BITS);

        int offset = bounds.w > SPAN ? bounds.w - SPAN : 0;
        nk_layout_row_push(ui_ctx, offset);
        if( ui_label_(label, NK_TEXT_LEFT) ) clicked = 1<<31;

        for( int i = 0; i < BITS; ++i ) {
            nk_layout_row_push(ui_ctx, 10);
            // bit
            int val = (*enabled >> i) & 1;
            int chg = nk_checkbox_label(ui_ctx, "", &val);
            *enabled = (*enabled & ~(1 << i)) | ((!!val) << i);
            // tooltip
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, HEIGHT }; // 10:padding,14:width
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_popups_active) {
                const char *tips[BITS] = {"Init","Tick","Draw","Quit","","","",""};
                if(tips[i][0]) nk_tooltipf(ui_ctx, "%s", tips[i]);
            }
        }

    nk_layout_row_end(ui_ctx);
    return clicked | (copy ^ *enabled);
}

typedef struct editor_node {
    void *obj;

    char *type;
    char *url;
    char *options;
    char *editor_options;
    uint8_t editor_bits;

    void (*init)();
    void (*tick)();
    void (*draw)();
    void (*quit)();

    // undo, redo
    void (*edit)();
    void (*save)(); // diff, mend
    void (*load)();

} editor_node;

static map(char*, editor_node*) offline;   // offline representation of world
static map(char*, editor_node*) instanced; // live representation of world
// selections
static char *objname = 0;
static editor_node *objnode = 0;
static int editor_playing = 1;

void editor_create() {
    do_once map_init(offline, less_str, hash_str);
    map_clear(offline);
}

editor_node* editor_add(const char *scene_path) {
    editor_node **found = map_find(offline, (char*)scene_path);
    if (!found) {
        found = map_insert(offline, (char*)scene_path, CALLOC(1, sizeof(editor_node)));
    }
    return *found;
}

void editor_action(unsigned action, const char *group_mask) {
    for each_map(offline, char*, key, editor_node*, objptr) {
        unsigned bits = objptr->editor_bits;
        if(objptr->init) if( (bits & action) & 1 ) objptr->init(objptr->obj);
        if(objptr->tick) if( (bits & action) & 2 ) objptr->tick(objptr->obj);
        if(objptr->draw) if( (bits & action) & 4 ) objptr->draw(objptr->obj);
        if(objptr->quit) if( (bits & action) & 8 ) objptr->quit(objptr->obj);
    }
}

void editor2() {
    // draw menu
#if 1
    if( ui_menu( ICON_BARS ";New Project;" ICON_TEST_GLYPH "Open Project;-Save Project;Settings;-Quit") ) printf("Selected File.%d\n", ui_item());
        if( ui_menu( ICON_FA_FOLDER_OPEN ));
        if( ui_menu( editor_playing ? ICON_FA_PAUSE : ICON_FA_PLAY )) editor_playing ^= 1;
        if( ui_menu( ICON_FA_STOP )) editor_playing = 0;
    if( ui_menu( va(ICON_FA_DATABASE " %s", xstats() ))); // 012/136MB
    if( ui_menu( va(ICON_FA_CUBE " %d", map_count(offline) )));
#endif
    if( ui_menu("Edit;Cut;Copy;Paste") ) printf("Selected Edit.%d\n", ui_item());
    if( ui_menu("Play;Init All;Tick All;Draw All;Quit All") ) {
        int action = ui_item();
        if( action == 1 ) editor_action(1, "*");
        if( action == 2 ) editor_action(2, "*");
        if( action == 3 ) editor_action(4, "*");
        if( action == 4 ) editor_action(8, "*");
    }

    // tick offline
    for each_map(offline, char*, key, editor_node*, node) {
        if(!node->editor_options) {
            strcatf(&node->editor_options, "%s", "(init)");
            if(node->init) node->init(node->obj);
        }
//        if(node->tick) node->tick(node->obj);
    }

    editor_action(2, "*"); // tick all

    // draw offline
    if( objnode && objnode->draw ) {
        fx_enable(0, 1);
        fx_begin();
        objnode->draw(objnode->obj);
        sprite_flush();
        fx_end();
        fx_enable(0, 0);
    }

    editor_action(4, "*"); // draw all

    if(0)
    for each_map(offline, char*, key, editor_node*, node) {
        if( node->draw ) {
            node->draw(node->obj);
        }
    }

    if( ui_begin("Scene", 0) ) {
        static char filter[128] = {0};
        ui_string("Filter", filter, 128);

        ui_separator();

        // node selector
        for each_map(offline, char*, key, editor_node*, ptr) {
            // exclude non-filtered
            if( filter[0] && !strstri(key, filter)) {
                continue;
            }

            // update selection
            unsigned bits_changed = editor_ui_bits(key, &ptr->editor_bits);
            unsigned label_clicked = bits_changed & (1<<31);
            if( bits_changed ) {
                objname = NULL;
                objnode = NULL;
                if( ptr->editor_bits || label_clicked ) {
                    objname = key;
                    objnode = ptr;
                }
            }
        }

        ui_separator();

        // edit selected node
        if( objname ) {
            ui_label( objname );
            if( objnode && objnode->edit ) objnode->edit(objnode->obj);
        }

        ui_end();
    }
}

// -----------------------------------------------------------------------------

typedef struct cat {
    char name[16];
    int x, y, cat;
    // private:
    texture_t tx;
    float vx, vy, elapsed;
} cat;

void cat_init( cat *c ) {
    c->tx = texture("cat.png", 0);
}
void cat_quit( cat *c ) {
    c->tx = texture_checker();
}
void cat_tick( cat *c ) {
    c->elapsed += 1/60.f;
}
void cat_draw( cat *c ) {
    float anim_speed = 1;
    double e = c->elapsed * anim_speed;
    double frame_num = c->cat * 8 + floor( ((int)(e * 8)) % 4 );
    frame_num = c->vx != 0 || c->vy != 0 ? frame_num + 4 : frame_num;

    float scale[2] = {2, 2};
    float position[3] = { c->x,c->y,c->y }, no_offset[2] = {0,0}, spritesheet[3] = { frame_num,8,4 };
    sprite(c->tx,
        position, 0,                // position(x,y,depth: sort by Y), angle
        no_offset, scale,           // offset(x,y), scale(x,y)
        0,~0u,                      // is_additive, tint color
        spritesheet                 // frame_number in a 8x4 spritesheet
    );
}
void cat_edit( cat *c ) {
    ui_int("x", &c->x);
    ui_int("y", &c->y);
    ui_int("cat", &c->cat);
    c->cat = abs(c->cat) % 4;
}


int main() {
    // 75% window, MSAAx8 flag
    window_create(75, WINDOW_MSAA8);
    window_title("editor");
    window_icon("logo.png");

    // config 2d camera
    camera_t cam = camera();
    cam.position = vec3(window_width()/2,window_height()/2,1);
    camera_enable(&cam);

    editor_create();

    fx_load("fxOutline.fs");

    // gameobjs
    cat cat1 = { "cat1", 600, 400, 0 }; // 8x4 spritesheet
    cat cat2 = { "cat2", 700, 400, 1 }; // 8x4 spritesheet
    cat cat3 = { "cat2", 800, 400, 2 }; // 8x4 spritesheet

    // tell editor to monitor cat1
    editor_node *node1 = editor_add("/cat-gray");
    node1->obj = &cat1; // instance...
    node1->tick = cat_tick; //...and type/class
    node1->draw = cat_draw;
    node1->edit = cat_edit;
    node1->init = cat_init;
    node1->quit = cat_quit;

    // tell editor to monitor cat2
    editor_node *node2 = editor_add("/cat-brown");
    *node2 = *node1; // type...
    node2->obj = &cat2; // ...and instance

    // tell editor to monitor cat3
    editor_node *node3 = editor_add("/cat-white");
    *node3 = *node1; // type...
    node3->obj = &cat3; // ...and instance

#if 0
    // config 3d model
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    float girl_frame = 0;
    mat44 girl_pivot; id44(girl_pivot);
    rotationq44(girl_pivot, eulerq(vec3(0,0,0)));
    scale44(girl_pivot, 2,2,2);

    // config & play music
    audio_t music = audio_stream("larry.mid"); // 'wrath_of_the_djinn.xm'
    //audio_play(music, 0);

    // config 3d camera
    camera_t cam3d = camera();
    // camera_enable(&cam3d);
#endif

    // editor loop
    while( window_swap() ) {
        // editor tick
        editor2();

#if 0
       // fps camera
       bool grabbed = input(MOUSE_L) || input(MOUSE_R);
       window_cursor( ui_active() || ui_hover() ? true : !grabbed );
       if( !window_has_cursor() ) {
          vec3 wasdec3 = vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)),input(KEY_W)-input(KEY_S));
          vec2 look2 = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2 * !window_has_cursor());
          vec3 move3 = scale3(wasdec3, cam.speed * !window_has_cursor());
          camera_move(&cam3d, wasdec3.x,wasdec3.y,wasdec3.z);
          camera_fps(&cam3d, look2.x,look2.y);
       }

       // draw grid/axis
       ddraw_grid(0);
       ddraw_flush();

       // animate girl
       float delta = window_delta() * 30; // 30fps anim
       if( editor_playing ) girl_frame = model_animate(girl, girl_frame + delta);

       // draw girl
       model_render(girl, cam3d.proj, cam3d.view, girl_pivot, 0);

       // showcase ui
       if(ui_begin("test", 0)) {
          ui_end();
       }
#endif
    }
}
