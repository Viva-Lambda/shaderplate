#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec3 viewDir; // world space
uniform mat4 view;

out vec2 TexCoord;
out mat4 invViewMat;
out vec3 viewDirInViewSpace;

void main() {
  viewDirInViewSpace = vec3(view * vec4(viewDir, 1.0));
  invViewMat = inverse(view);
  TexCoord = aTexCoord;
  // classic gl pos
  gl_Position = vec4(aPos, 1.0);
}
