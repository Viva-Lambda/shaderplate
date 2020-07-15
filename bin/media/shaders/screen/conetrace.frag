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

layout(binding = 0) uniform sampler2D gPosition;   // depth in viewspace
layout(binding = 1) uniform sampler2D gSDepth;     // depth in screen space
layout(binding = 2) uniform sampler2D lightBuffer; // uv passed
// convolved color buffer - all mip levels
layout(binding = 3) uniform sampler2D gNormal;
// normal buffer - from g-buffer in camera space
layout(binding = 4) uniform sampler2D gMaterial;
// specular buffer - from g-buffer (rgb = ior,

// --------------------- ray tracing uniforms --------------------------

uniform float mipCount; // total mipmap count for hiz buffer
uniform mat4 projection;
uniform mat4 view;
uniform vec2 nearFar;        // near and far plane for viewing frustum
uniform vec3 viewPos;        // world space
uniform mat4 viewProjection; // projection matrix from ndc to screen space
uniform mat4 invProjView;
uniform mat4 invView;

// mcguire implementation related
uniform float cb_maxDistance = 10.0;
uniform float cb_stride = 1.0;
uniform float cb_strideZCutoff = 10.0;
uniform float cb_maxSteps = 10.0;
uniform float cb_zThickness = 10.0;
uniform float cb_jtter = 0.0;

// binary implementation related
uniform float minRayStep = 0.1;
uniform float rayStepCount = 40.0;
uniform float rayStep = 1.0;
uniform float rayStepDecrase = 0.2;
uniform float rayBinaryMaxIter = 12.0;
uniform float ray_jitter = 0.2;
uniform float ray_stride = 0.1;
uniform float depthEpsilon = 0.01;
uniform float bounceNb = 5.0;
uniform float pixThickness = 0.2;
uniform float pixMaxDistance = 6;
uniform float pixResolution = 0.3;
uniform float pixSteps = 5.0;

// general
uniform int traceChoice = 5;

// ------------------- cone tracing uniforms --------------------------
uniform float sampling_zeta = 0.344;     // taken from GPU pro 5
uniform float cone_max_iterations = 7.0; // taken from GPU pro 5
uniform float fade_start = 0.5;
uniform float fade_end = 0.9;

// --------------------------------- Ray Tracing -----------------------------
// --------------------------------- utils -----------------------------------
float PI = 3.1415926535;
float INFINITY = 1.0 / 0.0;
// end constants
//
// --------------------- utility functions ------------------------------
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

vec3 fix_color(vec3 pcolor, int samples_per_pixel) {
  // scale sample
  pcolor /= samples_per_pixel;
  return clamp(sqrt(pcolor), 0.0, 0.999);
}

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
  float level = 2.0;
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

  while (level >= stop_level && iterCount < 7.0) {
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
vec2 screenToTextureSpace(vec2 p_ss) {
  p_ss /= vec2(textureSize(gDepth, 0));
  p_ss *= 2;
  return p_ss - 1.0;
}

vec3 viewToTextureSpace(vec3 p_vs) {
  vec4 p_cs = projection * vec4(p_vs, 1);
  p_cs /= p_cs.w;
  vec3 p_ts = p_cs.xyz;
  p_ts.xy *= 0.5;
  p_ts.xy += 0.5;
  // invert y for opengl
  // p_ts.y = -p_ts.y;
  return p_ts;
}
vec2 clipToScreenSpace(vec2 pointInClipSpace) {
  return (pointInClipSpace + 1.0) / 2.0 * vec2(textureSize(gDepth, 0));
}
vec3 clipToScreenSpace(vec3 pointInClipSpace) {
  pointInClipSpace.xy =
      (pointInClipSpace.xy + 1.0) / 2.0 * vec2(textureSize(gDepth, 0));
  return pointInClipSpace;
}

vec4 clipToScreenSpace(vec4 pointInClipSpace) {
  pointInClipSpace.xy =
      (pointInClipSpace.xy + 1.0) / 2.0 * vec2(textureSize(gDepth, 0));
  return pointInClipSpace;
}
vec4 clipToViewSpace(vec3 p_cs) {
  return inverse(projection * view) * vec4(p_cs, 1);
}
vec2 clipToTextureSpace(vec2 p_cs) { return (p_cs + 1) / 2; }
vec3 clipToTextureSpace(vec3 p_cs) { return (p_cs + 1) / 2; }
vec3 textureToClipSpace(vec3 tex) { return (tex * 2) - 1.0; }

vec4 viewToScreenSpace(vec3 p_vs) {
  return viewProjection * projection * vec4(p_vs, 1);
}
vec4 screenToViewSpace(vec3 p_ss) {
  return inverse(viewProjection * projection) * vec4(p_ss, 1);
}
vec3 screenToViewSpace(vec2 p_ss) {
  vec2 textureCoord = screenToTextureSpace(p_ss);
  return texture(gDepth, textureCoord).rgb;
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
void swap(inout float a, inout float b) {
  float t = a;
  a = b;
  b = t;
}

/**
 * Another ray tracing implementation adapted from
 * http://roar11.com/2015/07/screen-space-glossy-reflections/
 * which implements the method of Morgan McGuire efficient screen space ...
 *
 * Ray contains values in view space
 *
    \param csOrigin Camera-space ray origin, which must be
    within the view volume and must have z < -0.01 and project within the
 valid
 screen rectangle

    \param csDirection Unit length camera-space ray direction

    \param projectToPixelMatrix A projection matrix that maps to pixel
 coordinates (not [-1, +1] normalized device coordinates)

    \param csZBuffer The depth or camera-space Z buffer, depending on the
 value
 of \a csZBufferIsHyperbolic

    \param csZBufferSize Dimensions of csZBuffer

    \param csZThickness Camera space thickness to ascribe to each pixel in the
 depth buffer

    \param csZBufferIsHyperbolic True if csZBuffer is an OpenGL depth buffer,
 false (faster) if
     csZBuffer contains (negative) "linear" camera space z values. Const so
 that
 the compiler can evaluate the branch based on it at compile time

    \param clipInfo See G3D::Camera documentation

    \param nearPlaneZ Negative number

    \param stride Step in horizontal or vertical pixels between samples. This
 is
 a float
     because integer math is slow on GPUs, but should be set to an integer >=
 1

    \param jitterFraction  Number between 0 and 1 for how far to bump the ray
 in
 stride units
      to conceal banding artifacts

    \param maxSteps Maximum number of iterations. Higher gives better images
 but
 may be slow

    \param maxRayTraceDistance Maximum camera-space distance to trace before
 returning a miss

    \param hitPixel Pixel coordinates of the first intersection with the scene

    \param csHitPoint Camera space location of the ray hit
*/
bool traceSSRay(Ray r, float jitter, out vec2 hitPixel, out vec3 hitPointVS) {
  vec3 maximumEndPoint = getPointOnRay(r, cb_maxDistance);
  float nearPlane = nearFar.x;
  float farPlane = nearFar.y;
  float rayLength = (maximumEndPoint.z < nearPlane)
                        ? (nearPlane - r.origin.z) / r.direction.z
                        : farPlane;
  vec3 csEndPoint = getPointOnRay(r, rayLength);
  vec4 H0 = viewToScreenSpace(r.origin);
  vec2 depthBufferSize = vec2(textureSize(gDepth, 0));
  H0.xy *= depthBufferSize;

  vec4 H1 = viewToScreenSpace(csEndPoint);
  H1.xy *= depthBufferSize;

  float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;

  // The interpolated homogeneous version of the camera-space points
  vec3 Q0 = k0 * r.origin;
  vec3 Q1 = k1 * csEndPoint;

  // screen space end points
  vec2 P0 = H0.xy * k0;
  vec2 P1 = H1.xy * k1;

  hitPixel = vec2(-1.0, -1.0);

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 +=
      vec2((distanceSquared(P0, P1) < 0.0001f) ? vec2(0.01, 0.01) : vec2(0.0));
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
  vec2 dP = vec2(stepDir, delta.y * invdx);

  // optional stride scaling
  // float strideScale = 1.0 - min(1.0, r.origin.z * cb_strideZCutoff);
  // float stride = 1.0 + strideScale * cb_stride;
  // or regular
  float stride = cb_stride;
  dP *= stride;
  dQ *= stride;
  dk *= stride;

  P0 += dP * jitter;
  Q0 += dQ * jitter;
  k0 += dk * jitter;

  vec3 Q = Q0;
  float end = P1.x * stepDir;

  float k = k0;

  float prevZMaxEstimate = r.origin.z;
  float stepCount = 0.0f;
  float rayZMin = prevZMaxEstimate;
  float rayZMax = prevZMaxEstimate;
  float sceneZMax = rayZMax + 100.0;

  // Adjust end condition for iteration direction

  // stop conditions for looping

  for (vec2 P = P0;
       ((P.x * stepDir) <= end) && (stepCount < cb_maxSteps) &&
       ((rayZMax < sceneZMax - cb_zThickness) || (rayZMin > sceneZMax)) &&
       (sceneZMax != 0.0);
       P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {

    hitPixel = permute ? P.yx : P;

    // The depth range that the ray covers within this loop
    // iteration.  Assume that the ray is moving in increasing z
    // and swap if backwards.  Because one end of the interval is
    // shared between adjacent iterations, we track the previous
    // value and then swap as needed to ensure correct ordering
    rayZMin = prevZMaxEstimate;

    // Compute the value at 1/2 pixel into the future
    rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
    prevZMaxEstimate = rayZMax;
    if (rayZMin > rayZMax) {
      swap(rayZMin, rayZMax);
    }

    // Camera-space z of the background
    sceneZMax = texelFetch(gDepth, ivec2(hitPixel), 0).w;
  }
  Q.xy += dQ.xy * stepCount;
  hitPointVS = Q * (1.0 / k);
  hitPixel = viewToScreenSpace(hitPointVS).xy;
  hitPixel.y = depthBufferSize.y - hitPixel.y;

  // Matches the new loop condition:
  return (rayZMax >= sceneZMax - cb_zThickness) && (rayZMin <= sceneZMax);
}

/**
 * Get color contribution of traced rays for given point
 * */
vec3 getRayColorMorgan(vec3 pointVS, float depth) {
  vec3 result = vec3(0);
  vec3 current = vec3(1);
  vec3 viewPosVS = vec3(view * vec4(viewPos, 1));

  Ray r;
  r.origin = viewPosVS;
  r.direction = normalize(r.origin - pointVS);
  Ray temp = r;

  for (float i = 0.0; i < depth; i += 1.0) {
    vec3 hitPointVS;
    vec2 hitPixel;
    if (traceSSRay(temp, cb_jtter, hitPixel, hitPointVS)) {
      // we have a hit let's take its color contribution
      vec2 textureCoord = screenToTextureSpace(hitPixel);
      vec3 hitColor = texture(lightBuffer, textureCoord.xy).rgb;
      float glossiness = 1.0 - texture(gMaterial, textureCoord).y;
      // current *= hitColor;
      current *= mix(hitColor, current, glossiness);
      // scatter ray either along normal or using some kind of pdf
      vec3 hitPointNormalVS =
          texture(gNormal, textureCoord.xy).rgb; // viewspace

      // depending on the result sampleBeckmannD can be used here
      hitPointVS = texture(gDepth, textureCoord).rgb;
      vec3 hitDir = normalize(viewPosVS - hitPointVS);
      vec3 hitmeanDir = reflect(-hitDir, hitPointNormalVS);
      vec3 scatterDir = hitmeanDir;
      temp.origin = hitPointVS;
      temp.direction = scatterDir;
    } else {
      vec3 rayDirection = temp.direction;
      vec3 textureCoord = viewToTextureSpace(rayDirection);
      result += (current * texture(lightBuffer, textureCoord.xy).rgb);
      // result = vec3(hitPixel, 1);
      break;
    }
  }
  return result;
}
// ------------------------------ Third Strategy -----------------------------
//  based on
//  http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html
//  and
//  https://gitlab.com/congard/algine/-/blob/master/resources/shaders/SSR.frag.glsl
//
// Hermanns (Lukas), Screen space cone tracing for glossy reflections,
// Bachelor
// Thesis, Technische Universität Darmstadt, Darmstadt, 2015. URL :
// http://publica.fraunhofer.de/documents/N-336466.html.
//
/**
 * adapted from algorithm 2 p. 27
 * */
vec3 get_reflection_ray_direction(vec2 pixelCoord) {
  ivec2 tsize = textureSize(gDepth, 0);
  float texSize = tsize.x * tsize.y;
  vec3 PointVS = texture(gDepth, pixelCoord).xyz;
  vec3 PointSS = texture(gSDepth, pixelCoord).xyz;
  vec3 ViewDirection = normalize(PointVS);
  vec3 NormalVS = normalize(texture(gNormal, pixelCoord).xyz);
  vec3 ReflectRayDirection = reflect(ViewDirection, NormalVS);
  float epsilon = 1.0 / texSize;
  vec4 ScreenSpaceOffset =
      viewToScreenSpace(PointVS + ReflectRayDirection * epsilon);
  vec3 ReflectionVectorSS = vec3(ScreenSpaceOffset - vec4(PointSS, 1));
  return ReflectionVectorSS;
}

vec3 hash(vec3 a) {
  a = fract(a * vec3(0.8));
  a += dot(a, a.yxz + 19.19);
  return fract((a.xxy + a.yxx) * a.zyx);
}
vec3 binary_search(inout vec3 dir, // view space ray
                   inout vec3 hitCoord, inout float deltaDepth) {
  float depth;
  vec3 projected;
  for (int i = 0; i < rayBinaryMaxIter; i++) {
    projected = viewToTextureSpace(hitCoord);
    depth = texture(gDepth, projected.xy).z;
    deltaDepth = hitCoord.z - depth;
    if (deltaDepth > 0.0) {
      hitCoord += dir;
    } else {
      hitCoord -= dir;
    }
  }
  projected = viewToTextureSpace(hitCoord);
  return vec3(projected.xy, depth);
}
vec4 ray_march(inout vec3 dir,      // view space ray
               inout vec3 hitCoord, // hit pixel position
               out float deltaDepth, out bool intersected) {
  // using a view space depth buffer
  vec3 hitCoordOrig = hitCoord;
  dir *= minRayStep;
  float depth;
  vec4 projected;
  for (int i = 0; i < rayStepCount; i++) {
    hitCoord += dir;

    projected = projection * vec4(hitCoord, 1);
    projected.xy /= projected.w;
    projected.xy *= 0.5;
    projected.xy += 0.5;

    depth = texture(gDepth, projected.xy).z;
    deltaDepth = hitCoord.z - depth;

    if ((dir.z - deltaDepth) < nearFar.y) {
      if (deltaDepth <= 0.0) {
        // some threshold is necessary for stopping rays that goes to infinity
        vec4 res = vec4(binary_search(dir, hitCoord, deltaDepth), 1);
        intersected = true;
        return res;
      }
    }
  }
  intersected = false;
  return vec4(projected.xy, depth, 0.0);
}
vec3 toModel(vec3 p_vs) { return vec3(invView * vec4(p_vs, 1)); }
vec3 toView(vec3 p_ws) { return vec3(view * vec4(p_ws, 1)); }
vec3 getRayColorBinary() {
  float roughness = texture(gMaterial, TexCoord).y; // 1 - roughness
  float glossiness = 1.0 - roughness;
  vec3 normalvs = texture(gNormal, TexCoord).xyz;
  vec3 normalWS = toModel(normalvs);
  vec4 posVS = texture(gDepth, TexCoord).xyzw;
  vec3 pWS = toModel(posVS.xyz);
  vec3 refv = normalize(reflect(normalize(pWS), normalize(normalWS)));
  refv = toView(refv);

  vec3 dir = refv * max(minRayStep, -posVS.z);
  float depth;
  vec3 hitPoint = posVS.xyz;
  bool intersected;
  vec4 hitCoord = ray_march(dir, hitPoint, depth, intersected);
  vec3 fragcolor = texture(lightBuffer, TexCoord).rgb;
  vec3 color = texture(lightBuffer, hitCoord.xy).rgb;
  if (intersected) {
    return mix(color, fragcolor, 0.1);
  } else {
    return fragcolor;
  }
}

vec3 BinarySearch(vec3 ray, vec3 dir) {
  for (int i = 0; i < rayBinaryMaxIter; ++i) {
    // Sample depth value at current ray position
    float depth = textureLod(gSDepth, ray.xy, 0.0).z;
    // Check if the ’depth delta ’ is smaller than our epsilon
    float depthDelta = depth - ray.z;
    if (abs(depthDelta) < depthEpsilon) {
      break; // Final intersection found -> break iteration
    }
    // Move ray forwards if we are in front of geometry ,
    // and move ray backwards if we are behind geometry .
    if (depthDelta > 0.0)
      ray += dir;
    else
      ray -= dir;
    // Decrease direction vector for further refinement
    dir *= rayStepDecrase;
  }
  return ray;
  // Intersection already found , but we could not refine it to the
  // minimum
}
vec3 LinearMarch(vec3 ray, // texture space
                 vec3 dir, // texture space direction
                 float stride) {
  vec3 prevRay = ray;
  for (int i = 0; i < rayStepCount; ++i) {
    // Sample depth value at current ray position
    float depth = texture(gSDepth, ray.xy).z;
    // Check if ray steps through the depth buffer
    if (ray.z > depth) {
      // Intersection found -> now do a binary search
      return BinarySearch((prevRay + ray) * 0.5, dir * stride);
    }
    // Store previous position and step to the next sample position
    prevRay = ray;
    ray += dir * stride;
    // Increase stride to accelerate ray marching
    stride *= rayStep;
  }
  return vec3(-1); // No intersection found
}

bool hitColor(vec2 TexCoord, out vec2 hitCoordinate, out vec3 outColor) {
  float roughness = texture(gMaterial, TexCoord).y; // roughness
  float glossiness = 1.0 - roughness;
  float metallic = texture(gMaterial, TexCoord).x; // metallic
  vec3 fcolor = texture(lightBuffer, TexCoord).rgb;
  if (metallic < 0.2) {
    return false;
  }

  vec3 normalvs = texture(gNormal, TexCoord).xyz;
  vec3 posVS = texture(gDepth, TexCoord).xyz;
  vec3 posSS = texture(gSDepth, TexCoord).xyz;
  vec3 posTS = viewToTextureSpace(posVS);
  posTS.z = posSS.z;

  vec3 refvs = reflect(normalize(posVS.xyz), normalize(normalvs));
  vec3 refts = viewToTextureSpace(refvs);
  vec4 posWS = inverse(view) * vec4(posVS.xyz, 1);
  vec3 jitterFrac = hash(posWS.xyz) * roughness;

  // vec3 dir = jitterFrac + refv * max(minRayStep, -posVS.z);
  vec3 dir = refts * max(minRayStep, -posTS.z);
  float depth;
  vec3 hitPoint = posTS;
  vec3 hitCoord = LinearMarch(hitPoint, refts, ray_stride);
  vec3 color = texture(lightBuffer, hitCoord.xy).rgb;

  if (hitCoord != vec3(-1.0)) {
    hitCoordinate = hitCoord.xy;
    outColor = color;

    return true;
  }
  return false;
}

vec3 getRayColorBinary2() {
  vec3 current = vec3(1);
  vec3 result = vec3(0);
  bool intersect;
  vec2 hitCoordinate;
  vec2 inCoordinate = TexCoord;
  vec3 outColor;
  for (int i = 0; i < bounceNb; i++) {
    if (hitColor(inCoordinate, hitCoordinate, outColor)) {
      inCoordinate = hitCoordinate;
      current *= mix(outColor, current, texture(gMaterial, hitCoordinate).y);
    } else {
      result += mix(current, texture(lightBuffer, inCoordinate).rgb,
                    texture(gMaterial, hitCoordinate).y);
      break;
    }
  }
  return result;
}

vec3 LinearRayMarch(vec3 ray, vec3 dir, float stride) {
  for (int i = 0; i < rayStepCount; ++i) {
    // Sample depth value at current ray position
    float depth = texture(gSDepth, ray.xy).z;
    // Check if ray steps through the depth buffer
    if (ray.z > depth) {
      return ray; // Return final intersection
    }
    // Step to the next sample position
    ray += dir * stride;
  }
  return vec3(-1); // No intersection found
}
vec3 getRayColorBinary3() {
  float roughness = texture(gMaterial, TexCoord).y; // roughness
  float metallic = texture(gMaterial, TexCoord).x;  // metallic
  vec3 fcolor = texture(lightBuffer, TexCoord).rgb;
  if (metallic < 0.2) {
    return fcolor;
  }
  float glossiness = 1.0 - roughness;
  vec3 normalvs = texture(gNormal, TexCoord).xyz;
  vec3 posVS = texture(gDepth, TexCoord).xyz;
  vec3 posSS = texture(gSDepth, TexCoord).xyz;
  vec3 refvs = reflect(normalize(posVS.xyz), normalize(normalvs));
  vec3 refts = viewToTextureSpace(refvs);
  vec3 posTS = viewToTextureSpace(posVS);

  vec3 dir = refts * max(minRayStep, -posSS.z);
  float depth;
  vec3 hitPoint = posTS;
  vec3 hitCoord = LinearRayMarch(hitPoint, dir, ray_stride);
  vec3 color = texture(lightBuffer, hitCoord.xy).rgb;

  if (hitCoord != vec3(-1.0)) {
    return mix(color, fcolor, glossiness);
  }
  return fcolor;
}
/**
 * From
 *https://github.com/IanLilleyT/Real-Time-Reflections-OpenGL/blob/master/data/shaders/ReflectionFrag.frag

 * and from
 *
 https://github.com/lettier/3d-game-shaders-for-beginners/blob/master/demonstration/shaders/fragment/screen-space-reflection.frag
 * */
bool inScreen(vec3 p_ss) {
  float x = p_ss.x;
  float y = p_ss.y;
  float z = p_ss.z;
  bool x_check = x < 1.0 && x > 0.0;
  bool y_check = y < 1.0 && y > 0.0;
  bool z_check = z < 1.0 && z > 0.0;
  // testing against default values of opengl
  return x_check && y_check && z_check;
}
bool inScreen(vec2 uv) {
  bool x_check = uv.x < 1.0 && uv.x > 0.0;
  bool y_check = uv.y < 1.0 && uv.y > 0.0;
  return x_check && y_check;
}
bool inScreen(vec4 uv) {
  bool x_check = uv.x < 1.0 && uv.x > 0.0;
  bool y_check = uv.y < 1.0 && uv.y > 0.0;
  return x_check && y_check;
}
void getViewDistDepth(vec2 startFrag, vec2 endFrag, float s, vec2 startView,
                      vec2 endView, out float viewDistance, out float depth) {
  vec2 frag = mix(startFrag.xy, endFrag.xy, s);
  vec2 uv = frag / textureSize(gDepth, 0);
  vec3 positionToVs = texture(gDepth, uv).rgb;
  viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, s);
  depth = viewDistance - positionToVs.y;
}
/**Convert [0,1] screen space depth to view space depth*/
float linearDepth(float depth_ss) {
  return (2.0 * nearFar.x) /
         (nearFar.y + nearFar.x - depth_ss * (nearFar.y - nearFar.x));
}
vec4 getRay4() {
  vec2 texSize = textureSize(gSDepth, 0);
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec2 pointSS = texture(gSDepth, texCoord).xy;
  vec4 pointVSs = texture(gDepth, texCoord);
  vec4 positionFromVs = pointVSs;
  vec3 PointSS = vec3(pointSS, texture(gSDepth, texCoord).z);

  vec4 uv = vec4(0);

  // roughness
  float roughness = texture(gMaterial, texCoord).y;
  float glossiness = 1 - roughness;

  vec3 viewDirVS = normalize(positionFromVs.xyz);
  vec3 normalVS = normalize(texture(gNormal, texCoord).xyz);
  vec3 pivot = normalize(reflect(viewDirVS, normalVS));

  vec4 positionToVs = positionFromVs;

  vec4 startView = vec4(positionFromVs.xyz + (pivot * 0.0), 1.0);
  vec4 endView = vec4(positionFromVs.xyz + (pivot * pixMaxDistance), 1.0);

  vec4 startFrag = projection * startView;
  startFrag.xyz /= startFrag.w;
  startFrag.xy = startFrag.xy * 0.5 + 0.5;
  startFrag.xy *= texSize;

  vec4 endFrag = projection * endView;
  endFrag.xyz /= endFrag.w;
  endFrag.xy = endFrag.xy * 0.5 + 0.5;
  endFrag.xy *= texSize;

  vec2 frag = startFrag.xy;
  uv.xy = frag / texSize;

  float deltaX = endFrag.x - startFrag.x;
  float deltaY = endFrag.y - startFrag.y;

  // decide axis
  float useX = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;

  float delta =
      mix(abs(deltaY), abs(deltaX), useX) * clamp(pixResolution, 0.0, 1.0);

  vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);

  float search0 = 0.0;
  float search1 = 0.0;
  int hit0 = 0;
  int hit1 = 0;

  float steps = pixSteps;

  float viewDistance = startView.y;
  float depth = pixThickness;

  for (int i = 0; i < int(delta); i++) {
    frag += increment;
    uv.xy = frag / texSize;
    positionToVs = texture(gDepth, uv.xy);

    //
    search1 = mix((frag.y - startFrag.y) / deltaY,
                  (frag.x - startFrag.x) / deltaX, useX);

    // clamp to value to uv range
    search1 = clamp(search1, 0.0, 1.0);

    viewDistance =
        (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth = viewDistance - positionToVs.y;

    if (depth > 0 && depth < pixThickness) {
      hit0 = 1;
    } else {
      search0 = search1;
    }
  }
  search1 = search0 + ((search1 - search0) / 2.0);
  steps *= hit0;
  for (int i = 0; i < steps; i++) {
    frag = mix(startFrag.xy, endFrag.xy, search1);
    uv.xy = frag / texSize;
    positionToVs = texture(gDepth, uv.xy);

    viewDistance =
        (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth = viewDistance - positionToVs.y;

    if (depth > 0 && depth < pixThickness) {
      hit1 = 1;
      search1 = search0 + ((search1 - search0) / 2.0);
    } else {
      float temp = search1;
      search1 = search1 + ((search1 - search0) / 2.0);
      search0 = temp;
    }
  }
  //
  float depthAlpha = positionToVs.w;
  float costheta = max(dot(-viewDirVS, pivot), 0.0);
  // depth to uv range
  float uvdepth = (1 - clamp(depth / pixThickness, 0, 1));
  // scale ray length to uv range
  float scaled_ray = length(positionToVs - positionFromVs) / pixMaxDistance;
  float ray_len = 1 - clamp(scaled_ray, 0.0, 1.0);
  float visibility = hit1 * depthAlpha * (1 - costheta) * uvdepth * ray_len *
                     (inScreen(uv) ? 1.0 : 0.0);

  visibility = clamp(visibility, 0.0, 1.0); // alpha channel
  uv.ba = vec2(visibility);
  return uv;
}
vec4 getRayColorBinary4() {
  //
  vec4 uv = getRay4();
  vec4 color4X = textureGather(lightBuffer, uv.xy, 0);
  vec4 color4Y = textureGather(lightBuffer, uv.xy, 1);
  vec4 color4Z = textureGather(lightBuffer, uv.xy, 2);
  vec4 color4A = textureGather(lightBuffer, uv.xy, 3);
  vec4 first = vec4(color4X.x, color4Y.x, color4Z.x, color4A.x);
  vec4 second = vec4(color4X.y, color4Y.y, color4Z.y, color4A.y);
  vec4 third = vec4(color4X.z, color4Y.z, color4Z.z, color4A.z);
  vec4 fourth = vec4(color4X.a, color4Y.a, color4Z.a, color4A.a);
  vec4 color = (first + second + third + fourth) / 4.0;
  float alpha = clamp(uv.z, 0.0, 1.0);

  return vec4(mix(vec3(0.0), color.rgb, alpha), alpha);
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

vec3 getP2Ss() {
  vec3 p_vs = texture(gDepth, TexCoord).xyz;
  vec3 n_vs = texture(gNormal, TexCoord).xyz;
  vec3 v_vs = getViewDir(p_vs);
  vec4 p_cs = projection * vec4(p_vs + reflect(v_vs, n_vs), 1);
  p_cs /= p_cs.w; // doing the perspective division
  p_cs = clipToScreenSpace(p_cs);
  vec3 p2_ss = p_cs.xyz;
  return p2_ss;
}

void main() {
  // check if point is valid
  // effect for metals
  float metallic = texture(gMaterial, TexCoord).r;
  if (metallic < 0.2) {
    FragColor = texture(lightBuffer, TexCoord);
  } else {

    vec3 p_vs = texture(gDepth, TexCoord).xyz;
    vec4 ray_color;
    switch (traceChoice) {
    case 1:
      float depth = cb_maxSteps;
      ray_color.xyz = getRayColorMorgan(p_vs, depth);
      break;
    case 2:
      ray_color.xyz = getRayColorBinary();
      break;
    case 3:
      ray_color.xyz = getRayColorBinary2();
      break;
    case 4:
      ray_color.xyz = getRayColorBinary3();
      break;
    case 5:
      ray_color = getRayColorBinary4();
      break;
    }

    // vec4 color = ray_color;
    vec4 color = texture(lightBuffer, TexCoord);
    // hdr
    color.xyz = color.xyz / (color.xyz + vec3(1.0));
    //// gamma correct
    color.xyz = pow(color.xyz, vec3(1.0 / 2.2));
    FragColor = color;
  }
}
