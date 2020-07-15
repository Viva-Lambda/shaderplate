#version 430

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial;
layout(location = 4) out vec3 gAmbient;
layout(location = 5) out vec4 gSDepth;

in vec3 FragPosVS; // in view space
in vec4 FragPosCS; // in clip space
in vec3 FragPos;   // in world space

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D albedoMap;    // 0
uniform sampler2D normalMap;    // 1
uniform sampler2D roughnessMap; // 2
uniform sampler2D metallicMap;  // 3
uniform sampler2D aoMap;        // 4

// IBL
uniform samplerCube irradianceMap; // 5
uniform samplerCube prefilterMap;  // 6
uniform sampler2D brdfLUT;         // 7

uniform float maxMipLevels = 5.0;

uniform float fresnel = 0.4; // metal
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 lightPos;

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
/**
 * p. 16, of Cook Torrance 1982
 * */
float computeFresnelCT(float VdotH, float n) {
  float f0 = pow((n - 1.0) / (n + 1.0), 2);
  float eta = (1 + sqrt(f0)) / (1 - sqrt(f0));
  float g = sqrt(pow(eta, 2) + pow(VdotH, 2) - 1);
  float c = VdotH;
  float gminc = g - c;
  float gplusc = g + VdotH;
  float firstTerm = (pow(gminc, 2) / pow(gplusc, 2));
  float secondTerm = 1 + pow(c * gplusc - 1, 2) / pow(c * gminc + 1, 2);
  return 0.5 * firstTerm * secondTerm;
}

vec3 getIblSpecular(vec3 normal, vec3 viewDir, vec3 halfDir, float metallic,
                    vec3 albedo, float roughness, float ao, float fresnel) {

  float costheta = max(dot(viewDir, halfDir), 0.0);
  vec3 F = vec3(computeFresnelCT(costheta, fresnel));

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;
  vec3 R = normalize(reflect(-viewDir, normal));
  // another way to define reflection vector using vonmises
  // distribution, more rough the surface less biased
  // the reflection vector or greater the value of
  // kappa higher the output is concentrated on mean
  // direction
  // vec3 meandir = normalize(reflect(-viewDir, normal));
  // float kappa = 1 - texture(roughnessMap, TexCoord).r;
  // vec3 R = vonmises_dir(meandir, kappa);

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
  vec3 B = normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return TBN * tangentNormal;
}
vec3 getLightDir() { return normalize(FragPos - lightPos); }
vec3 getViewDir() { return normalize(FragPos - viewPos); }
vec3 getHalfDir() { return normalize(getViewDir() + getLightDir()); }

void main() {
  // set depth in view space
  gPosition.xyz = FragPosVS;
  gPosition.w = length(FragPosVS);
  // set depth in screen space
  gSDepth.xyz = gl_FragCoord.xyz;
  vec3 norm = getNormalFromMap(); // in world space
  vec3 NormalWS = normalize(norm);

  //
  vec3 albedo = texture(albedoMap, TexCoord).xyz;

  gAlbedo.xyz = albedo;
  gNormal.xyz = vec3(view * vec4(norm, 1)); // in view space

  float metallic = texture(metallicMap, TexCoord).r;
  float roughness = texture(roughnessMap, TexCoord).r;
  float ao = texture(aoMap, TexCoord).r;

  gMaterial.x = metallic;
  gMaterial.y = roughness;
  gMaterial.z = ao;
  if (metallic > 0.0) {
    // if the zone is metallic its fresnel
    // value is encoded in albedo
    gMaterial.w = gAlbedo.x;
  } else {
    gMaterial.w = fresnel;
  }
  vec3 viewDir = getViewDir();
  vec3 halfDir = getHalfDir();
  gAmbient.xyz = getIblSpecular(NormalWS, viewDir, halfDir, metallic, albedo,
                                roughness, ao, fresnel);
}
