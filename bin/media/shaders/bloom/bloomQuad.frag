#version 330
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloomCheck;
uniform float exposure;

void main() {
  //
  float gamma = 2.2;
  vec3 hdrColor = texture(scene, TexCoord).rgb;
  vec3 bloomColor = texture(bloomBlur, TexCoord).rgb;
  if (bloomCheck) {
    hdrColor += bloomColor;
  }
  vec3 res = vec3(1.0) - exp(-hdrColor * exposure); // exposure mapping

  res = pow(res, vec3(1.0 / gamma));
  FragColor = vec4(res, 1.0);
}
