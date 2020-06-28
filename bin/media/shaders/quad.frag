#version 330
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() { FragColor = vec4(texture2D(screenTexture, TexCoord).rgb, 1); }
