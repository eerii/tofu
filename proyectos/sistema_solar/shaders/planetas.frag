#version 330 core

in vec3 color;
in vec3 normal;
in vec4 frag_pos;
in float iluminar;

uniform vec3 viewpos;

out vec4 color_out;
out vec4 normal_out;

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

float iluminacion() {
    if (iluminar < 0.5)
        return 1.0;

    float ambiente = 0.1;

    vec3 norm = normalize(normal);
    vec3 dir_luz = normalize(pos_luz - frag_pos.xyz);
    float difusa = max(dot(norm, dir_luz) * 0.8 + 0.5, 0.0);

    vec3 dir_view = normalize(viewpos - frag_pos.xyz);
    vec3 dir_refl = reflect(-dir_luz, norm);
    float especular = pow(max(dot(dir_view, dir_refl), 0.0), brillo_especular) * fuerza_especular;
    
    return ambiente + difusa + especular;
}

// ---

void main() {
    vec3 hsv = rgb_to_hsv(color);
    float i = iluminacion();
    i = (floor(i * num_colores) + 0.3) / num_colores;

    vec3 c = hsv_to_rgb(vec3(hsv.r, saturacion, i));

    color_out = vec4(c, 1.0);
    normal_out = vec4(normal, 1.0);
}
