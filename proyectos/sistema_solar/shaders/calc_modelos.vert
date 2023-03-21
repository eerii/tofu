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
    vec4 derecha = (viewproj * translate(vec3(-r, 0, 0)) * m)[3];
    if (derecha.x > derecha.w)
        return 0;

    vec4 izquierda = (viewproj * translate(vec3(r, 0, 0)) * m)[3];
    if (izquierda.x < -izquierda.w)
        return 0;

    vec4 arriba = (viewproj * translate(vec3(0, -r, 0)) * m)[3];
    if (arriba.y < -arriba.w)
        return 0;

    vec4 abajo = (viewproj * translate(vec3(0, r, 0)) * m)[3];
    if (abajo.y > abajo.w)
        return 0;

    // No hacemos culling cerca-lejos ya que en este renderizado no queremos descartar nada lejano
    return 1;
}

// ---

mat4 calcularModelo(float r, float d, float i, bool padre) {
    float v = d > 1.0 ? (10.0 / d + rand(i * 10) * 0.05) * 0.5 : 0.0;

    mat4 m = rotate(time * v + rand(i) * 10.0, vec3(0.0, 1.0, 0.0));
    m *= translate(vec3(d, 0.0, 0.0));
    if (!padre)
        m *= scale(vec3(r));

    return m;
}

void main() {
    // Numero de vértice
    int ins = baseins + gl_VertexID;
    id = float(ins);

    // Obtener los datos del buffer de entrada
    vec3 buf = texelFetch(bplanetas, ins).xyz;

    // Transformación del planeta
    modelo = calcularModelo(buf.x, buf.y, float(ins), false);

    // Obtener transformación del padre
    float padre = buf.z;
    if (padre != -1.0) {
        mat4 p[MAX_REC];
        int i = 0;
        while (padre != -1.0) {
            buf = texelFetch(bplanetas, int(padre)).xyz;
            p[i++] = calcularModelo(buf.x, buf.y, padre, true);
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
