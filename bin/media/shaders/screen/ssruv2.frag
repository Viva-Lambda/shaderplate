// Algine SSR fragment shader
// from
// https://gitlab.com/congard/algine/-/blob/master/resources/shaders/SSR.frag.glsl

#version 430 core

layout(binding = 0) uniform sampler2D gPosition;   // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;     // normals in viewspace
layout(binding = 2) uniform sampler2D lightBuffer; // normals in viewspace
layout(binding = 3) uniform sampler2D gMaterial;   // normals in viewspace
uniform mat4 projection, view;

uniform int binarySearchCount = 100;
uniform int rayMarchCount = 100; // 60
uniform float rayStep = 0.1;   // 0.025
uniform float LLimiter = 0.1;
uniform float minRayStep = 0.2;

in vec2 TexCoords;

layout(location = 0) out vec4 fragColor;

// SSR based on tutorial by Imanol Fotia
// http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html

#define getPosition(TexCoords) texture(gPosition, TexCoords).xyz

vec2 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth) {
  float depth;

  vec4 projectedCoord;

  for (int i = 0; i < binarySearchCount; i++) {
    projectedCoord = projection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    depth = getPosition(projectedCoord.xy).z;

    dDepth = hitCoord.z - depth;

    dir *= 0.5;
    if (dDepth > 0.0)
      hitCoord += dir;
    else
      hitCoord -= dir;
  }

  projectedCoord = projection * vec4(hitCoord, 1.0);
  projectedCoord.xy /= projectedCoord.w;
  projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

  return vec2(projectedCoord.xy);
}

vec2 rayCast(vec3 dir, inout vec3 hitCoord, out float dDepth) {
  dir *= rayStep;

  for (int i = 0; i < rayMarchCount; i++) {
    hitCoord += dir;

    vec4 projectedCoord = projection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    float depth = getPosition(projectedCoord.xy).z;
    dDepth = hitCoord.z - depth;

    if ((dir.z - dDepth) < 1.2 && dDepth <= 0.0)
      return binarySearch(dir, hitCoord, dDepth);
  }

  return vec2(-1.0);
}

#define scale vec3(.8, .8, .8)
#define k 19.19

vec3 hash(vec3 a) {
  a = fract(a * scale);
  a += dot(a, a.yxz + k);
  return fract((a.xxy + a.yxx) * a.zyx);
}

// source:
// https://www.standardabweichung.de/code/javascript/webgl-glsl-fresnel-schlick-approximation
#define fresnelExp 5.0

float fresnel(vec3 direction, vec3 normal) {
  vec3 halfDirection = normalize(normal + direction);

  float cosine = dot(halfDirection, direction);
  float product = max(cosine, 0.0);
  float factor = 1.0 - pow(product, fresnelExp);

  return factor;
}

void main() {
  float reflectionStrength = 1 - texture(gMaterial, TexCoords).y;

  if (reflectionStrength <= 0.001) {
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    return;
  }
  if (texture(gMaterial, TexCoords).r < 0.2) {
    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    return;
  }

  vec3 normal = texture(gNormal, TexCoords).xyz;
  vec3 viewPos = getPosition(TexCoords);
  float NdotL = dot(normalize(normal), normalize(viewPos));

  vec3 worldPos = vec3(vec4(viewPos, 1.0) * inverse(view));
  vec3 jitt = hash(worldPos) * 1.0;

  // Reflection vector
  vec3 reflected = normalize(reflect(normalize(viewPos), normalize(normal)));

  // Ray cast
  vec3 hitPos = viewPos;
  float dDepth;
  vec2 coords =
      rayCast(jitt + reflected * max(-viewPos.z, minRayStep), hitPos, dDepth);

  float L = length(getPosition(coords) - viewPos);
  L = clamp(L * LLimiter, 0, 1);
  float error = 1 - L;

  float fresnel = fresnel(reflected, normal);

  vec3 color = texture(lightBuffer, coords.xy).rgb * error * fresnel;

  if (coords.xy != vec2(-1.0)) {
    fragColor.xyz = mix(texture(lightBuffer, TexCoords), vec4(color, 1.0),
                        reflectionStrength)
                        .rgb;
    fragColor.w = NdotL;
  } else {

    fragColor.xyz = texture(lightBuffer, TexCoords).rgb;
    fragColor.w = 0.0;
  }
}
