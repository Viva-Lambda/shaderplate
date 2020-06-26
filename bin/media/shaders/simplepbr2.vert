#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTan;
layout(location = 4) in vec3 aBiTan;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out vec3 TbnLightPos;
out vec3 TbnViewPos;
out vec3 TbnFragPos;

void main() {
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;
  TexCoord = aTexCoord;

  //
  mat3 nmat = transpose(inverse(mat3(model)));
  // compute tan world
  vec4 tanW = model * vec4(aTan, 0.0);
  vec3 Tan = normalize(vec3(tanW));
  // compute norm world
  vec4 norm4 = model * vec4(aNormal, 0.0);
  vec3 Normal = normalize(vec3(norm4));
  // make t perpendicular to n
  Tan = normalize(Tan - dot(Tan, Normal) * Normal);
  vec3 BiTan = cross(Normal, Tan);

  // get tbn mat
  mat3 tbn = transpose(mat3(Tan, BiTan, Normal));
  TbnLightPos = tbn * lightPos;
  TbnViewPos = tbn * viewPos;
  TbnFragPos = tbn * FragPos;

  // classic gl pos
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
