// naive debugdraw framework
// - rlyeh, public domain.
//
// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
// [x] arrow, point,
// [x] text
// [ ] plane, string, capsule, aabb, bone (pyramid?), sphere (proper)
// [ ] line batching
// [ ] line width and stipple

#ifndef RENDERDD_H
#define RENDERDD_H

void ddraw_color(vec3 rgb);
//
void ddraw_grid();
void ddraw_point(vec3 from);
void ddraw_line(vec3 from, vec3 to);
void ddraw_axis(float units);
void ddraw_text(vec3 pos, float scale, const char *text);
void ddraw_cube(vec3 center);
void ddraw_normal(vec3 pos, vec3 n);
void ddraw_circle(vec3 pos);
void ddraw_hexagon(vec3 pos);
void ddraw_pentagon(vec3 pos);
void ddraw_square(vec3 pos);
void ddraw_triangle(vec3 pos);
void ddraw_sphere(vec3 center);
void ddraw_pyramid(vec3 center, float height, int segments);
void ddraw_cone(vec3 center, vec3 top, float radius);
void ddraw_bone(vec3 center, vec3 end);
void ddraw_bounds(const vec3 points[8]);
void ddraw_aabb(vec3 minbb, vec3 maxbb);
void ddraw_frustum(float projview[16]);
void ddraw_arrow(vec3 begin, vec3 end);
void ddraw_plane(vec3 p, vec3 n, float scale);
void ddraw_demo();

#endif


#ifdef RENDERDD_C
#pragma once

const char *vs =
"#version 130\n"
"in vec3 att_position;\n"
"uniform mat4 u_MVP;\n"
"uniform vec3 u_color;\n"
"out vec3 out_color;\n"
"void main() {\n"
"    gl_Position = u_MVP * vec4(att_position, 1.0);\n"
"    gl_PointSize = 4.0;\n" // for GL_POINTS draw commands
"    out_color = u_color;\n"
"}";

const char *fs =
"#version 100\n"
"precision mediump float;\n"
"in vec3 out_color;\n"
"void main() {\n"
"   gl_FragColor = vec4(out_color, 1.0);\n"
"}";

vec3 u_color = {1,1,1};

// based on pico8 secret palette
const vec3 white = {1.000f,0.945f,0.910f};
const vec3 red = {1.000f,0.000f,0.302f};
const vec3 yellow = {1.000f,0.925f,0.153f};
const vec3 green = {0.000f,0.894f,0.212f};
const vec3 gray = {0.286f,0.200f,0.231f};
const vec3 blue = {0.024f,0.353f,0.710f};

void shader_print(const char *source) {
    for(int line = 0, i = 0; source[i]; ++i ) {
        printf("%03d: ", ++line);
        while( source[i] >= 32 || source[i] == '\t' ) fputc(source[i++], stdout);
        puts("");
    }
}

static
GLuint shader_generate( GLenum type, const char *source ) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE ) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        assert(length < 2048);
        char buf[2048] = { 0 };
        glGetShaderInfoLog(shader, length, NULL, buf);

        // dump log with line numbers
        shader_print( source );
        fprintf(stderr, "ERROR: %s shader:\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);

        shader = 0;
        exit(-1);
    }

    return shader;
}

unsigned shader2(const char *vs, const char *fs, const char *attribs) {
    GLuint vert = shader_generate(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_generate(GL_FRAGMENT_SHADER, fs);
  //GLuint geom = shader_generate(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        // glAttachShader(program, geom);

        for( int i = 0; attribs && attribs[0]; ++i ) {
            char attrib[128] = {0};
            sscanf(attribs, "%127[^,]", attrib);
            while( attribs[0] && attribs[0] != ',' ) attribs++;
            while( attribs[0] && attribs[0] == ',' ) attribs++;
            glBindAttribLocation(program, i, attrib);
            // printf("%d --> %s\n", i, attrib);
        } // puts("---");

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            assert(length < 2048);
            char buf[2048] = { 0 };
            glGetProgramInfoLog(program, length, NULL, buf);
            shader_print( vs );
            shader_print( fs );
            fprintf(stderr, "Error: Shader/program link: %s\n", buf);
            exit(-1);
            return 0;
        }

        // glDetachShader(program, vert);
        // glDetachShader(program, frag);
        // glDetachShader(program, geom);

        glDeleteShader(vert);
        glDeleteShader(frag);
        // glDeleteShader(geom);
    }

    return program;
}

static GLuint program;

void ddraw_color(vec3 rgb) {
    u_color = rgb;
    glUniform3fv(glGetUniformLocation(program, "u_color"), GL_TRUE, &u_color.x);
}
void ddraw_line_(vec3 from, vec3 to, int is_point) {
    // prepare vertex data
    GLfloat points[12] = { from.x,from.y,from.z, 1.0,1.0,1.0, to.x,to.y,to.z, 1.0,1.0,1.0 };

    static GLuint vao, vbo;
    if(!vao) glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    if(!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0); // #0 x3 float not-norm stride:24bytes offset:0
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS rendering

    // feed vertex data
    glBindVertexArray(vao);
    if( is_point )
    glDrawArrays(GL_POINTS, 0, 1/*2*/);
    else
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}
void ddraw_point(vec3 from) {
    ddraw_line_(from, from, 1);
}
void ddraw_line(vec3 from, vec3 to) {
    ddraw_line_(from, to, 0);
}
void ddraw_axis(float units) {
    ddraw_color(red);   ddraw_line(vec3(0,0,0), vec3(units,0,0));
    ddraw_color(green); ddraw_line(vec3(0,0,0), vec3(0,units,0));
    ddraw_color(blue);  ddraw_line(vec3(0,0,0), vec3(0,0,units));
}
void ddraw_grid_(float scale) { // 10x10
    for( float i = -scale, c = 0; c <= 20; ++c, i += (scale/10) ) {
        ddraw_color( c == 0 || c == 20 ? white : gray );
        ddraw_line(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
}
void ddraw_grid() {
    glDisable(GL_DEPTH_TEST);
        ddraw_grid_(100);
        ddraw_grid_(10);
        ddraw_grid_(1);
        ddraw_grid_(0.1);
        ddraw_grid_(0.01);
    glEnable(GL_DEPTH_TEST);
    ddraw_axis(100);
}


void ddraw_text(vec3 pos, float scale, const char *text) {
    // [ref] http://paulbourke.net/dataformats/hershey/ (PD)
    // [ref] https://sol.gfxile.net/hershey/fontprev.html (WTFPL2)
    static const signed char *hershey[] = { /* simplex font */
    "IKFVFH@@FCEBFAGBFC","AQ","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
    "M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
    "UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
    "OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
    "ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
    "C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
    "LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
    "VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
    "AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
    "EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
    "KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
    "EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
    "ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
    "MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
    "RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
    "SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
    "SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
    "EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
    "EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
    "RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
    "EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
    "IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
    "FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
    "COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
    "EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
    "LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
    "PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
    ":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
    "A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
    "NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
    "LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
    "ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
    "IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
    "FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
    "MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
    "YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
    "XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };
    vec3 src = pos, old = {0};
    for( signed char c, *glyph; (c = *text++, glyph = (char*)hershey[c - 32], c > 0 && c < 127); ) {
        if( c != '\n' && c != '\r' ) {
            if( c >= 32 ) for( int pen = 0, i = 0; i < (glyph[0] - 65); i++ ) { // verts @0
                int x = glyph[2 + i*2 + 0] - 65, y = glyph[2 + i*2 + 1] - 65;
                if( x == -1 && y == -1 ) pen = 0; else {
                    vec3 next = add3(pos, vec3(scale*x, scale*y, 0));
                    if( !pen ) pen = 1; else ddraw_line(old, next);
                    old = next;
                }
            }
            pos.x += scale * (glyph[1] - 65); // spacing @1
        } else {
            pos.x = src.x, pos.y -= scale * (hershey['W'-32][1] - 65) * 1.25f; // spacing @1
        }
    }
}

/*
void ddraw_prism(vec3 pos, int segments) {
    ddraw_color(vec3(1,1,1));
    float cycle = 2 * 3.14159 + 2 * 3.14159 / segments, acc = 0;
    for( int i = 0; i < segments; ++i, acc += cycle ) {
        ddraw_line(add3(pos,vec3(cos(acc),0,sin(acc))), add3(pos,vec3(cos(acc+cycle),0,sin(acc+cycle))));
    }
}
*/
void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments) {
    vec3 left = {0}, up = {0};
    ortho3(&left, &up, normal);

    vec3 point, lastPoint;
    up = scale3(up, radius);
    left = scale3(left, radius);
    lastPoint = add3(center, up);
    vec3 pivot = add3(center, scale3(normal, height));

    for (int i = 1; i <= segments; ++i) {
        const float radians = (C_PI * 2) * i / segments;

        vec3 vs = scale3(left, sinf(radians));
        vec3 vc = scale3(up,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        if( height > 0 ) ddraw_line(point, pivot);
        else if(height < 0) {
            ddraw_line(point, add3(point,scale3(normal, -height)));
        }
        lastPoint = point;
    }

    if(height < 0) ddraw_prism(add3(center, scale3(normal, -height)), radius, 0, normal, segments);
}
void ddraw_cube(vec3 center) { // draw_prism(center, 1, -1, vec3(0,1,0), 4);
    vec3 l = vec3(center.x-0.5f,center.y+0.5f,center.z-0.5f); // left-top-far
    vec3 r = vec3(center.x+0.5f,center.y-0.5f,center.z+0.5f); // right-bottom-near

    ddraw_line(l, vec3(r.x,l.y,l.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,l.y,r.z));
    ddraw_line(vec3(r.x,l.y,r.z), vec3(l.x,l.y,r.z));
    ddraw_line(vec3(l.x,l.y,r.z), l);
    ddraw_line(l, vec3(l.x,r.y,l.z));

    ddraw_line(r, vec3(l.x,r.y,r.z));
    ddraw_line(vec3(l.x,r.y,r.z), vec3(l.x,r.y,l.z));
    ddraw_line(vec3(l.x,r.y,l.z), vec3(r.x,r.y,l.z));
    ddraw_line(vec3(r.x,r.y,l.z), r);
    ddraw_line(r, vec3(r.x,l.y,r.z));

    ddraw_line(vec3(l.x,l.y,r.z), vec3(l.x,r.y,r.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,r.y,l.z));
}
void ddraw_normal(vec3 pos, vec3 n) {
    ddraw_color(yellow);
    ddraw_line(pos, add3(pos, norm3(n)));
}

void ddraw_circle(vec3 pos) { ddraw_prism(pos, 1, 0, vec3(0,1,0), 24); }
void ddraw_hexagon(vec3 pos) { ddraw_prism(pos, 1, 0, vec3(0,1,0), 6); }
void ddraw_pentagon(vec3 pos) { ddraw_prism(pos, 1, 0, vec3(0,1,0), 5); }
void ddraw_square(vec3 pos) { ddraw_prism(pos, 1, 0, vec3(0,1,0), 4); }
void ddraw_triangle(vec3 pos) { ddraw_prism(pos, 1, 0, vec3(0,1,0), 3); }
void ddraw_sphere(vec3 center) { // @fixme: improve billboard for outer ring
    ddraw_color(red);   ddraw_prism(center, 1.0, 0, vec3(1,0,0), 24);
    ddraw_color(green); ddraw_prism(center, 1.0, 0, vec3(0,1,0), 24);
    ddraw_color(blue);  ddraw_prism(center, 1.0, 0, vec3(0,0,1), 24);

    ddraw_color(white); ddraw_prism(center, 1.0, 0, norm3(camera_get_active()->look), 24);
}
void ddraw_pyramid(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, height, vec3(0,1,0), segments);
}
void ddraw_cone(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 24);
}
void ddraw_bone(vec3 center, vec3 end) {
    vec3 diff3 = sub3(end, center);
    float len = len3(diff3), len10 = len / 10;
    ddraw_prism(center, len10, 0, vec3(1,0,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,1,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,0,1), 24);
    ddraw_line(end, add3(center, vec3(0,+len10,0)));
    ddraw_line(end, add3(center, vec3(0,-len10,0)));
}
void ddraw_bounds(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        ddraw_line(points[i], points[(i + 1) & 3]);
        ddraw_line(points[i], points[4 + i]);
        ddraw_line(points[4 + i], points[4 + ((i + 1) & 3)]);
    }
}
void ddraw_aabb(vec3 minbb, vec3 maxbb) {
    vec3 points[8], bb[2] = { minbb, maxbb };
    for (int i = 0; i < 8; ++i) {
        points[i].x = bb[(i ^ (i >> 1)) & 1].x;
        points[i].y = bb[     (i >> 1)  & 1].y;
        points[i].z = bb[     (i >> 2)  & 1].z;
    }
    ddraw_bounds(points);
}
void ddraw_frustum(float projview[16]) {
    mat44 clipmatrix = {0}; // clip matrix
    invert44(clipmatrix, projview);

    // Start with the standard clip volume, then bring it back to world space.
    const vec3 planes[8] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1}, // near plane
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}, // far plane
    };

    vec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for( int i = 0; i < 8; ++i ) {
        // wCoords[i] = matTransformPointXYZW2(&points[i], planes[i], clipmatrix);
        vec3 *out = &points[i], in = planes[i]; const float *m = clipmatrix;
        out->x = (m[0] * in.x) + (m[4] * in.y) + (m[ 8] * in.z) + m[12]; // in.w (vec4) assumed to be 1
        out->y = (m[1] * in.x) + (m[5] * in.y) + (m[ 9] * in.z) + m[13];
        out->z = (m[2] * in.x) + (m[6] * in.y) + (m[10] * in.z) + m[14];
        wCoords[i] =  (m[3] * in.x) + (m[7] * in.y) + (m[11] * in.z) + m[15]; // rw

        // bail if any W ended up as zero.
        const float epsilon = 1e-9f;
        if (absf(wCoords[i]) < epsilon) {
            return;
        }
    }

    // Divide by the W component of each:
    for( int i = 0; i < 8; ++i ) {
        points[i].x /= wCoords[i];
        points[i].y /= wCoords[i];
        points[i].z /= wCoords[i];
    }

    // Connect the dots:
    ddraw_bounds(points);
}
void ddraw_arrow(vec3 begin, vec3 end) {
    vec3 diff = sub3(end, begin);
    float len = len3(diff);
    float stick_len = len * 2 / 3, head_radius = len / 6;

    ddraw_line(begin, end);
    ddraw_cone(add3(begin, scale3(norm3(diff), stick_len)), end, head_radius);
}

void ddraw_plane(vec3 p, vec3 n, float scale)  // @todo
{}

void ddraw_frame(void *userptr) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(program);

    //
    mat44 model; identity44(model);
    mat44 proj;  copy44(proj, camera_get_active()->proj);
    mat44 view;  copy44(view, camera_get_active()->view);
    mat44 mvp;   multiply344(mvp, proj, view, model);

    glUniformMatrix4fv(glGetUniformLocation(program, "u_MVP"), 1, GL_FALSE, mvp);
}

void ddraw_init() {
    program = shader2(vs,fs,"att_position");
    window_hook(ddraw_frame, NULL);
}


void ddraw_demo() {
#if 0
    // frustum camera forensics for 1st frame
    static mat44 projview_copy;
    static int first = 1; for(;first;first=0) {
        multiply44(projview_copy, proj, view);
    }
    ddraw_frustum(projview_copy);
#endif

    ddraw_grid();

    vec3 origin = {0,0,0};
    ddraw_line(origin, vec3(1,1,1));
    ddraw_normal(origin, vec3(1,1,1));

    ddraw_text(vec3(1,1,1), 0.008f, "hello world");

    const char abc[] = " !\"#$%&'()*+,-./\n"
        "0123456789:;<=>?@\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\n"
        "abcdefghijklmnopqrstuvwxyz{|}~";
    ddraw_text(vec3(2,2,2), 0.008f, abc);

    for( int i = -5; i <= 5; ++i ) {
        ddraw_pyramid(vec3(i*2,0,3),  0, i+5+2);
        ddraw_pyramid(vec3(i*2,0,6), -2, i+5+2);
        ddraw_pyramid(vec3(i*2,0,9), +2, i+5+2);
    }

#if 0
    ddraw_triangle(origin);
    ddraw_square(origin);
    ddraw_pentagon(origin);
    ddraw_hexagon(origin);
    ddraw_cube(origin);
    ddraw_pyramid(origin, 2, 3);
    ddraw_pyramid(origin, 2, 16);
    ddraw_cone(origin, add3(origin, vec3(0,1,0)), 0.5f);
    ddraw_arrow(origin, vec3(0,1,0));
    ddraw_bone(vec3(0,0,0), vec3(3,3,3));
    ddraw_aabb(vec3(0,0,0), vec3(1,1,1));
#endif

    ddraw_sphere(vec3(-3,0,-3));
    ddraw_point(vec3(-2,0,-2));
}

#endif
