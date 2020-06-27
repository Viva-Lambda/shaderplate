#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out Vertex_Out {
  vec4 Normal;
  vec4 ViewDir;
  vec4 LightDir;
  vec3 lightDirColor;
  vec3 viewDirColor;
  vec3 normalColor;
}
vs_out;

uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
  //
  vec4 worldLightPos = model * vec4(lightPos, 1);
  vec4 worldCameraPos = model * vec4(viewPos, 1);
  vec4 worldFragPos = model * vec4(aPos, 1);
  mat3 normalMatrix = mat3(transpose(inverse(view * model)));
  vs_out.Normal = vec4(normalMatrix * aNormal, 0.0);
  vs_out.ViewDir = worldCameraPos - worldFragPos;
  vs_out.LightDir = worldLightPos - worldFragPos;
  vs_out.lightDirColor = vec3(1, 0, 0);
  vs_out.viewDirColor = vec3(0, 1, 0);
  vs_out.normalColor = vec3(0, 0, 1);

  gl_Position = view * worldFragPos;
}
