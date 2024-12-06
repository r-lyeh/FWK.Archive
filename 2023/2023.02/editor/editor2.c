/*
<!-- editor = tree of nodes. levels and objects are nodes, and widgets are also nodes -->
<!-- you can perform actions on nodes, with or without descendants, top-bottom or bottom-top -->
<!-- operations include load/save, reset, play/render, etc -->
<!-- level is saved to disk as a filesystem layout -->

- [ ] Editor: Gizmos✱, scene tree, property editor✱, load/save✱, undo/redo✱, copy/paste, on/off (vis,tick,ddraw,log), vcs. 
- [ ] Editor: Scenenode pass: node singleton display, node console, node labels, node outlines✱.<!-- node == gameobj ? -->
- [ ] Editor: Debug pass: toggles on/off (billboards✱, materials, un/lit, cast shadows, wireframe, skybox✱/mie✱, fog/atmosphere, collide✱, physics).
- [ ] Editor: Level pass: volumes, triggers, platforms, level streaming.
- [ ] Editor: Edit pass: Procedural content, brushes, noise and CSG.
- [ ] Editor: GUI pass: timeline and data tracks, node graphs. <!-- worthy: will be reused into materials, animgraphs and blueprints -->
*/

#define COOK_ON_DEMAND 1
#define FWK_C
#include "fwk.c"
#include "editor2.h" // old editor interface

// ----------------------------------------------------------------------------------------

#define expr expr2 // 3rd_lua.h
#include "3rd_eval.h"

// ----------------------------------------------------------------------------------------

#include "labs.meta/meta_reflect.c"

void reflect_ui( const reflect *r, void *value, void *userdata ) {
    /**/ if( !strcmp(r->type, "int") )               ui_int(r->info, (int*)value);
    else if( !strcmp(r->type, "char") && r->is_ptr ) ui_string(r->info, (char*)value, strlen((char*)value)+1);
    else if( !strcmp(r->type, "string") )            ui_stringf(r->info, (char**)value);
    else if( !strcmp(r->type, "float") )             ui_float(r->info, (float*)value);
    else if( !strcmp(r->type, "double") )            ui_double(r->info, (double*)value);
    else if( !strcmp(r->type, "unsigned") )          ui_unsigned(r->info, (unsigned*)value);
    else if( !strcmp(r->type, "color") )             ui_color4(va("%s #%02X%02X%02X%02X", r->info, (int)(0[(float*)value]),(int)(1[(float*)value]),(int)(2[(float*)value]),(int)(3[(float*)value])), (float*)value);
    // else if( !strcmp(type, "vec3") ) ; // not supported. decays to 3 floats
    else ui_label2(r->info, va("(%s)%s", r->type, r->name));
}
bool reflect_parse(void *obj, const char *type, const char *val) {
    /**/ if( !strcmp(type, "int") )      *((int*)obj) = eval(val);
//  else if( !strcmp(r->type, "char") && r->is_ptr ) ; // @fixme: not supported, unless we do strncpy() or similar.
    else if( !strcmp(type, "string") )   *((char**)obj) = stringf("%s", val);
    else if( !strcmp(type, "float") )    *((float*)obj) = eval(val); // = v[0] == '~' ? (float)~atoi(val+1) : atof(val); // = atof(val);
    else if( !strcmp(type, "double") )   *((double*)obj) = eval(val); // = v[0] == '~' ? (float)~atoi(val+1) : atof(val); // = atof(val);
    else if( !strcmp(type, "unsigned") ) *((unsigned*)obj) = eval(val); // = v[0] == '~' ? (float)~atoi(val+1) : atof(val); // = atof(val);
    else if( !strcmp(type, "color") )    *(((float*)obj)+3) = 255, sscanf(val, "%f %f %f %f", ((float*)obj)+0, ((float*)obj)+1, ((float*)obj)+2, ((float*)obj)+3);
    else if( !strcmp(type, "vec3") )     sscanf(val, "%f %f %f", ((float*)obj)+0, ((float*)obj)+1, ((float*)obj)+2);
    else return 0;
    return 1;    
}

// ----------------------------------------------------------------------------------------

typedef void(*obj_ctor)(void*);
static map(char*, obj_ctor) obj_ctors;

#define STRUCT_CTOR(type, ctor) STRUCT_CTOR(#type, (obj_ctor)ctor)
void (STRUCT_CTOR)( const char *type, obj_ctor ctor ) {
    do_once map_init_str(obj_ctors);
    map_find_or_add(obj_ctors, STRDUP(type), ctor);
}

bool obj_make(void *obj, const char *ini_data) { // initialize object from ini datas
    char *hint = 0;

    for( ini_t read = ini_from_mem(ini_data); !!read; map_free(read), read = 0) {
        for each_map(read, char*, k, char*, v) {
            array(char*) tokens = strsplit(k, ".");
            if( array_count(tokens) != 2 ) continue;

            const char *type = 0;
            void *found = reflect_field( tokens[0], obj, tokens[1], &type );
            if( !found ) continue;

            if( reflect_parse(found, type, v) ) {
                hint = tokens[0];                
            }
        }

        // constructor (post-init call)
        obj_ctor *ctor = map_find(obj_ctors, hint);
        if( ctor ) (*ctor)( obj );
    }

    return hint != 0;
}

// ----------------------------------------------------------------------------

#define POD_TYPES \
    vec2i v2i; \
    vec2 v2; \
    vec3 v3; \
    vec4 v4; \
    quat q; \
    char *s; \
    double d; \
    int64_t i; \
    unsigned color; \
    void *ptr; \
    void (*fun)(void*); \
    char *nametype; /* "name\0type" */

typedef union pod {
    POD_TYPES
} pod;

typedef union var {
    POD_TYPES
    array(pod)   array;
    array(pod)   pair;  // array of 2: first,second. maybe used as key,value
    array(pod)   tuple; // array of N: first,second,third... maybe used as array of N*2: NAME1,val1,NAME2,val2, ...
    map(pod,pod) map;
    set(pod)     set;
} var;

typedef struct node {
    var k;
    var v;

    struct node *up;
    struct node *next;
    struct node *down;
} node;

char* node_name(node *n) {
    return n->k.nametype;
}
char* node_type(node *n) {
    return n->k.nametype + strlen(n->k.nametype) + 1;
}
char* node_set_nametype(node *n, const char *name, const char *type) {
    *strchr(n->k.nametype = stringf("%s\1%s", name, type), '\1') = '\0'; // @leak
    return n->k.nametype;
}
node* node_find(node *n, const char *path) {
    if (!n->k.nametype) return 0;
    if( !strcmp(node_name(n), path) ) return n;
    node *r = 0;
    if( n->next       ) r = node_find(n->next, path);
    if( n->down && !r ) if( !strcmp(node_name(n->down), path) ) return n->down; // r = node_find(n->down, path);
    return r;
}
node* node_find_recurse(node *n, const char *path) {
    array(char*) split = strsplit(path, "/");
    while( n && array_count(split) ) {
        n = node_find(n, split[0]);
        array_pop(split);
    }
    return n;
}
node* node_attach_sibling(node *n, node *sibling) {
    while( n->next ) n = n->next;
    return n->next = sibling;
}
node* node_attach_child(node *n, node *child) {
    child->up = n;
    if( n->down ) return node_attach_sibling(n->down, child);
    return n->down = child;
}
unsigned node_children(node *n) {
    unsigned c = 0;
    if( n->down ) {
        n = n->down;
        do ++c; while( (n = n->next) );
    }
    return c;
}
unsigned node_siblings(node *n) {
    return n->up ? node_children( n->up->down ) : 0;
}
void node_init(node *n) {
    profile_incstat("Editor.inits", +1);

    if(n->next) node_init(n->next);
    if(n->down) node_init(n->down);
}
void node_tick(node *n) {
    profile_incstat("Editor.ticks", +1);

    if(n->next) node_tick(n->next);
    if(n->down) node_tick(n->down);
}
void node_draw(node *n) {
    profile_incstat("Editor.draws", +1);

    if(n->next) node_draw(n->next);
    if(n->down) node_draw(n->down);
}
void node_quit(node *n) {
    profile_incstat("Editor.quits", +1);

    if(n->next) node_quit(n->next);
    if(n->down) node_quit(n->down);
}
void node_edit(node *n, node *root) {
    profile_incstat("Editor.edits", +1);

    if( ui_collapse(va("%s %s (%u)", n->down ? ICON_MD_SOURCE : ICON_MD_FOLDER, node_name(n), node_children(n)), va("%p%p",root,n->v.ptr)) ) { // @fixme v.ptr
        int open = 0;
        if( n->down && n->v.ptr && ui_collapse(ICON_MD_FOLDER " Properties", va("%p%p!",root,n->v.ptr))) open = 1;
        if( open || !n->down ) { ui_section(node_type(n));/**/ reflect_iterate_fields( node_type(n), n->v.ptr, reflect_ui, NULL ); } // @fixme v.ptr
        if( open ) ui_collapse_end();

        if( n->down ) node_edit(n->down,root);

        ui_collapse_end();
    }

    if(n->next) node_edit(n->next,root);
}

// ---

struct editor_t {
    unsigned frame;
    // root nodes
    node init;
    node  tick;
    node  draw;
    node  edit;
    node quit;
} editor;

enum { EDITOR_BUCKETS = 5 }; // init+tick+draw+edit+quit

void editor_reset() {
    node_quit(&editor.quit);
    editor.frame = 0;
}
void editor_frame() {
    editor_init(); // old editor interface
    editor_tick(); // old editor interface
    editor_menubar(); // old editor interface

    if( input_down(KEY_F5) ) {
        editor_reset();
    }
    if( editor.frame++ == 0 ) {
        node_init(&editor.init);
    }
    node_tick(&editor.tick);
    node_draw(&editor.draw);

    // content browser
    if( ui_window("File Browser", 0) ) {
        const char *file = 0;
        if( ui_browse(&file, NULL) ) {
            os_exec(va("%s %s", ifdef(win32, "start", ifdef(osx, "open", "xdg-open")), file));
        }
        ui_window_end();
    }

    // console/terminal window
    if( 0 && ui_window("Console", 0) ) { // @fixme half-working
        ui_console();
        ui_window_end();
    }

    if( ui_window("Outliner", 0) ) {

        static int do_filter = 0;
        int choice = ui_toolbar(ICON_MD_SEARCH"@Filter;"ICON_MD_UPLOAD"@Load;"ICON_MD_DOWNLOAD"@Save;"ICON_MD_REFRESH"@Reset;"ICON_MD_UNDO"@Undo;"ICON_MD_REDO"@Redo;");
        if( choice == 1 ) do_filter ^= 1;
        static char *filter = 0;
        if( do_filter ) {
            ui_stringf(ICON_MD_SEARCH " Filter", &filter);
        } else {
            if( filter ) filter[0] = '\0';
        }
        char *filter_mask = filter && filter[0] ? va("*%s*", filter) : "*";


        node_edit(&editor.edit,&editor.edit);
        node_edit(&editor.draw,&editor.draw);
        ui_window_end();
    }
}

unsigned editor_spawn(const char *path_id, const char *keytype, void *val) {
    do_once {
        node_set_nametype(&editor.init, "Init Group", "init_nodes"); // @leak
        node_set_nametype(&editor.tick, "Tick Group", "tick_nodes"); // @leak
        node_set_nametype(&editor.draw, "Draw Group", "draw_nodes"); // @leak
        node_set_nametype(&editor.edit, "Edit Group", "edit_nodes"); // @leak
        node_set_nametype(&editor.quit, "Quit Group", "quit_nodes"); // @leak
    }

    array(char*) tokens = strsplit(path_id, "/");
    char *keyname = *array_back(tokens);

        array(node) n = 0;
        array_resize(n, EDITOR_BUCKETS);
        for( int i = 0; i < EDITOR_BUCKETS; ++i ) {
            node_set_nametype(&n[i], keyname, keytype); // @leak
            n[i].v.ptr = val;        
        }

    array_pop(tokens);
    char *joint = array_count(tokens) ? strjoin(tokens, "/") : "/";

    node *p = 0;
    p = node_find_recurse(&editor.init, joint), node_attach_child(p ? p : &editor.init, n+0);
    p = node_find_recurse(&editor.tick, joint), node_attach_child(p ? p : &editor.tick, n+1);
    p = node_find_recurse(&editor.draw, joint), node_attach_child(p ? p : &editor.draw, n+2);
    p = node_find_recurse(&editor.edit, joint), node_attach_child(p ? p : &editor.edit, n+3);
    p = node_find_recurse(&editor.quit, joint), node_attach_child(p ? p : &editor.quit, n+4);
    return 0;
}

// demo -----------------------------------------------------------------------

typedef struct my_sprite {
    char *filename;
    vec3 position;
    float tilt;
    vec4 tint;
    // --- private
    texture_t texture_;
    unsigned bgra_;
} my_sprite;

void my_sprite_ctor(my_sprite *obj) {
    obj->texture_ = texture(obj->filename, TEXTURE_RGBA);
    obj->bgra_ = bgraf( obj->tint.r/255.0, obj->tint.g/255.0, obj->tint.b/255.0, obj->tint.a/255.0 );
}
void my_sprite_draw(my_sprite *obj) {
    obj->bgra_ = bgraf( obj->tint.r/255.0, obj->tint.g/255.0, obj->tint.b/255.0, obj->tint.a/255.0 ); // @fixme: del me
    sprite( obj->texture_, &(obj->position.x), obj->tilt, obj->bgra_ );
}

int main() {
    typedef char* string;
    typedef vec4 color;

    STRUCT( vec3, float, x, "X" );
    STRUCT( vec3, float, y, "Y" );
    STRUCT( vec3, float, z, "Z" );

//  STRUCT( texture_t, unsigned, flags, "Flags");
//  STRUCT( texture_t, string, filename, "Filename");

    STRUCT( my_sprite, string, filename, "Filename" );
    STRUCT( my_sprite, vec3, position, "Position" );
    STRUCT( my_sprite, float, tilt, "Tilt degrees" );
    STRUCT( my_sprite, color, tint, "Tint color" );
    STRUCT_CTOR( my_sprite, my_sprite_ctor );

    PRINTF("pod:%d, var:%d, node:%d warn\n", (int)sizeof(pod), (int)sizeof(var), (int)sizeof(node));
    PRINTF("reflected:%d bytes vs real:%d bytes warn\n", reflect_sizeof("my_sprite"), (int)sizeof(my_sprite));

    // cook_config("../../tools/cook.ini");
    window_create(0.80, 0);

    struct my_sprite spr1 = {0}, spr2 = {0}, spr3 = {0};
        obj_make(&spr1,
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=5 2 100\n"
            "tilt=45 + 45 -90\n"
            "tint=255 255 0\n"
        );
        obj_make(&spr2,
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1 2 100\n"
            "tilt=45 + 45 -90\n"
            "tint=255 0 0\n"
        );
        obj_make(&spr3,
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1 2 100\n"
            "tilt=45\n"
            "tint=0 0 255\n"
        );
    int hero1 = editor_spawn("/hero1", "my_sprite", &spr1);
    int hero2 = editor_spawn("/hero2", "my_sprite", &spr2);
    int hero3 = editor_spawn("/hero1/heroB", "my_sprite", &spr3);

    camera_t cam = camera();
    camera_enable(&cam);

    while( window_swap() ) {
        editor_frame();

        // @fixme: this should be drawn by game, not editor!
        my_sprite_draw(&spr1);
        my_sprite_draw(&spr2);
        my_sprite_draw(&spr3);
        // spr1.tilt = 5 * sin(time_ss());
    }
}
