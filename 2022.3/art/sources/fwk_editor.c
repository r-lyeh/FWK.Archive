// editing:
// nope > functions: add/rem property

static int editor_mode = 1;
static int editor_selected = -1; // object in scene selected
static vec2 editor_mouse; // 2d coord for ray/picking
static int editor_hz = 60;

vec3 editor_pick(float mouse_x, float mouse_y) {
    // unproject 2d coord as 3d coord
    camera_t *camera = camera_get_active();
    vec3 out, xyd = vec3(mouse_x,window_height()-mouse_y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
    bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);
    return out;
}

static int gizmo__mode;
static int gizmo__active;
bool gizmo_active() {
    return gizmo__active;
}
int gizmo(vec3 *pos, vec3 *rot, vec3 *sca) {
    uint32_t bak = dd_color;

#if 0
    ddraw_flush();
    mat44 copy; copy44(copy, camera_get_active()->view);
    if( 1 ) {
        float *mv = camera_get_active()->view;
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(4) mv[4*0+0] = d, mv[4*0+1] = 0, mv[4*0+2] = 0;
        if(2) mv[4*1+0] = 0, mv[4*1+1] = d, mv[4*1+2] = 0;
        if(1) mv[4*2+0] = 0, mv[4*2+1] = 0, mv[4*2+2] = d;
    }
#endif

    ddraw_ontop(1);

    vec3 mouse = ui_active() ? vec3(0,0,0) : vec3(input(MOUSE_X),input(MOUSE_Y),input_down(MOUSE_L)); // x,y,l
    vec3 from = camera_get_active()->position;
    vec3 to = editor_pick(mouse.x, mouse.y);
    ray r = ray(from, to);

    static vec3 src3, hit3, off3; static vec2 src2;
    #define on_gizmo_dragged(X,Y,Z,COLOR,DRAWCMD, ...) do { \
        vec3 dir = vec3(X,Y,Z); \
        line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
        plane ground = { vec3(0,0,0), vec3(Y?1:0,Y?0:1,0) }; \
        vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
        aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
        hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
        ddraw_color( hit_arrow || gizmo__active == (X*4+Y*2+Z) ? YELLOW : COLOR ); \
        DRAWCMD; \
        if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), src3 = *pos, hit3 = hit_ground->p, off3 = mul3(sub3(src3,hit3),vec3(X,Y,Z)), gizmo__active = X*4+Y*2+Z; \
        if( (gizmo_active && gizmo__active==(X*4+Y*2+Z)) || (!gizmo__active && hit_arrow) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line : ddraw_line_dashed)(axis.a, axis.b); } \
        if( gizmo__active == (X*4+Y*2+Z) && hit_ground ) {{ __VA_ARGS__ }; modified = 1; gizmo__active *= !!input(MOUSE_L); } \
    } while(0)
    #define gizmo_translate(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, ddraw_arrow(*pos,add3(*pos,vec3(X,Y,Z))), { \
            *pos = add3(line_closest_point(axis, hit_ground->p), off3); \
        } )
    #define gizmo_scale(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, (ddraw_line(*pos,add3(*pos,vec3(X,Y,Z))),ddraw_sphere(add3(*pos,vec3(X-0.1*X,Y-0.1*Y,Z-0.1*Z)),0.1)), { /*ddraw_aabb(arrow.min,arrow.max)*/ \
            int component = (X*1+Y*2+Z*3)-1; \
            float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
            float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
            float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
            float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
            sca->v[component] -= sgn * mag * 0.01; \
            src2 = vec2(mouse.x, mouse.y); \
        } )
    #define gizmo_rotate(X,Y,Z,COLOR) do { \
            vec3 dir = vec3(X,Y,Z); \
            line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
            plane ground = { vec3(0,0,0), vec3(0,1,0) }; \
                vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
                aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
                hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
                int hover = hit_arrow ? (X*4+Y*2+Z) : 0; \
            if( gizmo__active == (X*4+Y*2+Z) ) { ddraw_color(gizmo__active ? YELLOW : WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active && hover == (X*4+Y*2+Z) ) { ddraw_color(COLOR); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active ) { ddraw_color(WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), gizmo__active = hover; \
            if( (!gizmo__active && hover == (X*4+Y*2+Z)) || gizmo__active == (X*4+Y*2+Z) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line_thin : ddraw_line_dashed)(axis.a, axis.b); } \
            if( gizmo__active && gizmo__active == (X*4+Y*2+Z) && hit_ground && !ui_hover() ) { \
                int component = (Y*1+X*2+Z*3)-1; /*pitch,yaw,roll*/ \
                float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
                float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
                float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
                float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
                rot->v[component] += sgn * mag; \
                /*rot->v[component] = clampf(rot->v[component], -360, +360);*/ \
                src2 = vec2(mouse.x, mouse.y); \
                \
            } \
            gizmo__active *= !!input(MOUSE_L); \
        } while(0)

    int modified = 0;
    if(input_down(KEY_SPACE)) gizmo__active = 0, gizmo__mode = (gizmo__mode + 1) % 3;
    if(gizmo__mode == 0) gizmo_translate(1,0,0, RED);
    if(gizmo__mode == 0) gizmo_translate(0,1,0, GREEN);
    if(gizmo__mode == 0) gizmo_translate(0,0,1, BLUE);
    if(gizmo__mode == 1) gizmo_scale(1,0,0, RED);
    if(gizmo__mode == 1) gizmo_scale(0,1,0, GREEN);
    if(gizmo__mode == 1) gizmo_scale(0,0,1, BLUE);
    if(gizmo__mode == 2) gizmo_rotate(1,0,0, RED);
    if(gizmo__mode == 2) gizmo_rotate(0,1,0, GREEN);
    if(gizmo__mode == 2) gizmo_rotate(0,0,1, BLUE);

#if 0
    ddraw_flush();
    copy44(camera_get_active()->view, copy);
#endif

    ddraw_ontop(0);
    ddraw_color(bak);

    return modified;
}

void editor_update() {
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

    if( ui_begin("Editor", 0) ) {
        bool x;
        ui_float2("mouse (2d pick)", &editor_mouse.x);
        if( ui_bool("breakpoint", (x = 0, &x)) ) breakpoint("editor breakpoint");
        if( ui_bool("debugger", (x = has_debugger(), &x))) {}
        if( ui_bool("fullscreen", (x = window_has_fullscreen(), &x)) ) window_fullscreen( x );
        if( ui_bool("pause", (x = window_has_pause(), &x)) ) window_pause( x );
        if( ui_int(va("target fps (%.2f)", fps), &editor_hz) ) if(editor_hz < 0) editor_hz = 0; if(editor_hz > 0 && editor_hz < 5) editor_hz = 5;
        ui_separator();
        if( editor_selected >= 0 ) {
            ui_label(va("[%p]", obj));
            if(ui_float3("Position", &obj->pos.x))   object_teleport(obj, obj->pos), gizmo__mode = 0;
            if(ui_float3("Rotation", &obj->euler.x)) object_rotate(obj, obj->euler), gizmo__mode = 2;
            if(ui_float3("Scale", &obj->sca.x))      object_scale(obj, obj->sca),    gizmo__mode = 1;
        }
        ui_end();
    }

    window_lock_fps( editor_hz );
}
bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? 1 : 0;
}

void editor() {
    // input: keyboard.
#if 0
    if( input_up(KEY_TAB) ) editor_mode ^= 1; // cycle editor mode
    if( input_up(KEY_F1) )  window_pause( window_has_pause() ^ 1 );
    if( input_up(KEY_F5) )  app_reload();
    if( input_up(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1);
    if( input_up(KEY_F12) ) screenshot("_screenshot.png", 3, true);
#endif
    // @fixme: send keys to game
    // if( input_repeat(KEY_TAB, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    if( ui_menu(ICON_BARS "File;New;" ICON_TEST_GLYPH "Open;-Save;-Quit") ) printf("Selected File.%d\n", ui_item());
    if( ui_menu("Edit;Cut;Copy;Paste") ) printf("Selected Edit.%d\n", ui_item());

    if( editor_mode ) editor_update();
}

char* dialog_load() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };
    int allowMultipleSelections = 0;

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_openFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints, allowMultipleSelections );
}
char* dialog_save() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_saveFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints );
}
