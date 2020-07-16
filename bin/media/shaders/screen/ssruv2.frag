// Algine SSR fragment shader
// from
// https://gitlab.com/congard/algine/-/blob/master/resources/shaders/SSR.frag.glsl

#version 430 core

layout(binding = 0) uniform sampler2D gPosition;   // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;     // normals in viewspace
layout(binding = 2) uniform sampler2D lightBuffer; // normals in viewspace
layout(binding = 3) uniform sampler2D gMaterial;   // normals in viewspace
uniform mat4 projection, view;

uniform int binarySearchCount = 130;
uniform int rayMarchCount = 10; // 60
uniform float rayStep = 0.025;  // 0.025
uniform float minRayStep = 0.1;
uniform vec3 lightPos;  // in view space
uniform vec3 cameraPos; // in world space
uniform float depthEpsilon = 0.001;
uniform float thickness = 1.2;

in vec2 TexCoords;

layout(location = 0) out vec4 fragColor;
float PI = 3.1415926535;

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

// SSR based on tutorial by Imanol Fotia
// http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html

vec3 getPosition(vec2 TexCoords) { return texture(gPosition, TexCoords).xyz; }
vec3 getPositionW(vec2 TexCoords) {
  return vec3(inverse(view) * texture(gPosition, TexCoords));
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
vec3 worldToTextureSpace(vec3 p_ws) {
  vec4 p_cs = projection * view * vec4(p_ws, 1);
  p_cs /= p_cs.w;
  vec3 p_ts = p_cs.xyz;
  p_ts.xy *= 0.5;
  p_ts.xy += 0.5;
  // invert y for opengl
  // p_ts.y = -p_ts.y;
  return p_ts;
}
vec2 binary_search(inout vec3 dir,      // world space
                   inout vec3 hitCoord, // world space
                   inout float dDepth) {
  float depth;

  vec2 projectedCoord;

  for (int i = 0; i < binarySearchCount; i++) {
    projectedCoord = worldToTextureSpace(hitCoord).xy;

    vec3 wpos = getPositionW(projectedCoord.xy); // view space depth

    dDepth = distance(hitCoord, wpos); // view space depth difference

    // dir *= 0.5;
    dir *= minRayStep;
    if (dDepth > 0.0)
      hitCoord += dir;
    else
      hitCoord -= dir;
  }

  projectedCoord = worldToTextureSpace(hitCoord).xy;

  return vec2(projectedCoord.xy);
}
vec2 ray_march(vec3 dir,            // in world space
               float viewDepth,     // in world space
               inout vec3 hitCoord, // in world space as well
               out float dDepth) {
  dir *= rayStep;

  for (int i = 0; i < rayMarchCount; i++) {
    hitCoord += dir;

    vec2 projectedCoord = worldToTextureSpace(hitCoord).xy;
    vec3 wpos = getPositionW(projectedCoord);
    dDepth = distance(hitCoord, wpos);

    if ((viewDepth - dDepth) < 1.2 && dDepth <= 0.0)
      return binary_search(dir, hitCoord, dDepth);
  }

  return vec2(-1.0);
}

vec2 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth) {
  float depth;

  vec2 projectedCoord;

  for (int i = 0; i < binarySearchCount; i++) {
    projectedCoord = viewToTextureSpace(hitCoord).xy;

    depth = getPosition(projectedCoord.xy).z; // view space depth

    dDepth = hitCoord.z - depth; // view space depth difference

    dir *= 0.5;
    if (dDepth > 0.0)
      hitCoord += dir;
    else
      hitCoord -= dir;
  }

  projectedCoord = viewToTextureSpace(hitCoord).xy;

  return vec2(projectedCoord.xy);
}

vec2 rayCast(vec3 dir,            // in view space
             inout vec3 hitCoord, // in view space as well
             out float dDepth) {
  dir *= rayStep;

  for (int i = 0; i < rayMarchCount; i++) {
    hitCoord += dir;

    vec2 projectedCoord = viewToTextureSpace(hitCoord).xy;
    float depth = getPosition(projectedCoord).z;
    dDepth = hitCoord.z - depth;

    if ((dir.z - dDepth) < thickness && dDepth <= 0.0)
      return binarySearch(dir, hitCoord, dDepth);
  }

  return vec2(-1.0);
}

const vec3 scale = vec3(0.8);
const float k = 19.19;

vec3 hash(vec3 a) {
  a = fract(a * scale);
  a += dot(a, a.yxz + k);
  return fract((a.xxy + a.yxx) * a.zyx);
}

const float fresnelExp = 5.0;

float fresnel(vec3 direction, vec3 normal) {
  vec3 halfDirection = normalize(normal + direction);

  float cosine = dot(halfDirection, direction);
  float product = max(cosine, 0.0);
  float factor = 1.0 - pow(product, fresnelExp);

  return factor;
}

void main() {
  float fuzz = texture(gMaterial, TexCoords).y;
  float gloss = 1 - fuzz;

  if (gloss <= 0.001) {
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    fragColor.w = 1.0;
    return;
  }
  float metallic = texture(gMaterial, TexCoords).r;
  if (metallic < 0.2) {
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    fragColor.w = 0.0;
    return;
  }

  vec3 normal = texture(gNormal, TexCoords).xyz;
  vec3 viewPos = getPosition(TexCoords);
  float NdotL = dot(normalize(normal), normalize(viewPos));
  if (NdotL <= 0.001) {
    // then the surface does not scatter the ray as per
    // https://raytracing.github.io/books/RayTracingInOneWeekend.html#metal
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    fragColor.w = 0.0;
    return;
  }

  vec3 worldPos = vec3(vec4(viewPos, 1.0) * inverse(view));
  vec3 jitt = hash(worldPos) * 1.0;
  float jitter = minRayStep > 1.0f
                     ? float(int(TexCoords.x + TexCoords.y) & 1) * 0.5f
                     : 0.0f;

  // Reflection vector
  vec3 reflected = normalize(reflect(normalize(viewPos), normalize(normal)));
  reflected += (fuzz * random_in_unit_sphere());

  // Ray cast
  vec3 hitPos = viewPos;
  float dDepth;
  // vec2 coords =
  // rayCast(jitt + reflected * max(-viewPos.z, minRayStep), hitPos, dDepth);
  vec2 coords =
      rayCast(reflected * max(-viewPos.z, minRayStep), hitPos, dDepth);
  vec3 color = texture(lightBuffer, coords.xy).rgb;

  if (coords.xy != vec2(-1.0)) {
    fragColor.xyz = color;
    fragColor.w = 1.0;
  } else {
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    fragColor.w = 0.0;
  }
}
