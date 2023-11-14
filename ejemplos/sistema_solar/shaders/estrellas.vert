#version 330 core

layout (location = 0) in vec3 in_pos;

uniform float time;
uniform mat4 viewproj;
uniform int baseins;

uniform samplerBuffer bestrellas;

out vec3 color;

// ---

float rand(float n){
    return fract(sin(n) * 43758.5453123);
}

// ---

void main() {
    // Numero de instancia
    int ins = baseins + gl_InstanceID; 

    // Modelos
    mat4 m = mat4(
        texelFetch(bestrellas, ins * 4 + 0),
        texelFetch(bestrellas, ins * 4 + 1),
        texelFetch(bestrellas, ins * 4 + 2),
        texelFetch(bestrellas, ins * 4 + 3)
    );
    int id = int(m[0][3]);
    m[0][3] = 0.0;

    if (abs(m[3][0]) < 100) {
        gl_Position = vec4(0.0);
        return;
    }

    gl_Position = viewproj * m * vec4(in_pos, 1.0);

    // Color
    float r = rand(cos(id * 0.001));
    float extra = pow(1.0 - r, 7.0);
    if (extra < 0.1) extra = 1.0;
    else extra = 5.0 * extra;
    float intensidad = 0.5 + 0.5 * sin(time * (pow(r, 3.0) + 1.0) * extra + r * 10.0);
    color = vec3(intensidad) * vec3(1.0 - pow(r, 3.0), 1.0 - pow(r, 2.0), 1.0 - r);
}
