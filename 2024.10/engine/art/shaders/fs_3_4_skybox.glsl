uniform sampler2D u_skybox;

in vec3 v_direction;
out vec4 fragcolor;

#define PI 3.14159265

vec2 sphere_to_polar( vec3 normal ) {
    normal = normalize( normal );
    float theta = atan( -normal.x, normal.z );
    float phi = acos( normal.y );
    float u = (theta + PI) / (2*PI);
    float v = phi / PI;
    return vec2( u, v );
}

void main() {
    vec3 direction = v_direction;
    fragcolor = vec4(texture(u_skybox, sphere_to_polar(direction)).rgb, 1.0);
    fragcolor.rgb = pow(fragcolor.rgb, vec3(1.0/2.2));
}