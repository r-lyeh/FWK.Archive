// material demo
// - rlyeh, public domain
//
// @todo: node_print(obj, "");

#include "engine.h"


const char *skyboxes6[][3] = { // reflection, rad, env
    {"hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr"},
#if 0
    {"hdr/graffiti_shelter_4k.hdr","hdr/graffiti_shelter_Rad.hdr","hdr/graffiti_shelter_Env.hdr"},
    {"hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_Env.hdr"},
    {"hdr/Factory_Catwalk_1k.hdr","hdr/Factory_Catwalk_Rad.hdr","hdr/Factory_Catwalk_Env.hdr"},
    {"hdr/music_hall_01_4k.hdr","hdr/music_hall_01_4k.hdr","hdr/music_hall_01_Env.hdr"},
    {"hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_Env.hdr"},
    {"hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_Env.hdr"},
    {"hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_Env.hdr"},
    {"hdr/mesto.hdr","hdr/mesto.hdr","hdr/mesto_Env.hdr"},
#endif
};

int main() {
    // create the window
    window_create( 0.75f, WINDOW_VSYNC );
    window_color( GRAY );

    // create camera
    camera_t cam = camera();

    // fx: load all post fx files in all subdirs.
    fx_load("fx/**.fs");
    fx_enable_ordered(fx_find("fxTonemapACES.fs"));
    fx_enable_ordered(fx_find("fxFXAA3.fs"));
 
    // load video, RGB texture, no audio
    video_t *v = video( "pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB | VIDEO_NO_AUDIO | VIDEO_LOOP ); video_seek(v, 30);
    // load texture
    texture_t t1 = texture("kgirl/g01_texture.png", 0); 
    texture_t t2 = texture("matcaps/material3", 0); 
    // load model 
    model_t m1 = model("suzanne.obj", MODEL_UNIQUE|MODEL_NO_ANIMATIONS);
    model_t m5 = model("suzanne.obj", MODEL_UNIQUE|MODEL_NO_ANIMATIONS);
    model_t m2 = model("suzanne.obj", MODEL_UNIQUE|MODEL_NO_ANIMATIONS|MODEL_MATCAPS);
    model_t m3 = model("damagedhelmet.gltf", MODEL_UNIQUE|MODEL_NO_ANIMATIONS);
    // model_t m3 = model("Scutum_low.fbx", MODEL_UNIQUE|MODEL_NO_ANIMATIONS|MODEL_PBR); 
    model_t m4 = model("cube.obj", MODEL_UNIQUE|MODEL_NO_ANIMATIONS); 
    model_t m6 = model("cube.obj", MODEL_UNIQUE|MODEL_NO_ANIMATIONS); 
    // model_t m4 = model("avp/scene.gltf", MODEL_UNIQUE|MODEL_NO_ANIMATIONS|MODEL_PBR);
    // model_t m3 = model("Cerberus_LP.FBX", MODEL_UNIQUE|MODEL_NO_ANIMATIONS|MODEL_PBR); 

    array(char*) list = 0;
    for each_array( vfs_list("demos/art/shadertoys/**.fs"), char*, dir ) {
        array_push(list, STRDUP(file_name(dir)));
    }

    shadertoy_t sh = shadertoy(*list, SHADERTOY_IGNORE_MOUSE|SHADERTOY_FLIP_Y); // 0:no flags
    sh.dims.x = 1024;
    sh.dims.y = 1024;
    shadertoy_render(&sh, 0);

    scene_t *scene = obj_new(scene_t);

    // spawn object1 (diffuse)
    node_t* obj1 = obj_new(node_t);
    node_model(obj1, m1);
    node_diffuse(obj1, t1);
    node_scale(obj1, vec3(3,3,3));
    node_move(obj1, vec3(-10+5*0,0,-10));
    obj_attach(scene, obj1);

    // spawn object2 (matcap)
    node_t* obj2 = obj_new(node_t);
    node_model(obj2, m2);
    node_diffuse(obj2, t2);
    node_scale(obj2, vec3(3,3,3));
    node_move(obj2, vec3(-10+5*2,0,-10));
    obj_attach(scene, obj2);

    // spawn object3 (video)
    node_t* obj3 = obj_new(node_t);
    node_model(obj3, m5);
    node_diffuse(obj3, video_textures(v)[0]);
    node_scale(obj3, vec3(3,3,3));
    node_move(obj3, vec3(-10+5*1,0,-10));
    obj_attach(scene, obj3);

    // spawn object4 (pbr)
    node_t* obj4 = obj_new(node_t);
    node_model(obj4, m3);
    node_scale(obj4, vec3(3,3,3));
    node_move(obj4, vec3(-10+6*3,0,-30));
    node_pivot(obj4, vec3(0,180,180));
    obj_attach(scene, obj4);
    obj4->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value = 30.0f;

    // spawn object5 (shadertoy)
    node_t* obj5 = obj_new(node_t);
    node_model(obj5, m4); 
    node_diffuse(obj5, sh.tx); 
    node_scale(obj5, vec3(3,3,3));
    node_move(obj5, vec3(-10+8*3,0,-10));
    obj_attach(scene, obj5);

    // spawn object6 (solid)
    node_t* obj6 = obj_new(node_t);
    node_model(obj6, m6); 
    // node_diffuse(obj6, (texture)); 
    node_scale(obj6, vec3(3,3,3));
    node_move(obj6, vec3(-10+12*3,0,-10));
    obj_attach(scene, obj6);

    obj6->model.materials[0].layer[MATERIAL_CHANNEL_ALBEDO].map.color = vec4(1,1,1,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_AMBIENT].map.color = vec4(1,1,1,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].map.color = vec4(1,0,0,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value = 10.0f;
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_AO].map.color = vec4(1,1,1,1);
 
    light_t* l = obj_new(light_t); 
    light_type(l, LIGHT_POINT);
    l->diffuse = vec3(1,1,1);
    obj_attach(scene, l);
 
    // load skybox
    scene->skybox = skybox_pbr(skyboxes6[0][0], skyboxes6[0][1], skyboxes6[0][2]);
 
    fbo_t main_fb = fbo(window_width(), window_height(), 0, TEXTURE_FLOAT);

    bloom_params_t bloom_params = {
        .mips_count = 12,
        .filter_radius = 0.005f,
        .strength = 0.04f,
    };

    while(window_swap() && !input(KEY_ESC)) { 
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );

        // draw environment 
        fbo_resize(&main_fb, window_width(), window_height());
 
        // update video
        video_decode( v );

        // update light position
        light_pos(l, cam.position);

        // update shadertoy
        shadertoy_render(&sh, window_delta());

        // fps camera
        camera_freefly(&cam);

        fbo_resize(&main_fb, window_width(), window_height());
        fbo_bind(main_fb.id);
            viewport_clear(false, true);
            viewport_clip(vec2(0,0), vec2(window_width(), window_height()));
            scene_render(scene, SCENE_BACKGROUND|SCENE_FOREGROUND|SCENE_SHADOWS|SCENE_DRAWMAT);
            fx_drawpass(fx_find("fx/fxSSAO.fs"), main_fb.texture_color, main_fb.texture_depth);
        fbo_unbind();

        {
            texture_t bloom_fb = fxt_bloom(main_fb.texture_color, bloom_params);
            fbo_blit(main_fb.id, bloom_fb, 1);
            // fullscreen_quad_rgb_flipped(reflect_fb);
        }

        fx_apply(main_fb.texture_color, main_fb.texture_depth);

        if (ui_panel("FXs", 0)) {
            ui_fxs();
            ui_panel_end();
        }

        if (ui_panel("Materials", 0)) {
            ui_materials(&obj4->model);
            ui_panel_end();
        }

        if( ui_panel( "Viewer", 0 ) ) {
            for( int i = 0; i < countof(skyboxes6); i++ ) {
                const char *filename = skyboxes6[i][0];
                // bool selected = !strcmp(g_skybox.reflection->filename, file_name(filename));
                bool selected = false; 
                if( ui_bool( filename, &selected ) ) {
                    scene->skybox = skybox_pbr(skyboxes6[i][0], skyboxes6[i][1], skyboxes6[i][2]);
                }
            }
            ui_section("bloom");
            ui_int("Mips Count", &bloom_params.mips_count);
            ui_float("Filter Radius", &bloom_params.filter_radius);
            ui_float("Strength", &bloom_params.strength);
            ui_float("Threshold", &bloom_params.threshold);
            ui_float("Soft Threshold", &bloom_params.soft_threshold);
            ui_bool("Suppress Fireflies", &bloom_params.suppress_fireflies);
            ui_float("Cube emission",  &obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value);
            ui_panel_end();
        }

        static int selected = 0;
        if( ui_panel("Shadertoy", 0)) {
            for( int i = 0; i < array_count(list); ++i ) {
                bool in_use = i == selected;
                if( ui_bool(list[i], &in_use) ) {
                    sh = shadertoy( list[selected = i], SHADERTOY_IGNORE_MOUSE|SHADERTOY_FLIP_Y );
                    sh.dims.x = 1024;
                    sh.dims.y = 1024;
                    shadertoy_render(&sh, 0);
                    node_diffuse(obj5, sh.tx);
                }
            }
            ui_panel_end();
        }
    }
}
