#version 430

layout(location = 0) out vec3 gDepth;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial;

in float FragDistInView;
in vec3 NormalInView;
in vec2 TexCoord;
in mat3 TBN;

uniform sampler2D albedoMap;    // 0
uniform sampler2D normalMap;    // 1
uniform sampler2D roughnessMap; // 2
uniform sampler2D metallicMap;  // 3
uniform sampler2D aoMap;        // 4

uniform float fresnel = 0.04;

vec3 getNormalFromMap() {
  vec3 normalRayViewSpace = texture(normalMap, TexCoord).xyz;
  float normalDist = texture(normalMapGBuffer, TexCoord).a;
  vec3 Normal = normalRayViewSpace * normalDist + camPos;
  return normalize(Normal);
}

void main() {
  // set depth in view space
  // gDepth.xyz = normalize(FragPosInView);
  gDepth.w = DepthInView;
  gDepth.xyz = NormalInView;

  //
  gAlbedo.xyz = texture(albedoMap, TexCoord).xyz;
  gNormal.xyz = texture(normalMap, TexCoord).xyz;
  float metallic = texture(metallicMap, TexCoord).r;
  float roughness = texture(roughnessMap, TexCoord).r;
  float ao = texture(aoMap, TexCoord).r;
  gMaterial.x = metallic;
  gMaterial.y = roughness;
  gMaterial.z = ao;
  gMaterial.w = fresnel;
}
