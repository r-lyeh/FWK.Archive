//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

static
void camera_ctor(camera_t *cam) {
    cam->speed = 0.50f;
    cam->accel = 0.5f;
    cam->position = vec3(10,10,10);
    cam->updir = vec3(0,1,0);
    cam->rightdir = vec3(1,0,0);
    cam->fov = 45;
    cam->aspect = 0.0f;
    cam->frustum_fov_multiplier = 1.0f;
    cam->orthographic = false;
    cam->distance = 3; // len3(cam->position);
    cam->near_clip = 0.1f;
    cam->far_clip = 1000.f;

    cam->damping = false;
    cam->move_friction = 0.09f;
    cam->move_damping = 0.96f;
    cam->look_friction = 0.30f;
    cam->look_damping = 0.96f;
    cam->last_look = vec3(0,0,0);
    cam->last_move = vec3(0,0,0);

    // update proj & view
    camera_lookat(cam,vec3(-5,0,-5));

    // @todo: remove this hack that is used to consolidate dampings
    if( 1 ) {
        vec3 zero = {0};
        for( int i = 0; i < 1000; ++i ) {
            camera_moveby(cam, zero);
            camera_fps(cam,0,0);
        }
    }
}

AUTORUN {
    STRUCT(camera_t,mat44,view,"View matrix");
    STRUCT(camera_t,mat44,proj,"Projection matrix");
    STRUCT(camera_t,vec3,position,"Camera position");
    STRUCT(camera_t,vec3,updir,"Up direction vector");
    STRUCT(camera_t,vec3,lookdir,"Look direction vector");
    STRUCT(camera_t,vec3,rightdir,"Right direction vector");
    STRUCT(camera_t,float,yaw,"Yaw angle in degrees");
    STRUCT(camera_t,float,pitch,"Pitch angle in degrees");
    STRUCT(camera_t,float,roll,"Roll angle in degrees");
    STRUCT(camera_t,float,speed,"Camera movement speed");
    STRUCT(camera_t,float,accel,"Camera acceleration");
    STRUCT(camera_t,float,fov,"Field of view in degrees");
    STRUCT(camera_t,float,aspect,"Aspect ratio");
    STRUCT(camera_t,float,near_clip,"Near clipping plane distance");
    STRUCT(camera_t,float,far_clip,"Far clipping plane distance");
    STRUCT(camera_t,float,frustum_fov_multiplier,"Frustum FOV multiplier");
    STRUCT(camera_t,float,move_friction,"Movement friction");
    STRUCT(camera_t,float,move_damping,"Movement damping");
    STRUCT(camera_t,float,look_friction,"Look friction");
    STRUCT(camera_t,float,look_damping,"Look damping");
    STRUCT(camera_t,vec3,last_look,"Last look direction for friction and damping");
    STRUCT(camera_t,vec3,last_move,"Last movement for friction and damping");
    STRUCT(camera_t,bool,damping,"Damping enabled flag");
    STRUCT(camera_t,bool,orthographic,"Orthographic projection flag");
    STRUCT(camera_t,float,distance,"Distance to pivot when orbiting");
    obj_ctor[OBJTYPE_camera] = camera_ctor;
}

camera_t camera() {
    camera_t *old = last_camera;

    static camera_t cam = {0};
    do_once {
        camera_ctor(&cam);
    }

    if (!last_camera)
        *camera_get_active() = cam;
    last_camera = old;
    return cam;
}

camera_t *camera_get_active() {
    static camera_t defaults = {0};
    if( !last_camera ) {
        identity44(defaults.view);
        identity44(defaults.proj);
        last_camera = &defaults;
    }
    return last_camera;
}

void camera_moveby(camera_t *cam, vec3 inc) {
    // calculate camera damping
    if( cam->damping ) {
        float fr = cam->move_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->move_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_move = scale3(cam->last_move, 1 - fr);
        inc.x = cam->last_move.x = inc.x * (1 - sm) + cam->last_move.x * sm;
        inc.y = cam->last_move.y = inc.y * (1 - sm) + cam->last_move.y * sm;
        inc.z = cam->last_move.z = inc.z * (1 - sm) + cam->last_move.z * sm;
    }

    vec3 dir = norm3(cross3(cam->lookdir, cam->updir));
    cam->position = add3(cam->position, scale3(dir, inc.x)); // right
    cam->position = add3(cam->position, scale3(cam->updir, inc.y)); // up
    cam->position = add3(cam->position, scale3(cam->lookdir, inc.z)); // front

    camera_fps(cam, 0, 0);
}

void camera_teleport(camera_t *cam, vec3 pos) {
    bool damping = cam->damping;
    cam->damping = 0;
    cam->last_move = vec3(0,0,0);
    cam->position = pos;
    camera_fps(cam, 0, 0);
    cam->damping = damping;
}

void camera_lookat(camera_t *cam, vec3 target) {
    // invert expression that cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    // look.y = sin p > y = asin(p)
    // look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
    // look.z = sin y * cos p; -> cos p = look.z / sin y /
    // so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

    vec3 look = norm3(sub3(target, cam->position));
    const float rad2deg = 1 / 0.0174532f;
    float pitch = asin(look.y) * rad2deg;
    float yaw = atan2(look.z, look.x) * rad2deg; // coords swapped. it was (look.x, look.z) before. @todo: testme

    camera_fps(cam, yaw-cam->yaw, pitch-cam->pitch);
}

void camera_enable(camera_t *cam) {
    // camera_t *other = camera_get_active(); // init default camera in case there is none
    last_camera = cam;
    // trigger a dummy update -> update matrices
    camera_fps(cam, 0, 0);
}

void camera_freefly(camera_t *cam) {
    // static bool is_active = true;
    // if (input_down(KEY_HOME)) is_active = !is_active;
    // if (!is_active) return;
    bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
    window_cursor( !active );
    int mult_speed = input(KEY_LSHIFT) || input(KEY_LALT);

    static float speed_buildup = 1.0f;
    if( active ) cam->speed = clampf(cam->speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
    vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
    vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam->speed);
    if ( len3sq(wasdecq) ) speed_buildup += (cam->speed * cam->accel * (2.0f * mult_speed + 1.0f) * window_delta());
    // if (!active) speed_buildup = 1.0f;
    else speed_buildup = 1.0f;
    camera_moveby(cam, scale3(wasdecq, window_delta() * 60 * speed_buildup * (2.0f * mult_speed + 1.0f)));
    camera_fps(cam, mouse.x,mouse.y);
}

frustum camera_frustum_build(camera_t *cam) {
    float aspect = cam->aspect ? cam->aspect : window_width() / ((float)window_height()+!window_height());
    float fov = cam->fov * cam->frustum_fov_multiplier;
    mat44 proj;
    if( cam->orthographic ) {
        ortho44(proj, -fov * aspect, fov * aspect, -fov, fov, cam->near_clip, cam->far_clip);
    } else {
        perspective44(proj, fov, aspect, cam->near_clip, cam->far_clip);
    }
    mat44 projview; multiply44x2(projview, proj, cam->view);
    return frustum_build(projview);
}

void camera_fov(camera_t *cam, float fov) {
    last_camera = cam;

    float aspect = cam->aspect ? cam->aspect : window_width() / ((float)window_height()+!window_height());

    cam->fov = fov;

    if( cam->orthographic ) {
        ortho44(cam->proj, -cam->fov * aspect, cam->fov * aspect, -cam->fov, cam->fov, cam->near_clip, cam->far_clip);
        // [ref] https://commons.wikimedia.org/wiki/File:Isometric_dimetric_camera_views.png
        // float pitch = cam->dimetric ? 30.000f : 35.264f; // dimetric or isometric
        // cam->pitch = -pitch; // quickly reorient towards origin
    } else {
        perspective44(cam->proj, cam->fov, aspect, cam->near_clip, cam->far_clip);
    }
}

void camera_fps2(camera_t *cam, float yaw, float pitch, float roll) {
    last_camera = cam;

    // camera damping
    if( cam->damping ) {
        float fr = cam->look_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->look_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_look = scale3(cam->last_look, 1 - fr);
        yaw = cam->last_look.y = yaw * (1 - sm) + cam->last_look.y * sm;
        pitch = cam->last_look.x = pitch * (1 - sm) + cam->last_look.x * sm;
        roll = cam->last_look.z = roll * (1 - sm) + cam->last_look.z * sm;
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;
    cam->roll += roll;
    cam->roll += fmod(cam->roll, 360);

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad, r = cam->roll * deg2rad;
    cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    vec3 up = vec3(0,1,0);
    // calculate updir
    {
        float cosfa = cosf(r);
        float sinfa = sinf(r);
        vec3 right = cross3(cam->lookdir, up);
        cam->rightdir = right;
        float th = dot3(cam->lookdir, up);

        cam->updir.x = up.x * cosfa + right.x * sinfa + cam->lookdir.x * th * (1.0f - cosfa);
        cam->updir.y = up.y * cosfa + right.y * sinfa + cam->lookdir.y * th * (1.0f - cosfa);
        cam->updir.z = up.z * cosfa + right.z * sinfa + cam->lookdir.z * th * (1.0f - cosfa);
    }

    lookat44(cam->view, cam->position, add3(cam->position, cam->lookdir), cam->updir); // eye,center,up

    camera_fov(cam, cam->fov);
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    camera_fps2(cam, yaw, pitch, 0.0f);
}

void camera_orbit( camera_t *cam, float yaw, float pitch, float inc_distance ) {
    last_camera = cam;

    // update dummy state
    camera_fps(cam, 0,0);

    // @todo: add damping
    vec3 _mouse = vec3(yaw, pitch, inc_distance);
    cam->yaw += _mouse.x;
    cam->pitch += _mouse.y;
    cam->distance += _mouse.z;

    // look: limit pitch angle [-89..89]
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    // compute view matrix
    float x = rad(cam->yaw), y = rad(-cam->pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
    lookat44(cam->view, vec3( cx*cy*cam->distance, sy*cam->distance, sx*cy*cam->distance ), vec3(0,0,0), vec3(0,1,0) );

    // save for next call
    cam->last_move.x = _mouse.x;
    cam->last_move.y = _mouse.y;
}

int ui_camera( camera_t *cam ) {
    int changed = 0;
    changed |= ui_bool("Orthographic", &cam->orthographic);
    changed |= ui_bool("Damping", &cam->damping);
    if( !cam->damping ) ui_disable();
    changed |= ui_slider2("Move friction", &cam->move_friction, va("%5.3f", cam->move_friction));
    changed |= ui_slider2("Move damping", &cam->move_damping, va("%5.3f", cam->move_damping));
    changed |= ui_slider2("View friction", &cam->look_friction, va("%5.3f", cam->look_friction));
    changed |= ui_slider2("View damping", &cam->look_damping, va("%5.3f", cam->look_damping));
    if( !cam->damping ) ui_enable();
    ui_separator();
    changed |= ui_float("Speed", &cam->speed);
    changed |= ui_float3("Position", cam->position.v3);
    changed |= ui_float("Pitch", &cam->pitch);
    changed |= ui_float("Yaw", &cam->yaw);
    changed |= ui_float("Roll", &cam->roll);
    changed |= ui_float3("LookDir", cam->lookdir.v3);
    changed |= ui_float3("UpDir", cam->updir.v3);
    ui_disable();
    changed |= ui_mat44("View matrix", cam->view);
    ui_enable();
    ui_separator();
    changed |= ui_float("FOV (degrees)", &cam->fov);
    changed |= ui_float("Orbit distance", &cam->distance);
    ui_disable();
    changed |= ui_mat44("Projection matrix", cam->proj);
    ui_enable();
    return changed;
}

// -----------------------------------------------------------------------------

static
void node_update(node_t *obj) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(obj->euler.x,obj->euler.y,obj->euler.z));
    compose44(obj->transform, obj->pos, mulq(e, p), obj->sca);
}

static 
void node_ctor(node_t *obj) {
    identity44(obj->transform);
    obj->sca = vec3(1,1,1);
    node_rotate(obj, vec3(0,0,0));
    obj->cast_shadows = true;
    obj->batchable = true;
}

AUTORUN {
    STRUCT(node_t, uint64_t, renderbucket, "Render bucket");
    STRUCT(node_t, mat44, transform, "Transform matrix");
    STRUCT(node_t, quat, rot, "Rotation quaternion");
    STRUCT(node_t, vec3, sca, "Scale vector");
    STRUCT(node_t, vec3, pos, "Position vector");
    STRUCT(node_t, vec3, euler, "Euler angles");
    STRUCT(node_t, vec3, pivot, "Pivot point");
    STRUCT(node_t, array(texture_t), textures, "Textures array");
    STRUCT(node_t, model_t, model, "Model");
    STRUCT(node_t, model_t, model_shadow, "Shadow model");
    STRUCT(node_t, anim_t, anim, "Animation");
    STRUCT(node_t, float, anim_speed, "Animation speed");
    STRUCT(node_t, aabb, bounds, "Bounding box");
    STRUCT(node_t, unsigned, billboard, "Billboard mode"); // [0..7] x(4),y(2),z(1) masks
    STRUCT(node_t, bool, disable_frustum_check, "Disable frustum check");
    STRUCT(node_t, bool, cast_shadows, "Cast shadows");
    STRUCT(node_t, bool, fullbright, "Fullbright");
    STRUCT(node_t, bool, batchable, "Batchable");

    // internal states
    STRUCT(node_t, float, distance, "_Distance");
    STRUCT(node_t, bool, skip_draw, "_Skip draw");
    STRUCT(node_t, bool, was_batched, "_Was batched");
    STRUCT(node_t, array(mat44), instances, "_Instances");
    STRUCT(node_t, unsigned, num_instances, "_Number of instances");
    STRUCT(node_t, array(unsigned), pair_instance, "_Pair instance");
    STRUCT(node_t, uint32_t, checksum, "_Checksum");

    obj_ctor[OBJTYPE_node] = node_ctor;
}

node_t object() {
    node_t obj = {0};
    node_ctor(&obj);
    return obj;
}

bool node_compare(node_t *obj1, node_t *obj2) {
    // if (obj1->renderbucket != obj2->renderbucket) return false;
    // if (memcmp(obj1->transform, obj2->transform, sizeof(mat44))) return false;
    // if (memcmp(&obj1->rot, &obj2->rot, sizeof(quat))) return false;
    // if (memcmp(&obj1->sca, &obj2->sca, sizeof(vec3))) return false;
    // if (memcmp(&obj1->pos, &obj2->pos, sizeof(vec3))) return false;
    // if (memcmp(&obj1->euler, &obj2->euler, sizeof(vec3))) return false;
    // if (memcmp(&obj1->pivot, &obj2->pivot, sizeof(vec3))) return false;
    // if (memcmp(&obj1->bounds, &obj2->bounds, sizeof(aabb))) return false;
    if (memcmp(&obj1->anim, &obj2->anim, sizeof(anim_t))) return false;
    // if (obj1->anim_speed != obj2->anim_speed) return false;
    if (obj1->billboard != obj2->billboard) return false;
    if (obj1->fullbright != obj2->fullbright) return false;
    if (obj1->checksum != obj2->checksum) return false;
    return true;
}

void node_pivot(node_t *obj, vec3 euler) {
    obj->pivot = euler;
    node_update(obj);
}

void node_rotate(node_t *obj, vec3 euler) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(euler.x,euler.y,euler.z));
    obj->rot = mulq(p,e);
    obj->euler = euler;
    node_update(obj);
}

void node_teleport(node_t *obj, vec3 pos) {
    obj->pos = pos;
    node_update(obj);
}

void node_move(node_t *obj, vec3 inc) {
    obj->pos = add3(obj->pos, inc);
    node_update(obj);
}

void node_scale(node_t *obj, vec3 sca) {
    obj->sca = vec3(sca.x, sca.y, sca.z);
    node_update(obj);
}

void node_batchable(node_t *obj, bool batchable) {
    obj->batchable = batchable;
}

vec3 node_position(node_t *obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void node_model(node_t *obj, model_t model) {
    obj->model = model;
}

void node_model_shadow(node_t *obj, model_t model) {
    obj->model_shadow = model;
}

void node_anim(node_t *obj, anim_t anim, float speed) {
    obj->anim = anim;
    obj->anim_speed = speed;
}

void node_diffuse(node_t *obj, texture_t tex) {
    if (!obj->model.iqm) return;
    material_layer_t *layer = &obj->model.materials[obj->model.iqm->mesh_materials[0]].layer[MATERIAL_CHANNEL_ALBEDO];
    if (!layer->map.texture) {
        layer->map.texture = CALLOC(1, sizeof(texture_t));
    }
    *layer->map.texture = tex;
}

void node_billboard(node_t *obj, unsigned mode) {
    obj->billboard = mode;
}

// -----------------------------------------------------------------------------

static
void scene_ctor(scene_t *s) {
    // s->skybox = skybox(NULL, 0);
}

static
void scene_dtor(scene_t *s) {
    // skybox_destroy(&s->skybox);
    drawmat_destroy(&s->drawmat);
    shadowmap_destroy(&s->shadowmap);
}

static void scene_init() {
#ifndef __EMSCRIPTEN__ // @fixme ems -> shaders
    // do_once scene_push();      // create an empty scene by default
#endif
}

int scene_merge(scene_t *s, const char *source) {
    int count = 0;
    if( json_push(source) ) {
        for(int i = 0, e = json_count("/") - 1; i <= e ; ++i) {
            const char *skybox_folder = json_string("/[%d]/skybox",i);
            if( skybox_folder[0] ) {
                PRINTF("Loading skybox folder: %s\n", skybox_folder);
                s->skybox = skybox( skybox_folder, 0 );
                continue;
            }
            const char *mesh_file = json_string("/[%d]/mesh",i);
            const char *texture_file = json_string("/[%d]/texture",i);
            const char *animation_file = json_string("/[%d]/animation",i);
            vec3 position = vec3(json_float("/[%d]/position[0]",i),json_float("/[%d]/position[1]",i),json_float("/[%d]/position[2]",i));
            vec3 rotation = vec3(json_float("/[%d]/rotation[0]",i),json_float("/[%d]/rotation[1]",i),json_float("/[%d]/rotation[2]",i));
            vec3 scale = scale3(vec3(1,1,1), json_float("/[%d]/scale",i));
            bool opt_swap_zy = json_int("/[%d]/swapzy",i);
            bool opt_flip_uv = json_int("/[%d]/flipuv",i);
            bool opt_fullbright = json_int("/[%d]/fullbright",i);
            bool enable_pbr = json_int("/[%d]/pbr",i);
            PRINTF("Scene %d/%d Loading: %s\n", i, e, mesh_file);
            PRINTF("Scene %d/%d Texture: %s\n", i, e, texture_file);
            PRINTF("Scene %d/%d Animation: %s\n", i, e, animation_file);
            PRINTF("Scene %d/%d Position: (%f,%f,%f)\n", i, e, position.x, position.y, position.z);
            PRINTF("Scene %d/%d Rotation: (%f,%f,%f)\n", i, e, rotation.x, rotation.y, rotation.z);
            PRINTF("Scene %d/%d Scale: (%f,%f,%f)\n", i, e, scale.x, scale.y, scale.z);
            PRINTF("Scene %d/%d Swap_ZY: %d\n", i, e, opt_swap_zy );
            PRINTF("Scene %d/%d Flip_UV: %d\n", i, e, opt_flip_uv );
            model_t m = model(mesh_file, enable_pbr ? 0 : MODEL_NO_PBR);
            //char *a = archive_read(animation_file);
            node_t *o = obj_new(node_t);
            node_model(o, m);
            if( texture_file[0] ) node_diffuse(o, texture_compressed(texture_file, (opt_flip_uv ? IMAGE_FLIP : 0)) );
            node_scale(o, scale);
            node_teleport(o, position);
            node_pivot(o, rotation); // node_rotate(o, rotation);
            o->fullbright = opt_fullbright;
            obj_attach(s, o);
            //node_name(x), scene_find(name)
// o->bounds = aabb(mul3(m.bounds.min,o->sca),mul3(m.bounds.max,o->sca));
// PRINTF("aabb={%f,%f,%f},{%f,%f,%f}\n", o->bounds.min.x, o->bounds.min.y, o->bounds.min.z, o->bounds.max.x, o->bounds.max.y, o->bounds.max.z);

/*
            if(opt_swap_zy) {
                // swap zy bounds
                vec3 min = o->bounds.min, max = o->bounds.max;
                o->bounds = aabb( vec3(min.x,min.z,min.y), vec3(max.x,max.z,max.y) );
            }
*/

            count++;
        }
        json_pop();
    }
    // PRINTF("scene loading took %5.2fs\n", secs);
    PRINTF("%d objects merged into scene\n", count);
    return count;
}

static
int scene_obj_distance_compare(const void *a, const void *b) {
    const node_t *da = a, *db = b;
    return da->distance < db->distance ? 1 : da->distance > db->distance ? -1 : 0;
}

static
void scene_collect_nodes(scene_t *root, obj *parent) {
    for each_objchild(parent, obj*, o) {
        if (obj_typeid(o) == OBJTYPE_node) {
            array_push(root->renderlist, (node_t*)o);
        }
        else if (obj_typeid(o) == OBJTYPE_light) {
            array_push(root->lights, (light_t*)o);
        }
        scene_collect_nodes(root, o);
    }
}

static
int scene_draw(scene_t *s) {
    int flags = s->flags;
    camera_t *cam = camera_get_active();
    shadowmap_t *sm = &s->shadowmap;

    array_clear(s->renderlist);
    array_clear(s->lights);
    scene_collect_nodes(s, (obj*)s);

    static light_t light_block[MAX_LIGHTS];
    int light_count = 0;
    for (int i = 0; i < array_count(s->lights); ++i) {
        if (light_count >= MAX_LIGHTS) break;
        light_t *l = s->lights[i];
        light_block[light_count++] = *l;
    }

    if (flags & SCENE_POSTFX) {
        fx_begin();
    }

    if (flags & SCENE_SHADOWS) {
        if (sm->vsm_texture_width == 0) {
            *sm = shadowmap(DEFAULT_SHADOW_VSM_RESOLUTION, DEFAULT_SHADOW_CSM_RESOLUTION);
        }
    } else {
        sm = NULL;
    }

    if(flags & SCENE_BACKGROUND) {
        if(s->skybox.program) {
            skybox_render(&s->skybox, cam->proj, cam->view);
        }

        ddraw_flush();
    }

    if( flags & SCENE_FOREGROUND ) {
        for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
            node_t *obj = s->renderlist[j];
            model_t *model = &obj->model;
            obj->was_batched = false;

            model->billboard = obj->billboard;
            for (int p = 0; p < RENDER_PASS_OVERRIDES_BEGIN; ++p) {
                model->rs[p].cull_face_enabled = flags&SCENE_CULLFACE ? 1 : 0;
                model->rs[p].polygon_mode_draw = flags&SCENE_WIREFRAME ? GL_LINE : GL_FILL;
            }
            obj->checksum = obj->model.iqm ? model_checksum(&obj->model) : 0;
        }

        for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
            node_t *obj = s->renderlist[j];
            model_t *model = &obj->model;
            anim_t *anim = &obj->anim;
            mat44 *views = (mat44*)(&cam->view);

            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            if (anim) {
                float delta = window_delta() * obj->anim_speed;
                model->curframe = model_animate_clip(*model, model->curframe + delta, anim->from, anim->to, anim->flags & ANIM_LOOP );
            }

            if (!obj->fullbright) {
                model_skybox(model, s->skybox);
            } else {
                skybox_t sb = {0};
                model_skybox(model, sb);
            }
            
            array_resize(obj->instances, 64);
            copy44(obj->instances[0], obj->transform);

            int num_instances = 1;
            for (unsigned k = j+1; k < obj_count; ++k) {
                node_t *obj2 = s->renderlist[k];
                if (!obj2->batchable || obj2->skip_draw || !node_compare(obj, obj2)) {
                    continue;
                }
                if (num_instances >= array_count(obj->instances)) {
                    array_resize(obj->instances, array_count(obj->instances) + 64);
                }
                copy44(obj->instances[num_instances++], obj2->transform);
                obj2->was_batched = true;
            }
            obj->num_instances = num_instances;
        }

        /* Build shadowmaps */
        if (flags & SCENE_SHADOWS) { 
            shadowmap_begin(sm);
            for (unsigned j = 0; j < light_count; ++j) {
                light_t *l = &light_block[j];
                while (shadowmap_step(sm)) {
                    shadowmap_light(sm, l, cam->proj, cam->view);
                    if (!l->processed_shadows) continue;
                    for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
                        node_t *obj = s->renderlist[j];
                        model_t *model = obj->model_shadow.iqm ? &obj->model_shadow : &obj->model;
                        if (obj->model.iqm && obj->cast_shadows && !obj->was_batched) {
                            model_render_instanced(*model, cam->proj, cam->view, obj->instances, obj->num_instances);
                        }
                    }
                }
            }
            shadowmap_end(sm);
        }

        if (flags & SCENE_DRAWMAT) {
            drawmat_clear(&s->drawmat);

            for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
                node_t *obj = s->renderlist[j];
                model_t *model = &obj->model;
                if (!model->iqm) continue;
                if (obj->skip_draw) continue;
                if (obj->was_batched) continue;
                drawmat_render_instanced(&s->drawmat, *model, cam->proj, cam->view, obj->instances, obj->num_instances);
            }
        }

        /* Collect all transparency enabled models and sort them by distance */
        static array(node_t*) transparent_objects = 0;
        for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
            node_t *obj = s->renderlist[j];
            model_t *model = &obj->model;

            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            if (model_has_transparency(*model)) {
                obj->distance = len3sq(sub3(cam->position, transform344(model->pivot, obj->pos)));
                array_push(transparent_objects, obj);
            }
        }

        array_sort(transparent_objects, scene_obj_distance_compare);

        /* Opaque pass */
        for(unsigned j = 0, obj_count = array_count(s->renderlist); j < obj_count; ++j ) {
            node_t *obj = s->renderlist[j];
            model_t *model = &obj->model;
            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            model_shadow(model, sm);
            model_light(model, light_count, light_block);
            model_render_instanced_pass(*model, cam->proj, cam->view, obj->instances, obj->num_instances, RENDER_PASS_OPAQUE);
        }

        /* Transparency pass */
        for (unsigned j = 0; j < array_count(transparent_objects); ++j) {
            node_t *obj = transparent_objects[j];
            model_t *model = &obj->model;
            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            model_shadow(model, sm);
            model_light(model, light_count, light_block);
            model_render_instanced_pass(*model, cam->proj, cam->view, obj->instances, obj->num_instances, RENDER_PASS_TRANSPARENT);
        }

        array_resize(transparent_objects, 0);
        glBindVertexArray(0);
    }

    if (flags & SCENE_POSTFX) {
        fx_end(0,0);
    }
    return 1;
}

AUTORUN {
    STRUCT(scene_t, int, flags, "Flags");

    // special objects below:
    STRUCT(scene_t, skybox_t, skybox, "Skybox");
    STRUCT(scene_t, shadowmap_t, shadowmap, "Shadowmap");
    STRUCT(scene_t, drawmat_t, drawmat, "Drawmat");

    obj_ctor[OBJTYPE_scene] = scene_ctor;
    obj_dtor[OBJTYPE_scene] = scene_dtor;
    obj_draw[OBJTYPE_scene] = scene_draw;
}

void scene_render(scene_t *s, int flags) {
    s->flags = flags;
    scene_draw(s);
}
