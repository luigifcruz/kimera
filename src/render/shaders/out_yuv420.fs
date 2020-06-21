#version 300 es

precision highp float;

in vec2 TexCoord;
uniform sampler2D renderedTexture;
uniform int PlaneId;
out float FragColor;

void main() {
    vec3 c = texture(renderedTexture, TexCoord).rgb;

    if (PlaneId == 0) // Y
        FragColor =  (0.257 * c.r) + (0.504 * c.g) + (0.098 * c.b) + 0.0625;
    if (PlaneId == 1) // U
        FragColor = -(0.148 * c.r) - (0.291 * c.g) + (0.439 * c.b) + 0.5000;
    if (PlaneId == 2) // V
        FragColor =  (0.439 * c.r) - (0.368 * c.g) - (0.071 * c.b) + 0.5000;
} 
