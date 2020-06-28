#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  TexCoords = aTexCoord;
  gl_Position = projection * view * model * vec4(aPos, 1);
}
