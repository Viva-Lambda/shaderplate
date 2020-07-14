#version 430
in vec2 TexCoords;
layout(binding = 0) uniform sampler2D uvBuffer;
layout(binding = 1) uniform sampler2D lightBuffer;
out vec4 FragColor;
void main() {
  vec4 uv = texture(uvBuffer, TexCoords);
  if (uv.b <= 0.0) {
    FragColor = texture(lightBuffer, TexCoords);
  } else {
    vec4 color4X = textureGather(lightBuffer, uv.xy, 0);
    vec4 color4Y = textureGather(lightBuffer, uv.xy, 1);
    vec4 color4Z = textureGather(lightBuffer, uv.xy, 2);
    vec4 color4A = textureGather(lightBuffer, uv.xy, 3);
    vec4 first = vec4(color4X.x, color4Y.x, color4Z.x, color4A.x);
    vec4 second = vec4(color4X.y, color4Y.y, color4Z.y, color4A.y);
    vec4 third = vec4(color4X.z, color4Y.z, color4Z.z, color4A.z);
    vec4 fourth = vec4(color4X.a, color4Y.a, color4Z.a, color4A.a);
    vec4 color = (first + second + third + fourth) / 4.0;
    float alpha = clamp(uv.z, 0.0, 1.0);

    FragColor = vec4(mix(vec3(0.0), color.rgb, alpha), alpha);
  }
}
