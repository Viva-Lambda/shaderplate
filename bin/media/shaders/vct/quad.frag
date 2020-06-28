#version 430

in vec2 TexCoords;
uniform sampler2D quad;
out vec4 FragColor;

void main() { FragColor = vec4(texture2D(quad, TexCoords).rgb, 1.0); }
