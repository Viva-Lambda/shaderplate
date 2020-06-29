#version 330
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform float lightIntensity;

void main() {
  //
  FragColor = vec4(vec3(lightIntensity), 1);
}
