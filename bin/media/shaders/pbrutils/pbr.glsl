#version 330
// pbr related functions and their references
//
//

/**
 * View space vec2
 * */
struct Vec2vs {
  vec2 v;
};
/**
 * View space vec3
 * */

struct Vec3vs {
  vec3 v;
};
/**
 * View space vec4
 * */
struct Vec4vs {
  vec2 v;
};

/**
 * Clip space vec2
 * */
struct Vec2cs {
  vec2 v;
};
/**
 * Clip space vec3
 * */

struct Vec3cs {
  vec3 v;
};
/**
 * Clip space vec4
 * */
struct Vec4cs {
  vec2 v;
};

float degree_to_radian(float degree) {
  //
  return degree * PI / 180.0;
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
vec3 random_in_unit_sphere() {
  // random in unit sphere
  while (true) {
    //
    vec3 v = random_vec(-1.0, 1.0);
    if (dot(v, v) >= 1.0) {
      continue;
    }
    return v;
  }
}
vec3 random_unit_vector() {
  // unit vector
  float a = random_double(0, 2 * PI);
  float z = random_double(-1, 1);
  float r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
}
vec3 random_in_hemisphere(vec3 normal) {
  // normal ekseninde dagilan yon
  vec3 unit_sphere_dir = random_in_unit_sphere();
  if (dot(unit_sphere_dir, normal) > 0.0) {
    return unit_sphere_dir;
  } else {
    return -1 * unit_sphere_dir;
  }
}
vec3 random_in_unit_disk() {
  // lens yakinsamasi için gerekli
  while (true) {
    vec3 point = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    if (dot(point, point) >= 1) {
      continue;
    }
    return point;
  }
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
  float theta_i = acos(normalize(dot(inComingRay, surfaceNormal)));
  return asin(ni * sin(theta_i) / no);
}
float outAngle(float ni, float no, vec2 surfaceNormal, vec2 inComingRay) {
  float theta_i = acos(normalize(dot(inComingRay, surfaceNormal)));
  return asin(ni * sin(theta_i) / no);
}

float outVector(float ni, float no, vec2 surfaceNormal, vec2 inComingRay) {
  float angle = outAngle(ni, no, surfaceNormal, inComingRay);
  float x = -sin(angle);
  float y = -cos(angle);
  return vec2(x, y);
}
/**
 * From https://physics.stackexchange.com/a/436252
 * and from
 * http://www.cse.chalmers.se/edu/year/2013/course/TDA361/refractionvector.pdf
 * and from https://shaderbits.com/blog/optimized-snell-s-law-refraction
 * */
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
 * Equation 16 from Walter et.al 2007 Microfacet models ...
 * h_t = \frac{\vec{h_t}}{|h_t|} where \vec{h_t} = -(n_i * i + n_o * o)
 * i is the incoming ray direction n_i is the refractive index of the incoming
 * ray's medium, and n_o or n_t is the transmitted medium and o is the
 * transmitted direction
 * */
vec3 computeHt(float ni, float nt, vec3 inDir, vec3 normal) {
  vec3 outDir = outAngle(ni, nt, normal, inDir);
  vec3 ht = -(ni * inDir + nt * outDir);
  return normalize(ht);
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
 * Geometrical attenuation equation page 11 from Cook Torrance 1981 paper
 * */
float computeGeometryCT(float NdotL, float NdotV, float VdotH, float NdotH) {
  //
  float g1 = 2 * NdotH * NdotV / VdotH;
  float g2 = 2 * NdotH * NdotL / VdotH;
  float gmin = min(g1, g2);
  return min(1.0, gmin);
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
 * From Walter et. al. 2007 equation 28*/
float sampleBeckmannD(float roughness, float zeta) {
  float alpha2 = pow(roughnessToAlpha(roughness), 2);
  return atan(sqrt(-alpha2 * log(1 - zeta)));
}

/**
 * from pbrt edition 3, p. 539 equation 8.12 and Walter et. al 2007 equation
 * 33
 * */
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
/**Equation 35 from Walter et. al. 2007 Microfacet ...*/
float sampleTrowbridgeReitzD(float roughness, float zeta) {
  float alpha = roughnessToAlpha(roughness);
  return atan(alpha * sqrt(zeta) / sqrt(1 - zeta));
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
 * Equation 21 from Walter et. al. 2007
 * */
vec3 computeFTrans(vec3 inDir, vec3 outDir, vec3 normal, float ni, float nt,
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
  denominator = ni * LdotHt + VdotHt * nt;
  denominator *= denominator;
  return (LdotHt * VdotHt) / (LdotN * VdotN) * (nominator / denominator);
}

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
/**
 * Compute bsdf using equation 19-21 Walter et.al. 2007 Microfacet Models ...
 * */
vec3 computeBSDF(vec3 viewDir, vec3 lightDir, vec3 normal, float ni, float nt,
                 float roughness) {
  vec3 H_r = normalize(viewDir + lightDir);
  float NdotV = dot(normal, viewDir);
  float NdotH = dot(normal, H);
  float NdotL = dot(normal, lightDir);
  float VdotH = dot(viewDir, H);
  float LdotV = dot(lightDir, viewDir);

  vec3 f_r = computeFSpecCT(NdotH, NdotL, NdotV, VdotH);
  vec3 f_t = computeFTrans(lightDir, viewDir, normal, ni, nt, roughness);
  return f_r + f_t;
}
