#version 430
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 FragPos;

void main() {
  //
  FragPos = aPos;
  mat4 rotView = mat4(mat3(view));
  vec4 clipPos = projection * rotView * vec4(FragPos, 1);
  gl_Position = clipPos.xyww;
}
