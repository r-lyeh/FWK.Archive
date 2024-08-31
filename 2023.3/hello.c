// playground tests for FWK
// - rlyeh, public domain
//
// # quickstart
// - win/vc       : cl hello.c
// - win/clang-cl : clang-cl hello.c
// - win/tcc      : tcc   hello.c -m64
// - win/mingw    : gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - win/clang    : clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - linux        : cc  hello.c -lm -ldl -lpthread -lX11
// - linux/tcc    : tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
// - osx          : cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox

#define FWK_C // unrolls single-header implementation
#include "engine/joint/fwk.h" // single-header file

int main() {
    // options
    bool do_debugdraw = 0;

    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // fx: load all post fx files in all subdirs.
    fx_load("fx**.fs");

    // load skybox
    skybox_t sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", 0); // --mie for rayleigh/mie scattering

    // animated models loading
    int model_flags = 0;
    model_t girl = model("kgirl/kgirls01.fbx", model_flags);
    model_t alien = model("alien/alien_helmet.fbx", model_flags); compose44(alien.pivot, vec3(10,0,-10), eulerq(vec3(0,-90,0)), vec3(1,1,1));
    model_t george = model("robots/george.fbx", model_flags);
    model_t leela = model("robots/leela.fbx", model_flags);
    model_t mike = model("robots/mike.fbx", model_flags);
    model_t stan = model("robots/stan.fbx", model_flags);
    model_t robots[4] = { george, leela, mike, stan };
    for( int i = 0; i < countof(robots); ++i ) {
        rotation44(robots[i].pivot, -90,1,0,0);
        translate44(robots[i].pivot, i*3,0,0);
        scale44(robots[i].pivot, 0.5,0.5,0.5);
    }

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // audio (both clips & streams)
    audio_t SFX1 = audio_clip( "coin.wav" );
    audio_t SFX2 = audio_clip( "pew.sfxr" );
    audio_t BGM1 = audio_stream( "waterworld-map.fur"), BGM = BGM1;
    audio_t BGM2 = audio_stream( "larry.mid" );
    audio_t BGM3 = audio_stream( "monkey1.mid" ); // wrath_of_the_djinn.xm" );
    audio_play(SFX1, 0);
    audio_play(BGM, 0);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_W) && input_held(KEY_LCTRL) ) break;
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), do_gamepad_deadzone + 1e-3);
        // vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), do_gamepad_deadzone + 1e-3);

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        // apply post-fxs from here
        fx_begin();

        // queue debug drawcalls
        profile("Debugdraw") {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, va("(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z));
            if(do_debugdraw) ddraw_demo(); // showcase many debugdraw shapes
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // draw skybox
        profile("Skybox") {
            skybox_render(&sky, cam.proj, cam.view);
        }

        // animate all models
        profile("Skeletal update") if(!window_has_pause()) {
            float delta = window_delta() * 30; // 30fps anim

            // animate girl & alien
            girl.curframe = model_animate(girl, girl.curframe + delta);
            alien.curframe = model_animate(alien, alien.curframe + delta);

            // animate robots
            for(int i = 0; i < countof(robots); ++i)
            robots[i].curframe = model_animate(robots[i], robots[i].curframe + delta);
        }

        // draw all models
        profile("Skeletal render") {
            model_render(girl, cam.proj, cam.view, girl.pivot, 0);
            model_render(alien, cam.proj, cam.view, alien.pivot, 0);

            for(int i = 0; i < countof(robots); ++i)
            model_render(robots[i], cam.proj, cam.view, robots[i].pivot, 0);
        }

        // draw gizmo and bounding-box
        profile("DebugDraw") {
            static vec3 p = {-10,0,-10}, r = {0,0,0}, s = {2,2,2};
            gizmo(&p, &r, &s);
            compose44(girl.pivot, p, eulerq(r), s);

            aabb box = model_aabb(girl, girl.pivot);
            ddraw_color(YELLOW);
            ddraw_aabb(box.min, box.max);
        }

        // post-fxs end here
        fx_end();

        // font demo
        do_once font_scales(FONT_FACE1, 48, 24, 18, 12, 9, 6);
        font_print(va(FONT_TOP FONT_RIGHT FONT_H4 "%5.2f FPS", window_fps()));

        // draw ui
        if( ui_panel("App", 0)) {
            static bool do_about = 0;
            if(ui_bool("Show debugdraw demo", &do_debugdraw)) {}
            if(ui_button("Test Lua")) script_run("ui_notify(nil, \"Hello from \" .. _VERSION)");
            if(ui_separator()) {}
            if(ui_button("About...")) { do_about = 1; audio_play(SFX1, 0); }
            if(ui_dialog("About", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 0, &do_about)) {}
            ui_panel_end();
        }
        if( ui_panel("Camera", 0)) {
            if( ui_float("Speed", &cam.speed) ) {}
            if( ui_float3("Position", cam.position.v3) ) {}
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
        if( ui_panel("Audio", 0)) {
            static float bgm = 1, sfx = 1, master = 1;
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("SFX", &sfx, va("%.2f", sfx))) audio_volume_clip(sfx);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_label2_toolbar("BGM: Waterworld Map" /*Wrath of the Djinn"*/, ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM1, 0);
            if( ui_label2_toolbar("BGM: Leisure Suit Larry", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM2, 0);
            if( ui_label2_toolbar("BGM: Monkey Island", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM3, 0);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP)) audio_play(SFX1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP)) audio_play(SFX2, 0);
            ui_panel_end();
        }

        input_demo(); // show some keyboard/mouse/gamepad UI tabs
        ui_demo(true); // show all UI widgets in a tab (true=showcase windows)
    }

    // data tests (json5)
    const char json5[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( json_push(json5) ) {
        assert( json_float("/abc") == 42.67 );
        assert( json_int("/def") == 1 );
        assert( json_int("/integer") == 0x100 );
        assert( json_float("/huge") > 2.22e5 );
        assert( strlen(json_string("/hello")) == 35 );
        assert( json_int("/children/a") == 1 );
        assert( json_int("/children.b") == 2 );
        assert( json_int("/children[c]") == 3 );
        assert( json_int("/array[%d]", 2) == -3 );
        assert( json_count("/invalids") == 8 );
        assert( isnan(json_float("/invalids[0]")) );
        assert( !json_find("/non_existing") );
        assert( PRINTF("json5 tests OK\n") );
        json_pop();
    }

    // data tests (xml)
    const char *xml = // vfs_read("test1.xml");
    "<!-- XML representation of a person record -->"
    "<person created=\"2006-11-11T19:23\" modified=\"2006-12-31T23:59\">"
    "    <firstName>Robert</firstName>"
    "    <lastName>Smith</lastName>"
    "    <address type=\"home\">"
    "        <street>12345 Sixth Ave</street>"
    "        <city>Anytown</city>"
    "        <state>CA</state>"
    "        <postalCode>98765-4321</postalCode>"
    "    </address>"
    "</person>";
    if( xml_push(xml) ) {
        assert( !strcmp("Robert", xml_string("/person/firstName/$")) );
        assert( !strcmp("Smith", xml_string("/person/lastName/$")) );
        assert( !strcmp("home", xml_string("/person/address/@type")) );
        assert( PRINTF("xml tests OK\n") );
        xml_pop();
    }

    // network test (https)
    array(char) webfile = download("https://www.google.com/");
    printf("Network test: %d bytes downloaded from google.com\n", array_count(webfile));

    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );
}
