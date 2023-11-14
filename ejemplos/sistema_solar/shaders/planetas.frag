#version 330 core

in vec3 pos;
in vec4 color;
in vec3 normal;
in vec4 frag_pos;
in float iluminar;
in float mat;

uniform sampler2DArray albedo;

layout (location = 0) out vec4 color_out;
layout (location = 1) out vec4 normal_out;
layout (location = 2) out vec4 pos_out;

// ---

void main() {
    vec3 x = texture(albedo, vec3(pos.zy, color.w)).rgb;
    vec3 z = texture(albedo, vec3(pos.xy, color.w)).rgb;
    color_out = vec4((x + z) * 0.5 * color.rgb + 0.15, 1.0);

    normal_out = vec4(normal, 1.0);
    pos_out = vec4(frag_pos.xyz, iluminar);
}
