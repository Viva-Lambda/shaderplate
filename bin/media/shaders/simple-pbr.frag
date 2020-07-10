#version 330 core
// fragment shader for pbr
//
out vec4 FragColor;

in vec3 FragPos;  // frag position in world
in vec3 Normal;   // surface normal
in vec2 TexCoord; // texture coordinate

// texture related
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D aoMap;
uniform sampler2D roughnessMap;

// light position and color
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// normal choice,
uniform int ndfChoice;
uniform int geoChoice;
uniform bool fresnelChoice;
uniform bool hdrChoice;

// reflection at zero incidince
uniform float refAtZero = 0.04;

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
  return getTanTheta(normal, inLightDir) * getTanTheta(normal, inLightDir);
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
  vec3 albedo = pow(texture(albedoMap, TexCoord).rgb, vec3(2.2));
  return albedo;
}
vec3 getSurfaceNormal() {
  // get surface normal
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
vec3 getLightDir() { return normalize(lightPos - FragPos); }
// Functions are taken from
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// Fresnel function
vec3 getFresnelSchlick(vec3 viewDir, vec3 halfDir, vec3 refAtZero) {
  // FSchlick(v,h)=F0+(1−F0)(1−(v⋅h))^5
  float costheta = getCosTheta(halfDir, viewDir);
  return refAtZero + (1.0 - refAtZero) * pow(1.0 - costheta, 5.0);
}
vec3 getFresnelCookTorrance(vec3 viewDir, vec3 halfDir, vec3 refAtZero) {
  // Fresnel Cook torrance version
  vec3 sqrRef = sqrt(refAtZero);
  vec3 eta = (1 + sqrRef) / (1 - sqrRef);
  float cos2theta = getCos2Theta(halfDir, viewDir);
  float costheta = getCosTheta(halfDir, viewDir);
  vec3 gterm = sqrt(eta * eta + cos2theta - 1);
  vec3 gcdiff = gterm - costheta;
  vec3 gcsum = gterm + costheta;
  vec3 secondTerm = (gcdiff / gcsum) * (gcdiff / gcsum);
  vec3 thirdTerm1 = (gcsum * costheta) * costheta - 1;
  vec3 thirdTerm2 = (gcdiff * costheta) * costheta + 1;
  vec3 thirdTerm = (thirdTerm1 / thirdTerm2) * (thirdTerm1 / thirdTerm2) + 1;
  return 1 / 2 * secondTerm * thirdTerm;
}
// normal distribution functions
float blinnPhongD(vec3 normal, vec3 halfDir, float roughness) {
  // blinn phong distribution function
  // 1/2 * (n \dot h)^{\frac{2}{\alpha^2} - 2}
  float alpha = roughnessToAlpha(roughness);
  float coeff = getPiCoeff(alpha);
  float costheta = getCosTheta(normal, halfDir);
  return coeff * pow(costheta, (2 / pow(alpha, 2) - 2));
}
float beckmannSpizD(vec3 normal, vec3 halfDir, float roughness) {
  // traditional version of beckmann spizzichino
  float alpha = roughnessToAlpha(roughness);
  float coeff = getPiCoeff(alpha);
  float cos2theta = getCos2Theta(normal, halfDir);
  float expterm = exp((cos2theta - 1) / pow(alpha, 2) * cos2theta);
  return coeff * 1 / pow(cos2theta, 2) * expterm;
}
float trowbridgeReitzD(vec3 normal, vec3 halfDir, float roughness) {
  // traditional version of Trowbridge Reitz distribution
  float alpha2 = pow(roughnessToAlpha(roughness), 2);
  float cos2theta = getCos2Theta(normal, halfDir);
  return alpha2 / PI * pow(cos2theta * (alpha2 - 1) + 1, 2);
}
// geometric shadowing
float implicitG(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
                float roughness) {
  float nl = getCosTheta(normal, lightDir);
  float nv = getCosTheta(normal, viewDir);
  return nl * nv;
}
float neumannG(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
               float roughness) {
  // neumann geometric shadowing
  float nl = getCosTheta(normal, lightDir);
  float nv = getCosTheta(normal, viewDir);
  return nl * nv / max(nl, nv);
}
float cookTorranceG(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
                    float roughness) {
  // cook torrance geometric shadowing
  float nl = getCosTheta(normal, lightDir);
  float nv = getCosTheta(normal, viewDir);
  float nh = getCosTheta(normal, halfDir);
  float vh = getCosTheta(viewDir, halfDir);
  float cmp1 = min((2 * nh * nv) / vh, (2 * nh * nl) / vh);
  return min(1.0, cmp1);
}
float kelemanG(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
               float roughness) {
  // kelemen geometric shadowing
  float nl = getCosTheta(normal, lightDir);
  float nv = getCosTheta(normal, viewDir);
  float vh2 = getCos2Theta(viewDir, halfDir);
  return nl * nv / vh2;
}
float beckmannGSDir(vec3 dir, vec3 normal, float roughness) {
  // beckmann geometric
  float alpha = roughnessToAlpha(roughness);
  float nv = getCosTheta(normal, dir);
  float nv2 = getCos2Theta(normal, dir);
  float c = nv / alpha * sqrt(1 - nv2);
  if (c >= 1.6) {
    return 1.0;
  }
  float t1 = 3.535 * c + 2.181 * pow(c, 2);
  float t2 = 1 + 2.276 * c + 2.577 * pow(c, 2);
  return t1 / t2;
}
float beckmannGS(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
                 float roughness) {
  // geometric shadowing with smith's equation G_1(v)G_1(l)
  float lightG = beckmannGSDir(lightDir, normal, roughness);
  float viewG = beckmannGSDir(viewDir, normal, roughness);
  return lightG * viewG;
}
float ggxGSDir(vec3 dir, vec3 normal, float roughness) {
  // geometric shadowing
  float alpha = roughnessToAlpha(roughness);
  float alpha2 = pow(alpha, 2);
  float costheta = getCosTheta(normal, dir);
  float costheta2 = getCos2Theta(normal, dir);
  return 2 * costheta / (costheta + sqrt(alpha2 + (1 - alpha2) * costheta2));
}
float ggxGS(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
            float roughness) {
  // geometric shadowing with smith's equation G_1(v)G_1(l)
  float lightG = ggxGSDir(lightDir, normal, roughness);
  float viewG = ggxGSDir(viewDir, normal, roughness);
  return lightG * viewG;
}
float schlickBeckmannGSDir(vec3 dir, vec3 normal, float roughness) {
  // schlick approximation of beckmann
  float alpha = roughnessToAlpha(roughness);
  float k = alpha * sqrt(2 / PI);
  float nv = getCosTheta(normal, dir);
  return nv / (nv * (1 - k) + k);
}
float schlickBeckmannGS(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
                        float roughness) {
  // geometric shadowing with smith's equation G_1(v)G_1(l)
  float lightG = schlickBeckmannGSDir(lightDir, normal, roughness);
  float viewG = schlickBeckmannGSDir(viewDir, normal, roughness);
  return lightG * viewG;
}
float schlickGgxGSDir(vec3 dir, vec3 normal, float roughness) {
  // schlick approximation of beckmann
  float alpha = roughnessToAlpha(roughness);
  float k = alpha / 2;
  float nv = getCosTheta(normal, dir);
  return nv / (nv * (1 - k) + k);
}
float schlickGgxGS(vec3 lightDir, vec3 normal, vec3 halfDir, vec3 viewDir,
                   float roughness) {
  // geometric shadowing with smith's equation G_1(v)G_1(l)
  float lightG = schlickGgxGSDir(lightDir, normal, roughness);
  float viewG = schlickGgxGSDir(viewDir, normal, roughness);
  return lightG * viewG;
}
// PBR - Book implementations of functions
float geometryInOut(float lambdaIn, float lambdaOut) {
  // taken from pbr-book 3rd edition Pharr, Jakob
  return 1 / (1 + lambdaIn + lambdaOut);
}
float pbrBeckmannSpiD(vec3 normal, vec3 halfwayDir, float roughness) {
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
float pbrBeckmannSpizLambdaDir(vec3 normal, vec3 dir, float roughness) {
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  float alpha = roughnessToAlpha(roughness);
  float tantheta = getTanTheta(normal, dir);
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
float pbrBeckmannSpizGS(vec3 normal, vec3 halfDir, vec3 viewDir,
                        float roughness) {
  float lambdaIn = pbrBeckmannSpizLambdaDir(normal, halfDir, roughness);
  float lambdaOut = pbrBeckmannSpizLambdaDir(normal, viewDir, roughness);
  return geometryInOut(lambdaIn, lambdaOut);
}
float pbrTrowReitzLambdaDir(vec3 normal, vec3 dir, float roughness) {
  // lambda for traditional Trowbridge Reitz Distribution
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // \frac{-1 + \sqrt{1 + {\alpha}^2 * tan^2(\theta)} }{2}
  float alpha = roughnessToAlpha(roughness);
  float tan2theta = getTan2Theta(normal, dir);
  float alphaTan = (alpha * alpha) * tan2theta;
  float t1 = sqrt(1 + alphaTan);
  t1 += -1;
  return t1 / 2;
}
float pbrTrowReitzGS(vec3 normal, vec3 halfDir, vec3 viewDir, float roughness) {
  float lambdaIn = pbrTrowReitzLambdaDir(normal, halfDir, roughness);
  float lambdaOut = pbrTrowReitzLambdaDir(normal, viewDir, roughness);
  return geometryInOut(lambdaIn, lambdaOut);
}

void main() {
  // main func for pbr
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 surfaceNormal = getSurfaceNormal();

  // get albedo
  vec3 albedo = getAlbedo();

  // get metallic
  float metallic = texture(metallicMap, TexCoord).r;

  // get ao map
  float ao = texture(aoMap, TexCoord).r;

  // get roughness map
  float rough = texture(roughnessMap, TexCoord).r;

  // lightout
  vec3 L_out = vec3(0.0);

  // for each light source
  vec3 lightDir = getLightDir();
  vec3 halfDir = normalize(viewDir + lightDir);

  // ambient color
  vec3 ambient = 0.1 * albedo * ao;

  // attenuation
  vec3 attc = vec3(1.0, 0.0, 0.0);
  float lightToFragDist = length(lightPos - FragPos);
  float attenuation = computeAttenuation(attc, lightToFragDist);

  // radiance
  vec3 radiance = lightColor * attenuation;

  //
  refAtZero = mix(refAtZero, albedo, metallic); // x(1 - a) + a * y;
  vec3 F;
  float D;
  float G;

  if (fresnelChoice == true) {
    // schlick fresnel
    F = getFresnelSchlick(viewDir, halfDir, refAtZero);
  } else {
    // cook torrance fresnel
    F = getFresnelCookTorrance(viewDir, halfDir, refAtZero);
  }
  switch (ndfChoice) {
  case 0:
    // blinn phong normal distribution
    D = blinnPhongD(surfaceNormal, halfDir, rough);
    break;
  case 1:
    // beckmann spizzichino normal distribution
    D = beckmannSpizD(surfaceNormal, halfDir, rough);
    break;
  case 2:
    // Trowbridge Reitz normal distribution
    D = trowbridgeReitzD(surfaceNormal, halfDir, rough);
    break;
  case 3:
    // pbr beckmann spizzichino normal distribution
    D = pbrBeckmannSpiD(surfaceNormal, halfDir, rough);
    break;
  default:
    D = blinnPhongD(surfaceNormal, halfDir, rough);
    break;
  }
  switch (geoChoice) {
  case 0:
    // implicit geometry
    G = implicitG(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 1:
    // neumann geometry
    G = neumannG(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 2:
    // cook torrance geometry
    G = cookTorranceG(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 3:
    // kelemen geometry
    G = kelemanG(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 4:
    // beckmann spizzichino geometry shadow pbr
    G = beckmannGS(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 5:
    G = ggxGS(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 6:
    G = schlickBeckmannGS(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 7:
    G = schlickGgxGS(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  case 8:
    G = pbrBeckmannSpizGS(surfaceNormal, halfDir, viewDir, rough);
    break;
  case 9:
    G = pbrTrowReitzGS(surfaceNormal, halfDir, viewDir, rough);
    break;
  default:
    G = implicitG(lightDir, surfaceNormal, halfDir, viewDir, rough);
    break;
  }
  vec3 t1 = F * G * D;
  float outDir = getCosTheta(surfaceNormal, viewDir);
  float inDir = getCosTheta(surfaceNormal, lightDir);
  float t2 = 4.0 * outDir * inDir;
  vec3 specular = t1 / max(t2, 0.0001);
  vec3 c = albedo / PI;
  vec3 kd = 1 - specular;
  L_out = (kd * c + specular) * radiance * inDir;
  vec3 color = L_out + ambient;
  // hdr and gamma correction
  if (hdrChoice == true) {
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
  }
  FragColor = vec4(color, 1.0);
}
/*
 * Black Cube for layered-cliff
 *Current NDF Parameters
schlick fresnel equation
hdr correction
pbr-book Beckmann Spizzichino normal distribution
Pbr-book version of Beckmann Spizzichino geometric shadowing
 *
 ** Black Cube for layered-cliff
 *Current NDF Parameters
schlick fresnel equation
hdr correction
pbr-book Beckmann Spizzichino normal distribution
Pbr-book version of Beckmann Spizzichino geometric shadowing
 *

 *
 *
 * */
