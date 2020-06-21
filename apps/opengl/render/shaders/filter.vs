#version 300 es

layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 InTextCoord; 

out vec2 TexCoord;

void main() {
    gl_Position = vec4(position, 1.0);
    TexCoord = vec2(InTextCoord.s, 1.0 - InTextCoord.t);
}