#version 430
in vec2 TexCoords;
layout(binding = 0) uniform sampler2D uvBuffer;
layout(binding = 1) uniform sampler2D lightBuffer;

uniform float alpha = 0.1;

vec2 clipToTextureSpace(vec2 p_cs) { return (p_cs + 1) / 2; }
vec2 screenToTextureSpace(vec2 p_ss) {
  p_ss /= vec2(textureSize(uvBuffer, 0));
  p_ss *= 2;
  return p_ss - 1.0;
}

/**
 * Gaussian horizontal blur
 * */
vec3 blurColor() {
  //
  vec4 uv = texture(uvBuffer, TexCoords);
  vec2 toffset = 1.0 / textureSize(uvBuffer, 0);

  float weights[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
  vec3 result = uv.xyz * weights[0];
  for (uint i = 1; i < 5; i++) {
    result += texture(uvBuffer, TexCoords + vec2(toffset.x * i, 0.0)).rgb *
              weights[i];
    result += texture(uvBuffer, TexCoords - vec2(toffset.x * i, 0.0)).rgb *
              weights[i];
  }
  return result;
}

out vec4 FragColor;
void main() {
  vec3 f_r = texture(lightBuffer, TexCoords).rgb;
  vec4 uv = texture(uvBuffer, TexCoords);

  // following the regular rendering equation
  // \int f_r * L_in * NdotL
  if (uv.a == 0.0) {
    // L_in == 0
    FragColor = vec4(f_r, 1);
    return;
  }
  vec3 blurred = blurColor();
  vec3 color = (alpha * blurred) + (1 - alpha) * uv.rgb;
  // vec3 color = uv.rgb;
  // vec3 color = uv.rgb * f_r;

  // hdr
  color = color / (color + vec3(1.0));

  //// gamma correct
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1);
}
