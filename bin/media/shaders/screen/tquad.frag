#version 430
in vec2 TexCoords;
layout(binding = 0) uniform sampler2D uvBuffer;
layout(binding = 1) uniform sampler2D lightBuffer;

vec2 clipToTextureSpace(vec2 p_cs) { return (p_cs + 1) / 2; }
vec2 screenToTextureSpace(vec2 p_ss) {
  p_ss /= vec2(textureSize(uvBuffer, 0));
  p_ss *= 2;
  return p_ss - 1.0;
}

out vec4 FragColor;
void main() {
  vec4 uv = texture(uvBuffer, TexCoords);
  vec3 L_in = uv.rgb;
  vec3 f_r = texture(lightBuffer, TexCoords).rgb;
  vec3 color;

  // following the regular rendering equation
  // \int f_r * L_in * NdotL
  if (uv.w != 0.0) {
    color = L_in; // uv.w;
    // color = vec3(0.3,0,0);
  } else {
    color = f_r;
  }

  // hdr
  // color = color / (color + vec3(1.0));

  //// gamma correct
  // color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1);
  // FragColor = vec4(L_in, 1);
}
