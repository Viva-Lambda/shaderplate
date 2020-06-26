#version 330 core
// fragment shader for pbr

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 TbnFragPos;

// texture related
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D aoMap;
uniform sampler2D roughnessMap;

// light pos
in vec3 TbnLightPos;
in vec3 TbnViewPos;

// light position and color
uniform vec3 lightColor;

// normal choice,
// uniform int ndfChoice;
// uniform int geoChoice;
// uniform bool fresnelChoice;
// uniform bool hdrChoice;

// reflection at zero incidince
uniform vec3 refAtZero = vec3(0.04);

// pi value
const float PI = 3.14159265;

// utility functions
float computeAttenuation(vec3 att, float lightToFragDist) {
  float distSqr = lightToFragDist * lightToFragDist;
  float att1 = lightToFragDist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  result = 1 / result;
  float attenuation = min(result, 1);
  return attenuation;
}

float getCosTheta(vec3 normal, vec3 inLightDir) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return dot(normal, inLightDir);
}
float getPositiveCosTheta(vec3 normal, vec3 inLightDir) {
  return max(getCosTheta(normal, inLightDir), 0.0);
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
float getPiCoeff(float alpha) { return 1 / PI * pow(alpha, 2); }
// texture functions
vec3 getAlbedo() {
  // get albedo texture
  return texture(albedoMap, TexCoord).rgb;
}
vec3 getSurfaceNormal() {
  // get surface normal
  return normalize(texture(normalMap, TexCoord).rgb * 2 - 1.0);
}
vec3 getViewDir() { return normalize(TbnViewPos - TbnFragPos); }
vec3 getLightDir() { return normalize(TbnLightPos - TbnFragPos); }
float getFragLightDist() { return length(TbnLightPos - TbnFragPos); }
// Functions are taken from
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// Fresnel function
vec3 getFresnelSchlick(float costheta, vec3 refAtZero) {
  // taken from https://learnopengl.com/PBR/Lighting
  //
  return refAtZero + (1.0 - refAtZero) * pow(1.0 - costheta, 5.0);
}

// beckamn Spizzichino Distribution related
float bsNormalDistTraditional(vec3 normal, vec3 halfwayDir, float roughness);
float bsLambdaFn(vec3 normal, vec3 halfDir, float alpha);
// traditional Beckmann Spizzichino Distribution
float bsLamdaTFn(vec3 normal, vec3 halfway, float roughness);
// lambda for traditional Beckmann Spizzichino Distribution
void bsLamdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                  out float lambdaArr[2]);

//

float geometryDistributionIn(float lambda);
float geometryInOut(float lambdaIn, float lambdaOut);

void main() {
  // main func for pbr
  vec3 viewDir = getViewDir();
  vec3 surfaceNormal = getSurfaceNormal();

  // get albedo
  vec3 albedo = getAlbedo();

  // get metallic
  vec3 metallic = texture(metallicMap, TexCoord).rgb;

  // get ao map
  vec3 ao = texture(aoMap, TexCoord).rgb;

  // get roughness map
  vec3 rough = texture(roughnessMap, TexCoord).rgb;

  // lightout
  vec3 L_out = vec3(0.0);

  // for each light source
  vec3 lightDir = getLightDir();
  vec3 halfDir = normalize(viewDir + lightDir);

  vec3 attc = vec3(1.0, 0.0, 0.0);

  // ambient color
  vec3 ambient = 0.01 * albedo * ao;

  vec3 refAtZeroN = mix(refAtZero, albedo, metallic.r);
  // refAtZero = mix(metallic, albedo, 0.5);
  float hcostheta = getCosTheta(surfaceNormal, halfDir);
  float fresnelCostheta = getCosTheta(halfDir, viewDir);

  vec3 fresnel = getFresnelSchlick(fresnelCostheta, refAtZeroN);
  float dN = bsNormalDistTraditional(surfaceNormal, halfDir, rough.r);
  float lambdaArr[2];
  bsLamdaTFnIO(surfaceNormal, halfDir, viewDir, rough.r, lambdaArr);
  float lambdaIn = lambdaArr[0];
  float lambdaOut = lambdaArr[1];
  float gD = geometryInOut(lambdaIn, lambdaOut);

  vec3 ks = fresnel;
  vec3 kd = vec3(1.0) - ks;
  kd = kd * (1.0 - metallic.x);
  vec3 t1 = dN * gD * fresnel;
  float outDir = getPositiveCosTheta(surfaceNormal, viewDir);
  float inDir = getPositiveCosTheta(surfaceNormal, lightDir);
  float t2 = 4.0 * outDir * inDir;
  vec3 specular = t1 / max(t2, 0.0001);

  L_out = (kd * albedo / PI + specular) * lightColor * inDir;

  L_out += ambient;
  L_out = L_out / (L_out + vec3(1.0));
  L_out = pow(L_out, vec3(1.0 / 2.2));

  // FragColor = vec4(hcostheta * refAtZero, 1.0);
  FragColor = vec4(L_out, 1.0);
}

vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
}

// traditional version of beckmann spizzichino
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
float bsLamdaTFn(vec3 normal, vec3 halfDir, float roughness) {
  // lambda for traditional Beckmann Spizzichino Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  float alpha = roughnessToAlpha(roughness);
  return bsLambdaFn(normal, halfDir, alpha);
}
void bsLamdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                  out float lambdaArr[2]) {
  lambdaArr[0] = bsLamdaTFn(normal, halfDir, roughness);
  lambdaArr[1] = bsLamdaTFn(normal, viewDir, roughness);
}
float geometryInOut(float lambdaIn, float lambdaOut) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return 1 / (1 + lambdaIn + lambdaOut);
}
