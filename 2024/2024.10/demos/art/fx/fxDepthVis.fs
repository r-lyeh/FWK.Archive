uniform float near; /// set:0.1
uniform float far; /// set:1000.0
uniform float min_depth; /// set:0.0
uniform float max_depth; /// set:200.0

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    float depth = texture(iChannel0, uv).r;
    float linear_depth = (2.0 * near * far) / (far + depth * (near - far));
    float normalized_depth = clamp((linear_depth - min_depth) / (max_depth - min_depth), 0.0, 1.0);
    vec3 color = hsv2rgb(vec3(0.66 * (1 - normalized_depth), 1.0, 1.0));
    fragColor.rgb = color;
    fragColor.a = 1.0;
}
