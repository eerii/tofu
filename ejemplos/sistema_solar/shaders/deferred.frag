#version 330 core

in vec2 uv;
in vec2 coords[9];

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D pos;
uniform sampler2D depth;

uniform vec3 viewpos;
uniform float activar_bordes;
uniform float activar_toon;

out vec4 color_out;

const float near = 1.0;
const float far = 1000.0;

const vec3 pos_luz = vec3(0.0, 0.0, 0.0);
const float fuerza_especular = 0.5;
const float brillo_especular = 8.0;
const float num_colores = 3.0;
const float saturacion = 0.8;

// ---

vec3 hsv_to_rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgb_to_hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// ---

float iluminacion(vec3 normal, vec3 pos, float iluminar) {
    if (iluminar < 0.5)
        return 1.0;

    float ambiente = 0.1;

    normal = normalize(normal);
    vec3 dir_luz = normalize(pos_luz - pos);
    float difusa = max(dot(normal, dir_luz) * 0.8 + 0.5, 0.0);

    vec3 dir_view = normalize(viewpos - pos);
    vec3 dir_refl = reflect(-dir_luz, normal);
    float especular = pow(max(dot(dir_view, dir_refl), 0.0), brillo_especular) * fuerza_especular;
    
    return ambiente + difusa + especular;
}

// ---

float linearDepth(vec2 coord) {
    float d = length(texture(depth, coord).rgb);
    return (2.0 * near) / (far + near - d * (far - near));
}

vec2 coordenada(int i) { 
    vec2 c = uv + coords[i];
    return clamp(c, vec2(0.001), vec2(0.999));
}

void sobelKernel(inout float n[9]) {
    for (int i = 0; i < 9; i++)
        n[i] = linearDepth(coordenada(i));
}

// ---

void main() {
    vec4 c = texture(color, uv);
    vec4 n = texture(normal, uv);
    vec4 p = texture(pos, uv);

    // Iluminación
    vec3 hsv = rgb_to_hsv(c.rgb);
    float i = iluminacion(n.xyz, p.xyz, p.a);
    if (activar_toon > 0.5)
        i = (floor(i * num_colores) + 0.3) / num_colores;
    c.rgb = hsv_to_rgb(vec3(hsv.r, saturacion, i * (hsv.b * 0.5 + 0.5)));

    // Detección de ejes
    if (activar_bordes > 0.5) {
        float n[9];
        sobelKernel(n);
        float sobel_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
        float sobel_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
        float borde = sqrt(sobel_h*sobel_h + sobel_v*sobel_v);

        if (c.a == 0.0 && borde == 0.0)
            discard;

        // Color final
        vec3 cmix;
        if (c.a != 0)
            cmix = min(c.rgb, 1.0 - vec3(borde));
        else
            cmix = vec3(borde) * vec3(1.0, 0.9, 0.5);

        color_out = vec4(cmix, 1.0);
    } else {
        color_out = c;
    }
}
