#version 330 core

layout (points) in;
layout (points, max_vertices = 1) out;

in mat4 modelo[1];
in int visible[1];
in float id[1];

out mat4 out_modelo;

// ---

void main() {
    if (visible[0] == 1) {
        out_modelo = modelo[0];
        out_modelo[0][3] = id[0];
        EmitVertex();
        EndPrimitive();
    }
}
