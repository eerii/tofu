#version 330 core

uniform float time;
uniform mat4 viewproj;
uniform int baseins;
uniform int culling;

uniform samplerBuffer bplanetas;

out mat4 modelo;
out int visible;
out float id;

#define MAX_REC 4

// ---

float rand(float n){
    return fract(sin(n) * 43758.5453123);
}

mat4 translate(vec3 v) {
    return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                v.x, v.y, v.z, 1.0);
}

mat4 scale(vec3 v) {
    return mat4(v.x, 0.0, 0.0, 0.0,
                0.0, v.y, 0.0, 0.0,
                0.0, 0.0, v.z, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

mat4 rotate(float angle, vec3 v) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;

    return mat4(t * v.x * v.x + c,       t * v.x * v.y - s * v.z, t * v.x * v.z + s * v.y, 0.0,
                t * v.x * v.y + s * v.z, t * v.y * v.y + c,       t * v.y * v.z - s * v.x, 0.0,
                t * v.x * v.z - s * v.y, t * v.y * v.z + s * v.x, t * v.z * v.z + c,       0.0,
                0.0,                     0.0,                     0.0,                     1.0);
}

// ---

// Frustum culling en espacio clip
int fustrum(mat4 m, float r) {
    vec4 pos = viewproj * m[3];
    pos.w += (viewproj * vec4(r)).w;
    if (pos.x < -pos.w || pos.x > pos.w)
        return 0;
    if (pos.y < -pos.w || pos.y > pos.w)
        return 0;

    return 1;
}

// ---

mat4 calcularModelo(float r, float d, float i, float exc, bool padre) {
    float v = d > 1.0 ? (10.0 / d + rand(i * 55) * 0.05) * 0.5 : 0.0;
    float pos = time * v + rand(i * 67) * 10.0;

    mat4 m = translate(vec3(d * ((1.0 + exc) * cos(pos) - exc), 0.0, d * sin(pos)));
    if (!padre)
        m *= scale(vec3(r));

    return m;
}

void main() {
    // Numero de vértice
    int ins = baseins + gl_VertexID;
    id = float(ins);

    // Obtener los datos del buffer de entrada
    vec4 buf = texelFetch(bplanetas, ins);
    float padre = buf.z;

    // Transformación del planeta
    modelo = calcularModelo(buf.x, buf.y, float(ins), buf.w, false);

    // Obtener transformación del padre
    if (padre != -1.0) {
        mat4 p[MAX_REC];
        int i = 0;
        while (padre != -1.0) {
            buf = texelFetch(bplanetas, int(padre));
            p[i++] = calcularModelo(buf.x, buf.y, padre, buf.w, true);
            padre = buf.z;
        }
        mat4 mp = mat4(1.0);
        for (int j = i - 1; j >= 0; j--)
            mp *= p[j];
        modelo = mp * modelo;
    }

    // Frustum culling
    if (culling == 1)
        visible = fustrum(modelo, buf.x);
    else
        visible = 1;
}
