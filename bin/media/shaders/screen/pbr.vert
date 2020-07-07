#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPosVS; // in view space

out vec2 TexCoord;
out vec3 ViewRay;

void main() {
  TexCoord = aTexCoord;
  mat4 invProjMat = inverse(projection);
  vec3 fragVS = vec3(invProjMat * vec4(aPos, 1));
  ViewRay = normalize(fragVS - camPosVS); // might produce bug
  // classic gl pos
  gl_Position = vec4(aPos, 1);
}
