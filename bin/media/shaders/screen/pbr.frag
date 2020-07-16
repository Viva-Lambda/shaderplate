#version 430
layout(location = 0) out vec3 FragColor;
// out vec3 FragColor;

in vec2 TexCoord;
in vec3 ViewRay;

// material parameters
layout(binding = 0) uniform sampler2D gPosition; // from GBuffer
layout(binding = 1) uniform sampler2D gNormal;   // from GBuffer
layout(binding = 2) uniform sampler2D gAlbedo;   // from GBuffer
layout(binding = 3) uniform sampler2D gMaterial; // from GBuffer
layout(binding = 4) uniform sampler2D gAmbient;  // from GBuffer

// lights
uniform vec3 LightPosVS; // in view space
uniform vec3 lightColor;

uniform vec3 ViewPosVS; // in world space

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------

/**
 * from Möller Real time 2018 p. 338
 * */
float chiPlus(float x) {
  if (x <= 0) {
    return 0;
  } else {
    return 1;
  }
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

/**
 * Compute outgoing angle using snell's law ni \theta_i = no \theta_t
 * given the refractive index
 * */
float outAngle(float ni, float no, vec3 surfaceNormal, vec3 inComingRay) {
  float theta_i = acos(normalize(dot(inComingRay, surfaceNormal)));
  return asin(ni * sin(theta_i) / no);
}
vec3 outVector(float ni, float no, vec3 surfaceNormal, vec3 inComingRay) {
  vec3 i = -normalize(inComingRay);
  vec3 n = normalize(surfaceNormal);
  float costheta = dot(n, i);
  float eta = ni / no;
  vec3 t1 = eta * i;
  vec3 t2 =
      n * (eta * costheta - sqrt(1 - pow(eta, 2) * (1 - pow(costheta, 2))));
  return t1 + t2;
}

/**
 * Compute Geometric attenuation using lambda functions mostly taken
 * from pbrt
 * */

float lambdaPbrt(vec3 normal, vec3 dir, float roughness) {
  // partly taken from
  // taken from pbr-book 3rd edition Pharr, Jakob
  // p. 543 trowbridge reitz
  float alpha = roughnessToAlpha(roughness);
  float tan2theta = getTan2Theta(normal, dir);
  float alphaTan = (alpha * alpha) * tan2theta;
  float t1 = sqrt(1 + alphaTan);
  t1 += -1;
  return t1 / 2;
}

float computeGeometricPbrt(vec3 inDir, vec3 outDir, vec3 normal,
                           float roughness) {
  return 1.0 / (1 + lambdaPbrt(normal, inDir, roughness) +
                lambdaPbrt(normal, outDir, roughness));
}
float computeTrowbridgeReitzD(vec3 normal, vec3 halfDir, float roughness) {
  // traditional version of Trowbridge Reitz distribution
  float alpha2 = pow(roughnessToAlpha(roughness), 2);
  float cos2theta = getCos2Theta(normal, halfDir);
  float nominator = chiPlus(dot(halfDir, normal)) * alpha2;
  float costheta4 = pow(dot(halfDir, normal), 4);
  float tantheta2 = pow(getTanTheta(normal, halfDir), 2);
  tantheta2 += alpha2;
  tantheta2 *= tantheta2;
  float denominator = PI * costheta4 * tantheta2;
  return nominator / denominator;
}
// ----------------------------------------------------------------------------
/**
 * p. 16, of Cook Torrance 1982
 * f0 means that everything is surrounded by air
 * */
float computeFresnelCT(float VdotH) {
  float n = texture(gMaterial, TexCoord).w;
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

/**
 * Equation in page 10 I_{ra} = R_aI_{ia}f where f = 1/pi \int_i (N \dot L)dw_i
 * */
vec3 computeAmbientCT(float NdotL) {
  vec3 ambient = texture(gAmbient, TexCoord).rgb;
  return ambient * NdotL / PI;
}
/**
 * Geometrical attenuation equation page 11
 * */
float computeGeometryCT(float NdotL, float NdotV, float VdotH, float NdotH) {
  //
  float g1 = 2 * NdotH * NdotV / VdotH;
  float g2 = 2 * NdotH * NdotL / VdotH;
  float gmin = min(g1, g2);
  return min(1.0, gmin);
}
/**
 * Geometrical distribution the microfacet normal is considered as half vector
 * in conformance with p. 337 Möller 2018 Real time
 * beckmann distribution from equation 9.35
 * */
float computeDCT(float NdotH) {
  float nominator = chiPlus(NdotH);
  float roughness = texture(gMaterial, TexCoord).y;
  float alpha2 = roughness * roughness;
  float NdotH2 = pow(NdotH, 2);
  float nomInExp = NdotH2 - 1;
  nomInExp /= alpha2 * NdotH2;
  return (nominator / (PI * alpha2 * pow(NdotH, 4))) * exp(nomInExp);
}
/**
 * Equation 9.34 p. 337 Möller Realtime
 * */
float computeFSpecCT(float NdotH, float NdotL, float NdotV, float VdotH) {
  float F = computeFresnelCT(VdotH);
  float G = computeGeometryCT(NdotL, NdotV, VdotH, NdotH);
  float D = computeDCT(NdotH);
  return F * G * D / (4 * NdotL * NdotV);
}
float computeFSpecTrowReitz(vec3 lightDir, vec3 viewDir, vec3 normal) {
  vec3 halfdir = normalize(lightDir + viewDir);
  float roughness = texture(gMaterial, TexCoord).y;
  float G = computeGeometricPbrt(lightDir, viewDir, normal, roughness);
  float D = computeTrowbridgeReitzD(normal, halfdir, roughness);
  float F = computeFresnelCT(dot(viewDir, halfdir));
  return F * G * D / (4 * dot(normal, lightDir) * dot(normal, viewDir));
}
/**
 * Equation 16 from Walter et.al 2007 Microfacet models ...
 * h_t = \frac{\vec{h_t}}{|h_t|} where \vec{h_t} = -(n_i * i + n_o * o)
 * i is the incoming ray direction n_i is the refractive index of the incoming
 * ray's medium, and n_o or n_t is the transmitted medium and o is the
 * transmitted direction
 * */
vec3 computeHt(float ni, float nt, vec3 inDir, vec3 normal) {
  vec3 outDir = outVector(ni, nt, normal, inDir);
  vec3 ht = -(ni * inDir + nt * outDir);
  return normalize(ht);
}

/**
 * Equation 9.68 p. 355 and its parts from Möller Realtime
 * */
float f_multi(float alpha) {
  float roughness = alpha; // should be specular
  // roughness
  return 0.3641 * roughness;
}
float f_kf(float LdotV) { return 0.5 + 0.5 * LdotV; }
float f_rough(float NdotH, float LdotV) {
  float k_f = f_kf(LdotV);
  return ((0.5 + NdotH) / NdotH) * (k_f * (0.9 - 0.4 * k_f));
}
float f_smooth(float NdotL, float NdotV) {
  float n = texture(gMaterial, TexCoord).w;
  float f0 = pow((n - 1.0) / (n + 1.0), 2);
  float t1 = (21 / 20) * (1 - f0);
  float t2 = 1 - pow(1 - NdotL, 5);
  float t3 = 1 - pow(1 - NdotV, 5);
  return t1 * t2 * t3;
}
vec3 computeHammonFDiff(float NdotL, float NdotV, float NdotH, float LdotV) {
  float t1 = chiPlus(NdotL);
  float t2 = chiPlus(NdotV);
  vec3 rho_ss = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  vec3 unitAlbedo = rho_ss / PI;
  float alpha = roughnessToAlpha(texture(gMaterial, TexCoord).y); // roughness
  float fsmooth = f_smooth(NdotL, NdotV);
  fsmooth *= (1 - alpha);
  float frough = f_rough(NdotH, LdotV);
  frough *= alpha;
  float fmulti = f_multi(alpha);
  vec3 fmultiv = fmulti * rho_ss;
  return t1 * t2 * unitAlbedo * (fsmooth + frough + fmulti);
}

/**
 * Equation 9.63 p. 351 from Möller Realtime
 * */
vec3 computeFDiffCT(float VdotH) {
  vec3 albedo = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  return (albedo / PI) * (1 - computeFresnelCT(VdotH));
}

/**
 * Equation 9.69 from p. 355 from Möller Realtime
 * */

/**
 *
 *
 * f_s(i,o,m) = f_r(i,o,m) + f_t(i,o,m)
 * BSDF of Cook Torrance from Walter et. al. 2007, equation 19-21
 * */
vec3 CTBrdfColor(float s, vec3 lightDir, vec3 viewDir, vec3 normal, float NdotL,
                 vec3 lightColor) {
  float d = 1.0 - s;
  vec3 H = normalize(viewDir + lightDir);
  float NdotV = max(dot(normal, viewDir), 0);
  float NdotH = max(dot(normal, H), 0);
  float VdotH = max(dot(viewDir, H), 0);
  float R_s = computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
  vec3 R_d = computeFDiffCT(VdotH);
  return ((d * R_d) + (s * R_s)) * lightColor * max(NdotL, 0);
}
vec3 computeCTColor(float s, vec3 lightDir, vec3 viewDir, vec3 normal,
                    vec3 lightColor, float attenuation) {
  float NdotL = dot(normal, lightDir);
  vec3 ambient = computeAmbientCT(NdotL);
  return ambient + CTBrdfColor(s, lightDir, viewDir, normal, NdotL, lightColor);
}

/**
 * Basic brdf model using hammon f_diff and cook torrance f_spec
 * so that f_brdf = f_diff + f_spec
 * */
vec3 computeSolidBrdf(vec3 viewDir, vec3 lightDir, vec3 normal,
                      vec3 lightColor) {
  vec3 H = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H);
  float NdotL = dot(normal, lightDir);
  float VdotH = dot(viewDir, H);
  float LdotV = dot(lightDir, viewDir);

  return (computeFDiffCT(VdotH) +
          // computeHammonFDiff(NdotL, NdotV, NdotH, LdotV) + vec3(1000)
          computeFSpecTrowReitz(lightDir, viewDir, normal)) *
         lightColor * max(NdotL, 0);
}

/**
 * Equation 21 from Walter et. al. 2007
 * */
float computeFTrans(vec3 inDir, vec3 outDir, vec3 normal, float ni, float nt,
                    float roughness) {
  vec3 H_t = computeHt(ni, nt, inDir, normal);
  float LdotHt = dot(inDir, H_t);
  float VdotHt = dot(outDir, H_t);
  float LdotN = dot(inDir, normal);
  float VdotN = dot(outDir, normal);
  float n2 = pow(nt, 2);
  float nominator = n2;
  nominator *= (1 - computeFresnelCT(LdotHt));
  nominator *= computeGeometricPbrt(inDir, outDir, normal, roughness);
  nominator *= computeTrowbridgeReitzD(normal, H_t, roughness);
  float denominator = ni * LdotHt + VdotHt * nt;
  denominator *= denominator;
  return (LdotHt * VdotHt) / (LdotN * VdotN) * (nominator / denominator);
}

/**
 * Equation 19 from Walter et. al. Microfacet models ... 2007
 * */
vec3 computeBSDF(vec3 viewDir, vec3 lightDir, vec3 normal, float ni, float nt,
                 float roughness) {
  vec3 H_r = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H_r);
  float NdotL = dot(normal, lightDir);
  float VdotH = dot(viewDir, H_r);
  float LdotV = dot(lightDir, viewDir);

  float f_r = computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
  float f_t = computeFTrans(lightDir, viewDir, normal, ni, nt, roughness);
  return f_r + f_t;
}
/**
 * A combination of computeBSDF + hammonDiffuse
 * */
vec3 computeLocalColor(vec3 viewDir, vec3 lightDir, vec3 normal, float ni,
                       float nt, float roughness) {

  vec3 H = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H);
  float NdotL = dot(normal, lightDir);
  float VdotH = dot(viewDir, H);
  float LdotV = dot(lightDir, viewDir);
  vec3 f_d = computeHammonFDiff(NdotL, NdotV, NdotH, LdotV);
  float f_spec = computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
  float f_t = computeFTrans(lightDir, viewDir, normal, ni, nt, roughness);
  return f_d + f_spec + f_t;
}
vec3 computeLocalColor(vec3 viewDir, vec3 lightDir, vec3 normal, float nt,
                       float roughness) {
  float ni = 1.0; // assuming air as the incoming ray medium
  return computeLocalColor(viewDir, lightDir, normal, ni, nt, roughness);
}
vec3 computeLocalColor(vec3 viewDir, vec3 lightDir, vec3 normal,
                       float roughness) {
  float ni = 1.0;  // assuming air as the incoming ray medium
  float nt = 0.04; // a value true for most non metal surfaces
  return computeLocalColor(viewDir, lightDir, normal, ni, nt, roughness);
}

// ----------------------------------------------------------------------------
vec3 getLightDir(vec3 FragPosVS) { return normalize(FragPosVS - LightPosVS); }
vec3 getViewDir(vec3 FragPosVS) { return normalize(FragPosVS - ViewPosVS); }

void main() {
  vec3 FragPosVS = texture(gPosition, TexCoord).rgb;

  // material properties
  vec3 albedo = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  float metallic = texture(gMaterial, TexCoord).x;
  float roughness = texture(gMaterial, TexCoord).y;
  float ao = texture(gMaterial, TexCoord).z;
  ao = ao == 0.0 ? 1.0 : ao;
  float nt = texture(gMaterial, TexCoord).w;

  // input lighting data
  vec3 NormalVS = texture(gNormal, TexCoord).xyz;
  vec3 viewDir = getViewDir(FragPosVS);
  float s = 1 - roughness;
  float attenuation = 1.0 / pow(length(LightPosVS - FragPosVS), 2);

  // for each light source
  vec3 lightDir = getLightDir(FragPosVS);
  float NdotL = dot(NormalVS, lightDir);
  vec3 ambient = computeAmbientCT(NdotL);

  vec3 color = CTBrdfColor(s, lightDir, viewDir, NormalVS, NdotL, lightColor);
  color *= ao;
  color *= attenuation;
  color += ambient;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  FragColor = pow(color, vec3(1.0 / 2.2));
  // FragColor = vec3(0.6);
}
