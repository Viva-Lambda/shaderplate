#version 330 core

out vec4 FragColor;

in vec3 myColor;
in vec2 TexCoord;

uniform sampler2D myTexture;

void main() {
  vec4 tcolor = texture(myTexture, TexCoord);
  FragColor = vec4(tcolor.x, tcolor.y, tcolor.z, tcolor.w); // gray
}
