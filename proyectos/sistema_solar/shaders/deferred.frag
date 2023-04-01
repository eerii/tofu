#version 330 core

in vec2 uv;

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D depth;

out vec4 color_out;

const float w = 1.0 / 500.0;
const float h = 1.0 / 500.0;

const float near = 1.0;
const float far = 1000.0;

const vec2 coords[9] = vec2[](
    vec2( -w, -h),
    vec2(0.0, -h),
    vec2(  w, -h),
    vec2( -w, 0.0),
    vec2(0.0, 0.0),
    vec2(  w, 0.0),
    vec2( -w, h),
    vec2(0.0, h),
    vec2(  w, h)
);

// ---

float linearDepth(vec2 coord) {
    float d = length(texture(depth, coord).rgb);
    return (2.0 * near) / (far + near - d * (far - near));
}

vec2 coordenada(int i) {
    vec2 c = uv + coords[i];
    return clamp(c, vec2(0.001), vec2(0.999));
}

void sobelKernel(inout float n[9]) {
    for (int i = 0; i < 9; i++)
        n[i] = linearDepth(coordenada(i));
}

// ---

void main() {
    vec4 c = texture(color, uv);
    /*if (c.a == 0.0)
        discard;*/

    // Detección de ejes
    float n[9];
    sobelKernel(n);
    float sobel_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	float sobel_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
    float borde = sqrt(sobel_h*sobel_h + sobel_v*sobel_v);

    if (c.a == 0.0 && borde == 0.0)
        discard;

    // Color final
    vec3 cmix;
    if (c.a != 0)
        cmix = min(c.rgb, 1.0 - vec3(borde));
    else
        cmix = vec3(borde) * vec3(1.0, 0.9, 0.8);
    color_out = vec4(cmix, 1.0);
}
