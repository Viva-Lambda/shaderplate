#version 430
in vec2 TexCoord;
in vec3 viewRay;

out vec4 FragColor;

// material parameters
uniform sampler2D gDepth;       // from GBuffer
uniform sampler2D gNormal;      // from GBuffer
uniform sampler2D gAlbedo;      // from GBuffer
uniform sampler2D gMaterial;    // from GBuffer
uniform sampler2D gIblSpecular; // from GBuffer

// lights
uniform vec3 lightPosVS; // in view space
uniform vec3 lightColor;

uniform vec3 camPosVS; // in view space

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// vec3 getNormalFromMap() {
// vec3 normalRayViewSpace = texture(normalMapGBuffer, TexCoord).xyz;
// float normalDist = texture(normalMapGBuffer, TexCoord).a;
// vec3 Normal = normalRayViewSpace * normalDist + camPos;
// return normalize(Normal);
//}

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
void main() {
  float viewDist = texture(gDepth, TexCoord).x;
  vec3 FragPosVS = viewRay * viewDist + camPosVS;

  // material properties
  vec3 albedo = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  float metallic = texture(gMaterial, TexCoord).r;
  float roughness = texture(gMaterial, TexCoord).g;
  float ao = texture(gMaterial, TexCoord).b;

  // input lighting data
  // vec3 N = getNormalFromMap();
  vec3 N = texture(gNormal, TexCoord).rgb; // in view space
  vec3 V = normalize(camPosVS - FragPosVS);
  vec3 refbias = normalize(reflect(-V, N));
  float kappa = 1.0 - roughness;
  vec3 R = vonmises_dir(refbias, kappa);

  vec3 F0 = vec3(texture(gMaterial, TexCoord).w);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  // calculate per-light radiance
  vec3 L = normalize(lightPosVS - FragPosVS);
  vec3 H = normalize(V + L);
  float dist = length(lightPosVS - FragPosVS);
  float attenuation = 1.0 / (dist * dist);
  vec3 radiance = lightColor * attenuation;

  // Cook-Torrance BRDF
  float NDF = DistributionGGX(N, H, roughness);
  float G = GeometrySmith(N, V, L, roughness);
  vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

  vec3 nominator = NDF * G * F;
  float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) +
                      0.001; // 0.001 to prevent divide by zero.
  vec3 specular = nominator / denominator;

  // kS is equal to Fresnel
  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  // scale light by NdotL
  float NdotL = max(dot(N, L), 0.0);

  // add to outgoing radiance Lo
  Lo += (kD * albedo / PI + specular) * radiance * NdotL;

  vec3 ambient = texture(gIblSpecular, TexCoord).rgb;
  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0 / 2.2));
  FragColor = vec4(color, 1);
}
