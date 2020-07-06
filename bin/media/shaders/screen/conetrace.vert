#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTan;
layout(location = 4) in vec3 aBiTan;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

uniform vec3 viewDir;

out vec2 TexCoord;
out vec3 FragInViewSpace;
out vec3 FragInScreenSpace;
out mat4 invViewMat;
out vec3 viewDirInViewSpace;

void main() {
  vec4 vpos = view * model * vec4(aPos, 1.0);
  FragInViewSpace = vec3(vpos);
  FragInScreenSpace = vec3(projection * vpos);
  viewDirInViewSpace = vec3(view * model * vec4(viewDir, 1.0));
  invViewMat = inverse(view);
  TexCoord = aTexCoord;
  // classic gl pos
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
