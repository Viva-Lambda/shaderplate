#version 330
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D quadTex;

void main() {
  //
  vec3 pixel = texture(quadTex, TexCoord).rgb;
  FragColor = vec4(pixel, 1);
  // FragColor = vec4(1.0);
}
