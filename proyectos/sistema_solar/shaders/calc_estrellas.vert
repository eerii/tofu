#version 330 core

uniform mat4 viewproj;
uniform int baseins;
uniform int culling;

uniform samplerBuffer bestrellas;

out mat4 modelo;
out int visible;
out float id;

// ---

// Frustum culling en espacio clip
int fustrum(mat4 m, float r) {
    // En este caso solo usamos la posición porque están tan lejos que las podemos tratar como un punto
    vec4 pos = (viewproj * m)[3];
    if (pos.x > pos.w)
        return 0;

    return 1;
}

// ---

void main() {
    // Numero de vértice
    int ins = baseins + gl_VertexID;
    id = float(ins);

    // Obtener los datos del buffer de entrada
    modelo = mat4(
        texelFetch(bestrellas, ins * 4 + 0),
        texelFetch(bestrellas, ins * 4 + 1),
        texelFetch(bestrellas, ins * 4 + 2),
        texelFetch(bestrellas, ins * 4 + 3)
    );

    // Frustum culling
    if (culling == 1)
        visible = fustrum(modelo, 1.0);
    else
        visible = 1;
}
