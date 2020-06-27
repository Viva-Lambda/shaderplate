#version 330

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 lightSpaceMat;

void main() {
  FragPos = vec3(model * vec4(aPos, 1));
  Normal = transpose(inverse(mat3(model))) * aNormal;
  TexCoords = aTexCoord;
  FragPosLightSpace = lightSpaceMat * vec4(FragPos, 1.0);
  gl_Position = projection * view * model * vec4(aPos, 1);
}
