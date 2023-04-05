#version 330 core

in vec3 color;
in vec3 normal;
in vec4 frag_pos;
in float iluminar;

layout (location = 0) out vec4 color_out;
layout (location = 1) out vec4 normal_out;
layout (location = 2) out vec4 pos_out;

// ---

void main() { 
    color_out = vec4(color, 1.0);
    normal_out = vec4(normal, 1.0);
    pos_out = vec4(frag_pos.xyz, iluminar);
}
