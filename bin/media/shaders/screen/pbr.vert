#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos; // in world space

out vec2 TexCoord;
out vec3 ViewRay;

void main() {
  TexCoord = aTexCoord;
  mat4 invVP = inverse(projection * view);
  ViewRay = normalize(vec3(invVP * vec4(aPos, 1)) - viewPos);
  // classic gl pos
  gl_Position = vec4(aPos, 1);
}
