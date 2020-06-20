#version 330 core 

layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 InTextCoord; 

out vec2 TexCoord;

uniform vec2 ScaleFact;

void main() {
    gl_Position = vec4(ScaleFact * position.xy, 0.0, 1.0);
    TexCoord = vec2(InTextCoord.s, 1.0 - InTextCoord.t);
}