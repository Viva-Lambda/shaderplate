#version 430

in vec3 FragPosInView;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;

layout(location = 0) out vec4 gNormal;
layout(location = 1) out vec4 gMaterial;
layout(location = 2) out vec4 gDepth;

uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform float fresnel = 0.04; // reflectance at zero incidence
uniform mat4 view;

vec3 getSurfaceNormal() {
  vec3 normal1 = texture(normalMap, TexCoord).rgb;
  normal1 = normal1 * 2.0 - 1.0;
  return normalize(TBN * normal1);
}

void main() {
  // set depth in view space
  // gDepth.xyz = normalize(FragPosInView);
  gDepth.r = length(FragPosInView);

  // set values to material buffer
  gMaterial.x = fresnel;
  gMaterial.z = texture(roughnessMap, TexCoord).z;
  gMaterial.y = texture(metallicMap, TexCoord).y;

  // set normal in view space
  vec3 normalWorldSpace = getSurfaceNormal();
  vec3 normalView = vec3(view * vec4(normalWorldSpace, 1));
  gNormal.xyz = normalize(normalView);
  gNormal.w = length(normalView);
}
