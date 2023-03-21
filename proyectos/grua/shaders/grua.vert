#version 330 core

layout (location = 0) in vec3 in_pos;

uniform mat4 viewproj;

uniform int baseins;
uniform samplerBuffer modelos;
uniform samplerBuffer colores;

out vec3 color;

// ---

void main() {
    // Numero de instancia
    int ins = baseins + gl_InstanceID;

    // Modelo
    mat4 m = mat4(
        texelFetch(modelos, ins * 4 + 0),
        texelFetch(modelos, ins * 4 + 1),
        texelFetch(modelos, ins * 4 + 2),
        texelFetch(modelos, ins * 4 + 3)
    );

    // Color
    vec3 c = texelFetch(colores, ins).rgb;
    color = c * 0.9 + in_pos * 0.1;

    // Posicion
    gl_Position = viewproj * m * vec4(in_pos, 1.0);
}
