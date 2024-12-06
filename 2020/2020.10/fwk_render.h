// naive rendering framework
// - rlyeh, public domain
//
// roadmap:
// todo(1): models, anims,
// todo(2): anim controllers I, II, III,
// todo(3): instancing, bounding boxes, culling
// todo(4): compressed textures
// todo(5): projections (dimetric, isometric, ...)
// deliberately avoided in order to keep things small & simple. to be done in a different project:
// todo(1): light, shadowing, shading,
// todo(2): ao, aa, postprocesses,
// todo(3): skybox, cubemaps,
// todo(4): materials,
// todo(5): pbr, ibl,
// todo(6): shadertoy textures, video textures, animated textures,

#ifndef RENDER_H
#define RENDER_H

typedef unsigned handle; // GLuint

// texture

typedef struct {
    handle id;
    unsigned x, y, n;
} texture_t;

texture_t texture(const char* filename, int flags);

// mesh

typedef struct {
    handle vao, vbo, ibo;
    unsigned vertex_count;
    unsigned index_count;
    unsigned flags;
} mesh_t;

mesh_t mesh_from_mem(float* vertices, float* normals, float* uv, unsigned vertex_count, int flags);
mesh_t mesh_from_mem2(const char *format, int vertex_stride,int vertex_count,void *vertex_data, int index_count,void *index_data, int flags);

// model

typedef struct {
    mesh_t *mesh;
    unsigned mesh_count;
} model_t;

model_t model(const char* filename, int flags);

// camera

typedef struct {
    mat44 view, proj;
    vec3 position, up, look;
    float yaw, pitch, speed; // mirror_x, mirror_y;
} camera_t;

camera_t camera();
void camera_move(camera_t *cam, float x, float y, float z);
void camera_fps(camera_t *cam, float yaw, float pitch);
void camera_lookat(camera_t *cam, vec3 target);
camera_t *camera_get_active();

// object

typedef struct {
    mat44 transform;
    quat rot;
    vec3 sca;
    handle texture_id;
    model_t model;
} object_t;

object_t object(model_t);
void object_diffuse(object_t* obj, texture_t tex);
void object_rotate(object_t* obj, vec3 axis, float angle);
void object_teleport(object_t* obj, vec3 pos);
void object_move(object_t* obj, vec3 inc);
vec3 object_position(object_t* obj);
void object_scale(object_t* obj, vec3 sca); // @todo

// scene

typedef struct {
    handle program;
    object_t objs[1024];
    unsigned obj_count;
} scene_t;

scene_t* scene_new();
void scene_render(scene_t* scn, camera_t camera);
object_t* scene_spawn(scene_t* scn, model_t model);

#endif

#ifdef RENDER_C
#pragma once

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C
//
void glDebugCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char * message, void * userdata) {
    // whitelisted codes (also: 131169, 131204).
    if( id == 131154 ) return; // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    if( id == 131185 ) return; // Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if( id == 131218 ) return; // Program/shader state performance warning: Vertex shader in program 9 is being recompiled based on GL state.
    if( id == 2 ) return; // INFO: API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change. [ID: 2]

    const char * GL_ERROR_SOURCE[] = { "API", "WINDOW SYSTEM", "SHADER COMPILER", "THIRD PARTY", "APPLICATION", "OTHER" };
    const char * GL_ERROR_SEVERITY[] = { "HIGH", "MEDIUM", "LOW", "NOTIFICATION" };
    const char * GL_ERROR_TYPE[] = { "ERROR", "DEPRECATED BEHAVIOR", "UNDEFINED DEHAVIOUR", "PORTABILITY", "PERFORMANCE", "OTHER" };

    severity = severity == GL_DEBUG_SEVERITY_NOTIFICATION ? 3 : severity - GL_DEBUG_SEVERITY_HIGH;
    source = source - GL_DEBUG_SOURCE_API;
    type = type - GL_DEBUG_TYPE_ERROR;

    //printf("[SEVERITY: %s] [SOURCE: %s] [TYPE: %s] ", GL_ERROR_SEVERITY[severity], GL_ERROR_SOURCE[source], GL_ERROR_TYPE[type]);
    printf( "%s [ID: %u]\n", message, id );
}
void glDebugEnable() {
    static int once = 1; for(;once;once=0) {
    typedef void (*GLDEBUGPROC)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t, const char *, const void *);
    typedef void (*GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
    void (*glDebugMessageCallback)(GLDEBUGPROC, const void *) = (GLDEBUGMESSAGECALLBACKPROC)glfwGetProcAddress("glDebugMessageCallback");
    glDebugMessageCallback((GLDEBUGPROC)glDebugCallback, NULL);
    }
}
void glNewFrame() {
    glViewport(0, 0, window_width(), window_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0,0,0,1);
}

static const char* const vertex_shader_32_2 =
    "#version 150\n"

    "in vec3 vposition;"
    "in vec2 vtexcoords;"
    "out vec2 texcoords;"

    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"

    "void main() {"
    "    gl_Position = proj * view * model * vec4(vposition, 1.0);"
    "    texcoords = vtexcoords;"
    "}";

static const char* const fragment_shader_2_4 =
    "#version 150\n"

    "in vec2 texcoords;"
    "uniform sampler2D texunit;"
    "out vec4 fragcolor;"

    "void main() {"
    "    fragcolor = texture(texunit, texcoords);"
    "}";

static handle shader(const char *vertex_shader, const char *fragment_shader) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "vposition");
    glBindAttribLocation(program, 1, "vtexcoords");
    glBindFragDataLocation(program, 0, "fragcolor");
    glLinkProgram(program);

    GLsizei len;
    char log[1024] = {0};
    glGetProgramInfoLog(program, 1024-1, &len, log);
    if(log[0]) fprintf(stderr, "%s\n", log);

    return program;
}

texture_t texture(const char* filename, int flags) {
    texture_t t = {0};
    if( flags ) stbi_set_flip_vertically_on_load(1);
    unsigned char* image = stbi_load(filename, &t.x, &t.y, &t.n, 0);
    if( image ) {
        GLuint texture_type = t.n == 4 ? GL_RGBA : t.n == 3 ? GL_RGB : t.n == 2 ? GL_RG : GL_RED;
        glGenTextures(1, &t.id);
        glBindTexture(GL_TEXTURE_2D, t.id);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_type, t.x, t.y, 0, texture_type, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        stbi_image_free(image);
    }
    return t;
}

void scene_render(scene_t* scene, camera_t cam) {
    lookat44(cam.view, cam.position, add3(cam.look, cam.position), cam.up);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

    for( unsigned j = 0; j < scene->obj_count; j++ ) {
        object_t *object = &scene->objs[j];
        model_t *model = &object->model;
        for( unsigned i = 0; i < model->mesh_count; ++i ) {
            mesh_t *mesh = &model->mesh[i];

            glUseProgram(scene->program);
            glBindTexture(GL_TEXTURE_2D, object->texture_id);
            GLuint uni_location = glGetUniformLocation(scene->program, "model");
            glUniformMatrix4fv(uni_location, 1, GL_FALSE, object->transform);
            uni_location = glGetUniformLocation(scene->program, "view");
            glUniformMatrix4fv(uni_location, 1, GL_FALSE, cam.view);
            uni_location = glGetUniformLocation(scene->program, "proj");
            glUniformMatrix4fv(uni_location, 1, GL_FALSE, cam.proj);
            uni_location = glGetUniformLocation(scene->program, "texunit");
            glUniform1i(uni_location, 0); // ACTIVE_TEXTURE_UNIT

            glBindVertexArray(mesh->vao);
            // glDrawArrays(GL_TRIANGLES, 0, scene->objs[i].mesh.vertex_count);
            if( mesh->ibo ) { // with indices
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo); // <-- why intel?
                glDrawElements(/*mesh->flags & MESH2_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP :*/ GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, (char*)0);
            } else { // with vertices only
                glDrawArrays(/*mesh->flags & MESH2_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP :*/ GL_TRIANGLES, 0, mesh->vertex_count /* / 3 */);
            }
        }
    }
    glBindVertexArray(0);
}

// ---

mesh_t mesh_from_mem(float* vertices, float* normals, float* uv, unsigned vertex_count, int flags) {
    mesh_t mesh = {0};
    mesh.vertex_count = vertex_count;
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * vertex_count, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    if( uv ) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * vertex_count, uv, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }

    if( normals ) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * vertex_count, normals, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
    }

    glBindVertexArray(0);
    return mesh;
}

enum MESH2_FLAGS {
    MESH2_STATIC = 0, // , DYNAMIC, STREAM // zero|single|many updates per frame
    MESH2_STREAM = 1,
    MESH2_TRIANGLE_STRIP = 2,
};

mesh_t mesh_from_mem2(const char *format, int vertex_stride,int vertex_count,void *vertex_data, int index_count,void *index_data, int flags) {
    mesh_t z = {0}, *m = &z;

    m->flags = flags;

    // setup
    unsigned sizeof_index = sizeof(GLuint);
    unsigned sizeof_vertex = 0;
    m->index_count = index_count;
    m->vertex_count = vertex_count;

    // iterate vertex attributes { position, normal + uv + tangent + bitangent + ... }
    struct vertex_descriptor {
        int vertex_type, num_attribute, num_components;
        int stride, offset;
    } descriptor[16] = {0}, *dc = &descriptor[0];

    do switch( *format ) {
        break; case '0': dc->num_components = 0;
        break; case '1': dc->num_components = 1;
        break; case '2': dc->num_components = 2;
        break; case '3': dc->num_components = 3;
        break; case '4': dc->num_components = 4;
        break; case 'f': dc->vertex_type = GL_FLOAT;
        break; case 'u': dc->vertex_type = GL_UNSIGNED_INT;
        break; case 'b': if(format[-1] >= '0' && format[-1] <= '9') dc->vertex_type = GL_UNSIGNED_BYTE; //else bitangent.
        break; case ' ': while (format[1] == ' ') format++; case '\0':
            if (!dc->vertex_type) dc->vertex_type = GL_FLOAT;
            dc->offset = sizeof_vertex;
            sizeof_vertex += (dc->stride = dc->num_components * (dc->vertex_type == GL_UNSIGNED_BYTE ? 1 : 4));
            ++dc;
        break; default: if( !strchr("pntcwai", *format) ) PANIC("unsupported vertex type '%c'", *format);
    } while (*format++);

    if(vertex_stride > 0) sizeof_vertex = vertex_stride;

    // layout
    if(!m->vao) glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // index data
    if( index_data && index_count ) {
        m->index_count = index_count;

        if(!m->ibo) glGenBuffers(1, &m->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof_index, index_data, flags & MESH2_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex data
    if( vertex_data && vertex_count ) {
        m->vertex_count = vertex_count;

        if(!m->vbo) glGenBuffers(1, &m->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof_vertex, vertex_data, flags & MESH2_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex setup: iterate descriptors
    for( int i = 0; i < COUNTOF(descriptor); ++i ) {
        if( descriptor[i].num_components ) {
            glVertexAttribPointer(i,
                descriptor[i].num_components, descriptor[i].vertex_type, descriptor[i].vertex_type == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE,
                sizeof_vertex, (GLchar*)NULL + descriptor[i].offset);
            glEnableVertexAttribArray(i);
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    glBindVertexArray(0);

    return *m;
}

mesh_t mesh(const struct aiMesh *m, int flags) {
    unsigned vertex_count = m->mNumVertices;
    float *position = calloc(vertex_count * 3, sizeof(float));
    float *normal = calloc(vertex_count * 3, sizeof(float));
    float *texcoord = calloc(vertex_count * 2, sizeof(float));
    float *interleaved = calloc(vertex_count * (3+3+2), sizeof(float));

    for (int i = 0; i < vertex_count; i++) {
    	if( flags ) { // MODEL_SWAPZY
	        position[i*3+0] = m->mVertices[i].x;
	        position[i*3+1] = m->mVertices[i].z;
	        position[i*3+2] = m->mVertices[i].y;
    	} else {
	        position[i*3+0] = m->mVertices[i].x;
	        position[i*3+1] = m->mVertices[i].y;
	        position[i*3+2] = m->mVertices[i].z;
    	}

        if (m->mNormals) {
            normal[i*3+0] = m->mNormals[i].x;
            normal[i*3+1] = m->mNormals[i].y;
            normal[i*3+2] = m->mNormals[i].z;
        }

        if (m->mTextureCoords[0]) {
            texcoord[i*2+0] = m->mTextureCoords[0][i].x;
            texcoord[i*2+1] = 1 - m->mTextureCoords[0][i].y;
        }
    }
    
    float *ptr = interleaved;
    unsigned pnt_dirty_mask = 4|/*2|*/1; // if shader supports pos(4), normals(2), tex(1)
    for (int i = 0; i < vertex_count; i++) {
        if ( pnt_dirty_mask & 4 ) {
            *ptr++ = position[i*3+0];
            *ptr++ = position[i*3+1];
            *ptr++ = position[i*3+2];
        }
        if ( pnt_dirty_mask & 2 ) {
            *ptr++ = normal[i*3+0];
            *ptr++ = normal[i*3+1];
            *ptr++ = normal[i*3+2];
        }
        if ( pnt_dirty_mask & 1 ) {
            *ptr++ = texcoord[i*2+0];
            *ptr++ = texcoord[i*2+1];
        }
    }

    unsigned element_count = m->mNumFaces * 3;
    int *element = calloc(element_count, sizeof(int));

    for (int i = 0; i < m->mNumFaces; i++) {
        struct aiFace *face = m->mFaces + i;
        element[i*3+0] = face->mIndices[0];
        element[i*3+1] = face->mIndices[1];
        element[i*3+2] = face->mIndices[2];
    }

    mesh_t mt = //mesh_from_mem(vertices, normals, uvs, m->mNumVertices, scale);
    mesh_from_mem2( "p3 "/*n3*/" t2", 0,vertex_count,interleaved, element_count,element, 0 );
    REALLOC(position,0);
    REALLOC(normal,0);
    REALLOC(texcoord,0);
    REALLOC(interleaved,0);
    REALLOC(element,0);
    return mt;
}

model_t model(const char* filename, int flags) {
    int aiFlags = 0
   //| aiProcess_FlipUVs
   | aiProcessPreset_TargetRealtime_MaxQuality
   //| aiProcessPreset_TargetRealtime_Quality
   //| aiProcessPreset_TargetRealtime_Fast
   //| aiProcess_ConvertToLeftHanded // D3D
   | aiProcess_LimitBoneWeights //(4, defined as AI_LMW_MAX_WEIGHTS in config.h)
   | aiProcess_JoinIdenticalVertices 
   | aiProcess_RemoveRedundantMaterials
   | aiProcess_ImproveCacheLocality 
   | aiProcess_OptimizeGraph
   | aiProcess_OptimizeMeshes
   | aiProcess_SplitLargeMeshes;

    model_t mo = {0};
    const struct aiScene* scene = aiImportFile(filename, flags);
    if( scene ) {
        mo.mesh = REALLOC(0, sizeof(mesh_t) * (mo.mesh_count = scene->mNumMeshes) );
        for( int i = 0; i < mo.mesh_count; ++i ) {
            const struct aiMesh* m = scene->mMeshes[i];
            mo.mesh[i] = mesh(m, flags);
        }
        aiReleaseImport(scene);
    }
    return mo;
}

// ---

static camera_t *last_camera;

camera_t camera() {
    camera_t cam = {0};
    cam.speed = 1;
    cam.position = vec3(10,10,10);
    cam.up = vec3(0,1,0);
    // update proj & view
    camera_lookat(&cam,vec3(-5,0,-5));
    return cam;
}

camera_t *camera_get_active() {
    return last_camera;
}

void camera_move(camera_t *cam, float x, float y, float z) {
    last_camera = cam;

    vec3 dir = norm3(cross3(cam->look, cam->up));
    cam->position = add3(cam->position, scale3(dir, x)); // right
    cam->position = add3(cam->position, scale3(cam->up, y)); // up
    cam->position = add3(cam->position, scale3(cam->look, z)); // front

    camera_fps(cam, 0, 0);
}

void camera_lookat(camera_t *cam, vec3 target) {
    last_camera = cam;

    // invert expression that cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    // look.y = sin p > y = asin(p)
    // look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
    // look.z = sin y * cos p; -> cos p = look.z / sin y /
    // so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

    vec3 look = norm3(sub3(target, cam->position));
    const float rad2deg = 1 / 0.0174532f;
    float pitch = asin(look.y) * rad2deg;
    float yaw = atan2(look.x, look.z) * rad2deg;

    camera_fps(cam, yaw-cam->yaw, pitch-cam->pitch);
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    last_camera = cam;

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad;
    cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));

    lookat44(cam->view, cam->position, add3(cam->position, cam->look), cam->up); // eye,center,up
    perspective44(cam->proj, 45, window_width() / ((float)window_height()+!!window_height()), 0.01f, 1000.f);
}

// ---

object_t object(model_t model) {
    object_t obj = {0};
    identity44(obj.transform);
    obj.rot = idq();
    obj.sca = vec3(1,1,1);
    obj.model = model;
    return obj;
}

void object_rotate(object_t* obj, vec3 axis, float angle) {
    obj->rot = rotationq(angle, axis.x, axis.y, axis.z);
    rotationq44(obj->transform, obj->rot);
    scale44(obj->transform, obj->sca.x, obj->sca.y, obj->sca.z);
}

void object_teleport(object_t* obj, vec3 pos) {
    obj->transform[12] = pos.x;
    obj->transform[13] = pos.y;
    obj->transform[14] = pos.z;
}

void object_move(object_t* obj, vec3 inc) {
    obj->transform[12] += inc.x;
    obj->transform[13] += inc.y;
    obj->transform[14] += inc.z;
}

vec3 object_position(object_t* obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void object_scale(object_t* obj, vec3 sca) {
    obj->sca = sca;
    object_rotate(obj, vec3(1,1,1), 0); // refresh transform
}

void object_diffuse(object_t* obj, texture_t tex) {
    obj->texture_id = tex.id;
}

// ---

scene_t* scene_new() {
    scene_t clear = {0}, *p = REALLOC(0, sizeof(scene_t)); *p = clear;
    p->program = shader(vertex_shader_32_2, fragment_shader_2_4);
    return p;
}

object_t* scene_spawn(scene_t* scene, model_t model) {
    scene->objs[scene->obj_count] = object(model);
    scene->obj_count++;
    return &scene->objs[scene->obj_count-1];
}

#endif // RENDER_C
