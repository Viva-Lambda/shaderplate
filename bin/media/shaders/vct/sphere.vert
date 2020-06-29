#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main() {
  TexCoords = aTexCoord;
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(model) * aNormal;
  //
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
