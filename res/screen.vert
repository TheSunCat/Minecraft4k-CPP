#version 330 core

layout (location = 0) in vec2 P;
layout (location = 2) in vec2 T;

out vec2 t;

void main() {
    vec4 pos = vec4(P, 1.0F, 1.0F);
    gl_Position = pos;

    t = T;
}