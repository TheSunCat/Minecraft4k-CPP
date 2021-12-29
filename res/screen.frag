#version 330 core

in vec2 t;

uniform sampler2D T;

void main() {
    gl_FragColor = texture(T, t);
}