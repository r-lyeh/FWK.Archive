// model + skybox + gizmo demo
// - rlyeh, public domain.

#include "fwk.h"

// @todo: root motion on/off
// @todo: anim controller.lua
// @todo: modify bone transform (bone space)

typedef struct animations_t {
    int   inuse; // animation number
    float speed; // x1.00
    array(char*) names; // name of every animation in set
    array(vec2i) anims; // [begin,end] frames of every animation in set
} animations_t;

animations_t animations(const char *pathfile, int flags) {
    animations_t a = {0};
    char *anim_file = vfs_read(pathfile);
    for each_substring(anim_file, "\r\n", anim) {
        int from, to;
        char anim_name[128] = {0};
        if( sscanf(anim, "%*s %d-%d %127[^\r\n]", &from, &to, anim_name) != 3) continue;
        array_push(a.anims, vec2i(from, to)); // [from,to]
        array_push(a.names, STRDUP(anim_name));
    }
    a.speed = 1.0;
    return a;
}

int main() {
    bool do_showaabb = 0;
    bool do_showbones = 0;
    bool do_showmarker = 1;
    bool do_showmodel = 1;
    bool do_showgizmo = 1;
    bool is_dragging_slider = 0;

    window_create(75, WINDOW_MSAA8);
    window_title(__FILE__);

    for(const char **list = file_list("./","fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);

    model_t custom = model("kgirls01.fbx", 0);
    animations_t a = animations("kgirl/animlist.txt", 0);

    while( window_swap() ) {
        if(input(KEY_F5)) window_reload();
        if(input(KEY_ESC)) break;

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // rendering
        fx_begin();

            ddraw_ground(0);
            ddraw_flush();

            profile("Skybox") {
                skybox_render(&sky, cam.proj, cam.view);
            }

            profile("Skeletal render") {
                static vec3 p = {0,0,0}, r = {0,0,0}, s = {2,2,2};
                if( do_showgizmo ) gizmo(&p, &r, &s);
                mat44 M; rotationq44(M, eulerq(r)); scale44(M, s.x,s.y,s.z); relocate44(M, p.x,p.y,p.z);

                // pivot
                r.x -= 170;
                    // control
                    float speed = 0.2f;
                    quat q = eulerq(r); // += custom.pivot
                    vec3 rgt = rotate3q(vec3(1,0,0), q);
                    vec3 up  = rotate3q(vec3(0,1,0), q);
                    vec3 fwd = rotate3q(vec3(0,0,1), q);
                    vec3 dir = scale3(fwd, speed * (input(KEY_I) - input(KEY_K)));
                    r.x += input(KEY_L) - input(KEY_J); // yaw
                    p = add3(p, dir);
                r.x += 170;

                if(do_showmodel)
                model_render(custom, cam.proj, cam.view, M, 0); // custom.pivot);

                if(do_showbones)
                model_render_skeleton(custom, M);

                if(do_showmarker)
                ddraw_position_dir(p, fwd, 1.0f);

                if(do_showaabb) {
                    aabb box = model_aabb(custom, M); // custom.pivot);
                    ddraw_aabb(box.min, box.max);
                }
            }

        fx_end();

        // anim controller
        if( input_down(KEY_UP) ) a.speed = 1, ++a.inuse; if(a.inuse>=array_count(a.anims)) a.inuse = 0; // next anim
        if( input_down(KEY_DOWN) ) a.speed = 1, --a.inuse, custom.curframe = 0; if(a.inuse<0) a.inuse = array_count(a.anims) - 1; // previous anim
        if( input_down(KEY_LEFT) || input_repeat(KEY_LEFT, 300) ) a.speed = 0, custom.curframe=(int)custom.curframe - 1; // previous frame
        if( input_down(KEY_RIGHT) || input_repeat(KEY_RIGHT, 300) ) a.speed = 0, custom.curframe=(int)custom.curframe + 1; // next frame
        if( input_down(KEY_SPACE)) a.speed = !a.speed; // toggle pause
        if( input_down(KEY_ENTER) ) custom.curframe = 0; // restart anim
        if( input(KEY_N) ) a.speed = a.speed - 0.01; // decrease anim speed
        if( input(KEY_M) ) a.speed = a.speed + 0.01; // increase anim speed
        vec2i anim = a.anims[ a.inuse ];
        float delta = window_delta() * 30 * a.speed * !is_dragging_slider; // 30fps anim timer
        profile("Skeletal update") if(!window_has_pause()) custom.curframe = model_animate_clip(custom, custom.curframe + delta, anim.min, anim.max, !!strstri(a.names[a.inuse], "loop"));

        // UI
        if( ui_panel("Animation", 0)) {
            if( ui_bool("Show aabb", &do_showaabb) );
            if( ui_bool("Show bones", &do_showbones) );
            if( ui_bool("Show model", &do_showmodel) );
            if( ui_bool("Show marker", &do_showmarker) );
            if( ui_bool("Show gizmo", &do_showgizmo) );

            ui_separator();

            ui_label(va("Anim %s [%d.. %.2f ..%d]@UP/DOWN keys", a.names[ a.inuse ], anim.min, custom.curframe, anim.max ));

            // normalize curframe into [0..1] range
            float range = (custom.curframe - anim.min) / ((anim.max - anim.min) + !(anim.max - anim.min));
            if( ui_slider2("Frame@LEFT/RIGHT keys", &range, va("%.2f/%d %.2f%%", custom.curframe - anim.min, anim.max - anim.min, range * 100.f)) ) {
                custom.curframe = range * (anim.max - anim.min) + anim.min;
                is_dragging_slider = 1;
            } else {
                is_dragging_slider = 0;
            }
            
            ui_slider2("Speed@N/M keys", &a.speed, va("x%.2f", a.speed));

            ui_separator();

            for( int i = 0; i < array_count(a.names); ++i ) {
                vec2i anim = a.anims[ i ];
                bool selected = a.inuse == i;
                float progress = selected ? (custom.curframe - anim.min) * 100.f / (anim.max - anim.min) : 0.f;
                const char *caption = va("%s%s %.2f%%", selected ? "*":"", a.names[i], progress);
                int choice = ui_label2_toolbar(caption, ICON_MD_PAUSE_CIRCLE " " ICON_MD_PLAY_CIRCLE);
                if( choice == 1 ) { // play
                    a.speed = 1.0f;
                    a.inuse = i;
                    custom.curframe = anim.min; // restart animation (ENTER key)
                }
                if( choice == 2 ) {  // pause
                    if(a.inuse != i) a.speed = 0.0f, custom.curframe = 0;
                    else a.speed = !a.speed; // pause animation (SPACE key)
                    a.inuse = i;
                }
            }

            ui_panel_end();
        }
        if( ui_panel("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_panel_end();
        }
    }
}
