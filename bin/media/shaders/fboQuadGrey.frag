#version 330
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D quadTex;

void main() {
  //
  vec3 pixel = texture(quadTex, TexCoord).rgb;
  vec3 mean = vec3(0.212 * pixel.r + 0.715 * pixel.g + 0.07 * pixel.b);
  FragColor = vec4(mean, 1);
  // FragColor = vec4(1.0);
}
