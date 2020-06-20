#version 330 core

precision highp float;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D PlaneTex0;
uniform sampler2D PlaneTex1;
uniform sampler2D PlaneTex2;

const vec3 Rcoeff = vec3( 1.1640,  0.000,  1.596);
const vec3 Gcoeff = vec3( 1.1640, -0.391, -0.813);
const vec3 Bcoeff = vec3( 1.1640,  2.018,  0.000);
const vec3 offset = vec3(-0.0625, -0.500, -0.500);

void main() {
    vec3 yuv;
    yuv.x = texture(PlaneTex0, TexCoord).r; 
    yuv.y = texture(PlaneTex1, TexCoord).r;
    yuv.z = texture(PlaneTex2, TexCoord).r;
    yuv = clamp(yuv, 0.0, 1.0);
    yuv += offset;

    FragColor.r = dot(yuv, Rcoeff);
    FragColor.g = dot(yuv, Gcoeff);
    FragColor.b = dot(yuv, Bcoeff);
    FragColor.a = 1.0;
} 
