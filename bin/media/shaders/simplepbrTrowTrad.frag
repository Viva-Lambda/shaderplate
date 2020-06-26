#version 330 core
// fragment shader for pbr

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// texture related
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D aoMap;
//uniform sampler2D roughnessMap;

// light pos
uniform vec3 lightPos;
uniform vec3 viewPos;

// pi value
const float PI = 3.14159265;

// utility functions
vec3 getSurfaceNormal();
vec3 getLightDir();
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color);
float getCosTheta(vec3 normal, vec3 inLightDir);
float getPositiveCosTheta(vec3 normal, vec3 inLightDir);
float getCos2Theta(vec3 normal, vec3 inLightDir);
float getSin2Theta(vec3 normal, vec3 lightDir);
float getTanTheta(vec3 normal, vec3 lightDir);
float getTan2Theta(vec3 normal, vec3 lightDir);
float roughnessToAlpha(float roughness);

vec3 getAlbedo();
vec3 getFresnelSchlick(float costheta, vec3 refAtZero);

// Trowbridge Reitz Distribution related
float ndfFn(vec3 normal, vec3 halfwayDir, float roughness);
float lambdaFn(vec3 normal, vec3 halfDir, float alpha);
// traditional Trowbridge Reitz Distribution lambda
float lambdaTFn(vec3 normal, vec3 halfway, float roughness);
// lambda for traditional Trowbridge Reitz Distribution
void lambdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                 float lambdaArr[2]);

//

float geometryDistributionIn(float lambda);
float geometryInOut(float lambdaIn, float lambdaOut);

void main() {
  // main func for pbr
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 surfaceNormal = getSurfaceNormal();

  // get albedo
  vec3 albedo = getAlbedo();

  // get metallic
  vec3 metallic = texture(metallicMap, TexCoord).rgb;

  // get ao map
  vec3 ao = texture(aoMap, TexCoord).rgb;

  // get roughness map
  //vec3 rough = texture(roughnessMap, TexCoord).rgb;
  vec3 rough = vec3(0.1);

  // lightout
  vec3 L_out = vec3(0.0);

  // for each light source
  vec3 lightDir = getLightDir();
  vec3 halfDir = normalize(viewDir + lightDir);

  vec3 attc = vec3(1.0, 0.0, 0.0);

  // ambient color
  vec3 ambient = 0.01 * albedo * ao;

  vec3 refAtZero = vec3(0.04);
  refAtZero = mix(refAtZero, albedo, metallic.r);
  // refAtZero = mix(metallic, albedo, 0.5);
  float hcostheta = getCosTheta(surfaceNormal, halfDir);
  float fresnelCostheta = getCosTheta(halfDir, viewDir);

  vec3 fresnel = getFresnelSchlick(fresnelCostheta, refAtZero);
  float dN = ndfFn(surfaceNormal, halfDir, rough.r);
  float lambdaArr[2];
  lambdaTFnIO(surfaceNormal, halfDir, viewDir, rough.r, lambdaArr);
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

  L_out = (kd * albedo / PI + specular) * 1.0f * inDir;

  L_out += ambient;
  L_out = L_out / (L_out + vec3(1.0));
  L_out = pow(L_out, vec3(1.0 / 2.2));

  // FragColor = vec4(hcostheta * refAtZero, 1.0);
  FragColor = vec4(L_out, 1.0);
}

vec3 getLightDir() { return normalize(lightPos - FragPos); }
vec3 getSurfaceNormal() {
  vec3 normal = normalize(texture(normalMap, TexCoord).rgb * 2 - 1.0);
  vec3 q1 = dFdx(FragPos);
  vec3 q2 = dFdy(FragPos);
  vec2 tex1 = dFdx(TexCoord);
  vec2 tex2 = dFdy(TexCoord);
  vec3 N = normalize(Normal);
  vec3 T = normalize(q1 * tex2.t - q2 * tex1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);
  return normalize(TBN * normal);
}
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
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

vec3 getAlbedo() {
  vec3 albedo = pow(texture(albedoMap, TexCoord).rgb, vec3(2.2));
  return albedo;
}
vec3 getFresnelSchlick(float costheta, vec3 refAtZero) {
  // taken from https://learnopengl.com/PBR/Lighting
  //
  return refAtZero + (1.0 - refAtZero) * pow(1.0 - costheta, 5.0);
}
// traditional version of Trowbridge Reitz
float lambdaFn(vec3 normal, vec3 halfDir, float alpha) {
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // \frac{-1 + \sqrt{1 + {\alpha}^2 * tan^2(\theta)} }{2}
  float tan2theta = getTan2Theta(normal, halfDir);
  float alphaTan = (alpha * alpha) * tan2theta;
  float t1 = sqrt(1 + alphaTan);
  t1 += -1;
  return t1 / 2;
}

float ndfFn(vec3 normal, vec3 halfwayDir, float roughness) {
  // Trowbridge Reitz Distribution related
  // distribution function
  // NDF(n, h, r) = \frac{\alpha^2}{\pi ((h \dot n)^2 (\alpha^2 - 1) + 1)^2 }
  // taken from
  // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
  float alpha = roughnessToAlpha(roughness);
  float alphasqr = alpha * alpha;
  float costheta2 = getCos2Theta(normal, halfwayDir);
  float mult = costheta2 * (alphasqr - 1);
  mult += 1;
  float denominator = mult * mult * PI;
  return alphasqr / denominator;
}
float lambdaTFn(vec3 normal, vec3 halfDir, float roughness) {
  // partly taken from
  float alpha = roughnessToAlpha(roughness);
  return lambdaFn(normal, halfDir, alpha);
}
void lambdaTFnIO(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness,
                 float lambdaArr[2]) {
  lambdaArr[0] = lambdaTFn(normal, halfDir, roughness);
  lambdaArr[1] = lambdaTFn(normal, viewDir, roughness);
}

float geometryInOut(float lambdaIn, float lambdaOut) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return 1 / (1 + lambdaIn + lambdaOut);
}
