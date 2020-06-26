#version 430

/*
 * From
 * https://github.com/Global-Illuminati/Precomputed-Light-Field-Probes/blob/master/src/shaders/octahedral.glsl
 * */

float signNotZero(float f) { return (f > 0.0) ? 1 : -1; }

vec2 signNotZero(vec2 v) { return vec2(signNotZero(v.x), signNotZero(v.y)); }
vec2 size(in sampler2D tex) { return vec2(textureSize(tex, 0)); }

vec2 invSize(in sampler2D tex) { return vec2(1.0) / size(tex); }
vec3 unpackNormal(vec3 packedNormal) {
  return normalize(packedNormal * vec3(2.0) - vec3(1.0));
}

/**
 Comes with the accompanying code of light probs
  "Real-Time Global Illumination using Precomputed Light Field Probes" by M.
 McGuire, M. Mara


 Why use octahedral mapping ?
Octahedral parameterization maps an entire sphere to a square that has lower
distortion, fewer boundaries, and simpler boundaries than a cube map at
comparable resolution. This is not essential to the derivation of the
algorithm, however it gives approximately a fac- tor of four space and
bandwidth reduction at the same quality level compared to storing cube maps.
Octahedral mapping also preserves the piecewise-linear projection required for
efficient ray marching, unlike most other sphere-to-square mappings.
 */
vec2 octEncode(in vec3 v) {
  float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
  vec2 result = v.xy * (1.0 / l1norm);
  if (v.z < 0.0) {
    result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
  }
  return result;
}

/** Returns a unit vector. Argument o is an octahedral vector packed via
   octEncode,
    on the [-1, +1] square*/
vec3 octDecode(vec2 o) {
  vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
  if (v.z < 0.0) {
    v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
  }
  return normalize(v);
}

// modified from the accompanying code
uniform float minThickness = 0.03; // meters
uniform float maxThickness = 0.50; // meters

// Points exactly on the boundary in octahedral space (x = 0 and y = 0 planes)
// map to two different locations in octahedral space. We shorten the segments
// slightly to give unambigous locations that lead to intervals that lie
// within an octant.
uniform float rayBumpEpsilon = 0.001; // meters

// If we go all the way around a cell and don't move farther than this (in m)
// then we quit the trace
uniform float minProgressDistance = 0.01;

// taken directly from the paper
//  zyx bit pattern indicating which probe we're currently using within the cell
//  on [0, 7]
#define CycleIndex int

// On [0, L.probeCounts.x * L.probeCounts.y * L.probeCounts.z - 1]
#define ProbeIndex int

// probe xyz indices
#define GridCoord ivec3

// Enumerated value
#define TraceResult int
#define TRACE_RESULT_MISS 0
#define TRACE_RESULT_HIT 1
#define TRACE_RESULT_UNKNOWN 2

// some data types
#define Point3 vec3
#define Point2 vec2

struct LightFieldSurface {
  sampler2DArray radianceProbeGrid;
  sampler2DArray normalProbeGrid;
  sampler2DArray distanceProbeGrid;
  sampler2DArray lowResolutionDistanceProbeGrid;
  ivec3 probeCounts;
  Point3 probeStartPosition;
  vec3 probeStep;
  int lowResolutionDownsampleFactor;
  samplerCubeArray irradianceProbeGrid;
  samplerCubeArray meanDistProbeGrid;
};

/**
 * Squared distance between any two points*/
float squared_distance(Point2 p1, Point2 p2) {
  Point2 dist = p1 - p2;
  return dot(dist, dist);
}
float squared_distance(Point3 p1, Point3 p2) {
  Point3 dist = p1 - p2;
  return dot(dist, dist);
}

/**
 @param probeCoords Integer (stored in float) coordinates of the probe on the
 probe grid
 */
ProbeIndex gridCoordToProbeIndex(in LightFieldSurface L,
                                 in Point3 probeCoords) {
  return int(probeCoords.x + probeCoords.y * L.probeCounts.x +
             probeCoords.z * L.probeCounts.x * L.probeCounts.y);
}

/**
 * base grid coord need for
 * No manual placement is necessary and all of our results use a naı̈ve uni-
   form grid placement of the probes:
 * */
GridCoord baseGridCoord(in LightFieldSurface L, Point3 X) {
  return clamp(GridCoord((X - L.probeStartPosition) / L.probeStep),
               GridCoord(0, 0, 0),
               GridCoord(L.probeCounts) - GridCoord(1, 1, 1));
}

/** Returns the index of the probe at the floor along each dimension. */
ProbeIndex baseProbeIndex(in LightFieldSurface L, Point3 X) {
  return gridCoordToProbeIndex(L, baseGridCoord(L, X));
}

/**
 * findMSB — find the index of the most significant
 * bit set to 1 in an integer
 * >> bitwise shift operator
 * */
GridCoord probeIndexToGridCoord(in LightFieldSurface L, ProbeIndex indx) {
  // Assumes probeCounts are powers of two.
  // Precomputing the MSB actually slows this code down substantially
  ivec3 iPos;
  iPos.x = indx & (L.probeCounts.x - 1);
  iPos.y = (indx & ((L.probeCounts.x * L.probeCounts.y) - 1)) >>
           findMSB(L.probeCounts.x);
  iPos.z = indx >> findMSB(L.probeCounts.x * L.probeCounts.y);

  return iPos;
}

/**
 * Transforms probe index to color
 * */
vec3 probeIndexToColor(in LightFieldSurface L, ProbeIndex indx) {
  return gridCoordToColor(probeIndexToGridCoord(L, indx));
}

/**
 *
 * probeCoords Coordinates of the probe, computed as part of the process.
 * obtain the index of the nearest probe
 * @param L the LightProbe structure
 * @param X a point */
ProbeIndex nearestProbeIndex(in LightFieldSurface L, Point3 X,
                             out point3 probeCoords) {
  probeCoords = clamp(round((X - L.probeStartPosition) / L.probeStep),
                      Point3(0, 0, 0), Point3(L.probeCounts) - Point3(1, 1, 1));

  return gridCoordToProbeIndex(L, probeCoords);
}

/**
    We can define a simple and efficient procedural iteration sequence that
    respects this selection heuristic (Figure 3). Without loss of gen- erality,
    assume that we translate and scale the scene (including the ray) to place
   the
    eight probes within the 1 m 3 cube with the origin at the lower corner. The
    index of the probe with the smallest x-, y-, and z- coordinates is br o c,
    and the relative indices of adjacent probes 0 ≤ i < 8 are br o c+(i mod 2,
    bi/2c mod 2, bi/4c mod 2).

    An exhaustive search of the permutations yields
    an optimal eight- element probe index selection sequence: namely, one that
    consis- tently moves to the farthest probe (without revisiting a probe). The
    sequence, starting at the relative index i closest to the ray, is given by i
    0 = (i + 3) mod 8. This selection sequence can be efficiently evaluated with
    bitwise operations.

    @param neighbors The 8 probes surrounding X
    @return Index into the neighbors array of the index of the nearest probe to
   X
*/
CycleIndex nearestProbeIndices(in LightFieldSurface L, Point3 X) {
  Point3 maxProbeCoords = Point3(L.probeCounts) - Point3(1, 1, 1);
  Point3 floatProbeCoords = (X - L.probeStartPosition) / L.probeStep;
  Point3 baseProbeCoords =
      clamp(floor(floatProbeCoords), Point3(0, 0, 0), maxProbeCoords);

  float minDist = 10.0f;
  int nearestIndex = -1;

  for (int i = 0; i < 8; ++i) {
    Point3 newProbeCoords =
        min(baseProbeCoords + vec3(i & 1, (i >> 1) & 1, (i >> 2) & 1),
            maxProbeCoords);
    float d = length(newProbeCoords - floatProbeCoords);
    if (d < minDist) {
      minDist = d;
      nearestIndex = i;
    }
  }

  return nearestIndex;
}

Point3 gridCoordToPosition(in LightFieldSurface L, GridCoord c) {
  return L.probeStep * Vector3(c) + L.probeStartPosition;
}
Point3 gridCoordToPosition(in LightFieldSurface L, GridCoord c) {
  return L.probeStep * Vector3(c) + L.probeStartPosition;
}

Point3 probeLocation(in LightFieldSurface L, ProbeIndex index) {
  return gridCoordToPosition(L, probeIndexToGridCoord(L, index));
}

/** GLSL's dot on ivec3 returns a float. This is an all-integer version */
int idot(ivec3 a, ivec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/**
   \param baseProbeIndex Index into L.radianceProbeGrid's TEXTURE_2D_ARRAY. This
   is the probe
   at the floor of the current ray sampling position.

   \param relativeIndex on [0, 7]. This is used as a set of three 1-bit offsets

   Returns a probe index into L.radianceProbeGrid. It may be the *same* index as
   baseProbeIndex.

   This will wrap when the camera is outside of the probe field probes...but
   that's OK.
   If that case arises, then the trace is likely to
   be poor quality anyway. Regardless, this function will still return the index
   of some valid probe, and that probe can either be used or fail because it
   does not
   have visibility to the location desired.

   \see nextCycleIndex, baseProbeIndex
 */
ProbeIndex relativeProbeIndex(in LightFieldSurface L, ProbeIndex baseProbeIndex,
                              CycleIndex relativeIndex) {
  // Guaranteed to be a power of 2
  ProbeIndex numProbes = L.probeCounts.x * L.probeCounts.y * L.probeCounts.z;

  ivec3 offset = ivec3(relativeIndex & 1, (relativeIndex >> 1) & 1,
                       (relativeIndex >> 2) & 1);
  ivec3 stride = ivec3(1, L.probeCounts.x, L.probeCounts.x * L.probeCounts.y);

  return (baseProbeIndex + idot(offset, stride)) & (numProbes - 1);
}

/** Given a CycleIndex [0, 7] on a cube of probes, returns the next CycleIndex
   to use.
    \see relativeProbeIndex
*/
CycleIndex nextCycleIndex(CycleIndex cycleIndex) {
  return (cycleIndex + 3) & 7;
}

float lengthSquared(Vector3 v) { return dot(v, v); }

/** Two-element sort: maybe swaps a and b so that a' = min(a, b), b' = max(a,
 * b). */
void minSwap(inout float a, inout float b) {
  float temp = min(a, b);
  b = max(a, b);
  a = temp;
}

/** Sort the three values in v from least to
    greatest using an exchange network (i.e., no branches) */
void sort(inout float3 v) {
  minSwap(v[0], v[1]);
  minSwap(v[1], v[2]);
  minSwap(v[0], v[1]);
}

/** Segments a ray into the piecewise-continuous rays or line segments that
    each lie within one Euclidean octant, which correspond to piecewise-linear
    projections in octahedral space.

    @param boundaryT  all boundary distance ("time") values in units of
    world-space distance
      along the ray. In the (common) case where not all five elements are
    needed, the unused
      values are all equal to tMax, creating degenerate ray segments.

    @param origin Ray origin in the Euclidean object space of the probe

    @param directionFrac 1 / ray.direction
 */
void computeRaySegments(in Point3 origin, in Vector3 directionFrac,
                        in float tMin, in float tMax, out float boundaryTs[5]) {

  boundaryTs[0] = tMin;

  // Time values for intersection with x = 0, y = 0, and z = 0 planes, sorted
  // in increasing order
  Vector3 t = origin * -directionFrac;
  sort(t);

  // Copy the values into the interval boundaries.
  // This loop expands at compile time and eliminates the
  // relative indexing, so it is just three conditional move operations
  for (int i = 0; i < 3; ++i) {
    boundaryTs[i + 1] = clamp(t[i], tMin, tMax);
  }

  boundaryTs[4] = tMax;
}

struct Ray {
  vec3 origin, direction;
};

/** Returns the distance along v from the origin to the intersection
    with ray R (which it is assumed to intersect) */
float distanceToIntersection(in Ray R, in Vector3 v) {
  float numer;
  float denom = v.y * R.direction.z - v.z * R.direction.y;

  if (abs(denom) > 0.1) {
    numer = R.origin.y * R.direction.z - R.origin.z * R.direction.y;
  } else {
    // We're in the yz plane; use another one
    numer = R.origin.x * R.direction.y - R.origin.y * R.direction.x;
    denom = v.x * R.direction.y - v.y * R.direction.x;
  }

  return numer / denom;
}

/**
 * maximum of a vector component wise*/
float maxComp(vec4 v) { return max(max(max(v.x, v.y), v.z), v.w); }
int maxComp(ivec4 v) { return max(max(max(v.x, v.y), v.z), v.w); }
uint maxComp(uvec4 v) { return max(max(max(v.x, v.y), v.z), v.w); }

float maxComp(vec3 v) { return max(max(v.x, v.y), v.z); }
int maxComp(ivec3 v) { return max(max(v.x, v.y), v.z); }
uint maxComp(uvec3 v) { return max(max(v.x, v.y), v.z); }

float maxComp(vec2 v) { return max(v.x, v.y); }
int maxComp(ivec2 v) { return max(v.x, v.y); }
uint maxComp(uvec2 v) { return max(v.x, v.y); }

/**
 * To trace a ray within a single probe, we compute the 2D polyline endpoints
   and optimally march along the polyline in the octahedral map. Intersection
   tests are computed at each texel by depth testing the polyline point against
   the radial depthmap value stored in the probe at the texel: when the stored
   radial depth of the scene is less than the radial distance from the probe’s
   center to the 3D polyline at the currently marched polyline texel, the ray
   has either hit a surface or passed behind the surface.

  On a TRACE_RESULT_MISS, bumps the endTexCoord slightly so that the next
  segment will start at the
  right place. We do that in the high res trace because
  the ray direction variables are already available here.

  TRACE_RESULT_HIT:      This probe guarantees there IS a surface on this
  segment
  TRACE_RESULT_MISS:     This probe guarantees there IS NOT a surface on this
  segment
  TRACE_RESULT_UNKNOWN:  This probe can't provide any information
*/
TraceResult highResolutionTraceOneRaySegment(
    in LightFieldSurface lightFieldSurface, in Ray probeSpaceRay,
    in Point2 startTexCoord, in Point2 endTexCoord, in ProbeIndex probeIndex,
    inout float tMin, inout float tMax, inout vec2 hitProbeTexCoord) {

  // compute texture coord distance
  Vector2 texCoordDelta = endTexCoord - startTexCoord;
  float texCoordDistance = length(texCoordDelta);

  // normalize the segment to obtain a direction
  Vector2 texCoordDirection = texCoordDelta * (1.0 / texCoordDistance);

  //
  ivec3 igridSize = textureSize(lightFieldSurface.distanceProbeGrid);
  vec3 invGridSize = 1.0 / igridSize;
  float texCoordStep =
      invGridSize.x * (texCoordDistance / maxComp(abs(texCoordDelta)));

  Vector3 directionFromProbeBefore = octDecode(startTexCoord * 2.0 - 1.0);
  float distanceFromProbeToRayBefore =
      max(0.0, distanceToIntersection(probeSpaceRay, directionFromProbeBefore));

  // Special case for singularity of probe on ray
  if (false) {
    float cosTheta = dot(directionFromProbeBefore, probeSpaceRay.direction);
    if (abs(cosTheta) > 0.9999) {
      // Check if the ray is going in the same direction as a ray from the probe
      // through the start texel
      if (cosTheta > 0) {
        // If so, return a hit
        float distanceFromProbeToSurface =
            texelFetch(lightFieldSurface.distanceProbeGrid,
                       ivec3(igridSize.xy * startTexCoord, probeIndex), 0)
                .r;
        tMax = length(probeSpaceRay.origin -
                      directionFromProbeBefore * distanceFromProbeToSurface);
        hitProbeTexCoord = startTexCoord;
        return TRACE_RESULT_HIT;
      } else {
        // If it is going in the opposite direction, we're never going to find
        // anything useful, so return false
        return TRACE_RESULT_UNKNOWN;
      }
    }
  }

  // iterate over texture coordinates
  for (float d = 0.0; d <= texCoordDistance; d += texCoordStep) {
    Point2 texCoord =
        (texCoordDirection * min(d + texCoordStep * 0.5, texCoordDistance)) +
        startTexCoord;

    // Fetch the probe data
    float distanceFromProbeToSurface =
        texelFetch(lightFieldSurface.distanceProbeGrid,
                   ivec3(igridSize.xy * texCoord, probeIndex), 0)
            .r;

    // Find the corresponding point in probe space. This defines a line
    // through the probe origin
    Vector3 directionFromProbe = octDecode(texCoord * 2.0 - 1.0);

    // using the implicit equation Point = Origin + Distance * Direction
    Point2 texCoordAfter =
        (texCoordDirection * min(d + texCoordStep, texCoordDistance)) +
        startTexCoord;

    Vector3 directionFromProbeAfter = octDecode(texCoordAfter * 2.0 - 1.0);
    float distanceFromProbeToRayAfter = max(
        0.0, distanceToIntersection(probeSpaceRay, directionFromProbeAfter));
    float maxDistFromProbeToRay =
        max(distanceFromProbeToRayBefore, distanceFromProbeToRayAfter);

    if (maxDistFromProbeToRay >= distanceFromProbeToSurface) {
      // At least a one-sided hit; see if the ray actually passed through the
      // surface, or was behind it

      float minDistFromProbeToRay =
          min(distanceFromProbeToRayBefore, distanceFromProbeToRayAfter);

      // Find the 3D point *on the trace ray* that corresponds to the tex coord.
      // This is the intersection of the ray out of the probe origin with the
      // trace ray.
      float distanceFromProbeToRay =
          (minDistFromProbeToRay + maxDistFromProbeToRay) * 0.5;

      // Use probe information
      Point3 probeSpaceHitPoint =
          distanceFromProbeToSurface * directionFromProbe;
      float distAlongRay = dot(probeSpaceHitPoint - probeSpaceRay.origin,
                               probeSpaceRay.direction);

      // Read the normal for use in detecting backfaces
      vec2 normalProbeData =
          texelFetch(lightFieldSurface.normalProbeGrid,
                     ivec3(igridSize.xy * texCoord, probeIndex), 0)
              .xy;
      vec3 normal = vec3(0.0);
      if (lengthSquared(normalProbeData) > 0.0001) {
        normal = unpackNormal(packedNormal);
      }

      // Only extrude towards and away from the view ray, not perpendicular to
      // it

      // Don't allow extrusion TOWARDS the viewer, only away
      // Alignment of probe and view ray
      float alignmentOfProbeViewRay =
          max(dot(probeSpaceRay.direction, directionFromProbe), 0.0);

      // Alignment of probe and normal (glancing surfaces are assumed to
      // be thicker because they extend into the pixel)
      float alignmentOfProbeNormal =
          (2 - abs(dot(probeSpaceRay.direction, normal)));

      float thicknessDiff = (maxThickness - minThickness);

      // Scale with distance along the ray
      float scaledDistance = clamp(distAlongRay * 0.1, 0.05, 1.0);

      float surfaceThickness = minThickness +
                               thicknessDiff * alignmentOfProbeViewRay *
                                   alignmentOfProbeNormal * scaledDistance;

      float cosThetaOfRay = dot(normal, probeSpaceRay.direction);

      if ((minDistFromProbeToRay <
           distanceFromProbeToSurface + surfaceThickness) &&
          (cosThetaOfRay < 0)) {
        // Two-sided hit
        // Use the probe's measure of the point instead of the ray distance,
        // since
        // the probe is more accurate (floating point precision vs. ray march
        // iteration/oct resolution)
        tMax = distAlongRay;
        hitProbeTexCoord = texCoord;

        return TRACE_RESULT_HIT;
      } else {
        // "Unknown" case. The ray passed completely behind a surface. This
        // should trigger moving to another
        // probe and is distinguished from "I successfully traced to infinity"

        // Back up conservatively so that we don't set tMin too large
        // using Point = Origin + Direction * Distance
        Point3 probeSpaceHitPointBefore =
            distanceFromProbeToRayBefore * directionFromProbeBefore;
        float distAlongRayBefore =
            dot(probeSpaceHitPointBefore - probeSpaceRay.origin,
                probeSpaceRay.direction);

        // Max in order to disallow backing up along the ray (say if beginning
        // of this texel is before tMin from probe switch)
        // distAlongRayBefore in order to prevent overstepping
        // min because sometimes distAlongRayBefore > distAlongRay
        tMin = max(tMin, min(distAlongRay, distAlongRayBefore));

        return TRACE_RESULT_UNKNOWN;
      }
    }
    distanceFromProbeToRayBefore = distanceFromProbeToRayAfter;
  } // ray march

  return TRACE_RESULT_MISS;
}

/** Returns true on a conservative hit, false on a guaranteed miss.
    On a hit, advances lowResTexCoord to the next low res texel *after*
    the one that produced the hit.

    The texture coordinates are not texel centers...they are sub-texel
    positions true to the actual ray. This allows chopping up the ray
    without distorting it.

    segmentEndTexCoord is the coordinate of the endpoint of the entire segment
   of the ray

    texCoord is the start coordinate of the segment crossing
    the low-res texel that produced the conservative hit, if the function
    returns true.  endHighResTexCoord is the end coordinate of that
    segment...which is also the start of the NEXT low-res texel to cross
    when resuming the low res trace.

  */
bool lowResolutionTraceOneSegment(in LightFieldSurface lightFieldSurface,
                                  in Ray probeSpaceRay,
                                  in ProbeIndex probeIndex,
                                  inout Point2 texCoord,
                                  in Point2 segmentEndTexCoord,
                                  inout Point2 endHighResTexCoord) {

  Vector2 lowResSize = lightFieldSurface.lowResolutionDistanceProbeGrid.size.xy;
  Vector2 lowResInvSize =
      lightFieldSurface.lowResolutionDistanceProbeGrid.invSize.xy;

  // Convert the texels to pixel coordinates:
  Point2 P0 = texCoord * lowResSize;
  Point2 P1 = segmentEndTexCoord * lowResSize;

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
  // In pixel coordinates
  Vector2 delta = P1 - P0;

  // Permute so that the primary iteration is in x to reduce
  // large branches later
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
  Vector2 dP = vec2(stepDir, delta.y * invdx);

  Vector3 initialDirectionFromProbe = octDecode(texCoord * 2.0 - 1.0);
  float prevRadialDistMaxEstimate = max(
      0.0, distanceToIntersection(probeSpaceRay, initialDirectionFromProbe));
  // Slide P from P0 to P1
  float end = P1.x * stepDir;

  float absInvdPY = 1.0 / abs(dP.y);

  // Don't ever move farther from texCoord than this distance, in texture space,
  // because you'll move past the end of the segment and into a different
  // projection
  float maxTexCoordDistance = lengthSquared(segmentEndTexCoord - texCoord);

  for (Point2 P = P0; ((P.x * sign(delta.x)) <= end);) {

    Point2 hitPixel = permute ? P.yx : P;

    float sceneRadialDistMin =
        texelFetch(lightFieldSurface.lowResolutionDistanceProbeGrid,
                   ivec3(hitPixel, probeIndex), 0)
            .r;

    // Distance along each axis to the edge of the low-res texel
    Vector2 intersectionPixelDistance =
        (sign(delta) * 0.5 + 0.5) - sign(delta) * fract(P);

    // abs(dP.x) is 1.0, so we skip that division
    // If we are parallel to the minor axis, the second parameter will be inf,
    // which is fine
    float rayDistanceToNextPixelEdge = min(
        intersectionPixelDistance.x, intersectionPixelDistance.y * absInvdPY);

    // The exit coordinate for the ray (this may be *past* the end of the
    // segment, but the
    // callr will handle that)
    endHighResTexCoord = (P + dP * rayDistanceToNextPixelEdge) * lowResInvSize;
    endHighResTexCoord = permute ? endHighResTexCoord.yx : endHighResTexCoord;

    if (lengthSquared(endHighResTexCoord - texCoord) > maxTexCoordDistance) {
      // Clamp the ray to the segment, because if we cross a segment boundary in
      // oct space
      // then we bend the ray in probe and world space.
      endHighResTexCoord = segmentEndTexCoord;
    }

    // Find the 3D point *on the trace ray* that corresponds to the tex coord.
    // This is the intersection of the ray out of the probe origin with the
    // trace ray.
    Vector3 directionFromProbe = octDecode(endHighResTexCoord * 2.0 - 1.0);
    float distanceFromProbeToRay =
        max(0.0, distanceToIntersection(probeSpaceRay, directionFromProbe));

    float maxRadialRayDistance =
        max(distanceFromProbeToRay, prevRadialDistMaxEstimate);
    prevRadialDistMaxEstimate = distanceFromProbeToRay;

    if (sceneRadialDistMin < maxRadialRayDistance) {
      // A conservative hit.
      //
      //  -  endHighResTexCoord is already where the ray would have LEFT the
      //  texel
      //     that created the hit.
      //
      //  -  texCoord should be where the ray entered the texel
      texCoord = (permute ? P.yx : P) * lowResInvSize;
      return true;
    }

    // Ensure that we step just past the boundary, so that we're slightly inside
    // the next
    // texel, rather than at the boundary and randomly rounding one way or the
    // other.
    const float epsilon = 0.001; // pixels
    P += dP * (rayDistanceToNextPixelEdge + epsilon);
  } // for each pixel on ray

  // If exited the loop, then we went *past* the end of the segment, so back up
  // to it (in practice, this is ignored
  // by the caller because it indicates a miss for the whole segment)
  texCoord = segmentEndTexCoord;

  return false;
}

TraceResult traceOneRaySegment(in LightFieldSurface lightFieldSurface,
                               in Ray probeSpaceRay, in float t0, in float t1,
                               in ProbeIndex probeIndex,
                               inout float tMin, // out only
                               inout float tMax, inout vec2 hitProbeTexCoord) {

  // Euclidean probe-space line segment, composed of two points on the
  // probeSpaceRay
  Vector3 probeSpaceStartPoint =
      probeSpaceRay.origin + probeSpaceRay.direction * (t0 + rayBumpEpsilon);
  Vector3 probeSpaceEndPoint =
      probeSpaceRay.origin + probeSpaceRay.direction * (t1 - rayBumpEpsilon);

  // If the original ray origin is really close to the probe origin, then
  // probeSpaceStartPoint will be close to zero
  // and we get NaN when we normalize it. One common case where this can happen
  // is when the camera is at the probe
  // center. (The end point is also potentially problematic, but the chances of
  // the end landing exactly on a probe
  // are relatively low.) We only need the *direction* to the start point, and
  // using probeSpaceRay.direction
  // is safe in that case.
  if (squaredLength(probeSpaceStartPoint) < 0.001) {
    probeSpaceStartPoint = probeSpaceRay.direction;
  }

  // Corresponding octahedral ([-1, +1]^2) space line segment.
  // Because the points are in probe space, we don't have to subtract off the
  // probe's origin
  Point2 startOctCoord = octEncode(normalize(probeSpaceStartPoint));
  Point2 endOctCoord = octEncode(normalize(probeSpaceEndPoint));

  // Texture coordinates on [0, 1]
  Point2 texCoord = startOctCoord * 0.5 + 0.5;
  Point2 segmentEndTexCoord = endOctCoord * 0.5 + 0.5;

  while (true) {
    Point2 endTexCoord;

    // Trace low resolution, min probe until we:
    // - reach the end of the segment (return "miss" from the whole function)
    // - "hit" the surface (invoke high-resolution refinement, and then iterate
    // if *that* misses)

    // If lowResolutionTraceOneSegment conservatively "hits", it will set
    // texCoord and endTexCoord to be the high-resolution texture coordinates.
    // of the intersection between the low-resolution texel that was hit and the
    // ray segment.
    Vector2 originalStartCoord = texCoord;
    if (!lowResolutionTraceOneSegment(lightFieldSurface, probeSpaceRay,
                                      probeIndex, texCoord, segmentEndTexCoord,
                                      endTexCoord)) {
      // The whole trace failed to hit anything
      return TRACE_RESULT_MISS;
    } else {

      // The low-resolution trace already guaranted that endTexCoord is no
      // farther along the ray than segmentEndTexCoord if this point is reached,
      // so we don't need to clamp to the segment length
      TraceResult result = highResolutionTraceOneRaySegment(
          lightFieldSurface, probeSpaceRay, texCoord, endTexCoord, probeIndex,
          tMin, tMax, hitProbeTexCoord);

      if (result != TRACE_RESULT_MISS) {
        // High-resolution hit or went behind something, which must be the
        // result for the whole segment trace
        return result;
      }
    } // else...continue the outer loop; we conservatively refined and didn't
      // actually find a hit

    // Recompute each time around the loop to avoid increasing the peak register
    // count
    Vector2 texCoordRayDirection = normalize(segmentEndTexCoord - texCoord);

    if (dot(texCoordRayDirection, segmentEndTexCoord - endTexCoord) <=
        lightFieldSurface.distanceProbeGrid.invSize.x) {
      // The high resolution trace reached the end of the segment; we've failed
      // to find a hit
      return TRACE_RESULT_MISS;
    } else {
      // We made it to the end of the low-resolution texel using the
      // high-resolution trace, so that's
      // the starting point for the next low-resolution trace. Bump the ray to
      // guarantee that we advance
      // instead of getting stuck back on the low-res texel we just
      // verified...but, if that fails on the
      // very first texel, we'll want to restart the high-res trace exactly
      // where we left off, so
      // don't bump by an entire high-res texel
      texCoord = endTexCoord +
                 texCoordRayDirection *
                     lightFieldSurface.distanceProbeGrid.invSize.x * 0.1;
    }
  } // while low-resolution trace

  // Reached the end of the segment
  return TRACE_RESULT_MISS;
}

/**
   @param tMax On call, the stop distance for the trace. On return, the distance
        to the new hit, if one was found. Always finite.
  @param tMin On call, the start distance for the trace. On return, the start
  distance
        of the ray right before the first "unknown" step.
  @param hitProbeTexCoord Written to only on a hit
  @param index probe index
 */
TraceResult traceOneProbeOct(in LightFieldSurface lightFieldSurface,
                             in ProbeIndex index, in Ray worldSpaceRay,
                             inout float tMin, inout float tMax,
                             inout vec2 hitProbeTexCoord) {
  // How short of a ray segment is not worth tracing?
  const float degenerateEpsilon = 0.001; // meters

  Point3 probeOrigin = probeLocation(lightFieldSurface, index);

  Ray probeSpaceRay;
  probeSpaceRay.origin = worldSpaceRay.origin - probeOrigin;
  probeSpaceRay.direction = worldSpaceRay.direction;

  // Maximum of 5 boundary points when projecting ray onto octahedral map;
  // ray origin, ray end, intersection with each of the XYZ planes.
  float boundaryTs[5];
  computeRaySegments(probeSpaceRay.origin,
                     Vector3(1.0) / probeSpaceRay.direction, tMin, tMax,
                     boundaryTs);

  // for each open interval (t[i], t[i + 1]) that is not degenerate
  for (int i = 0; i < 4; ++i) {
    if (abs(boundaryTs[i] - boundaryTs[i + 1]) >= degenerateEpsilon) {
      TraceResult result = traceOneRaySegment(
          lightFieldSurface, probeSpaceRay, boundaryTs[i], boundaryTs[i + 1],
          index, tMin, tMax, hitProbeTexCoord);

      switch (result) {
      case TRACE_RESULT_HIT:
        // Hit!
        return TRACE_RESULT_HIT;

      case TRACE_RESULT_UNKNOWN:
        // Failed to find anything conclusive
        return TRACE_RESULT_UNKNOWN;
      } // switch
    }   // if
  }     // For each segment

  return TRACE_RESULT_MISS;
}

/** Traces a ray against the full lightfield.
    Returns true on a hit and updates \a tMax if there is a ray hit before \a
   tMax.
   Otherwise returns false and leaves tMax unmodified

   \param hitProbeTexCoord on [0, 1]

   \param fillHoles If true, this function MUST return a hit even if it is
   forced to use a coarse approximation
 */
bool trace(LightFieldSurface lightFieldSurface, Ray worldSpaceRay,
           inout float tMax, out Point2 hitProbeTexCoord,
           out ProbeIndex hitProbeIndex, const bool fillHoles) {

  hitProbeIndex = -1;

  int i = nearestProbeIndices(lightFieldSurface, worldSpaceRay.origin);
  int probesLeft = 8;
  float tMin = 0.0f;
  while (probesLeft > 0) {
    TraceResult result = traceOneProbeOct(
        lightFieldSurface, relativeProbeIndex(lightFieldSurface, baseIndex, i),
        worldSpaceRay, tMin, tMax, hitProbeTexCoord);
    if (result == TRACE_RESULT_UNKNOWN) {
      i = nextCycleIndex(i);
      --probesLeft;
    } else {
      if (result == TRACE_RESULT_HIT) {
        hitProbeIndex = relativeProbeIndex(lightFieldSurface, baseIndex, i);
      }
      // Found the hit point
      break;
    }
  }

  if ((hitProbeIndex == -1) && fillHoles) {
    // No probe found a solution, so force some backup plan
    Point3 ignore;
    hitProbeIndex =
        nearestProbeIndex(lightFieldSurface, worldSpaceRay.origin, ignore);
    hitProbeTexCoord = octEncode(worldSpaceRay.direction) * 0.5 + 0.5;

    float probeDistance =
        texelFetch(lightFieldSurface.distanceProbeGrid.sampler,
                   ivec3(ivec2(hitProbeTexCoord *
                               lightFieldSurface.distanceProbeGrid.size.xy),
                         hitProbeIndex),
                   0)
            .r;
    if (probeDistance < 10000) {
      Point3 hitLocation = probeLocation(lightFieldSurface, hitProbeIndex) +
                           worldSpaceRay.direction * probeDistance;
      tMax = length(worldSpaceRay.origin - hitLocation);
      return true;
    }
  }

  return (hitProbeIndex != -1);
}
