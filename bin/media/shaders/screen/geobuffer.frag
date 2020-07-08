#version 430

layout(location = 0) out vec3 gDepth;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial;
layout(location = 4) out vec3 gIblSpecular;

in vec3 FragPos;
in vec3 FragPosInView;
in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D albedoMap;    // 0
uniform sampler2D normalMap;    // 1
uniform sampler2D roughnessMap; // 2
uniform sampler2D metallicMap;  // 3
uniform sampler2D aoMap;        // 4

// ibl
// IBL
uniform samplerCube irradianceMap; // 5
uniform samplerCube prefilterMap;  // 6
uniform sampler2D brdfLUT;         // 7

uniform float maxMipLevels = 5.0;

uniform mat4 view;

uniform float fresnel = 0.4; // metal
uniform vec3 viewPos;        // world space

/**
 * Utility code for von fischer distribution
 * */
const float PI = 3.14159265359;

float rand(vec2 co) {
  // random gen
  float a = 12.9898;
  float b = 78.233;
  float c = 43758.5453;
  float dt = dot(co.xy, vec2(a, b));
  float sn = mod(dt, PI);
  return fract(sin(sn) * c);
}
float random_double() {
  // random double
  return rand(vec2(0.0, 1.0));
}
float random_double(float mi, float mx) {
  // random double
  return rand(vec2(mi, mx));
}
int random_int(int mi, int mx) { return int(random_double(mi, mx)); }

vec3 random_vec() {
  // random vector
  return vec3(random_double(), random_double(), random_double());
}
vec3 random_vec(float mi, float ma) {
  // random vector in given seed
  return vec3(random_double(mi, ma), random_double(mi, ma),
              random_double(mi, ma));
}
vec3 random_unit_vector() {
  // unit vector
  float a = random_double(0, 2 * PI);
  float z = random_double(-1, 1);
  float r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
}
vec3 to_spherical(vec3 v) {
  //
  float r = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  float phi = atan(v.y / v.x);
  float theta = acos(v.z / r);
  return vec3(r, phi, theta);
}
vec3 vonmises_dir(vec3 bias_dir, float kappa) {
  // not done
  vec3 unit_bias = normalize(bias_dir);
  vec3 spherical = to_spherical(unit_bias);
  float theta = spherical.z;
  float normalization = kappa / (2 * PI * (exp(kappa) - exp(-kappa)));
  return exp(kappa * cos(theta)) * normalization;
}

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

  return normalize(TBN * tangentNormal);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 getIblSpecular(vec3 normal, vec3 viewDir, float metallic, vec3 albedo,
                    float roughness, float ao) {

  vec3 F0 = vec3(fresnel);
  F0 = mix(F0, albedo, metallic);
  float costheta = max(dot(normal, viewDir), 0.0);
  vec3 F = fresnelSchlickRoughness(costheta, F0, roughness);

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;
  vec3 refbias = normalize(reflect(-viewDir, normal));
  float kappa = 1.0 - roughness;
  vec3 R = vonmises_dir(refbias, kappa);

  vec3 irradiance = texture(irradianceMap, normal).rgb;
  vec3 diffuse = irradiance * albedo;

  const float MAX_REFLECTION_LOD = maxMipLevels - 1.0;
  vec3 prefilteredColor =
      textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec2 brdf = texture(brdfLUT, vec2(costheta, roughness)).rg;
  vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

  vec3 ambient = (kD * diffuse + specular) * ao;
  return ambient;
}

void main() {
  // set depth in view space
  // gDepth.xyz = normalize(FragPosInView);
  // gDepth.x = length(FragPosInView);
  gDepth = FragPos;
  // gDepth.y = length(FragPosInView);
  vec3 norm = getNormalFromMap(); // in view space
  vec3 viewDir = normalize(viewPos - FragPos);

  //
  gAlbedo.xyz = texture(albedoMap, TexCoord).xyz;

  gNormal.xyz = normalize(norm); // in world space
  gNormal.w = length(norm);

  float metallic = texture(metallicMap, TexCoord).r;
  float roughness = texture(roughnessMap, TexCoord).r;
  float ao = texture(aoMap, TexCoord).r;

  gMaterial.x = metallic;
  gMaterial.y = roughness;
  gMaterial.z = ao;
  gMaterial.w = fresnel;

  gIblSpecular = getIblSpecular(normalize(norm), viewDir, metallic, gAlbedo.xyz,
                                roughness, ao);
  // gIblSpecular = 0.1* gAlbedo.xyz;
}
