// ## editor (v1)
// The v1 editor is a tool that understands Assets and is able to *edit the details of such Assets*. 
// This understanding is configured via reflection fields in .ini files. Can be reflected from C as well.
// The reflected properties will enable loading, saving and creating generic widget views to edit the Assets.
// Because we load and save the state of Assets, we can also undo and redo changes by simulating load/saves from/into memory. 
// And we can also dump the contents into disk, and create diffs and patches from them.
// - [x] Load Assets
// - [x] Edit Assets
// - [x] Save Assets
// - [x] Undo Assets (automatic, via loading older state)
// - [x] Redo Assets (automatic, via loading newer state)
// - [ ] Diff Assets (from two states)
// - [ ] Mend Assets (from two states)
//
// Lastly, the editor would be data driven; ie, be able to parse & interpret text commands from any input stream. 
// This would allow for remote control (via OSC), extending scripts, offline commands, Telnet sessions or external tools; like GDB does.
// - [ ] Remote control
//
// Note that the editor is dumb and does not tick/draw your GameObjects. It does not handle the Scene/World either. 
// Those are game-driven systems. Your game should provide the meanings to actually:
// - [?] Spawn Assets
// - [?] Delete Assets
// - [?] Draw Assets 
// - [?] Tick Assets
// - [?] Scenes (parent->children, visibility, collisions and such)
//
// Your game could use fancy containers everywhere. However, for simplicity purposes, Editor would be ignorant about them as well.
// Editor can only use containers that can decay to vectors and strides. Examples:
// - [x] Vectors (Supported in the Editor)
// - [?] Arrays (Could be processed as a fixed inmutable vector)
// - [?] Sparse/Unordered/Ordered Sets (Could be processed as vector of values)
// - [?] Sparse/Unordered/Ordered Maps (Could be processed as vector of keys + vector of values)
// - [?] Other fancy containers (Could iterate elements on this and send each item to editor individually; might be slow)
//
// PS: Asset versioning is external to the editor. We should add a few plugins within the editor: p4, git, svn, ...
// PS: Asset pipeline is external to the editor. Exotic assets could use some fancy plugins to deal with the import/export; eg, Substance 3D.
//
// ## editor (v2)
// The v2 editor would be a graphical update from previous editor. It will become a window manager shortly.
// We allow here for others to extend or *override the behavior and look of each window* by using .lua and .dll plugins. This editor does v1 and also:
// - [ ] Draw Windows              --> Custom overrides to alter or enhance the renderer while editing. Via C/C++/dll/lua plugins
// - [ ] Tick Windows              --> Custom overrides to alter or enhance the behavior while editing. Via C/C++/dll/lua plugins
// - [ ] Draw Assets while editing --> Custom overrides to alter or enhance the renderer while editing. Via C/C++/dll/lua plugins
// - [ ] Tick Assets while editing --> Custom overrides to alter or enhance the behavior while editing. Via C/C++/dll/lua plugins
//
// ## editor (v3)
// The v3 editor is a bootstrapped v2 editor with tons of .luas and .dlls. The C skeleton is only a window manager at this point.
// The intention here is to *leverage editing workflow purely into data-driven files*, so the engine can grow up exponentially from here.
// Data-driven on steroids. It would be totally a success if the editor could be bootstrapped to include these kind of sub-editors without much work on the C codebase:
// - Level 2D/Blockout editor
// - Level 3D/Blockout editor
// - World outliner [*]
// - Nodegraph editor (ShaderGraph, AnimGraph, AudioGraph, MaterialGraph, DialogGraph, AIGraph, QuestGraph, ...)
// - Sequencer
// - Tracker (music tracker)
// - Etc...
//
// ## editor (v4)
// Go social & marketplace. Allow others to expand, share, publish, subscribe, discuss their sub-editors within a small community.
// I really like the way the way OpenFrameworks.cc does their addons, and I think we should do same: just discover and monitor github repos, and list everything on a website (fwk- prefix?).
// Wishlist for a github-based community flow: discovery, transparent installs, publish on github, star there, watch commits & releases, track issues+discussions, etc
//
// We should have a generic, extensible, script/plugin-driven, working editor at this point (hopefully) that does not require maintenance.
//
// to be continued...

#include "fwk.c"

#if 1
#define EDITOR_PRINTF PRINTF
#else
#define EDITOR_PRINTF(...) do {} while(0)
#endif

typedef void*        obj_t;
typedef array(obj_t) objs_t;


typedef struct property { // meta: "vec3 namespace.position = {1,2,3}; // minv=(0,0,0) key1=value1 key2=value2 [...] @this is a tooltip @@this is a comment"
    char *mark; // namespace
    char *name; // display name
    char *type; // pointed type
    char *hint; // tooltip
    char *minv; // min value
    char *maxv; // max value
    char *incv; // inc value
    char *defv; // default value
    char *isro; // is read-only/enabled
    char *issv; // is save pending
    void *value;
    unsigned typebits;
    unsigned flags;
} property;


// low-level operations
int save1(bool apply, array(char) *buffer, array(property) arrp) {
    // iterate and save
    unsigned total = 0;
    for each_array_ptr(arrp, property, p) {
        unsigned bytes = 0;
        /**/ if( p->type[0] == 'f') bytes = sizeof(float);
        else if( p->type[0] == 'v') bytes = sizeof(vec3);
        else if( p->type[0] == 'i') bytes = sizeof(int);
        else if( p->type[0] == 'b') bytes = sizeof(bool);

        if( !apply ) continue;

        if( bytes ) {
            array_resize(*buffer, array_count(*buffer) + bytes);
            memcpy( &(*buffer)[array_count(*buffer) - bytes], p->value, bytes);
            total += bytes;
        }
    }

    EDITOR_PRINTF("%d bytes written\n", total);
    return total;
}
int load1(bool apply, const char *buffer, unsigned buflen, array(property) arrp, unsigned skip_bytes) {
    // iterate and load properties
    unsigned cursor = 0, loaded = 0, limit = buflen;

    while( cursor <= skip_bytes )
    for each_array_ptr(arrp, property, p) {
        unsigned bytes = 0;
        /**/ if( p->type[0] == 'f') bytes = sizeof(float);
        else if( p->type[0] == 'v') bytes = sizeof(vec3);
        else if( p->type[0] == 'i') bytes = sizeof(int);
        else if( p->type[0] == 'b') bytes = sizeof(bool);

        if( (cursor + bytes) > limit ) {
            return -1;
        }

        if( apply && cursor >= skip_bytes ) {
        memcpy( p->value, &buffer[cursor], bytes);
        loaded += bytes;
        }

        cursor += bytes;
    }

    EDITOR_PRINTF("%d bytes read, %d bytes loaded\n", cursor, loaded);
    return cursor;
}

// syntax sugars for collections/containers

#define bulk_load(obj_min,obj_max,objs,...)  for( unsigned i = 0; i < array_count(objs); ++i ) { bool apply = obj_min >= i && i < obj_max; load1(apply, objs[i], __VA_ARGS__); }
#define bulk_save(obj_min,obj_max,objs,...)  for( unsigned i = 0; i < array_count(objs); ++i ) { bool apply = obj_min >= i && i < obj_max; save1(apply, objs[i], __VA_ARGS__); }


// state - retained mode

typedef struct edit_state {
    array(property) properties;
    array(char)     buffer;
    array(vec2)     history;
    unsigned        cursor;
} edit_state;

static map(void*, edit_state) editor_state; 

void editor_init() {
    do_once map_init(editor_state, less_ptr, hash_ptr);
}

// bool editor_clear_redo(void *obj) {}

bool file_overwrite( const char *file, const void *ptr, int len ) {
    bool ok = 0;
    if( file && ptr && len >= 0 ) {
        for( FILE *fp = fopen(file, "wb"); fp; fclose(fp), fp = 0 ) {
            ok = fwrite(ptr, len, 1, fp) == 1;
        }
    }
    return ok;
}

char *editor_filename(void *obj) {
    return va("%s", "$$$"); // @fixme
}

bool editor_save_disk(void *obj, const char *outfile) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    static threadlocal array(char) buffer = 0;
    array_resize(buffer, 0); // <-- reused as an optimization

    bool ok = 0;
    if( save1(true, &buffer, ed->properties) > 0 ) {
        ok = file_overwrite(outfile, buffer, array_count(buffer));
    }

    return ok;
}

bool editor_load_disk(void *obj, const char *infile) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    int buflen;
    char *buffer = file_load(infile, &buflen);
    if( buffer && buflen ) {
        if( load1(true, buffer, buflen, ed->properties, 0) > 0 ) {
            return true;
        }
    }

    return false;
}

bool editor_save_checkpoint(void *obj) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

#if 1
    // discard redo
    if( ed->cursor < array_count(ed->history) ) {
        array_resize(ed->buffer, ed->history[ed->cursor].to);
        array_resize(ed->history, ed->cursor + 1);
    }
#else
    array_resize(ed->buffer, 0);
    array_resize(ed->history, 0);
    ed->cursor = 0;
#endif

    // save
    int checkpoint = array_count(ed->buffer);
    if( save1(true, &ed->buffer, ed->properties) <= 0 ) return false;
    array_push(ed->history, vec2(checkpoint, array_count(ed->buffer)));
 
    // move cursor to latest
    ed->cursor = array_count(ed->history) - 1;

    return true;
}

bool editor_load_checkpoint(void *obj) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    // load latest & update history
    int slots = array_count(ed->history);
    if( slots )
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[slots - 1].from) > 0 ) 
            return ed->cursor = slots - 1, true;

    return false;
}

bool editor_reset(void *obj) { // load first checkpoint
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});
    
    // load first slot
    if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[0].from) > 0 ) {
        // discard redo
        array_resize(ed->buffer, ed->history[0].to);
        // update history
        array_resize(ed->history, 1);
        // move cursor to latest
        ed->cursor = array_count(ed->history) - 1;
        return true;
    }

    return false;
}

bool editor_undo(void *obj) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    // load previous & rewind history by -1
    if( ed->cursor > 0 ) 
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[ed->cursor - 1].from) >= 0 )
            return ed->cursor -= 1, true;

    return false;
}

bool editor_redo(void *obj) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    // load next & forward history by +1
    if( ed->cursor < (array_count(ed->history)-1) )
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[ed->cursor + 1].from) >= 0 )
            return ed->cursor += 1, true;

    return false;
}

bool editor_ui(void *obj) {
    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});

    // auto-load from disk. @fixme kvdb database
    if( array_count(ed->history) == 0 )
        if( editor_load_disk(obj, editor_filename(obj)) ) 
            {}

    // auto-save in-mem on first edit
    if( array_count(ed->history) == 0 )
        editor_save_checkpoint(obj);

    const char *section = 0;
    for each_array(ed->properties, struct property, p) {
        if( p.mark ) {
            if( section == 0 || strcmp(section, p.mark) ) {
                if( section != 0 ) ui_separator();
                section = p.mark;
                ui_label(va("*%s", section)); // '*' adds bold style in labels
            }
        }
        /**/ if( p.type[0] == 'l') ui_label(p.name);
        else if( p.type[0] == 'f') ui_float(p.name, p.value);
        else if( p.type[0] == 'v') ui_float3(p.name, p.value);
        else if( p.type[0] == 'i') ui_int(p.name, p.value);
        else if( p.type[0] == 'b') ui_bool(p.name, p.value);
    }
    return true;
}

void editor_add_property(void *obj, void *value, const char *meta_) {
    ASSERT( obj );
    ASSERT( value );
    ASSERT( meta_ );
    char *meta = va("%s", meta_);

    struct property p = {0};

    // parse tooltip, if present
    for( char *tooltip = strstr(meta, " @"); tooltip; *tooltip = 0, tooltip = 0) {
        p.hint = STRDUP(tooltip + 2);
    }

    // parse metas, if present
    for( char *metas = strstr(meta, "//"); metas; *metas = 0, metas = 0) {
        for each_substring(metas + 2, " ", token) {
        /**/ if(strbegi(token, "hint=")) token = token + 5 + strspn(token + 5, " "), p.hint = STRDUP(token);
        else if(strbegi(token, "minv=")) token = token + 5 + strspn(token + 5, " "), p.minv = STRDUP(token);
        else if(strbegi(token, "maxv=")) token = token + 5 + strspn(token + 5, " "), p.maxv = STRDUP(token);
        }
    }

    // parse declaration
    unsigned field = ~0u;
    for each_substring(meta, "={,}(); ", token) {
        // next field
        ++field;
        // parse fields
        /**/ if(field == 0) p.type = STRDUP(token);
        else if(field == 1) { // either name or namespace.name
            p.name = strchr(token, '.');
            if( !p.name ) p.name = STRDUP(token);
            else p.name = STRDUP(p.name + 1), p.mark = STRDUP(token), *strchr(p.mark, '.') = '\0';
        }
        else {} // any initialization values here
    }

    // required fields
    ASSERT(p.name);
    ASSERT(p.type);

    // combine name+hint together
    if( p.hint ) {
        strcatf(&p.name, "@%s", p.hint);
        FREE(p.hint);
    }

    // defaults
    p.value = value;

    edit_state *ed = map_find_or_add(editor_state, obj, (edit_state){0});
    array_push( ed->properties, p );
}

// interfacing

static int  editor_mode = 1;
static int  editor_selected = -1; // object in scene selected
static vec2 editor_mouse; // 2d coord for ray/picking
static int  editor_hz = 60;
static int  editor_hz_midcost = 12;
static int  editor_hz_lowcost = 6;
static bool editor_battery_saving = 0;

void editor_render_windows() {
    // content browser
    if( ui_window("File Browser", 0) ) {
        const char *browsed_file = 0;
        if( ui_browse(&browsed_file, NULL) ) puts(browsed_file);
        ui_window_end();
    }

    // to deprecate
    if( ui_begin("Editor (deprecated)", 0) ) {
        bool x;
        ui_float2(ICON_MD_MOUSE "mouse (2d pick)", editor_mouse.v2);
        if( ui_bool(ICON_MD_FRONT_HAND "breakpoint", (x = 0, &x)) ) breakpoint("editor breakpoint");
        if( ui_bool(ICON_MD_BUG_REPORT "debugger", (x = has_debugger(), &x))) {}


        #if 0
        ui_separator();
        if( editor_selected >= 0 ) {
            ui_label(va("[%p]", obj));
            if(ui_float3("Position", obj->pos.v3))   object_teleport(obj, obj->pos), gizmo__mode = 0;
            if(ui_float3("Rotation", obj->euler.v3)) object_rotate(obj, obj->euler), gizmo__mode = 2;
            if(ui_float3("Scale", obj->sca.v3))      object_scale(obj, obj->sca),    gizmo__mode = 1;
        }
        #endif

        ui_end();
    }

    // terminal window
    #define TERMINAL
    #include "art/editor/labs/editor"

    // UI properties
    if( ui_window("Properties", 0) ) {
        int button = ui_toolbar(ICON_MD_LOOP ";" ICON_SD_CARD ";" ICON_MD_UNDO ";" ICON_MD_REDO ";Save;Load");
        if(!button) if(input(KEY_LCTRL) || input(KEY_RCTRL)) {
            if( input_down(KEY_Z) ) button = 3;
            if( input_down(KEY_Y) ) button = 4;
            if( input_down(KEY_S) ) button = 5;
        }

        ui_separator();

        for each_map(editor_state, void *, obj, edit_state, e) {
            if( button == 1 ) { const char *ok = editor_reset(obj) ? "ok" : "err"; EDITOR_PRINTF("reset: %s\n", ok); }
            if( button == 2 ) { const char *ok = editor_save_checkpoint(obj) ? "ok" : "err"; EDITOR_PRINTF("save chkpt: %s\n", ok); }
            if( button == 3 ) { const char *ok = editor_undo(obj) ? "ok" : "err"; EDITOR_PRINTF("undo: %s\n",  ok); }
            if( button == 4 ) { const char *ok = editor_redo(obj) ? "ok" : "err"; EDITOR_PRINTF("redo: %s\n",  ok); }
            if( button == 5 ) { const char *ok = editor_save_disk(obj, editor_filename(obj)) ? "ok" : "err"; EDITOR_PRINTF("save: %s\n",  ok); }
            if( button == 6 ) { const char *ok = editor_load_disk(obj, editor_filename(obj)) ? "ok" : "err"; EDITOR_PRINTF("load: %s\n",  ok); }

            editor_ui(obj);

            ui_separator();
            ui_label("*Debug");

            char *s = 0;
            for( int i = 0; i < array_count(e.history); ++i ) strcatf(&s, ",%s%d..%d", e.cursor == i ? "->":"", (int)e.history[i].from, (int)e.history[i].to);
            if(s) ui_label(va("Savepoints: %s", s+1)), FREE(s);
            ui_string("Object log", va("%s","(empty)"), 7+1);
        }

        ui_window_end();
    }

    // icon browser
    if( ui_window("Icon Palette", 0 )) {
        static const char *icons[] = {
            #define ICON(x) ICON_MD_##x
            #include "art/editor/labs/editor"
        };
        static const char *titles[] = {
            #define ICON(x) #x
            #include "art/editor/labs/editor"
        };

        for( int i = 0, cols = 8; i < countof(icons); i += cols ) {
            char buf[256], *p = buf;
            for( int j = i, jmax = mini(i+cols, countof(icons)); j < jmax; ++j ) p += sprintf(p, ";%s%03d@%s", icons[j], j, titles[j]);
            ui_toolbar(buf+1);
        }

        ui_window_end();
    }
}

void editor_pickup() {
    scene_t *scene = scene_get_active();
    camera_t *camera = camera_get_active();

    // input: mouse
    bool any_active = (ui_hover() || ui_active()) || gizmo_active();
    if( !any_active && input_down(MOUSE_L) ) {
        editor_mouse = vec2(input(MOUSE_X), input(MOUSE_Y));
    }
    if( !any_active && input_click(MOUSE_L, 500) ) { // pick entity
        // unproject 2d coord as 3d coord
        vec3 out = editor_pick(editor_mouse.x, editor_mouse.y);
        vec3 from = camera_get_active()->position, to = out;
        ray r = ray(from, to);
        //ddraw_line(from, to); // visualize ray

        int found = -1, count = scene_count();
        for( int i = 0; i < count; ++i ) {
            object_t *obj = scene_index(i);
            // bring aabb box to object position
            aabb box = model_aabb(obj->model, obj->transform); //add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
            // test ray hit
            if( ray_hit_aabb(r, box) ) {
                editor_selected = i;
                break;
            }
        }
    }

    object_t *obj = 0;
    if( editor_selected >= 0 ) {
        obj = scene_index(editor_selected);
        // bring aabb box to object position
        aabb box = model_aabb(obj->model, obj->transform); // aabb box = aabb(add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
        ddraw_color(YELLOW);
        ddraw_aabb(box.min, box.max);
        // draw gizmo
        if( gizmo(&obj->pos, &obj->euler, &obj->sca) ) {
            object_update(obj);
        }
    }
}


char *file_name_counter(const char *name) { // @fixme threadlocal?
    static char outfile[DIR_MAX];
    static map(char*, int) ext_counters;

    char *base = va("%s",name), *ext = file_ext(name); 
    if(ext && ext[0]) *strstr(base, ext) = '\0';

    do_once map_init(ext_counters, less_str, hash_str);
    int *counter = map_find_or_add(ext_counters, ext, 0);
    while( *counter >= 0 ) {
        *counter = *counter + 1;
        sprintf(outfile, "%s(%03d)%s", base, *counter, ext);
        if( !file_exist(outfile) ) {
            return va("%s", outfile);
        }
    }
    return 0;
}

static double editor_t = 0, editor_dt = 0;

double editor_time() {
    return editor_t;
}
double editor_delta() {
    return editor_dt;
}

bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? editor_mode : 0;
}

void editor() {
    editor_dt = window_delta() * !window_has_pause(); if(editor_dt > 1) editor_dt = 1;
    editor_t += editor_dt;

    if( input_up(KEY_TAB) ) editor_mode ^= 1; // cycle editor mode
    if( !editor_mode ) return;

    editor_pickup();

    int do_pause = 0;
    int do_reload = 0;
    int do_browser = 0;
    int do_recording = 0;
    int do_fullscreen = 0;
    int do_screenshot = 0;
    int do_quit = 0;
    int do_mute = 0; static bool mastervol = 1;
    int do_battery = 0;
    int do_profiler = 0;
    int do_stop = 0;

    // input: keyboard.
    int modifiers = input(KEY_LALT) || input(KEY_LSHIFT) || input(KEY_LCTRL);
    if( input_down(KEY_F1) )  do_pause = 1;
    if( input_down(KEY_F5) )  do_reload = 1;
    if( input_down(KEY_F11) ) do_fullscreen = 1;
    if( input_down(ifdef(cl,has_debugger()?KEY_F10:)KEY_F12) && !modifiers ) do_screenshot = 1;
    if( input_down(ifdef(cl,has_debugger()?KEY_F10:)KEY_F12) &&  modifiers ) do_recording = 1;
    // if( input_down(KEY_W) && input_held(KEY_LCTRL) ) do_quit = 1;

    // menubar

    if( ui_menu( ICON_MD_SETTINGS "@Preferences;" 
                ICON_MD_G_TRANSLATE " Language;" 
                ICON_MD_CREATE_NEW_FOLDER " Folders;" 
                ICON_MD_KEYBOARD " Keyboard;" 
                ICON_MD_MOUSE " Mouse;" 
                ICON_MD_GAMEPAD " Gamepad;" 
                ICON_MD_BUILD " Project;"
                ICON_MD_FACE " Profile;"
                ICON_MD_REPLAY " Restart;"
                ICON_MD_CLOSE " Quit;"
                "-" ICON_MD_RECYCLING " Reset all preferences;" ICON_MD_SAVE_AS " Save all preferences"
    ) ) {
        printf("%d\n", ui_item());
        if( ui_item() == 1 ) {}
        if( ui_item() == 2 ) {}
        if( ui_item() == 3 ) {} // key mappings
        if( ui_item() == 4 ) {} // sensitivity, invert xylrw
        if( ui_item() == 5 ) {} // sensitivity, invert xy,ab, deadzones
        if( ui_item() == 6 ) {} // 
        if( ui_item() == 7 ) {} // name,email,icon,link,github
        if( ui_item() == 8 ) do_reload = 1;
        if( ui_item() == 9 ) do_quit = 1;
    }

    if( ui_menu( window_has_pause() ? ICON_MD_PLAY_ARROW : ICON_MD_PAUSE )) do_pause = 1;
    if( ui_menu( ICON_MD_STOP )) do_stop = 1;

    if( ui_menu( ICON_MD_FOLDER_SPECIAL )) do_browser = 1;
    if( ui_menu( va(ICON_MD_VIEW_IN_AR " %d", 0 ))) {} // map_count(offline) )));

    if( ui_menu( va(ICON_MD_ROCKET_LAUNCH))) {}

    if( ui_menu( ICON_MD_PHOTO_CAMERA "@Screenshot" )) do_screenshot = 1;
    if( ui_menu( videorec_active() ? ICON_MD_VIDEOCAM_OFF : ICON_MD_VIDEOCAM )) { if(videorec_active()) videorec_stop(); else do_recording = 1; }
    if( ui_menu( mastervol ? ICON_MD_VOLUME_UP : ICON_MD_VOLUME_OFF )) do_mute = 1;
    if( ui_menu(editor_battery_saving ? ICON_MD_BATTERY_ALERT : ICON_MD_POWER )) do_battery = 1;
    // @todo
    // input settings here: gamepad, keyboard, mouse, invert controls, remappings, etc
    // if( ui_menu( ICON_MD_EDIT ";" ICON_MD_CONTENT_CUT "Cut (^X);" ICON_MD_CONTENT_COPY "Copy (^C);" ICON_MD_CONTENT_PASTE "Paste (^Z)") ) printf("Selected Edit.%d\n", ui_item());
    if( ui_menu( window_has_fullscreen() ? ICON_MD_FULLSCREEN_EXIT : ICON_MD_FULLSCREEN )) do_fullscreen = 1;
    if( ui_menu( va(ICON_MD_SIGNAL_CELLULAR_ALT " 0/0KiB" ))) {} // SIGNAL_CELLULAR_1_BAR SIGNAL_CELLULAR_2_BAR
    if( ui_menu( va(ICON_MD_STORAGE " %s", xstats() ))) do_profiler = 1; // 012/136MB
    if( ui_menu( va(ICON_MD_SPEED " %5.2f/%d", window_fps(), editor_hz))) do_profiler = 1; // 012/136MB

    if( do_stop )       window_pause(1);
    if( do_quit)        videorec_stop(), exit(0);
    if( do_mute )       audio_volume_master( mastervol ^= 1 ), ui_notify("Audio", mastervol ? "Unmuted." : "Muted.");
    if( do_pause )      window_pause( window_has_pause() ^ 1 );
    if( do_reload )     window_reload();
    if( do_battery )    editor_battery_saving ^= 1;
    if( do_browser )    ui_window_show("File Browser", ui_window_visible("File Browser") ^ true);
    if( do_recording )  for(char *name = file_name_counter(va("%s.mp4",app_name())); name; name = 0) ui_notify(va("Video capturing: %s", name), date_string()), window_record(name);
    if( do_screenshot ) for(char *name = file_name_counter(va("%s.png",app_name())); name; name = 0) window_screenshot(name), ui_notify(va("Screenshot: %s", name), date_string());
    // @todo: cycle mem -> cpu/profiler -> network mon -> debugger . also open windows
    if( do_profiler )   ui_window_show("Profiler", profiler_enabled = ui_window_visible("Profiler") ^ true);
    // stop any video recording in progress here because changing the framebuffer on-the-fly corrupts video stream
    if( do_fullscreen ) { videorec_stop(); window_fullscreen( window_has_fullscreen() ^ 1 ); }


    // other
    editor_render_windows();

    // @fixme: send keys to game
    // if( input_repeat(KEY_TAB, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    // adaptive editor-hz (via battery levels)
    int battery_read = battery();
    int battery_discharging = battery_read < 0 && abs(battery_read) < 100;
    int app_on_background = !window_has_focus();

    int hz = app_on_background ? editor_hz_lowcost : (battery_discharging && editor_battery_saving) ? editor_hz_midcost : editor_hz; 

    // apply editor hz
    window_lock_fps( hz < 5 ? 5 : hz );
}






int main() {
    editor_init();

    // 75% window, MSAAx2 flag
    window_create(75, WINDOW_MSAA2);
    window_title("editor");
    window_icon("logo.png");

    // enable outlines
    fx_load("fxOutline.fs");
    fx_enable(0, 1);

    // config 3d camera
    camera_t cam = camera();

    // config 3d model #1
    mat44 witch_pivot; id44(witch_pivot); scale44(witch_pivot, 0.1,-0.1,0.1); rotate44(witch_pivot, 180,0,1,0);
    model_t witch = model("witch/witch.obj", 0);
    model_set_texture(witch, texture("witch/witch_diffuse.tga.png", 0));

    // config 3d model #2
    float girl_frame = 0;
    mat44 girl_pivot; id44(girl_pivot);
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    vec3 p = {2,0,2}, r = {0,0,0}, s = {2,2,2};

    // test properties
    editor_add_property(&girl, &girl_frame, "float Animation.Frame;    // @Animation frame");
    editor_add_property(&girl, &p,          "vec3  Transform.Position; // @Position in world units");
    editor_add_property(&girl, &r,          "vec3  Transform.Rotation; // @Rotation in degrees");
    editor_add_property(&girl, &s,          "vec3  Transform.Scale;    // @Scale factor (decimal)");

    // meta(&girl_frame, "float Animation.Frame; // @Animation frame");

    // editor loop
    while( window_swap() ) {

        // editor tick
        profile("Editor") {
            editor();
        }

        gizmo(&p,&r,&s);

        // fps camera
        {
            vec3 move = {0};
            vec2 view = {0};

            // show/hide cursor
            bool dragging = input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
            if( ui_active() || ui_hover() || gizmo_active() || input_touch_active() ) dragging = false;
            window_cursor( !dragging );

            // keyboard/mouse
            if( dragging ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
            vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
            vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * dragging);
            move = add3(move, wasdec);
            view = add2(view, mouse);

            // gamepad
            vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f /*15% deadzone*/);
            vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), 0.15f /*15% deadzone*/);
            vec3 gamepad_move = scale3(vec3(filtered_lpad.x, input(GAMEPAD_LT) - input(GAMEPAD_RT), filtered_lpad.y), 1.0f);
            vec2 gamepad_view = scale2(filtered_rpad, 1.0f);
            move = add3(move, gamepad_move);
            view = add2(view, gamepad_view);

            // multi-touch
            vec2 touch_move = input_touch_delta_from_origin(0, 0.0125f /*sensitivityFwd*/); // button #0 (left border)
            vec2 touch_view = input_touch_delta(1, 0.125f /*sensitivityRot*/);              // button #1 (right border) 
            move = add3(move, vec3(touch_move.x, 0, -touch_move.y));
            view = add2(view, vec2(touch_view.x, -touch_view.y));

            // apply inputs
            camera_move(&cam, move.x,move.y,move.z);
            camera_fps(&cam, view.x,view.y);
        }

        // draw grid/axis
        ddraw_grid(0);
        ddraw_flush();

        // animate girl
        float delta = editor_delta() * 30; // 30fps anim
        girl_frame = model_animate(girl, girl_frame + delta);

        // draw girl
        fx_begin();
        rotationq44(girl_pivot, eulerq(r)); scale44(girl_pivot, s.x,s.y,s.z); relocate44(girl_pivot, p.x,p.y,p.z);
        model_render(girl, cam.proj, cam.view, girl_pivot, 0);
        fx_end();

        // draw witch
        model_render(witch, cam.proj, cam.view, witch_pivot, 0);

        ui_demo();
    }
}
