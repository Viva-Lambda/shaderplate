#version 430

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTexture;
void main() {
  vec3 hdrColor = texture2D(screenTexture, TexCoord).rgb;
  hdrColor /= (hdrColor + 1.0);
  float gamma = 2.2;
  hdrColor = pow(hdrColor, vec3(1.0 / gamma));
  FragColor = vec4(hdrColor, 1);
}
