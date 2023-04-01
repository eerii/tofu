#version 330 core

layout (location = 0) in vec3 in_pos;

uniform int baseins;
uniform mat4 viewproj;

uniform samplerBuffer bmodelos;
uniform samplerBuffer bcolor;

out vec3 color;
out vec3 normal;
out vec4 frag_pos;
out float iluminar;

// ---

#define M_PI 3.14159265358979323846

// Hashes pseudo-aleatorios
float rand (vec2 co){return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);}
float rand (vec2 co, float l) {return rand(vec2(rand(co), l));}
float rand (vec2 co, float l, float t) {return rand(vec2(rand(co, l), t));}

// Ruído Perlin
float perlin(vec2 p, float dim, float time) {
	vec2 pos = floor(p * dim);
	vec2 posx = pos + vec2(1.0, 0.0);
	vec2 posy = pos + vec2(0.0, 1.0);
	vec2 posxy = pos + vec2(1.0);
	
	float c = rand(pos, dim, time);
	float cx = rand(posx, dim, time);
	float cy = rand(posy, dim, time);
	float cxy = rand(posxy, dim, time);
	
	vec2 d = fract(p * dim);
	d = -0.5 * cos(d * M_PI) + 0.5;
	
	float ccx = mix(c, cx, d.x);
	float cycxy = mix(cy, cxy, d.x);
	float center = mix(ccx, cycxy, d.y);
	
	return center * 2.0 - 1.0;
}

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

    // Outputs
    color = texelFetch(bcolor, id).rgb;
    normal = mat3(transpose(inverse(m))) * in_pos;
    iluminar = float(m[3][0] != 0.0 || m[3][2] != 0.0);

    // Posición
    float disp = 1.0;
    if (iluminar > 0.5)
        disp = perlin(in_pos.xy, 4.0 / ceil(m[1][1] * 4.0), 0.0) * 0.1 + 0.9;
    frag_pos = m * vec4(in_pos * disp, 1.0);
    gl_Position = viewproj * frag_pos;

}
