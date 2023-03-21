#version 330 core

layout (location = 0) in vec3 in_pos;

uniform int baseins;
uniform mat4 viewproj;

uniform samplerBuffer bmodelos;
uniform samplerBuffer bcolor;

out vec3 color;

// ---

// Hash
float hash(float n) { return fract(sin(n) * 43758.5453123); }

// ---

void main() {
    // Numero de instancia
    int ins = baseins + gl_InstanceID;

    // Datos de la instancia de los buffers
    mat4 m = mat4(
        texelFetch(bmodelos, ins * 4 + 0),
        texelFetch(bmodelos, ins * 4 + 1),
        texelFetch(bmodelos, ins * 4 + 2),
        texelFetch(bmodelos, ins * 4 + 3)
    );
    int id = int(m[0][3]);
    m[0][3] = 0.0;

    vec3 c = texelFetch(bcolor, id).rgb;

    // Posición
    gl_Position = viewproj * m * vec4(in_pos, 1.0);

    // Color
    color = c * 0.9 + in_pos * 0.1;
}
