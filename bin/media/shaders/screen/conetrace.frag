#version 430
// adapted from
/*
 *https://github.com/LukasBanana/ForkENGINE/blob/master/shaders/SSCTReflectionPixelShader.glsl
 * */
// layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 TexCoord;

out vec4 FragColor;

// ---------------------------- uniforms -----------------------------------
// ------------------------- from other passes -----------------------------

layout(binding = 0) uniform sampler2D gDepth;  // depth in viewspace
layout(binding = 1) uniform sampler2D gSDepth; // depth in screen space
layout(binding = 2) uniform sampler2D lightBuffer;
// convolved color buffer - all mip levels
layout(binding = 3) uniform sampler2D gNormal;
// normal buffer - from g-buffer in camera space
layout(binding = 4) uniform sampler2D gMaterial;
// specular buffer - from g-buffer (rgb = ior,
layout(binding = 5) uniform sampler2D visibilityBuffer;
layout(binding = 6) uniform sampler2D HizBuffer;

// --------------------- ray tracing uniforms --------------------------

uniform float mipCount; // total mipmap count for hiz buffer
uniform mat4 projection;
uniform mat4 view;
uniform vec2 nearFar; // near and far plane for viewing frustum
uniform vec3 viewPos; // world space

uniform float hiz_start_level = 5.0;
uniform float hiz_stride = 2.0;
uniform float hiz_max_iterations = 6.0;
uniform float cb_maxdistance = 15.0;
uniform float cb_stride = 1.0;
uniform float cb_strideZCutoff = 10.0;
uniform float cb_maxSteps = 7.0;
uniform float cb_zThickness = 2.0;

// ------------------- cone tracing uniforms --------------------------
uniform float sampling_zeta = 0.344;     // taken from GPU pro 5
uniform float cone_max_iterations = 7.0; // taken from GPU pro 5
uniform float fade_start = 0.5;
uniform float fade_end = 0.9;

// --------------------------------- Ray Tracing -----------------------------

struct Ray {
  vec3 origin;
  vec3 direction;
};
/** P = O + D*t */
vec3 getPointOnRay(Ray r, float dist) { return r.origin + r.direction * dist; }

/**
 * Get corresponding cell position given size of texture
*/
vec2 getCell(vec2 pos, vec2 size) { return vec2(pos * size); }

/**
 * Get cell count,ie, size of given texture level
*/
vec2 getCellCount(float level) {
  return vec2(textureSize(HizBuffer, int(level)));
}

/**
 * As per comment in gpu pro 5 this checks if two cells are same or not
*/
bool crossedCellBoundary(vec2 oldCell, vec2 newCell) {
  return oldCell == newCell;
}

/**
 * Compute new ray position when ray intersects with the specified cell's
 * boundary
 * */
vec3 intersectCellBoundary(Ray r, vec2 cellIndex, vec2 levelSize,
                           vec2 crossStep, vec2 crossOffset) {
  //
  vec2 cell = cellIndex + crossStep;
  cell /= levelSize;
  cell += crossOffset;

  vec2 delta = cell - r.origin.xy;
  delta /= r.direction.xy;

  float dist = min(delta.x, delta.y);

  return getPointOnRay(r, dist);
}

/**
 * Find intersection point of ray the variable names are taken from the paper
 * Y. Uludag GPU Pro 5
 * */
vec3 hiz_trace(vec3 p, vec3 v) {
  const float root = mipCount - 1.0;
  float level = hiz_start_level;
  const ivec2 HizSize = textureSize(HizBuffer, 0);

  float iterations = 0.0;

  vec2 crossStep = vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
  vec2 crossOffset;
  vec2 crossEpsilon = 0.5 / vec2(HizSize);

  // crossStep.x = v.x >=0.0? 1.0:-1.0 ;
  // crossStep.y = ;
  crossOffset = crossStep * crossEpsilon;
  crossStep = clamp(crossStep, 0.0, 1.0);

  //
  vec3 d = v / v.z;
  // get to starting point from p
  Ray r;
  r.origin = p;
  r.direction = d;
  vec3 o = getPointOnRay(r, -p.z);

  // ray cell index
  vec2 rayCell = getCell(r.origin.xy, vec2(HizSize));

  vec3 intersectionPoint =
      intersectCellBoundary(r, rayCell, vec2(HizSize), crossStep, crossOffset);

  Ray r2;
  r2.origin = intersectionPoint;
  r2.direction = r.direction;

  float stop_level = 0.0;
  float iterCount = 0.0;

  // starting level for traversal

  while (level >= stop_level && iterCount < hiz_max_iterations) {
    //
    float minz = textureLod(HizBuffer, r2.origin.xy, level).r;
    float maxz = textureLod(HizBuffer, r2.origin.xy, level).g;

    const vec2 levelSize = getCellCount(level);

    // get cell minimum depth plane
    const vec2 oldCell = getCell(r2.origin.xy, levelSize);

    Ray temp;
    vec3 tempOrigin = getPointOnRay(r2, max(r2.origin.z, minz));
    temp.origin = tempOrigin;
    temp.direction = r2.direction;

    // get cell boundary plane
    const vec2 newCell = getCell(temp.origin.xy, levelSize);

    // get closest of both planes + intersect with closest plane
    bool isCellBoundaryCrossed = crossedCellBoundary(oldCell, newCell);

    if (isCellBoundaryCrossed) {
      vec3 intersectionPoint =
          intersectCellBoundary(r2, oldCell, levelSize, crossStep, crossOffset);
      level = min(mipCount, level + 2.0);
      // 2 is necessary for going up since we are decrementing at the bottom
      temp.origin = intersectionPoint;
    }
    r2.origin = temp.origin;
    --level;
    ++iterCount;
  }
  // get hitpoint
  return r2.origin;
}
/**
 * Map view space coordinates to range [0, 1]*/
vec4 viewToTextureSpace(vec3 p_vs) {
  vec4 p_cs = projection * vec4(p_vs, 1);
  p_cs /= p_cs.w;
  return p_cs * 0.5 + 0.5;
}
float distanceSquared(vec2 v, vec2 v2) { return pow(distance(v, v2), 2); }
float distanceSquared(vec3 v, vec3 v2) { return pow(distance(v, v2), 2); }
float distanceSquared(vec4 v, vec4 v2) { return pow(distance(v, v2), 2); }
bool intersectsDepthBuffer(float z, float minZ, float maxZ) {
  /*
   * Based on how far away from the camera the depth is,
   * adding a bit of extra thickness can help improve some
   * artifacts. Driving this value up too high can cause
   * artifacts of its own.
   */
  float depthScale = min(1.0f, z * cb_strideZCutoff);
  z += cb_zThickness + mix(0.0f, 2.0f, depthScale);
  return (maxZ >= z) && (minZ - cb_zThickness <= z);
}
/**
 * Another ray tracing implementation adapted from
 * http://roar11.com/2015/07/screen-space-glossy-reflections/
 * which implements the method of Morgan McGuire efficient screen space ...*/
bool traceSSRay(Ray r, float jitter, out vec2 hitPixel, out vec3 hitPointVS) {
  vec3 maximumEndPoint = getPointOnRay(r, cb_maxDistance);
  float nearPlane = nearFar.x;
  float farPlane = nearFar.y;
  float rayLength = (maximumEndPoint.z < nearPlane)
                        ? (nearPlane - r.origin.z) / r.direction.z
                        : farPlane;
  vec3 csEndPoint = getPointOnRay(r, rayLength);
  vec4 H0 = viewToTextureSpace(r.origin);
  H0.xy = H0 * vec2(textureSize(lightBuffer, 0));

  vec4 H1 = viewToTextureSpace(csEndPoint);
  H1.xy = H1 * vec2(textureSize(lightBuffer, 0));

  float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;

  // The interpolated homogeneous version of the camera-space points
  vec3 Q0 = k0 * r.origin;
  vec3 Q1 = k1 * csEndPoint;

  // screen space end points
  vec2 P0 = H0.xy * k0;
  vec2 P1 = H1.xy * k1;

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 += (distanceSquared(P0, P1) < 0.0001f) ? vec2(0.01, 0.01) : 0.0;
  vec2 delta = P1 - P0;

  // Permute so that the primary iteration is in x to collapse
  // all quadrant-specific DDA cases later
  bool permute = false;
  if (abs(delta.x) < abs(delta.y)) {
    // This is a more-vertical line
    permute = true;
    delta = delta.yx;
    P0 = P0.yx;
    P1 = P1.yx;
  }

  float stepDir = sign(delta.x);
  float invdx = stepDir / delta.x;

  // Track the derivatives of Q and k
  vec3 dQ = (Q1 - Q0) * invdx;
  float dk = (k1 - k0) * invdx;
  vec3 dP = vec2(stepDir, delta.y * invdx);

  float strideScale = 1.0 - min(1.0, r.origin.z * cb_strideZCutoff);
  float stride = 1.0 + strideScale * cb_stride;
  dP *= stride;
  dQ *= stride;
  dk *= stride;

  P0 += dP * jitter;
  Q0 += dQ * jitter;
  k0 += dk * jitter;

  // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
  vec4 PQk = vec4(P0, Q0.z, k0);
  vec4 dPQk = vec4(dP, dQ.z, dk);
  vec3 Q = Q0;

  // Adjust end condition for iteration direction
  float end = P1.x * stepDir;

  float stepCount = 0.0f;
  float prevZMaxEstimate = r.origin.z;
  float rayZMin = prevZMaxEstimate;
  float rayZMax = prevZMaxEstimate;
  float sceneZMax = rayZMax + 100.0f;

  // stop conditions for looping
  bool endNotPassed = PQk.x * stepDir <= end;
  bool stepNotCountReached = stepCount < cb_maxSteps;
  bool notFinishedScene = sceneZMax != 0.0;
  bool notIntersecting = not intersectsDepthBuffer(sceneZMax, rayZMin, rayZMax);

  // now to iterate over lines
  while (endNotPassed && stepNotCountReached && notFinishedScene &&
         notIntersecting) {
  }
}
// ------------------------------ Cone Tracing -------------------------------
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
 * From Walter et. al. 2007 equation 28
 *
 * The D function in cook torrance is beckmann distribution
 * so we are going to use a sampling function for that distribution
 * */
float sampleBeckmannD(float roughness, float zeta) {
  float alpha2 = pow(roughnessToAlpha(roughness), 2);
  return atan(sqrt(-alpha2 * log(1 - zeta)));
}

float findOtherAnglesIsosclesTriangle(float coneAngle) {
  return (180 - coneAngle) / 2.0;
}
float findBaseIsosclesTriangle(float coneAngle, float side) {
  float anotherAngle = findOtherAnglesIsosclesTriangle(coneAngle);
  // base/sin(coneAngle) = side/sin(anotherAngle)
  return sin(coneAngle) * side / sin(anotherAngle);
}
float findHeightIsosclesTriangle(float base, float side) {
  return 0.5 * sqrt(4 * pow(side, 2) - pow(base, 2));
}
float findInradiusIsosclesTriangle(float a, float b) {
  // direct translation of formula
  // https://en.wikipedia.org/wiki/Isosceles_triangle
  float h = findHeightIsosclesTriangle(b, a);
  return (2 * a * b - pow(b, 2)) / 4 * h;
}
vec4 coneSampleWeightedColor(vec2 samplePos, float mipmapChannel) {
  // simple version
  vec3 color = textureLod(lightBuffer, samplePos, mipmapChannel).rgb;
  float visibility = textureLod(visibilityBuffer, samplePos, mipmapChannel).r;
  return vec4(color * 0.1, 0.1);
}
/*GPU pro 5 listing 4.8*/
vec4 fadeColor(vec4 color, vec2 hitPoint) {
  float boundary = distance(hitPoint, vec2(0.5)) * 2.0;
  float fadeOnBorder =
      1.0 - clamp((boundary - fade_start) / (fade_end - fade_start), 0.0, 1.0);
  return color * fadeOnBorder;
}

/**
 * Implementing HiZ cone tracing from Gpu pro 5 Y. Uludag
 * */
vec4 hiz_cone_trace(vec2 hitPoint) {
  //
  float roughness = texture(gMaterial, TexCoord).y;
  float coneTheta = sampleBeckmannD(roughness, sampling_zeta);
  vec2 sideEndPoint = hitPoint - TexCoord;
  float side = length(sideEndPoint);
  vec2 sideDirection = normalize(sideEndPoint);
  const vec2 hsize = vec2(textureSize(HizBuffer, 0));
  float msize = max(hsize.x, hsize.y);

  vec4 reflectionColor = vec4(0);
  float iterCount = 0;
  while (reflectionColor.a < 1.0 && iterCount < cone_max_iterations) {
    float base = findBaseIsosclesTriangle(coneTheta, side);
    float radius = findInradiusIsosclesTriangle(side, base);
    vec2 samplePos = hitPoint + sideDirection * (side - radius);

    // convert the in radius into screen size
    float mipmapChannel = log2(radius * msize);

    reflectionColor += coneSampleWeightedColor(samplePos, mipmapChannel);
    // compute the smaller triangle's side length
    side -= (radius * 2.0);
    iterCount++;
  }
  // normalizing visibility color
  return reflectionColor;
}

vec3 getViewDir(vec3 FragPosVS) {
  return normalize(FragPosVS - vec3(view * vec4(viewPos, 1)));
}
vec2 clipToScreenSpace(vec4 pointInClipSpace) {
  return (pointInClipSpace.xy + 1.0) / 2.0 * vec2(textureSize(gDepth, 0));
}

vec3 getP2Ss() {
  vec3 p_vs = texture(gDepth, TexCoord).xyz;
  vec3 n_vs = texture(gNormal, TexCoord).xyz;
  vec3 v_vs = getViewDir(p_vs);
  vec4 p_cs = projection * vec4(p_vs + reflect(v_vs, n_vs), 1);
  p_cs /= p_cs.w; // doing the perspective division
  p_cs.xy = clipToScreenSpace(p_cs);
  vec3 p2_ss = p_cs.xyz;
  return p2_ss;
}

void main() {
  // check if point is valid
  // first p'_ss for finding v. p. 163 of gpu gems 5 y. uludagli
  vec3 p_cs = texture(gSDepth, TexCoord).xyz;
  vec3 p_ss = vec3(clipToScreenSpace(vec4(p_cs, 1)), 1);

  vec3 p2_ss = getP2Ss();
  vec3 v_ss = p2_ss - p_ss;

  vec3 ray_hitpoint = hiz_trace(p_ss, v_ss);
  vec4 ray_color = hiz_cone_trace(ray_hitpoint.xy);
  // ray_color = fadeColor(ray_color, ray_hitpoint.xy);
  // vec4 ray_color = texture(lightBuffer, ray_hitpoint.xy);
  // in most basic setup
  vec3 color = ray_color.rgb + texture(lightBuffer, ray_hitpoint.xy).rgb;
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0 / 2.2));
  FragColor = vec4(color, 1);
  // FragColor = vec4(ray_hitpoint, 1);
  // FragColor = vec4(texture(lightBuffer, TexCoord).rgb, 1);

  // FragColor = vec4(vec3(0.7), 1);
}
