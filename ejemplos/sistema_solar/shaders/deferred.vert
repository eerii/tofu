#version 330 core

uniform vec2 tam_win;

out vec2 uv;
out vec2 coords[9];

const float grosor_linea = 1.0;

// ---

void main() {
    uv = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3))[gl_VertexID];
    gl_Position = vec4(uv, 0.0, 1.0);
    uv = 0.5 * uv + vec2(0.5);

    float w = grosor_linea / tam_win.x;
    float h = grosor_linea / tam_win.y;
    coords = vec2[9](
        vec2( -w, -h),
        vec2(0.0, -h),
        vec2(  w, -h),
        vec2( -w, 0.0),
        vec2(0.0, 0.0),
        vec2(  w, 0.0),
        vec2( -w, h),
        vec2(0.0, h),
        vec2(  w, h)
    );
}
