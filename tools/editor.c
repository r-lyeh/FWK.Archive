// ## editor (v1)
// A tool that understands Assets and is able to edit the details of such Assets. This understanding is configured via reflection fields in .ini files.
// The reflected properties in the .ini files will allow us for loading, saving and creating generic widget views to edit the Assets.
// Because we load and save the state of Assets, we can also undo and redo changes by simulating load/saves from/into memory. Also diffs and patches.
// - Load Assets
// - Edit Assets
// - View Assets (automatic, via reflection)
// - Save Assets
// - Undo Assets (automatic, via loading older state)
// - Redo Assets (automatic, via loading newer state)
// - Diff Assets
// - Mend Assets
//
// The editor uses collections everywhere. Edition of single objects are just bulk-editing collections of 1 item. Collections may include:
// - Arrays
// - Vectors
// - Ordered Maps/Sets
// - Unordered Maps/Sets
// - Sparse maps/sets
// - Custom implementations of the above collections (via C/C++ files). Also custom per-type collections.
//
// At this point we can also:
// - Spawn Assets in Collections of Assets
// - Delete Assets in Collections of Assets
// - Save/Load Collections of Assets
// - Edit Collections of Assets
// - View Collections of Assets
//
// ## editor (v2)
// The previous version could be a blackbox controlled maybe via scripts, offline commands, Telnet sessions or similar tools; like GDB does.
// However, editor v2 is a graphical tool that extends previous types with .lua and .dll plugins. This editor can also:
// - Draw Collections of Assets (while ingame)
// - Tick Collections of Assets (while ingame)
// - Draw Collections of Assets (while editing)
// - Tick Collections of Assets (while editing)
// - Edit Collections of Assets (custom Viewports with custom UI specializations). These views directly write their data into disk bypassing v1.
//
// ## editor (v3)
// Bootstrap previous editor into a bigger thing. Basic components of existing editors could be bootstrapped here at this moment. Eg,
// - Gizmo editor
// - Property editor
// - Level editor
// - World outliner
// - Nodegraph editor
//
// Once merged with the game/world ECS logic, we should have an extensible, lua-driven, plugin-based, working editor at that point.
//


#include "fwk.c"
#include "3rd_fa.h"

typedef void*        obj_t;
#if 0
typedef struct collection_t {
    unsigned type;
    union {
        // - Arrays
        array(obj_t) vec;        // - Vectors
        // - Ordered Maps/Sets
        // - Unordered Maps/Sets
        // - Sparse maps/sets
        // - Custom implementations of the above collections (via C/C++ files). Also custom per-type collections.
    };
} collection_t;
#else
typedef array(obj_t) collection_t;
#endif


// editor history
bool action() {
    // intent: resize(states, position); push(states, state); position++;
    return 0;
}
bool undo(unsigned steps) {
    // intent: if(position > 0) apply(states[--position])
    return 0;
}
bool redo(unsigned steps) {
    // intent: if(position < count(states)) apply(states[++position])
    return 0;
}

// editor operations
collection_t bulk_load(const char* resource, unsigned begin, unsigned end) {
    return 0;
}
char* bulk_save(collection_t objects, unsigned begin, unsigned end) {
    return 0;
}
bool bulk_edit(collection_t objects, unsigned begin, unsigned end) {
    return 0;
}
bool bulk_view(collection_t objects, unsigned begin, unsigned end) {
    return 0;
}
obj_t *bulk_make(collection_t objects, unsigned begin, unsigned end) {
    return 0;
}
bool bulk_kill(collection_t objects, unsigned begin, unsigned end) {
    return 0;    
}

bool bulk_draw(collection_t objects, unsigned begin, unsigned end, bool ingame) {
    return 0;
}
bool bulk_tick(collection_t objects, unsigned begin, unsigned end, bool ingame) {
    return 0;    
}

// syntax sugars
#define load(x)       bulk_load(x, 0,1)
#define save(x)       bulk_save(x, 0,1)
#define edit(x)       bulk_edit(x, 0,1)
#define view(x)       bulk_view(x, 0,1)
#define make(x)       bulk_make(x, 0,1)
#define kill(x)       bulk_kill(x, 0,1)
#define draw(x,i)     bulk_draw(x, 0,1, i)
#define tick(x,i)     bulk_tick(x, 0,1, i)



// state - retained mode

struct property { // meta: "vec3 namespace::varname = {1,2,3}; // min=(0,0,0) @this is a tooltip"
    char *mark; // namespace
    char *name; // display name
    char *type; // pointed type
    char *hint; // tooltip
    char *minv; // min value
    char *maxv; // max value
    char *incv; // inc value
    char *defv; // default value
    char *isro; // is read-only/enabled
    char *issv; // is pending-save
    void *value;
    unsigned typebits;
    unsigned flags;
};

static map(void*, array(struct property)) scene_properties;

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
        else if(field == 1) { // either name or namespace::name
            p.name = strstr(token, "::");
            if( !p.name ) p.name = STRDUP(token);
            else p.name = STRDUP(p.name + 2), p.mark = STRDUP(token), *strchr(p.mark, ':') = '\0';
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

    do_once map_init(scene_properties, less_ptr, hash_ptr);
    array(struct property) *found = map_find_or_add(scene_properties, obj, NULL);
    array_push( *found, p );
}
void editor_render_properties() {
    // UI properties
    if( ui_window("Properties", 0) ) {

        int button = ui_toolbar(ICON_FA_FILE ";" ICON_FA_FLOPPY_DISK ";" ICON_FA_TRASH );
        if( button == 1 ) puts("1.load");
        if( button == 2 ) puts("2.save"); // ICON_FA_SD_CARD
        if( button == 3 ) puts("3.reset");

        ui_separator();

        const char *section = 0;

        for each_map(scene_properties, void *, obj, array(struct property), arrp) {
            for each_array(arrp, struct property, p) {
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
        }

        ui_window_end();
    }
}






int main() {
    // 75% window, MSAAx8 flag
    window_create(75, WINDOW_MSAA8);
    window_title("editor");
    window_icon("logo.png");

    // enable outlines
    fx_load("fxOutline.fs");
    fx_enable(0, 1);

    // config 3d camera
    camera_t cam = camera();

    // config 3d model
    float girl_frame = 0;
    mat44 girl_pivot; id44(girl_pivot);
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    vec3 p = {0,0,0}, r = {0,0,0}, s = {2,2,2};
    
    editor_add_property(&girl, &girl_frame, "float Animation::Frame;    // @Animation frame");
    editor_add_property(&girl, &p,          "vec3  Transform::Position; // @Position in world units");
    editor_add_property(&girl, &r,          "vec3  Transform::Rotation; // @Rotation in degrees");
    editor_add_property(&girl, &s,          "vec3  Transform::Scale;    // @Scale factor (decimal)");

    // editor loop
    while( window_swap() ) {
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1);
        if( input_down(KEY_F5) ) app_reload();

        // editor tick
        static double editor_t = 0, editor_dt = 0;
        editor_dt = window_delta() * !window_has_pause(); if(editor_dt > 1/60.0) editor_dt = 1/60.0;
        editor_t += editor_dt;
        editor();
        gizmo(&p,&r,&s);
        rotationq44(girl_pivot, eulerq(r)); scale44(girl_pivot, s.x,s.y,s.z); relocate44(girl_pivot, p.x,p.y,p.z);

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

        ddraw_text2d( vec2(window_width() - (16*12),12*3), va("%+5.2f,%+5.2f,%+5.2f", cam.position.x,cam.position.y,cam.position.z) );

        // draw grid/axis
        ddraw_grid(0);
        ddraw_flush();

        // animate girl
        float delta = editor_dt * 30; // 30fps anim
        girl_frame = model_animate(girl, girl_frame + delta);

        // draw girl
        fx_begin();
        model_render(girl, cam.proj, cam.view, girl_pivot, 0);
        fx_end();

        ui_demo();

        editor_render_properties();
    }
}
