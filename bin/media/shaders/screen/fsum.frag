#version 330
// compute first sum of the split sum of B. Karis, 2013
layout(location = 0) out vec4 irradianceMap;
layout(location = 1) out vec4 prefilterMap;
layout(location = 2) out vec4 brdfLutTexture;

out vec4 FragColor;
in vec3 FragPos; // local object space

uniform sampler2D envMap;
uniform float roughness;
uniform vec3 viewPos; // should be in local object space as well.
uniform float cubemapResolution = 512.0;
uniform float sampleSize = 10.0;
uniform float sampleStep = 0.1;

const float PI = 3.14159265358979;

/**
 * adapted from raytracing in one weekend*/
void get_sphere_uv(in vec3 outNormal, out float u, out float v) {
  float phi = atan(outNormal.z, outNormal.x);
  float theta = asin(outNormal.y);
  u = 1 - (phi + PI) / (2 * PI);
  v = (theta + PI / 2) / PI;
}
vec2 phiThetaToUv(vec2 phiTheta) {
  float u = 1 - (phiTheta.x + PI) / (2 * PI);
  float v = 1 - (phiTheta.y + PI / 2) / PI;
  return vec2(u, v);
}
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
vec3 to_spherical(vec3 v) {
  //
  float r = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  float phi = atan(v.y / v.x);
  float theta = acos(v.z / r);
  return vec3(r, phi, theta);
}
vec2 screenToTextureSpace(vec2 p_ss) {
  p_ss /= vec2(textureSize(envMap, 0));
  p_ss *= 2;
  return p_ss - 1.0;
}

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
float VanDerCorpus(uint n, uint base) {
  float invBase = 1.0 / float(base);
  float denom = 1.0;
  float result = 0.0;

  for (uint i = 0u; i < 32u; ++i) {
    if (n > 0u) {
      denom = mod(float(n), 2.0);
      result += denom * invBase;
      invBase = invBase / 2.0;
      n = uint(float(n) / 2.0);
    }
  }

  return result;
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N) {
  return vec2(float(i) / float(N), VanDerCorpus(i, 2u));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness * roughness;

  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates - halfway vector
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space H vector to world-space sample vector
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}
float getMipMapLevel(vec2 uv) {
  vec2 dx = dFdx(uv);
  vec2 dy = dFdy(uv);
  return max(dot(dx, dx), dot(dy, dy));
}

vec3 getEnvMapColor() {
  float u, v;
  get_sphere_uv(normalize(FragPos), u, v);
  vec2 uv = vec2(u, v);
  return texture(envMap, uv).rgb;
}
vec3 getIrradianceColor(vec3 fpos) {

  vec3 irradiance = vec3(0);
  for (float i = 0.0; i < sampleSize; i += sampleStep) {
    float phi = random_double(0.0, 2.0 * PI);
    float theta = random_double(0.0, 0.5 * PI);

    float u = 1 - (phi + PI) / (2 * PI);
    float v = (theta + PI / 2) / PI;

    irradiance += texture(envMap, vec2(u, v)).rgb * sin(theta) * cos(theta);
  }
  return PI * irradiance * (1.0 / sampleSize);
}

vec3 getPrefilterColor(vec3 fpos, float roughness) {
  vec3 N = normalize(fpos);

  vec3 V = normalize(viewPos - fpos);
  vec3 R = reflect(V, N);

  const uint SAMPLE_COUNT = 1024u;
  vec3 prefilteredColor = vec3(0.0);
  float totalWeight = 0.0;

  for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
    // generates a sample vector that's biased towards the preferred alignment
    // direction (importance sampling).
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);
    vec3 spherical = to_spherical(L);
    float phi = spherical.y;
    float theta = spherical.z;
    vec2 uv = phiThetaToUv(vec2(phi, theta));
    vec2 wh = textureSize(envMap);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL > 0.0) {
      // sample from the environment's mip level based on roughness/pdf
      float D = DistributionGGX(N, H, roughness);
      float NdotH = max(dot(N, H), 0.0);
      float HdotV = max(dot(H, V), 0.0);
      float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

      float saTexel = 4.0 * PI / (6.0 * wh.x * wh.y);
      float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

      float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

      prefilteredColor += textureLod(envMap, uv, mipLevel).rgb;
    }
  }
  return prefilteredColor / float(SAMPLE_COUNT);
}
float GeometrySchlickGGX(float NdotV, float roughness) {
  // note that we use a different k for IBL
  float a = roughness;
  float k = (a * a) / 2.0;

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

vec2 IntegrateBRDF(float NdotV, float roughness) {
  vec3 V;
  V.x = sqrt(1.0 - NdotV * NdotV);
  V.y = 0.0;
  V.z = NdotV;

  float A = 0.0;
  float B = 0.0;

  vec3 N = vec3(0.0, 0.0, 1.0);

  const uint SAMPLE_COUNT = 1024u;
  for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
    // generates a sample vector that's biased towards the
    // preferred alignment direction (importance sampling).
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(L.z, 0.0);
    float NdotH = max(H.z, 0.0);
    float VdotH = max(dot(V, H), 0.0);

    if (NdotL > 0.0) {
      float G = GeometrySmith(N, V, L, roughness);
      float G_Vis = (G * VdotH) / (NdotH * NdotV);
      float Fc = pow(1.0 - VdotH, 5.0);

      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  A /= float(SAMPLE_COUNT);
  B /= float(SAMPLE_COUNT);
  return vec2(A, B);
}

void main() {
  float u, v;
  vec3 normal = normalize(FragPos);
  get_sphere_uv(normal, u, v);
  vec3 irradiance = getIrradianceColor(normal);
  vec3 prefilter = getPrefilterColor(normal, roughness);

  vec3 V = normalize(viewPos - fpos);
  float NdotV = dot(normal, V);
  vec2 brdf = IntegrateBRDF(NdotV, roughness);
  vec2 uv = vec2(u, v);
  vec3 ecolor = texture(envMap, uv).rgb;
  FragColor = vec4(color, 1.0);
}
