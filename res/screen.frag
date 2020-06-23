#version 330 core

in vec2        texCoord;

uniform sampler2D   tex;

void main() {
    gl_FragColor = /*vec4(1 - texCoord.xyx, 1);*/texture(tex, texCoord);
}