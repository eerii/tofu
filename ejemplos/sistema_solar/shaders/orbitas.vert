#version 330 core

layout (location = 0) in vec3 in_pos;

uniform mat4 viewproj;
uniform int baseins;

uniform samplerBuffer borbitas;

out vec3 pos;

// ---

void main() {
    // Numero de instancia
    int ins = baseins + gl_InstanceID; 

    // Modelos
    mat4 m = mat4(
        texelFetch(borbitas, ins * 4 + 0),
        texelFetch(borbitas, ins * 4 + 1),
        texelFetch(borbitas, ins * 4 + 2),
        texelFetch(borbitas, ins * 4 + 3)
    );

    gl_Position = viewproj * m * vec4(in_pos, 1.0);
    pos = in_pos;
}
