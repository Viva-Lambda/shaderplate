#version 430
out vec4 FragColor;

in vec3 fcolor;

void main() { FragColor = vec4(fcolor,1); }
