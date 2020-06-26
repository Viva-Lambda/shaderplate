#version 330
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D quadTex;

void main() {
  //
  const float offsetval = 1.0 / 300.0;
  vec2 texOffset[9];
  texOffset[0] = vec2(-offsetval, offsetval);
  texOffset[1] = vec2(0, offsetval);
  texOffset[2] = vec2(offsetval, offsetval);
  texOffset[3] = vec2(-offsetval, 0);
  texOffset[4] = vec2(0, 0);
  texOffset[5] = vec2(offsetval, 0);
  texOffset[6] = vec2(-offsetval, -offsetval);
  texOffset[7] = vec2(0, -offsetval);
  texOffset[8] = vec2(offsetval, -offsetval);
  float kernel[9] = float[](1, 1, 1, 1, -10, 1, 1, 1, 1);

  vec3 stex[9];
  for (int i = 0; i < 9; i++) {
    stex[i] = texture(quadTex, TexCoord + texOffset[i]).rgb;
  }
  vec3 conv = vec3(0.0);
  for (int i = 0; i < 9; i++) {
    conv += stex[i] * kernel[i];
  }
  FragColor = vec4(conv, 1);
  // FragColor = vec4(1.0);
}
