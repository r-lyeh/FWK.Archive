// framework demo
// - rlyeh, public domain

#define FWK_C
#include "fwk.h"

int main(int argc, char **argv) {
    // args
    const char *option_model = argc > 1 ? argv[1] : "3rd/3rd_assets/witch.obj";
    float option_scale = argc > 2 ? atof(argv[2]) : 0.10f;
    bool option_swapzy = argc > 3 ? atoi(argv[3]) : true;
    const char *option_diffuse = argc > 4 ? argv[4] : "3rd/3rd_assets/witch_diffuse.tga";
    bool option_flipuv = argc > 5 ? atof(argv[5]) : false;

    // window (75% sized, no flags)
    window_create(75, 0);
    window_title(__FILE__);

    // script (lua)
    script_run( "-- hello.lua\nio.write(\"script: Hello world from \", _VERSION, \"\\n\")" );

    // network (https)
    FILE *web = tmpfile();
    download("https://google.com", web);
    printf("network: %d bytes downloaded\n", (int)ftell(web));

    // audio (clip & stream)
    audio_t voice = audio_clip("3rd/3rd_assets/coin.wav");
    audio_play(voice, 0);
    audio_t stream = audio_stream("3rd/3rd_assets/wrath_of_the_djinn.xm");
    audio_play(stream, 0);

    // scene
    scene_t *scene = scene_new();

    // model
    model_t model1 = model(option_model, option_swapzy);
    texture_t tex1 = texture(option_diffuse, option_flipuv);
    object_t *obj1 = scene_spawn(scene, model1);
    object_diffuse(obj1, tex1);
    object_scale(obj1, vec3(option_scale, option_scale, option_scale));

    // camera
    camera_t cam = camera();

    // demo loop
    while (window_swap(NULL))
    {
        // input
        if( input(KEY_ESC) ) break;
        float up =      (input(KEY_E) - input(KEY_C));
        float right =   (input(KEY_D) - input(KEY_A)) + input(GAMEPAD_LPADX);
        float forward = (input(KEY_W) - input(KEY_S)) + input(GAMEPAD_LPADY);
        float speed = (2 + 3 * max(input(GAMEPAD_LT), input(KEY_LSHIFT))) * cam.speed;
        // mouse_show(false);

        // debugdraw. @fixme: works only before 3d models for now.
            ddraw_grid();
            ddraw_color(yellow);
            char spos[128]; sprintf(spos, "(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z);
            ddraw_text(vec3(+1,+1,-1), 0.04f, spos);

        // ddraw_demo(); // uncomment to showcase a few debug shapes

        // model (rotation)
        object_rotate(obj1, vec3(0,1,0), window_time() * 10);

        // scene (render)
        scene_render(scene, cam);

        // camera (move)
        cam.speed += input_diff(MOUSE_W); if(cam.speed<1) cam.speed = 1;
        vec3 camdir = scale3(vec3(right, up, forward), dt * speed);
        camera_move(&cam, camdir.x, camdir.y, camdir.z);

        // camera (free look)
        if( input(MOUSE_L) || input(MOUSE_R) ) {
            vec2 polarity = vec2(+1, -1);
            vec2 sensitivity = vec2(0.2f, 0.2f);
            vec2 dir = mul2(input_diff2(MOUSE), mul2(polarity, sensitivity));

            camera_fps(&cam, dir.x, dir.y);
        } else {
            vec2 filtered_pad = filter_deadzone(input2(GAMEPAD_RPAD), 0.1f /*deadzone*/);

            vec2 polarity = vec2(+1, +1);
            vec2 sensitivity = scale2( vec2(0.2f, 0.2f), speed/2);
            vec2 dir = mul2(filtered_pad, mul2(polarity, sensitivity));

            camera_fps(&cam, dir.x, dir.y);
        }

        // ui (showcase)
        static int integer = 0;
        static bool boolean = 0;
        static float floating = 0;
        static float rgb[3] = {255,192,0};
        static float rgba[4] = {180,40,255};
        static float slider = 0.5f;
        static char string[64] = {0};
        static int item = 0; const char *list[] = {"one","two","three"};
        static bool show_dialog = false;

        if( ui_begin("UI Demo", 0)) {
            if( ui_label("label with tooltip@compiled on " __DATE__ " " __TIME__)) {}
            if( ui_separator() ) {}
            if( ui_bool("my bool", &boolean) ) puts("boolean changed");
            if( ui_int("my int", &integer) ) puts("integer changed");
            if( ui_float("my float", &floating) ) puts("floating changed");
            if( ui_string("my string", string, 64) ) puts("string changed");
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_list("my list", list, 3, &item ) ) puts("list changed");
            if( ui_color3("my color3", rgb) ) puts("color3 changed");
            if( ui_color4("my color4@this is a tooltip", rgba) ) puts("color4 changed");
            if( ui_button("about...") ) { puts("button clicked"); show_dialog = 1; audio_play(voice, 0); }
            if( ui_dialog("about", __FILE__ "\n" __DATE__ "\n" "Public Domain licensed.", 0, &show_dialog) ) {}
            ui_end();
        }

        if( ui_begin("Camera", 0)) {
            if( ui_float("Speed", &cam.speed) ) {}
            ui_end();
        }
        if( ui_begin("Model", 0)) {
            static float scale = 1; if(ui_float("Scale", &scale)) { object_scale(obj1, vec3(scale,scale,scale)); }
            ui_end();
        }

        input_demo(); // comment to hide keyboard/mouse/gamepad UI tabs
        ui_render();
    }
}
