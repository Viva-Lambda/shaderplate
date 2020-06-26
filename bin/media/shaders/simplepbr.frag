#version 330 core
// fragment shader for pbr

out vec4 FragColor;
in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

// pbr materials
uniform sampler2D albedoMap;    // in main
uniform sampler2D metallicMap;  // in main
uniform sampler2D normalMap;    // in main
uniform sampler2D roughnessMap; // in main

// light related
uniform vec3 lightPos;
uniform vec3 lightColor;

// camera related
uniform vec3 viewPos;

// pi value
const float PI = 3.14159265;

// pbr functions

uniform int ndf; // normal distribution function choice

// Normal Distribution Function

float roughnessToAlpha(float roughness);

// How much of microsurface area is aligned with
// halfway vector

float bsNormalDistTraditional(vec3 normal, vec3 halfwayDir, float roughness);
// traditional Beckmann Spizzichino Distribution

float bsNormalDistAnisotropic(vec3 normal, vec3 halfwayDir, vec2 roughness);
// anisotropic version Beckmann Spizzichino Distribution

float trowReitzTraditional(vec3 normal, vec3 halfwayDir, float roughness);
// traditional Trowbridge Reitz normal distribution

float trowReitzAnisotropic(vec3 normal, vec3 halfwayDir, vec2 roughness);
// anisotropic version Trowbridge Reitz normal distribution

// Geometry Function

// How much of microsurface area is occluded due
// to self shadowing with respect to viewing direction

float getAlpha(vec3 normal, vec3 halfDir, vec2 roughness);

float geometryDistributionIn(float lambda);
float geometryInOut(float lambdaIn, float lambdaOut);

float bsLamdaTFn(vec3 normal, vec3 halfway, float roughness);
// lambda for traditional Beckmann Spizzichino Distribution

void bsLamdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                  float lambdaArr[2]);

float bsLamdaAFn(vec3 normal, vec3 halfway, vec2 roughness);
// lambda for anisoptric Beckmann Spizzichino Distribution
void bsLamdaAFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, vec2 roughness,
                  float lambdaArr[2]);

float bsLambdaFn(vec3 normal, vec3 halfway, float alpha);

float trowReitzLambdaT(vec3 normal, vec3 halfway, float roughness);
// lambda for antisoptric Trowbridge Reitz Distribution

float trowReitzLambdaA(vec3 normal, vec3 halfway, vec2 roughness);
// lambda for antisoptric Trowbridge Reitz Distribution
float trowReitzLambda(vec3 normal, vec3 halfway, float alpha);

void trowReitzLambdaTIO(vec3 normal, vec3 halfDir, vec3 viewDir,
                        float roughness, float lambdaArr[2]);
void trowReitzLambdaAIO(vec3 normal, vec3 halfDir, vec3 viewDir, vec2 roughness,
                        float lambdaArr[2]);

// Fresnel Function

vec3 getFresnelSchlick(float costheta, vec3 refAtZero);
// roughness included due to indirect lightening
vec3 diffuseCT(); // cook torrence diffuse color

// some utility
float getCosTheta(vec3 normal, vec3 lightDir);
float getSinTheta(vec3 normal, vec3 lightDir);
float getCos2Theta(vec3 normal, vec3 lightDir);
float getSin2Theta(vec3 normal, vec3 lightDir);
float getTanTheta(vec3 normal, vec3 lightDir);
float getTan2Theta(vec3 normal, vec3 lightDir);
float getCosPhi(vec3 normal, vec3 lightDir);
float getCos2Phi(vec3 normal, vec3 lightDir);
float getSinPhi(vec3 normal, vec3 lightDir);
float getSin2Phi(vec3 normal, vec3 lightDir);
float getTanPhi(vec3 normal, vec3 lightDir);
float getTan2Phi(vec3 normal, vec3 lightDir);

float computeAttenuation(vec3 att, float lfragdist);
vec3 getSurfaceNormal();

void main() {
  // main func for pbr
  vec3 viewDir = normalize(viewPos - WorldPos);

  // get albedo
  vec3 albedo = texture(albedoMap, TexCoord).rgb;

  // get metallic
  vec3 metallic = texture(metallicMap, TexCoord).rgb;

  // get normalMap
  vec3 snormal = getSurfaceNormal();

  // get ao map
  vec3 ao = vec3(1.0);

  vec3 L_out = vec3(0.0);

  // start light source loop

  vec3 lightDir = normalize(lightPos - WorldPos);
  vec3 halfDir = normalize(viewDir + lightDir);
  vec3 attc = vec3(1.0, 0.0, 0.0);
  vec3 refAtZero = vec3(0.04);
  // mix albedo + metallic + fresnel
  refAtZero = mix(refAtZero, metallic, albedo);
  float hcostheta = getCosTheta(snormal, halfDir);
  vec3 fresnel = getFresnelSchlick(hcostheta, refAtZero);

  float lightToFragDist = length(lightPos - WorldPos);
  float attenuation = computeAttenuation(attc, lightToFragDist);
  float dN; // Normal Distribution
  float gD;
  float lambdaArr[2];
  float lambdaIn;
  float lambdaOut;
  vec3 radiance = lightColor * attenuation;
  if (ndf == 0) { // Beckmann traditional
    float rough = texture(roughnessMap, TexCoord).x;
    dN = bsNormalDistTraditional(snormal, halfDir, rough);
    bsLamdaTFnIO(snormal, halfDir, viewDir, rough, lambdaArr);
    lambdaIn = lambdaArr[0];
    lambdaOut = lambdaArr[1];
    gD = geometryInOut(lambdaIn, lambdaOut);
  } else if (ndf == 1) { // Beckmann anisoptric
    vec2 rough = texture(roughnessMap, TexCoord).xy;
    dN = bsNormalDistAnisotropic(snormal, halfDir, rough);
    bsLamdaAFnIO(snormal, halfDir, viewDir, rough, lambdaArr);
    lambdaIn = lambdaArr[0];
    lambdaOut = lambdaArr[1];
    gD = geometryInOut(lambdaIn, lambdaOut);
  } else if (ndf == 2) { // Trowbridge Reitz traditional
    float rough = texture(roughnessMap, TexCoord).x;
    dN = trowReitzTraditional(snormal, halfDir, rough);
    trowReitzLambdaTIO(snormal, halfDir, viewDir, rough, lambdaArr);
    lambdaIn = lambdaArr[0];
    lambdaOut = lambdaArr[1];
    gD = geometryInOut(lambdaIn, lambdaOut);
  } else if (ndf == 3) { // Trowbridge Reitz anisoptric
    vec2 rough = texture(roughnessMap, TexCoord).xy;
    dN = trowReitzAnisotropic(snormal, halfDir, rough);
    trowReitzLambdaAIO(snormal, halfDir, viewDir, rough, lambdaArr);
    lambdaIn = lambdaArr[0];
    lambdaOut = lambdaArr[1];
    gD = geometryInOut(lambdaIn, lambdaOut);
  }
  vec3 ks = fresnel;
  vec3 kd = vec3(1.0) - ks;
  kd = kd * (1.0 - metallic.x);
  vec3 t1 = dN * gD * fresnel;

  float outDir = getCosTheta(snormal, viewDir);
  float inDir = getCosTheta(snormal, lightDir);

  float t2 = 4.0 * outDir * inDir;
  vec3 specular = t1 / max(t2, 0.0001);
  L_out = (kd * albedo.r / PI + specular) * radiance * inDir;

  // end light source loop done
  // ambient coefficient k_a
  // vec3 ambient = albedo.r * ao * k_a;
  vec3 ambient = albedo.r * ao;
  vec3 color = ambient + L_out;
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));
  //FragColor = vec4(color, 1.0);
  //FragColor = vec4(t1, 1.0); // black cube
  //FragColor = vec4(t2); // black cube
  //FragColor = vec4(ambient, 1.0); // white cube
  //FragColor = vec4(gD); // white cube
  FragColor = vec4(color, 1.0); // black cube
}

vec3 diffuseCT() {
  vec3 albedo = texture(albedoMap, TexCoord).rgb;
  return albedo / PI;
}

float getCosTheta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return dot(normal, inLightDir);
}
float getCos2Theta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  float costheta = getCosTheta(normal, inLightDir);
  return costheta * costheta;
}
float getSin2Theta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  float cos2theta = getCos2Theta(normal, inLightDir);
  return 1 - cos2theta;
}
float getSinTheta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return sqrt(max(0, getSin2Theta(normal, inLightDir)));
}
float getTanTheta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return getSinTheta(normal, inLightDir) / getCosTheta(normal, inLightDir);
}
float getTan2Theta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return getTanTheta(normal, inLightDir) / getTanTheta(normal, inLightDir);
}
float getCosPhi(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  float radius = getSinTheta(normal, inLightDir);
  float res;
  if (radius == 0) {
    res = 1.0;
  } else {
    res = clamp(inLightDir.x / radius, -1, 1);
  }
  return res;
}
float getCos2Phi(vec3 normal, vec3 inLightDir) {
  return pow(getCosPhi(normal, inLightDir), 2);
}
float getSin2Phi(vec3 normal, vec3 inLightDir) {
  return 1 - getCos2Phi(normal, inLightDir);
}
float getSinPhi(vec3 normal, vec3 inLightDir) {
  return sqrt(max(0, getSin2Theta(normal, inLightDir)));
}
float computeAttenuation(vec3 att, float lightToFragDist) {
  float distSqr = lightToFragDist * lightToFragDist;
  float att1 = lightToFragDist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  result = 1 / result;
  float attenuation = min(result, 1);
  return attenuation;
}
vec3 getSurfaceNormal() {
  vec3 normal = texture(normalMap, TexCoord).rgb;
  vec3 q1 = dFdx(WorldPos);
  vec3 q2 = dFdy(WorldPos);
  vec2 tex1 = dFdx(TexCoord);
  vec2 tex2 = dFdy(TexCoord);
  vec3 N = normalize(Normal);
  vec3 T = normalize(q1 * tex2.t - q2 * tex1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);
  return normalize(TBN * normal);
}

float roughnessToAlpha(float roughness) {
  // taken from
  // https://github.com/mmp/pbrt-v3/blob/9f717d847a807793fa966cf0eaa366852efef167/src/core/microfacet.h
  float rough = max(roughness, pow(10, -3));
  float rlog = log(rough);
  float t1 = 0.000640711 * pow(rlog, 4);
  t1 += 0.0171201 * pow(rlog, 3);
  t1 += 0.1734 * pow(rlog, 2);
  t1 += 0.819955 * rlog;
  t1 += 1.62142;
  return t1;
}

float bsNormalDistTraditional(vec3 normal, vec3 halfwayDir, float roughness) {
  // traditional version of beckmann spizzichino
  // distribution function
  // taken from pbr-book 3rd edition Pharr, Jakob
  float tan2Half = getTan2Theta(normal, halfwayDir);
  float rough2 = roughness * roughness;
  float expTerm = -1 * tan2Half / rough2;
  expTerm = exp(expTerm);
  float cos2theta = getCos2Theta(normal, halfwayDir);
  float denominator = cos2theta * cos2theta * rough2 * PI;
  return expTerm / denominator;
}

float bsNormalDistAnisotropic(vec3 normal, vec3 halfwayDir, vec2 roughness) {
  // anisotropic version of beckmann spizzichino
  // distribution function
  // taken from pbr-book 3rd edition Pharr, Jakob
  float cos2phi = getCos2Phi(normal, halfwayDir);
  float sin2phi = getSin2Phi(normal, halfwayDir);
  float rx = roughness.x;
  float ry = roughness.y;
  float costerm = cos2phi / rx * rx;
  float sinterm = cos2phi / ry * ry;
  float tan2Half = getTan2Theta(normal, halfwayDir);
  float expterm = -1 * tan2Half * (costerm + sinterm);
  expterm = exp(expterm);
  float cos2theta = getCos2Theta(normal, halfwayDir);
  float denominator = cos2theta * cos2theta * rx * ry * PI;
  return expterm / denominator;
}

float trowReitzTraditional(vec3 normal, vec3 halfwayDir, float roughness) {
  // taken from https://learnopengl.com/PBR/Theory
  float rough2 = roughness * roughness;
  float cos2theta = getCos2Theta(normal, halfwayDir);
  float denom = PI * pow((cos2theta * (rough2 - 1) + 1),2);
  return rough2 / denom;
}
float trowReitzAnisotropic(vec3 normal, vec3 halfwayDir, vec2 roughness) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  float cos2phi = getCos2Phi(normal, halfwayDir);
  float sin2phi = getSin2Phi(normal, halfwayDir);
  float rx = roughness.x;
  float ry = roughness.y;
  float costerm = cos2phi / rx * rx;
  float sinterm = cos2phi / ry * ry;
  float tan2Half = getTan2Theta(normal, halfwayDir);
  float tanterm = (costerm + sinterm) * tan2Half;
  tanterm += 1;
  tanterm *= tanterm;
  float cos2theta = getCos2Theta(normal, halfwayDir);
  cos2theta *= cos2theta;
  return 1 / (cos2theta * rx * ry * PI * tanterm);
}
float getAlpha(vec3 normal, vec3 halfDir, vec2 roughness) {
  // compute alpha from halfway vector and surface normal
  float rx = roughness.x;
  float ry = roughness.y;
  float cos2phi = getCos2Phi(normal, halfDir);
  float sin2phi = getSin2Phi(normal, halfDir);
  return sqrt((cos2phi * rx * rx) + (sin2phi * ry * ry));
}

float bsLambdaFn(vec3 normal, vec3 halfDir, float alpha) {
  // lambda for anisoptric Beckmann Spizzichino Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  float tantheta = getTanTheta(normal, halfDir);
  tantheta = abs(tantheta);
  float aAlpha = 1 / (alpha * tantheta);
  float t2 = aAlpha * aAlpha * 0.396;
  float t1 = 1 - (aAlpha * 1.259);
  t1 += t2;
  float t3 = 3.535 * aAlpha;
  float t4 = 2.181 * aAlpha * aAlpha;
  t3 += t4;
  return t1 / t3;
}

float bsLamdaAFn(vec3 normal, vec3 halfDir, vec2 roughness) {
  // lambda for anisoptric Beckmann Spizzichino Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  float alpha = getAlpha(normal, halfDir, roughness);
  return bsLambdaFn(normal, halfDir, alpha);
}
void bsLamdaAFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, vec2 roughness,
                  float lambdaArr[2]) {
  lambdaArr[0] = bsLamdaAFn(normal, halfDir, roughness);
  lambdaArr[1] = bsLamdaAFn(normal, viewDir, roughness);
}

float bsLamdaTFn(vec3 normal, vec3 halfDir, float roughness) {
  // lambda for traditional Beckmann Spizzichino Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  float alpha = roughnessToAlpha(roughness);
  return bsLambdaFn(normal, halfDir, alpha);
}

void bsLamdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                  float lambdaArr[2]) {
  lambdaArr[0] = bsLamdaTFn(normal, halfDir, roughness);
  lambdaArr[1] = bsLamdaTFn(normal, viewDir, roughness);
}
float trowReitzLambda(vec3 normal, vec3 halfDir, float alpha) {
  // lambda for traditional Trowbridge Reitz Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // \frac{-1 + \sqrt{1 + {\alpha}^2 * tan^2(\theta)} }{2}
  float tan2theta = getTan2Theta(normal, halfDir);
  float alphaTan = (alpha * alpha) * tan2theta;
  float t1 = sqrt(1 + alphaTan);
  t1 += -1;
  return t1 / 2;
}

float trowReitzLambdaA(vec3 normal, vec3 halfDir, vec2 roughness) {
  // lambda for traditional Trowbridge Reitz Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // \frac{-1 + \sqrt{1 + {\alpha}^2 * tan^2(\theta)} }{2}
  float alpha = getAlpha(normal, halfDir, roughness);
  return trowReitzLambda(normal, halfDir, alpha);
}
void trowReitzLambdaAIO(vec3 normal, vec3 halfDir, vec3 viewDir, vec2 roughness,
                        float lambdaArr[2]) {
  lambdaArr[0] = trowReitzLambdaA(normal, halfDir, roughness);
  lambdaArr[1] = trowReitzLambdaA(normal, viewDir, roughness);
}

float trowReitzLambdaT(vec3 normal, vec3 halfway, float roughness) {
  // lambda for traditional Trowbridge Reitz Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // \frac{-1 + \sqrt{1 + {\alpha}^2 * tan^2(\theta)} }{2}
  float alpha = roughnessToAlpha(roughness);
  return trowReitzLambda(normal, halfway, alpha);
}
void trowReitzLambdaTIO(vec3 normal, vec3 halfDir, vec3 viewDir,
                        float roughness, float lambdaArr[2]) {
  lambdaArr[0] = trowReitzLambdaT(normal, halfDir, roughness);
  lambdaArr[1] = trowReitzLambdaT(normal, viewDir, roughness);
}

float geometryDistributionIn(float lambda) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return 1 / (1 + lambda);
}

float geometryInOut(float lambdaIn, float lambdaOut) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return 1 / (1 + lambdaIn + lambdaOut);
}

vec3 getFresnelSchlick(float costheta, vec3 refAtZero) {
  // taken from https://learnopengl.com/PBR/Lighting
  //
  return refAtZero + (1.0 - refAtZero) * pow(1.0 - costheta, 5.0);
}
