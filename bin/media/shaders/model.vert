#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTan;
layout(location = 4) in vec3 aBiTan;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 TbnLightPos;
out vec3 TbnViewPos;
out vec3 TbnFragPos;

uniform sampler2D texture_normal1;
uniform float world_scale = 0.1;

vec4 bilinear_texture(sampler2D dmap, vec2 uv) {
  // perform bilinear texture
  // code adapted from here:
  // http://www.ozone3d.net/tutorials/vertex_displacement_mapping_p03.php
  ivec2 wh = textureSize(dmap, 0);
  float totalSize = float(wh.x * wh.y);
  vec2 tfract = fract(uv.xy * totalSize);
  vec4 t00 = texture2D(dmap, uv);
  vec4 t10 = texture2D(dmap, uv + vec2(totalSize, 0.0));
  vec4 ta = mix(t00, t10, tfract.x);
  vec4 t01 = texture2D(dmap, uv + vec2(0.0, totalSize));
  vec4 t11 = texture2D(dmap, uv + vec2(totalSize, totalSize));
  vec4 tb = mix(t01, t11, tfract.x);
  return mix(ta, tb, tfract.y);
}

vec3 new_pos() {
  //
  // http://www.ozone3d.net/tutorials/vertex_displacement_mapping_p03.php
  vec3 dn = texture2D(texture_normal1, aTexCoord.xy).rgb * 2.0 - 1.0;
  vec4 dv = bilinear_texture(texture_normal1, aTexCoord.xy);
  float df = 0.30 * dv.x + 0.59 * dv.y + 0.11 * dv.z;
  return vec3(aNormal * dn * world_scale) + aPos;
}

void main() {
  vec3 npos = aPos;
  FragPos = vec3(model * vec4(npos, 1.0));
  TexCoord = aTexCoord;

  //
  // compute tan world
  vec4 tanW = model * vec4(aTan, 0.0);
  vec3 Tan = normalize(vec3(tanW));
  // compute norm world
  vec4 norm4 = model * vec4(aNormal, 0.0);
  vec3 Norm = normalize(vec3(norm4));
  // make t perpendicular to n
  Tan = normalize(Tan - dot(Tan, Norm) * Norm);
  vec3 BiTan = cross(Norm, Tan);

  // get tbn mat
  mat3 tbn = transpose(mat3(Tan, BiTan, Norm));
  TbnLightPos = tbn * lightPos;
  TbnViewPos = tbn * viewPos;
  TbnFragPos = tbn * FragPos;
  // TbnLightPos = lightPos;
  // TbnViewPos = viewPos;
  // TbnFragPos = FragPos;

  // classic gl pos
  gl_Position = projection * view * model * vec4(npos, 1.0);
}
