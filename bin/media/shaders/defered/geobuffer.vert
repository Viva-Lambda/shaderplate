#version 430

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;

out vec3 FragPos;  // in world coordinates
out vec2 TexCoord; // in local coordinates if I am not mistaken
out vec3 Normal;   // in world coordinates as well

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main() {
  vec4 worldPos = model * vec4(aPos, 1);
  FragPos = worldPos.xyz; //
  TexCoord = aTex;
  Normal = transpose(inverse(mat3(model))) * aNormal;

  gl_Position = projection * view * worldPos;
}
