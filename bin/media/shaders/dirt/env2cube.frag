#version 430

out vec4 FragColor;
in vec3 FragPos;

uniform sampler2D envMap;

const float PI = 3.14159265358979;

/**
 * adapted from raytracing in one weekend*/
void get_sphere_uv(in vec3 outNormal, out float u, out float v) {
  float phi = atan(outNormal.z, outNormal.x);
  float theta = asin(outNormal.y);
  u = 1 - (phi + PI) / (2 * PI);
  v = (theta + PI / 2) / PI;
}

void main() {
  float u, v;
  get_sphere_uv(normalize(FragPos), u, v);
  vec2 uv = vec2(u, v);
  vec3 color = texture(envMap, uv).rgb;
  FragColor = vec4(color, 1.0);
}
