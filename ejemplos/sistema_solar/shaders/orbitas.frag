#version 330 core

in vec3 pos;

out vec4 color_out;

// ---

float cerca = 1.0; 
float lejos = 1000.0; 

float profundidadLineal(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * cerca * lejos) / (lejos + cerca - z * (lejos - cerca));
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// ---

void main() {
    // Convertir posición a tonalidad
    float hue = atan(pos.z, pos.x) / (2.0 * 3.1415926) + 0.5;
    float sat = 1.0;
    float val = 1.0;
    vec3 color = hsv2rgb(vec3(hue, sat, val));

    float depth = profundidadLineal(gl_FragCoord.z) / lejos;
    color_out = vec4(min(color + 0.7, 1.0), vec3(depth) * 0.6);
}
