#version 330 core

out vec2 uv;

// ---

void main() {
    uv = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3))[gl_VertexID];
    gl_Position = vec4(uv, 0.0, 1.0);
    uv = 0.5 * uv + vec2(0.5);
}
