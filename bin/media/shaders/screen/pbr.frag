#version 430
// layout(location = 0) out vec3 FragColor;
out vec3 FragColor;

in vec2 TexCoord;
in vec3 ViewRay;

// material parameters
uniform sampler2D gDepth;    // from GBuffer
uniform sampler2D gNormal;   // from GBuffer
uniform sampler2D gAlbedo;   // from GBuffer
uniform sampler2D gMaterial; // from GBuffer

// IBL
uniform samplerCube irradianceMap; // 5
uniform samplerCube prefilterMap;  // 6
uniform sampler2D brdfLUT;         // 7

uniform float maxMipLevels = 5.0;

// lights
uniform vec3 lightPos; // in world space
uniform vec3 lightColor;

uniform vec3 viewPos; // in world space
uniform mat4 view;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------

/**
 * Utility code for von fischer distribution
 * */

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
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 getIblSpecular(vec3 normal, vec3 viewDir, float metallic, vec3 albedo,
                    float roughness, float ao, float fresnel) {

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
  float viewDist = texture(gDepth, TexCoord).x;
  vec3 FragPos = ViewRay * viewDist + viewPos;
  vec3 FragPosVS = vec3(view * vec4(FragPos,1));

  // material properties
  vec3 albedo = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  float metallic = texture(gMaterial, TexCoord).x;
  float roughness = texture(gMaterial, TexCoord).y;
  float ao = texture(gMaterial, TexCoord).z;

  // input lighting data
  vec2 NormalVSxy = texture(gDepth, TexCoord).yz; // in view space
  float NormalVSz = texture(gNormal, TexCoord).x; // in view space
  vec3 NormalVS = vec3(NormalVSxy, NormalVSz);
  vec3 ViewPosVS = vec3(view * vec4(viewPos, 1));
  vec3 V = normalize(ViewPosVS - FragPosVS);
  vec3 refbias = normalize(reflect(-V, NormalVS));
  float kappa = 1.0 - roughness;
  vec3 R = vonmises_dir(refbias, kappa);
  float fresnel = texture(gMaterial, TexCoord).w;

  vec3 F0 = vec3(fresnel);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  // calculate per-light radiance
  vec3 LightPosVS = vec3(view * vec4(lightPos,1));
  vec3 L = normalize(LightPosVS - FragPosVS);
  vec3 H = normalize(V + L);
  float dist = length(LightPosVS - FragPosVS);
  float attenuation = 1.0 / (dist * dist);
  vec3 radiance = lightColor * attenuation;

  // Cook-Torrance BRDF
  float NDF = DistributionGGX(NormalVS, H, roughness);
  float G = GeometrySmith(NormalVS, V, L, roughness);
  vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

  vec3 nominator = NDF * G * F;
  float denominator = 4 * max(dot(NormalVS, V), 0.0) * max(dot(NormalVS, L), 0.0) +
                      0.0001; // 0.001 to prevent divide by zero.
  vec3 specular = nominator / denominator;

  // kS is equal to Fresnel
  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  // scale light by NdotL
  float NdotL = max(dot(NormalVS, L), 0.0);

  // add to outgoing radiance Lo
  Lo += (kD * albedo / PI + specular) * radiance * NdotL;

  vec3 ambient = getIblSpecular(NormalVS, V, metallic, albedo, roughness, ao, fresnel);
  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  //FragColor = pow(color, vec3(1.0 / 2.2));
  FragColor = vec3(1.0);
}
