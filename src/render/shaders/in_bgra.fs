#version 300 es

precision highp float;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D PlaneTex0; // BGRA

void main() {
    vec4 tex = texture(PlaneTex0, TexCoord).rgba;
    
    FragColor.r = tex.b;
    FragColor.g = tex.g;
    FragColor.b = tex.r;
    FragColor.a = tex.a;
} 
