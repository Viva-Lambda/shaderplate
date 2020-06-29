#version 430

uniform sampler2D hdrQuad;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
  vec3 hdrColor = texture2D(hdrQuad, TexCoords).rgb;
  // reinhard tone mapping
  hdrColor /= (hdrColor + 1.0);

  // gamma correction
  float gamma = 2.2;
  // hdrColor = pow(hdrColor, vec3(1.0 / gamma));
  FragColor = vec4(hdrColor, 1.0);
  // FragColor = vec4(1.0);
}
