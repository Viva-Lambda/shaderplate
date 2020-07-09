#version 330
// pbr related functions and their references
//
//
/**
 * Notation
 *
 * from E. Hecht 2017, Optics, p 121-122
 * E = electric field
 * E_r = reflected wave
 * E_t = transmitted wave
 * E_i = incident monochromatic lightwave
 *
 * Fresnel equations define a relation among these variables
 * p. 124, eq. 4.32 - 4.33
 *
 * For lightwaves that are perpendicular to the plane of incidence
 *
 * \frac{E_{0r}}{E_{0i}} =
 * \frac{\frac{n_i}{m_i}\cos(\theta_i) - \frac{n_t}{m_t}\cos(\theta_t)}
 * {\frac{n_i}{m_i}\cos(\theta_i) + \frac{n_t}{m_t}\cos(\theta_t)}
 *
 * \frac{E_{0t}}{E_{0i}} =
 * \frac{2 \frac{n_i}{m_i}\cos(\theta_i) }
 * {\frac{n_i}{m_i}\cos(\theta_i) + \frac{n_t}{m_t}\cos(\theta_t)}
 *
 * where m is magnetic effect of media m_i is for incident m_t for transmitted
 * media, n represents the refractive index n_i for incident, n_t for
 * transmitted.
 *
 * The first equation calculates amplitude reflection coefficient
 * The second equation calcluates amplitude transmission coefficient
 *
 * This is also called reflectance for s polarized light
 *
 * When magnetic effects of all participating media are more or less same. We
 * can use a simpler equation where we ignore m and use just refractive
 * indices
 *
 * For lightwaves whose electric field is parallel to the field of incidence
 * we can use the following equations
 *
 * \frac{E_{0r}}{E_{0i}} =
 * \frac{\frac{n_t}{m_t}\cos(\theta_i) - \frac{n_i}{m_i}\cos(\theta_t)}
 * {\frac{n_i}{m_i}\cos(\theta_t) + \frac{n_t}{m_t}\cos(\theta_i)}
 *
 *
 * \frac{E_{0t}}{E_{0i}} =
 * \frac{2\frac{n_i}{m_i}\cos(\theta_i) }
 * {\frac{n_i}{m_i}\cos(\theta_t) + \frac{n_t}{m_t}\cos(\theta_i)}
 *
 * For natural light reflectance is calculated as
 * R_eff = \frac{1}{2}(R_s + R_p)
 *
 * where R_s is reflectance for s polarized light
 * where R_p is reflectance for p polarized light
 *
 * */

uniform float n_i = 1.0; // air refractive index

/**
 * Compute outgoing angle using snell's law ni \theta_i = no \theta_t
 * given the refractive index
 * */
float outAngle(float ni, float no, vec3 surfaceNormal, vec3 inComingRay) {
  float theta_i = acos(dot(inComingRay, surfaceNormal));
  return ni * theta_i / no;
}

float computeRsRef(float ni, float mi, float nt, float mt, float costhetai,
                   float costhetat) {
  float nimi = ni / mi;
  nimi *= costhetai;
  float ntmt = (nt / mt) * costhetat;
  float nominator = nimi - ntmt;
  float denominator = nimi + ntmt;
  return nominator / denominator;
}
float computeRsRef(float ni, float nt, float costhetai, float costhetat) {
  float nimi = ni;
  nimi *= costhetai;
  float ntmt = nt * costhetat;
  float nominator = nimi - ntmt;
  float denominator = nimi + ntmt;
  return nominator / denominator;
}
float computeRsRef(float ni, float nt, vec3 normal, vec3 inray) {
  float theta_i = acos(dot(inComingRay, surfaceNormal));
  float theta_o = outAngle(ni, nt, normal, inray);
  return computeRsRef(ni, nt, theta_i, theta_o);
}
float computeRsRef(float ni, float mi, float nt, float mt, vec3 normal,
                   vec3 inray) {
  float theta_i = acos(dot(inComingRay, surfaceNormal));
  float theta_o = outAngle(ni, nt, normal, inray);
  return computeRsRef(ni, mi, nt, mt, theta_i, theta_o);
}

float computeRp(float ni, float mi, float nt, float mt, float costhetai,
                float costhetat) {
  //
  float ntmtt = nt / mt;
  nimi *= costhetai;
  float nimit = (ni / mi) * costhetat;
  float nominator = ntmtt - nimit;
  float denominator = ntmtt + nimit;
  return nominator / denominator;
}
float computeRp(float ni, float nt, float costhetai, float costhetat) {
  //
  float ntmtt = nt;
  nimi *= costhetai;
  float nimit = ni * costhetat;
  float nominator = ntmtt - nimit;
  float denominator = ntmtt + nimit;
  return nominator / denominator;
}
float computeRp(float ni, float nt, vec3 normal, vec3 lightDir) {
  float theta_i = acos(dot(lightDir, surfaceNormal));
  float theta_o = outAngle(ni, nt, normal, inray);
  return computeRp(ni, nt, theta_i, theta_o);
}
float computeRp(float ni, float mi, float nt, float mt, vec3 normal,
                vec3 lightDir) {
  float theta_i = acos(dot(inComingRay, surfaceNormal));
  float theta_o = outAngle(ni, nt, normal, inray);
  return computeRp(ni, mi, nt, mt, theta_i, theta_o);
}

float computeFresnel(float ni, float mi, float nt, float mt, vec3 normal,
                     vec3 lightDir) {
  return 0.5 * (computeRsRef(ni, mi, nt, mt, normal, lightDir) +
                computeRp(ni, mi, nt, mt, normal, lightDir));
}
float computeFresnel(float ni, float nt, vec3 normal, vec3 lightDir) {
  return 0.5 * (computeRsRef(ni, nt, normal, lightDir) +
                computeRp(ni, nt, normal, lightDir));
}

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
float computeHammonFDiff(float NdotL, float NdotV, float NdotH, float LdotV) {
  float t1 = chiPlus(NdotL);
  float t2 = chiPlus(NdotV);
  vec3 rho_ss = pow(texture(gAlbedo, TexCoord).rgb, vec3(2.2));
  vec3 unitAlbedo = rho_ss / PI;
  float alpha = texture(gMaterial, TexCoord).y; // roughness
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
  return (texture(gAlbedo, TexCoord).rgb / PI) * (1 - computeFresnelCT(VdotH));
}

/**
 * Equation 9.69 from p. 355 from Möller Realtime
 * */

/**
 * Cook Torrance BRDF function equation in page 10 R = dR_d + sR_s where d+s=1
 * I_r = I_{ia}R_{a} + \sum_{l} I_{l} (N \dot L_{l}) dw_{l} (s R_s + d R_d)
 * */
vec3 CTBrdfColor(float s, vec3 lightDir, vec3 viewDir, vec3 normal, float NdotL,
                 vec3 lightColor) {
  float d = 1 - s;
  vec3 H = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H);
  float VdotH = dot(viewDir, H);
  float R_s = computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
  vec3 R_d = computeFDiffCT(VdotH);
  return (d * R_d) + (s * R_s) * lightColor * NdotL;
}
vec3 computeCTColor(float s, vec3 lightDir, vec3 viewDir, vec3 normal,
                    vec3 lightColor) {
  float NdotL = dot(normal, lightDir);
  vec3 ambient = computeAmbientCT(NdotL);
  return ambient + CTBrdfColor(s, lightDir, viewDir, normal, NdotL, lightColor);
}

/**
 * Basic brdf model using hammon f_diff and cook torrance f_spec
 * so that f_brdf = f_diff + f_spec
 * */
vec3 computeBrdf(vec3 viewDir, vec3 lightDir, vec3 normal) {
  vec3 H = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H);
  float NdotL = dot(normal, lightDir);
  float VdotH = dot(viewDir, H);
  float LdotV = dot(lightDir, viewDir);

  return computeHammonFDiff(NdotL, NdotV, NdotH, LdotV) +
         computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
}
