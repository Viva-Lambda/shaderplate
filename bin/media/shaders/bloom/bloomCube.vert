#version 330
layout(location = 0) in vec3 aPos;
layout(location = 0) in vec3 aNormal;
layout(location = 0) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  //
  FragPos = vec3(model * vec4(aPos, 1));
  TexCoord = aTexCoord;

  mat3 normalMatrix = transpose(inverse(mat3(model)));
  Normal = normalize(normalMatrix * aNormal);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
