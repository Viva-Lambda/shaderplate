#version 430
layout(binding = 0) uniform sampler2D gPosition;   // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;     // normals in viewspace
layout(binding = 2) uniform sampler2D lightBuffer; // normals in viewspace
// normal buffer - from g-buffer in in view space
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

const float PI = 3.14159265359;
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

// ------------------------------- Morgan McGuire Method --------------------
uniform vec3 lightPos; // view space
// -----------------------------
// mcguire implementation related
uniform float cb_maxDistance = 100.0;
uniform float cb_stride = 1.0;
uniform float cb_strideZCutoff = 100.0;
uniform float cb_maxSteps = 50.0;
uniform float cb_zThickness = 1.0;
uniform float cb_jtter = 0.6;
uniform float cb_nearPlaneZ = -0.1;
uniform float bounceNb = 3.0;

uniform mat4 viewProjection; // projection matrix from ndc to screen space
uniform mat4 projection;

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

vec4 viewToScreenSpace(vec3 p_vs) {
  return viewProjection * projection * vec4(p_vs, 1);
}
vec4 screenToViewSpace(vec3 p_ss) {
  return inverse(viewProjection * projection) * vec4(p_ss, 1);
}
vec2 screenToTextureSpace(vec2 p_ss) {
  p_ss /= vec2(textureSize(gPosition, 0));
  p_ss *= 2;
  return p_ss - 1.0;
}
vec3 screenToViewSpace(vec2 p_ss) {
  vec2 textureCoord = screenToTextureSpace(p_ss);
  return texture(gPosition, textureCoord).rgb;
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
    \param csOrigin Camera-space ray origin, which must be
    within the view volume and must have z < -0.01 and project within the valid
   screen rectangle

    \param csDirection Unit length camera-space ray direction

    \param projectToPixelMatrix A projection matrix that maps to pixel
   coordinates (not [-1, +1] normalized device coordinates)

    \param csZBuffer The depth or camera-space Z buffer, depending on the value
   of \a csZBufferIsHyperbolic

    \param csZBufferSize Dimensions of csZBuffer

    \param csZThickness Camera space thickness to ascribe to each pixel in the
   depth buffer

    \param csZBufferIsHyperbolic True if csZBuffer is an OpenGL depth buffer,
   false (faster) if
     csZBuffer contains (negative) "linear" camera space z values. Const so that
   the compiler can evaluate the branch based on it at compile time

    \param clipInfo See G3D::Camera documentation

    \param nearPlaneZ Negative number

    \param stride Step in horizontal or vertical pixels between samples. This is
   a float
     because integer math is slow on GPUs, but should be set to an integer >= 1

    \param jitterFraction  Number between 0 and 1 for how far to bump the ray in
   stride units
      to conceal banding artifacts

    \param maxSteps Maximum number of iterations. Higher gives better images but
   may be slow

    \param maxRayTraceDistance Maximum camera-space distance to trace before
   returning a miss

    \param hitPixel Pixel coordinates of the first intersection with the scene

    \param csHitPoint Camera space location of the ray hit

    Single-layer

 */
bool traceScreenSpaceRay1(vec3 csOrigin,    // current fragment
                          vec3 csDirection, // reflected fragment
                          mat4 projectToPixelMatrix, vec2 csZBufferSize,
                          float csZThickness, float nearPlaneZ, float stride,
                          float jitterFraction, float maxSteps,
                          in float maxRayTraceDistance, out vec2 hitPixel,
                          out vec3 csHitvec) {
  // screen space range is [0, width] and [0, height]

  // Clip ray to a near plane in 3D (doesn't have to be *the* near plane,
  // although that would be a good idea)
  float rayLength =
      ((csOrigin.z + csDirection.z * maxRayTraceDistance) > nearPlaneZ)
          ? (nearPlaneZ - csOrigin.z) / csDirection.z
          : maxRayTraceDistance;
  vec3 csEndvec = csDirection * rayLength + csOrigin;

  // Project into screen space
  vec4 H0 = projectToPixelMatrix * vec4(csOrigin, 1.0);
  vec4 H1 = projectToPixelMatrix * vec4(csEndvec, 1.0);

  // There are a lot of divisions by w that can be turned into multiplications
  // at some minor precision loss...and we need to interpolate these 1/w values
  // anyway.
  //
  // Because the caller was required to clip to the near plane,
  // this homogeneous division (projecting from 4D to 2D) is guaranteed
  // to succeed.
  float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;

  // Switch the original points to values that interpolate linearly in 2D
  vec3 Q0 = csOrigin * k0;
  vec3 Q1 = csEndvec * k1;

  // Screen-space endpoints
  vec2 P0 = H0.xy * k0;
  vec2 P1 = H1.xy * k1;

  // [Optional clipping to frustum sides here]

  // Initialize to off screen
  hitPixel = vec2(-1.0, -1.0);

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

  vec2 delta = P1 - P0;

  // Permute so that the primary iteration is in x to reduce
  // large branches later
  bool permute = false;
  if (abs(delta.x) < abs(delta.y)) {
    // More-vertical line. Create a permutation that swaps x and y in the output
    permute = true;

    // Directly swizzle the inputs
    delta = delta.yx;
    P1 = P1.yx;
    P0 = P0.yx;
  }

  // From now on, "x" is the primary iteration direction and "y" is the
  // secondary one

  float stepDirection = sign(delta.x);
  float invdx = stepDirection / delta.x;
  vec2 dP = vec2(stepDirection, invdx * delta.y);

  // Track the derivatives of Q and k
  vec3 dQ = (Q1 - Q0) * invdx;
  float dk = (k1 - k0) * invdx;

  // Scale derivatives by the desired pixel stride
  dP *= stride;
  dQ *= stride;
  dk *= stride;

  // Offset the starting values by the jitter fraction
  P0 += dP * jitterFraction;
  Q0 += dQ * jitterFraction;
  k0 += dk * jitterFraction;

  // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, and k from k0 to
  // k1
  vec3 Q = Q0;
  float k = k0;

  // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid
  // voxels. Because the depth at -1/2 for a given pixel will be the same as at
  // +1/2 for the previous iteration, we actually only have to compute one value
  // per iteration.
  float prevZMaxEstimate = csOrigin.z;
  float stepCount = 0.0;
  float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
  float sceneZMax = rayZMax + 1e4;

  // P1.x is never modified after this point, so pre-scale it by
  // the step direction for a signed comparison
  float end = P1.x * stepDirection;

  // We only advance the z field of Q in the inner loop, since
  // Q.xy is never used until after the loop terminates.

  for (vec2 P = P0;
       ((P.x * stepDirection) <= end) && (stepCount < maxSteps) &&
       ((rayZMax < sceneZMax - csZThickness) || (rayZMin > sceneZMax)) &&
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
    sceneZMax = length(texelFetch(gPosition, ivec2(hitPixel), 0));

  } // pixel on ray

  Q.xy += dQ.xy * stepCount;
  csHitvec = Q * (1.0 / k);

  // Matches the new loop condition:
  return (rayZMax >= sceneZMax - csZThickness) && (rayZMin <= sceneZMax);
}

vec3 getRayHit2(vec2 texCoords, vec3 target, out vec3 hitColor) {
  vec4 csOrigin = texture(gPosition, texCoords);
  vec3 viewDir = normalize(csOrigin.xyz);
  vec3 normal = normalize(texture(gNormal, texCoords).xyz);
  vec3 csDirection = normalize(csOrigin.xyz - target);

  mat4 projectToPixelMatrix = viewProjection;
  vec2 csZBufferSize = textureSize(gPosition, 0);
  float csZThickness = cb_zThickness;
  float nearPlaneZ =
      cb_nearPlaneZ; // might be positive as well due to different right
                     // handed left handed system difference
  float stride = cb_stride;
  float jitterFraction = cb_jtter;
  float maxSteps = cb_maxSteps;
  float maxRayTraceDistance = cb_maxDistance;
  vec2 hitPixel;
  vec3 csHitvec;
  vec3 uv;
  if (traceScreenSpaceRay1(csOrigin.xyz, csDirection, projectToPixelMatrix,
                           csZBufferSize, csZThickness, nearPlaneZ, stride,
                           jitterFraction, maxSteps, maxRayTraceDistance,
                           hitPixel, csHitvec)) {
    hitPixel.y = csZBufferSize.y - hitPixel.y;
    vec3 inDir = normalize(csHitvec - csOrigin.xyz);
    float NdotL = dot(normal, inDir);
    vec2 hitTexCoord = viewToTextureSpace(csHitvec).xy;
    hitColor = texelFetch(lightBuffer, ivec2(hitPixel), 0).rgb;

    uv = vec3(hitTexCoord, NdotL);
  } else {
    uv = vec3(0);
  }
  return uv;
}

/**
 * Evaluate the current state of color*/
vec3 evaluateColor(vec2 uv1, vec2 uv2, float NdotL) {
  return texture(lightBuffer, uv1).rgb * texture(lightBuffer, uv2).rgb * NdotL;
}

vec3 getRayColor() {
  vec3 current = vec3(1);
  float depth = bounceNb;
  vec3 uv = vec3(TexCoords, 0);
  vec3 result = vec3(0);
  while (true) {
    if (depth <= 0) {
      return texture(lightBuffer, TexCoords).rgb;
    }
    vec3 temp = uv;
    vec3 hitColor;
    uv = getRayHit2(temp.xy, lightPos, hitColor);
    if (uv == vec3(0)) {
      result += (current * texture(lightBuffer, temp.xy).rgb);
      return result;
    } else {
      current *= hitColor;
      depth--;
    }
  }
  return result;
}
vec3 getRayColor2() {
  vec3 uv = vec3(TexCoords, 0);
  vec3 f_r = texture(lightBuffer, TexCoords).rgb;
  vec3 L_in;
  vec3 uvNdotL1 = getRayHit2(uv.xy, lightPos, L_in);
  vec3 L_out1 = L_in + f_r; // uvNdotL1.z;
  return L_out1;
}

void main() {
  //
  FragColor = vec4(getRayColor2(), 1);
}
