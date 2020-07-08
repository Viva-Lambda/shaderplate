#version 430

// adapted from
/*
 * http://roar11.com/2015/07/screen-space-glossy-reflections/
 *
 * */

in vec2 TexCoord;
in mat4 invViewMat;         // vertex
in vec3 viewDirInViewSpace; // view direction, ie camera front

out vec4 FragColor;

// ---------------------------- uniforms -----------------------------------
// ------------------------- from other passes -----------------------------

uniform sampler2D gDepth;
uniform sampler2D lightBuffer; // convolved color buffer - all mip levels
uniform sampler2D gNormal;     // normal buffer - from g-buffer in camera space
uniform sampler2D gMaterial;   // specular buffer - from g-buffer (rgb = ior,

// -------------------------- ray tracing uniforms --------------------------
//
uniform mat4 projection;
uniform mat4 view;
uniform vec3 viewDir; // world space
uniform vec3 viewPos; // world space

// thickness to ascribe to each pixel in the depth buffer in camera space
uniform float csDepthThickness = 1.0;
uniform float csNearPlaneZ = -0.2; // the camera's near z plane

// Step in horizontal or vertical pixels between
// samples. This is a float
// because integer math is slow on GPUs, but should be set to an integer >= 1.
uniform float traceStride = 1.0;
// Maximum number of iterations. Higher gives better
// images but may be slow.
uniform float csMaxSteps = 3.0;

// Maximum camera-space distance to trace before
// returning a miss.
uniform float csMaxDistance = 10.0;

// the number of mip levels in the convolved color buffer

uniform float jitter = 0.2; // value [0, 1]

// ------------------------ cone tracing related -----------------------------

uniform float maxMipLevels = 5.0;

uniform float coneAngleZeta = 0.244;
uniform float max_shine = 256.0;
uniform float cone_trace_iteration = 18.0;
uniform float fadeStart = 0.4; // value between [0, 1]
uniform float fadeEnd = 1.0;

// ----------------------------- utility function --------------------------
const float PI = 3.14159265;

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
vec3 random_unit_vector() {
  // unit vector
  float a = random_double(0, 2 * PI);
  float z = random_double(-1, 1);
  float r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
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

vec3 fresnelSchlickRoughness(float costheta, vec3 F0, float roughness) {

  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - costheta, 5.0);
}
float lerp(float n, float n2, float f) { return n + f * (n2 - n); }

// --------------------------- Ray Tracing Code ------------------------------

/**
* utils.glsl of the supplemented code of the paper *McGuire(Morgan) et
* Mara(Michael), « Efficient GPU Screen - Space Ray Tracing »,
* Journal of Computer Graphics Techniques, vol.3, nᵒ 4(décembre 2014)
* .URL : http : // jcgt.org/published/0003/04/04/., p. 73‑85.
*/

void swap(inout float a, inout float b) {
  float temp = a;
  a = b;
  b = temp;
}

float distanceSquared(vec2 a, vec2 b) {
  a -= b;
  return dot(a, a);
}

/**
 * raytrace.glsl of the supplemented code of the paper
 * McGuire (Morgan) et Mara (Michael), « Efficient GPU Screen-Space Ray
    Tracing », Journal of Computer Graphics Techniques, vol. 3, nᵒ 4 (décembre
    2014). URL : http://jcgt.org/published/0003/04/04/., p. 73‑85.
 * */

/**
    \param csOrigin Camera-space ray origin, which must be
    within the view volume and must have z < -0.01 and project within the
   valid
   screen rectangle

    \param csDirection Unit length camera-space ray direction

    \param projection A projection matrix that maps to pixel
   coordinates (not [-1, +1] normalized device coordinates)

    \param gDepth The depth or camera-space Z buffer, depending
   on
   the value
   of \a csZBufferIsHyperbolic

    \param csDepthThickness Camera space thickness to ascribe to each pixel
   in
   the
   depth buffer


    \param csNearPlaneZ Negative number

    \param traceStride Step in horizontal or vertical pixels between
   samples.
   This is
   a float
     because integer math is slow on GPUs, but should be set to an integer
   >= 1

    \param jitter Number between 0 and 1 for how far to bump the
   ray in
   stride units
      to conceal banding artifacts

    \param csMaxDistance Maximum camera-space distance to trace before
   returning a miss

    \param hitPixel Pixel coordinates of the first intersection with the
   scene

    \param csHitPoint Camera space location of the ray hit

    Single-layer

 */

bool traceScreenSpaceRay1(vec3 csOrigin, vec3 csDirection, out vec2 hitPixel,
                          out vec3 csHitPoint) {

  // Clip ray to a near plane in 3D (doesn't have to be *the* near plane,
  // although that would be a good idea)
  float rayLength =
      ((csOrigin.z + csDirection.z * csMaxDistance) > csNearPlaneZ)
          ? (csNearPlaneZ - csOrigin.z) / csDirection.z
          : csMaxDistance;
  vec3 csEndPoint = csDirection * rayLength + csOrigin;

  // Project into screen space
  vec4 H0NDC = projection * vec4(csOrigin, 1.0);

  vec4 H0 = H0NDC / H0NDC.w;
  vec4 H1NDC = projection * vec4(csEndPoint, 1.0);
  vec4 H1 = H1NDC / H1NDC.w;

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
  vec3 Q1 = csEndPoint * k1;

  // Screen-space endpoints
  vec2 P0 = H0.xy * k0;
  vec2 P1 = H1.xy * k1;

  // Initialize to off screen
  hitPixel = vec2(-1.0, -1.0);
  int which = 0; // Only one layer

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
  dP *= traceStride;
  dQ *= traceStride;
  dk *= traceStride;

  // Offset the starting values by the jitter fraction
  P0 += dP * jitter;
  Q0 += dQ * jitter;
  k0 += dk * jitter;

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
       ((P.x * stepDirection) <= end) && (stepCount < csMaxSteps) &&
       ((rayZMax < sceneZMax - csDepthThickness) || (rayZMin > sceneZMax)) &&
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
    sceneZMax = texelFetch(gDepth, ivec2(hitPixel), 0).r;

  } // pixel on ray

  Q.xy += dQ.xy * stepCount;
  csHitPoint = Q * (1.0 / k);

  // Matches the new loop condition:
  return (rayZMax >= sceneZMax - csDepthThickness) && (rayZMin <= sceneZMax);
}

vec3 rayHitPointViewSpace = vec3(0);

float getLinearDepth(vec2 tcoord) {
  vec3 FragPos = texture(gDepth, tcoord).rgb;
  vec3 FragPosVS = vec3(view * vec4(FragPos, 1));
  vec3 viewPosVS = vec3(view * vec4(viewPos, 1));
  float linearDepth = length(viewPosVS - FragPosVS);
  return linearDepth;
}

vec4 ray_trace_screen() {
  // get normal in view space
  vec3 normalInView = vec3(view * vec4(texture(gNormal, TexCoord).rgb, 1));
  vec3 viewPosVS = vec3(view * vec4(viewPos, 1));
  float linearDepth = getLinearDepth(TexCoord);
  vec3 rayOriginInView =
      viewDirInViewSpace * linearDepth + viewPosVS; // P = O + viewDir * Depth
  vec3 normRayOrigInView = normalize(rayOriginInView);
  vec3 normRayDirBias = normalize(reflect(normRayOrigInView, normalInView));
  // biasing ray scatter direction based on surface roughness
  float roughness = texture(gMaterial, TexCoord).y;
  float kappa = 1.0 - roughness;
  vec3 rayDirection = vonmises_dir(normRayDirBias, kappa);

  // output rDotV to the alpha channel for use in determining how much to fade
  // the ray
  float rDotV = dot(rayDirection, normRayOrigInView);

  // out parameters
  vec2 hitPixel = vec2(0.0, 0.0);
  vec3 hitPoint = vec3(0.0, 0.0, 0.0);

  // perform ray tracing - true if hit found, false otherwise
  bool intersection =
      traceScreenSpaceRay1(normRayOrigInView, rayDirection, hitPixel, hitPoint);

  float depth = getLinearDepth(hitPixel);

  // move hit pixel from pixel position to UVs
  ivec2 texSize = textureSize(gDepth, 0);
  float texelWidth = float(texSize.x);
  float texelHeight = float(texSize.y);
  hitPixel *= vec2(texelWidth, texelHeight);
  if (hitPixel.x > 1.0 || hitPixel.x < 0.0 || hitPixel.y > 1.0 ||
      hitPixel.y < 0.0) {
    intersection = false;
  }
  rayHitPointViewSpace = hitPoint;

  return vec4(hitPixel, depth, rDotV) * (intersection ? 1.0f : 0.0f);
}

// --------------------------- Cone Tracing Code -----------------------------

vec3 get_fallback_color() { return texture(lightBuffer, TexCoord).rgb; }

float roughnessToSpecularPower(float roughness) {
  // from graphics rant
  // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
  // valid for phong models
  return (2 / pow(roughness, 4)) - 2;
}

float specularPowerToConeAngle(float specularPower) {
  // based on phong distribution model
  if (specularPower >= exp2(max_shine)) {
    return 0.0f;
  }
  const float xi = coneAngleZeta;
  float exponent = 1.0f / (specularPower + 1.0f);
  return acos(pow(xi, exponent));
}

float isoscelesTriangleOpposite(float adjacentLength, float coneTheta) {
  // simple trig and algebra - soh, cah, toa - tan(theta) = opp/adj, opp =
  // tan(theta) * adj, then multiply * 2.0f for isosceles triangle base
  return 2.0f * tan(coneTheta) * adjacentLength;
}

float isoscelesTriangleInRadius(float a, float h) {
  float a2 = a * a;
  float fh2 = 4.0f * h * h;
  return (a * (sqrt(a2 + fh2) - a)) / (4.0f * h);
}

vec4 coneSampleWeightedColor(vec2 samplePos, float mipChannel, float gloss) {
  vec3 sampleColor = texture(lightBuffer, samplePos, mipChannel).rgb;
  return vec4(sampleColor * gloss, gloss);
}

float isoscelesTriangleNextAdjacent(float adjacentLength,
                                    float incircleRadius) {
  // subtract the diameter of the incircle to get the adjacent side of the next
  // level on the cone
  return adjacentLength - (incircleRadius * 2.0f);
}

float get_fade_value(vec4 hitInfo, float gloss, float remainingAlpha) {
  // fade rays close to screen edge
  vec2 hitPixel = hitInfo.xy;
  vec2 boundary = abs(hitPixel - vec2(0.5f, 0.5f)) * 2.0f;
  const float fadeDiffRcp = 1.0f / (fadeEnd - fadeStart);
  float fadeOnBorder =
      1.0f - clamp((boundary.x - fadeStart) * fadeDiffRcp, 0.0, 1.0);
  fadeOnBorder *=
      1.0f - clamp((boundary.y - fadeStart) * fadeDiffRcp, 0.0, 1.0);
  fadeOnBorder = smoothstep(0.0f, 1.0f, fadeOnBorder);
  vec3 FragInViewSpace = vec3(view * vec4(texture(gDepth, TexCoord).rgb, 1));
  float fadeOnDistance =
      1.0f -
      clamp(distance(rayHitPointViewSpace, FragInViewSpace) / csMaxDistance,
            0.0, 1.0);
  // ray tracing steps stores rdotv in w component - always > 0 due to check at
  // start of this method
  float fadeOnPerpendicular =
      clamp(lerp(0.0f, 1.0f, clamp(hitInfo.w * 4.0f, 0.0, 1.0)), 0.0, 1.0);
  float fadeOnRoughness = clamp(lerp(0.0f, 1.0f, gloss * 4.0f), 0.0, 1.0);
  float totalFade = fadeOnBorder * fadeOnDistance * fadeOnPerpendicular *
                    fadeOnRoughness * (1.0f - clamp(remainingAlpha, 0.0, 1.0));
  return totalFade;
}

vec4 cone_trace() {
  // implements cone tracing
  vec4 hitInfo = ray_trace_screen();
  vec3 fallbackColor = get_fallback_color();
  if (vec4(0.0) == hitInfo) {
    return fallbackColor;
  }
  vec3 normalInView = texture(gNormal, TexCoord).rgb;
  float linearDepth = texture(gDepth, TexCoord).r;

  //
  float roughness = texture(gMaterial, TexCoord).y;
  float gloss = 1 - roughness;
  float specularPower = roughnessToSpecularPower(roughness);

  // convert to cone angle (maximum extent of the specular lobe aperture)
  // only want half the full cone angle since we're slicing the isosceles
  // triangle in half to get a right triangle
  float coneAngle = specularPowerToConeAngle(specularPower) * 0.5;

  // P1 = positionSS, P2 = raySS, adjacent length = ||P2 - P1||
  vec2 hitPixel = hitInfo.xy;
  vec4 fragInNDC =
      vec4(projection * view * vec4(texture(gDepth, TexCoord).rgb, 1));

  vec3 FragInScreenSpace = fragInNDC.xyz / fragInNDC.w;
  vec2 deltaP = hitPixel - FragInScreenSpace.xy;
  float adjacentLength = length(deltaP);
  vec2 adjacentUnit = normalize(deltaP);

  vec4 totalColor = vec4(0.0);
  float remainingAlpha = 1.0;
  float maxMipLevel = maxMipLevels - 1.0;
  float glossMult = gloss;

  // cone tracing starts
  ivec2 texSize = textureSize(gDepth, 0);
  float texelWidth = float(texSize.x);
  float texelHeight = float(texSize.y);

  // cone-tracing using an isosceles triangle to approximate a cone in screen
  // space
  //
  for (uint i = 0; i < cone_trace_iteration; i++) {
    // intersection length is the adjacent side, get the opposite side using
    // trig
    float oppositeLength = isoscelesTriangleOpposite(adjacentLength, coneAngle);

    // calculate in-radius of the isosceles triangle
    float incircleSize =
        isoscelesTriangleInRadius(oppositeLength, adjacentLength);

    // get the sample position in screen space

    vec2 samplePos =
        FragInScreenSpace.xy + adjacentUnit * (adjacentLength - incircleSize);

    // convert the in-radius into screen size then check what power N to raise 2
    // to reach it - that power N becomes mip level to sample from
    float mipChannel = clamp(log2(incircleSize * max(texelWidth, texelHeight)),
                             0.0f, maxMipLevel);

    /*
     * Read color and accumulate it using trilinear filtering and weight it.
     * Uses pre-convolved image (color buffer) and glossiness to weigh color
     * contributions.
     * Visibility is accumulated in the alpha channel. Break if visibility is
     * 100% or greater (>= 1.0f).
     */
    vec4 newColor = coneSampleWeightedColor(samplePos, mipChannel, glossMult);

    remainingAlpha -= newColor.a;
    if (remainingAlpha < 0.0f) {
      newColor.rgb *= (1.0f - abs(remainingAlpha));
    }
    totalColor += newColor;

    if (totalColor.a >= 1.0f) {
      break;
    }
    adjacentLength =
        isoscelesTriangleNextAdjacent(adjacentLength, incircleSize);
    glossMult *= gloss;
  }

  vec3 normalInWorld = texture(gNormal, TexCoord).rgb;
  vec3 viewDirInWorld = viewDir;
  // vec3 reflectInWorld = reflect(-viewDirInWorld, normalInWorld);
  float costheta = max(dot(normalInWorld, viewDirInWorld), 0.0);
  float refAtZero = texture(gMaterial, TexCoord).w;
  vec3 specular = fresnelSchlickRoughness(costheta, vec3(refAtZero), roughness);
  specular *= 1.0 / PI;

  // fading harsh edges
  float totalFade = get_fade_value(hitInfo, gloss, remainingAlpha);

  return vec4(mix(fallbackColor, totalColor.rgb * specular, totalFade), 1.0f);
}

void main() { FragColor = cone_trace(); }
