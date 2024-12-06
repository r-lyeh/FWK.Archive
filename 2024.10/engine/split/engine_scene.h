// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t { OBJ
    mat44 view, proj;
    vec3 position, updir, lookdir, rightdir;
    float yaw, pitch, roll; // mirror of (x,y) lookdir in deg;
    float speed, accel, fov, aspect; // fov in deg(45)
    float near_clip, far_clip;
    float frustum_fov_multiplier;

    float move_friction, move_damping;
    float look_friction, look_damping;
    vec3 last_look; vec3 last_move; // used for friction and damping
    bool damping;

    bool orthographic; // 0 perspective, 1 orthographic; when ortho: dimetric[if pitch == -30º], isometric[if pitch == 35.264º]
    float distance;    // distance to pivot, when orbiting
    // vec2 polarity = { +1,-1 }; // @todo
    // vec2 sensitivity = { 2,2 }; // @todo
} camera_t;
OBJTYPEDEF(camera_t,OBJTYPE_camera);

API camera_t camera();
API void camera_teleport(camera_t *cam, vec3 pos);
API void camera_moveby(camera_t *cam, vec3 inc);
API void camera_fov(camera_t *cam, float fov);
API void camera_fps(camera_t *cam, float yaw, float pitch);
API void camera_fps2(camera_t *cam, float yaw, float pitch, float roll);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API void camera_freefly(camera_t *cam);
API frustum camera_frustum_build(camera_t *cam);
API camera_t *camera_get_active();

API int  ui_camera(camera_t *cam);
API void ddraw_camera(camera_t *cam);

// scene node

typedef struct node_t { ENTITY
    uint64_t renderbucket;
    mat44 transform;
    quat rot;
    vec3 sca, pos, euler, pivot;
    array(texture_t) textures;
    model_t model, model_shadow;
    anim_t anim;
    float anim_speed;
    aabb bounds;
    unsigned billboard; // [0..7] x(4),y(2),z(1) masks
    bool disable_frustum_check;
    bool cast_shadows;
    bool fullbright;
    bool batchable;

    // internal states
    float distance;
    bool skip_draw;
    bool was_batched;
    array(mat44) instances;
    unsigned num_instances;
    array(unsigned) pair_instance;
    uint32_t checksum;
} node_t;
OBJTYPEDEF(node_t,OBJTYPE_node);

API bool node_compare(node_t *obj1, node_t *obj2);
API void node_rotate(node_t *obj, vec3 euler);
API void node_pivot(node_t *obj, vec3 euler);
API void node_teleport(node_t *obj, vec3 pos);
API void node_move(node_t *obj, vec3 inc);
API vec3 node_position(node_t *obj);
API void node_scale(node_t *obj, vec3 sca);
API void node_batchable(node_t *obj, bool batchable);
//
API void node_model(node_t *obj, model_t model);
API void node_model_shadow(node_t *obj, model_t model);
API void node_anim(node_t *obj, anim_t anim, float speed);
API void node_diffuse(node_t *obj, texture_t tex);
API void node_billboard(node_t *obj, unsigned mode);

// scene

enum SCENE_FLAGS {
    SCENE_WIREFRAME = 1,
    SCENE_CULLFACE = 2,
    SCENE_BACKGROUND = 4,
    SCENE_FOREGROUND = 8,
    SCENE_UPDATE_SH_COEF = 16,
    SCENE_SHADOWS = 32,
    SCENE_POSTFX = 64,
    SCENE_DRAWMAT = 128,
    // SCENE_DISABLE_BATCHING = 64,
};

typedef struct scene_t { OBJ
    int flags; // used by obj2
    array(node_t*) renderlist;
    array(light_t*) lights;

    // special objects below:
    skybox_t skybox;
    shadowmap_t shadowmap;
    drawmat_t drawmat;
} scene_t;
OBJTYPEDEF(scene_t,OBJTYPE_scene);

API int  scene_merge(scene_t *s, const char *source);
API void scene_render(scene_t *s, int flags);
