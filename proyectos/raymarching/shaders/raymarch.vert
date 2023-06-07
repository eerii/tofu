#version 330 core

out vec2 uv;

// ---

void main() {
    vec2 pos = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3))[gl_VertexID];
    gl_Position = vec4(pos, 0.0, 1.0);
    uv = vec2(pos.x + 0.5, pos.y * -1.0 - 0.5) * 2;
}
