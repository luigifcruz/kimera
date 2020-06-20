#version 330 core

precision highp float;

in vec2 TexCoord;
uniform sampler2D renderedTexture;

out float FragColor0;
out float FragColor1;
out float FragColor2;

void main() {
    vec3 c = texture(renderedTexture, TexCoord).rgb; 
    FragColor0 =  (0.257 * c.r) + (0.504 * c.g) + (0.098 * c.b) + 0.0625;
    FragColor1 =  (0.439 * c.r) - (0.368 * c.g) - (0.071 * c.b) + 0.5000;
    FragColor2 = -(0.148 * c.r) - (0.291 * c.g) + (0.439 * c.b) + 0.5000;
} 
