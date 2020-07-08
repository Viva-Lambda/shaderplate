#version 430

layout(location = 0) out vec3 gDepth;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial;

in vec3 FragPosVS;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D albedoMap;    // 0
uniform sampler2D normalMap;    // 1
uniform sampler2D roughnessMap; // 2
uniform sampler2D metallicMap;  // 3
uniform sampler2D aoMap;        // 4

// ibl

uniform float fresnel = 0.4; // metal
uniform mat4 view;


/**
 * Compute normal in world space using TBN matrix
 * */

vec3 getNormalFromMap() {
  vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

  vec3 Q1 = dFdx(FragPos);
  vec3 Q2 = dFdy(FragPos);
  vec2 st1 = dFdx(TexCoord);
  vec2 st2 = dFdy(TexCoord);

  vec3 N = normalize(Normal);
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return TBN * tangentNormal;
}

void main() {
  // set depth in view space
  gDepth.xyz = FragPosVS;
  vec3 norm = getNormalFromMap(); // in view space
  vec3 NormalWS = normalize(norm);

  //
  gAlbedo = texture(albedoMap, TexCoord).xyz;

  gNormal.xyz = vec3(view *  vec4(norm,1)); // in world space

  float metallic = texture(metallicMap, TexCoord).r;
  float roughness = texture(roughnessMap, TexCoord).r;
  float ao = texture(aoMap, TexCoord).r;

  gMaterial.x = metallic;
  gMaterial.y = roughness;
  gMaterial.z = ao;
  gMaterial.w = fresnel;
}
