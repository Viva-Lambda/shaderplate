#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTan;
layout(location = 4) in vec3 aBiTan;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

uniform float fresnel = 0.04; // reflectance at zero incidence

out mat3 TBN;
out vec3 FragPosInView;
out vec2 TexCoord;

void main() {
  FragPosInView = vec3(view * model * vec4(aPos, 1.0));
  TBN = mat3(vec3(model * vec4(aTan, 0.0)), vec3(model * vec4(aBiTan, 0.0)),
             vec3(model * vec4(aNormal, 0.0)));
  TexCoord = aTexCoord;

  // classic gl pos
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}