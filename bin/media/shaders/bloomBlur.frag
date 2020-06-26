#version 330

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D img;
uniform bool horizontal;
uniform float weight[5] = {0.2, 0.18, 0.12, 0.04, 0.012};

void main() {
  vec2 toffset = 1.0 / textureSize(img, 0);
  vec3 res = texture2D(img, TexCoord).rgb * weight[0];
  if (horizontal) {
    //
    for (int i = 1; i < 5; i++) {
      res +=
          texture2D(img, TexCoord + vec2(toffset.x + i, 0.0)).rgb * weight[i];
      res +=
          texture2D(img, TexCoord - vec2(toffset.x + i, 0.0)).rgb * weight[i];
    }
  } else {
    for (int i = 1; i < 5; i++) {
      res += texture(img, TexCoord + vec2(0.0, toffset.y + i)).rgb * weight[i];
      res += texture(img, TexCoord - vec2(0.0, toffset.y + i)).rgb * weight[i];
    }
  }
  FragColor = vec4(res, 1.0);
}
