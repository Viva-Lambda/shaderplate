#version 330
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 cameraPos; // eyePos, viewPos

uniform samplerCube skybox;

void main() {
  vec3 viewDir = normalize(FragPos - cameraPos);
  vec3 R = reflect(viewDir, normalize(Normal));
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
  // FragColor = vec4(1.0);
}
