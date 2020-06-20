#version 330 core

precision highp float;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D renderedTexture;

void main() {
    FragColor = texture2D(renderedTexture, TexCoord);
}