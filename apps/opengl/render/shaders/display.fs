#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D renderedTexture;

void main() {
    FragColor = texture(renderedTexture, TexCoord);
}