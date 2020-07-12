#version 430

// adapted from
/*
 *https://github.com/LukasBanana/ForkENGINE/blob/master/shaders/SSCTReflectionPixelShader.glsl
 * */

in vec2 TexCoord;
in mat4 invViewMat;         // vertex
in vec3 viewDirInViewSpace; // view direction, ie camera front

out vec4 FragColor;

// ---------------------------- uniforms -----------------------------------
// ------------------------- from other passes -----------------------------

uniform sampler2D gDepth;      // depth in viewspace
uniform sampler2D gSDepth;     // depth in screen space
uniform sampler2D lightBuffer; // convolved color buffer - all mip levels
uniform sampler2D gNormal;     // normal buffer - from g-buffer in camera space
uniform sampler2D gMaterial;   // specular buffer - from g-buffer (rgb = ior,
uniform sampler2D visibilityBuffer;
uniform sampler2D HizBuffer;

uniform int mipCount; // total mipmap count for hiz buffer
uniform mat4 projection;
uniform mat4 view;
uniform vec2 nearFar; // near and far plane for viewing frustum
uniform vec3 viewPos; // world space

uniform float hiz_start_level = 2.0;
uniform float hiz_stride = 2.0;
uniform float max_iterations = 32.0;

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
vec2 getCellCount(float level) { return vec2(textureSize(HizBuffer, level)); }

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

  vec2 crossStep, crossOffset;
  vec2 crossEpsilon = 0.5 / vec2(HizSize);

  crossStep.x = (v_ss.x >= 0.0) ? 1.0 : −1.0;
  crossStep.y = (v_ss.y >= 0.0) ? 1.0 : −1.0;
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
      intersectCellBoundary(r, rayCell, crossStep, crossOffset);

  Ray r2;
  r2.origin = intersectionPoint;
  r2.direction = r.direction;

  float stop_level = 0.0;
  float iterCount = 0.0;

  // starting level for traversal

  while (level >= stop_level && iterCount < max_iterations) {
    //
    float minz = textureLod(HizBuffer, r2.origin.xy, level).r;
    float maxz = textureLod(HizBuffer, r2.origin.xy, level).g;

    const vec2 levelSize = getCellCount(level);

    // get cell minimum depth plane
    const vec2 oldCell = getCell(r2.origin.xy, levelSize);

    Ray temp;
    vec3 tempOrigin = getPointOnRay(r2, max(ray.z, minz));
    temp.origin = tempOrigin;
    temp.direction = r2.direction;

    // get cell boundary plane
    const vec2 newCell = getCell(tempRay.xy, levelSize);

    // get closest of both planes + intersect with closest plane
    bool isCellBoundaryCrossed = crossedCellBoundary(oldCell, newCell);

    if (isCellBoundaryCrossed) {
      vec3 intersectionPoint =
          intersectCellBoundary(r2, oldCell, levelSize, crossStep, crossOffset);
      level = min(float(mipCount), level + 2.0);
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
// ------------------------------ Cone Tracing -------------------------------



/**
 * Implementing HiZ cone tracing from Gpu pro 5 Y. Uludag
 * */
vec3 hiz_cone_trace(vec2 hitPoint) {
  //
}

vec3 getViewDir(vec3 FragPosVS) {
  return normalize(FragPosVS - vec3(view * vec4(viewPos)));
}
vec3 getP2Ss() {
  vec3 p_vs = texture(gDepth, TexCoord).xyz;
  vec3 n_vs = texture(gNormal, TexCoord).xyz;
  vec3 v_vs = getViewDir(p_vs);
  vec3 p_cs = vec3(projection * vec4(p_vs + reflect(v_vs, n_vs), 1));
  vec3 p2_ss = p_cs / p_cs.w * vec2(0.5, -0.5) + 0.5;
  return p2_ss;
}

void main() {
  // check if point is valid
  float validCheck = texture(gSDepth, TexCoord).w;
  if (validCheck != 0.2) {
    // valid points are marked with 0.2 value for their screen space depth.
    // all that pass from gbuffer have this value, all others do not.
    FragColor = texture(lightBuffer, TexCoord);
  } else {
    // first p'_ss for finding v. p. 163 of gpu gems 5 y. uludagli
    vec3 p_ss = texture(gSDepth, TexCoord).xyz;
    vec3 p2_ss = getP2Ss();
    vec3 v_ss = p2_ss - p_ss;

    vec3 ray_hitpoint = hiz_trace(p_ss, v_ss);
    vec3 ray_color = hiz_cone_trace(ray_hitpoint.xy);
  }
}
