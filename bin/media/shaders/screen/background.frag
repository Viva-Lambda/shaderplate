#version 430

out vec4 FragColor;
in vec3 FragPos;

uniform samplerCube envMap;

void main() {
  vec3 envColor = texture(envMap, FragPos).rgb;

  // tone mapping reinhard
  envColor = envColor / (envColor + vec3(1.0));
  envColor = pow(envColor, vec3(1.0 / 2.2));

  FragColor = vec4(envColor, 1.0);
}
