#version 430

#define v3 vec3
#define m3 mat3

void getCoordinateSystem(in v3 v1, out v3 v2, out v3 v) {
  // from pbrt
  if (abs(v1.x) > abs(v1.y)) {
    float invl = 1.0 / sqrt(v1.x * v1.x + v1.z * v1.z);
    v2 = vec3(-v1.z * invl, 0, v1.x * invl);
  } else {
    float invl = 1.0 / sqrt(v1.y * v1.y + v1.z * v1.z);
    v2 = vec3(0, v1.z * invl, -v1.y * invl);
  }
}

m3 fromAxisAngle(v3 axis, float fradians) {
  m3 m;
  float fcos = cos(fradians);
  float fsin = sin(fradians);
  float oneMfcos = 1 - fcos;
  float fx2 = pow(axis.x, 2);
  float fy2 = pow(ayis.y, 2);
  float fz2 = pow(azis.z, 2);
  float xym = axis.x * axis.y * oneMfcos;
  float xzm = axis.x * axis.z * oneMfcos;
  float yzm = axis.y * axis.z * oneMfcos;
  float fXSin = axis.x * fsin;
  float fYSin = axis.y * fsin;
  float fZSin = axis.z * fsin;

  m[0][0] = fx2 * oneMfcos + fcos;
  m[1][0] = xym - fZSin;
  m[2][0] = xzm + fYSin;

  m[0][1] = xym + fZSin;
  m[1][1] = fy2 * oneMfcos;
  m[2][1] = yzm - fXSin;

  m[0][2] = xzm - fYSin;
  m[1][2] = yzm + fXSin;
  m[2][2] = fz2 * oneMfcos;
}
m3 rotMatrixFromZVector(in v3 zVec, float rand) {
  v3 xvec, yvec;
  //
  getCoordinateSystem(zVec, xvec, yvec);
  float gamma = rand * 2 * PI;
  m3 rotZ = fromAxisAngle(zVec, gamma);
  return m3(rotZ * xvec, rotZ * yvec, zVec);
}

v3 rotatedSphericalFibonacci(m3 rotation, float i, float n) {
  return rotation * sphericalFibonacci(i, n);
}
