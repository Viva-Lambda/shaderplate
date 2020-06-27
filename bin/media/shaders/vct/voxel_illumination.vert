#version 430
// voxelize pixel
//
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out Vertex_Out {
  vec3 Normal;
  vec4 FragPos;
  vec2 TexCoord;
  mat3 TBN;
}
vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  vs_out.TexCoord = aTexCoord;
  vs_out.FragPos = model * vec4(aPos, 1);
  vs_out.TBN =
      mat3(vec3(model * vec4(aTan, 0.0)), vec3(model * vec4(aBiTan, 0.0)),
           vec3(model * vec4(aNormal, 0.0)));

  vs_out.Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
  gl_Position = projection * view * FragPos;
}
