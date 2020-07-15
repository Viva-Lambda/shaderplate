#version 430
layout(binding = 2) uniform sampler2D lightBuffer; // normals in viewspace
layout(binding = 0) uniform sampler2D gPosition;   // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;     // normals in viewspace
layout(binding = 3) uniform sampler2D gMaterial;   // normals in viewspace
uniform vec2 gTexSizeInv;

// Consts should help improve performance
const float rayStep = 0.25;
const float minRayStep = 0.1;
const float maxSteps = 20;
const float searchDist = 5;
const float searchDistInv = 0.2;
const int numBinarySearchSteps = 5;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;

const float reflectionSpecularFalloffExponent = 3.0;

uniform mat4 projection;

vec3 BinarySearch(vec3 dir, inout vec3 hitCoord, out float dDepth) {
  float depth;

  for (int i = 0; i < numBinarySearchSteps; i++) {
    vec4 projectedCoord = projection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    depth = texture2D(gPosition, projectedCoord.xy).z;

    dDepth = hitCoord.z - depth;

    if (dDepth > 0.0)
      hitCoord += dir;

    dir *= 0.5;
    hitCoord -= dir;
  }

  vec4 projectedCoord = projection * vec4(hitCoord, 1.0);
  projectedCoord.xy /= projectedCoord.w;
  projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

  return vec3(projectedCoord.xy, depth);
}

vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth) {
  dir *= rayStep;

  float depth;

  for (int i = 0; i < maxSteps; i++) {
    hitCoord += dir;

    vec4 projectedCoord = projection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    depth = texture2D(gPosition, projectedCoord.xy).z;

    dDepth = hitCoord.z - depth;

    if (dDepth < 0.0)
      return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
  }

  return vec4(0.0, 0.0, 0.0, 0.0);
}

void main() {
  vec2 gTexCoord = gl_FragCoord.xy * gTexSizeInv;

  // Samples
  float specular = texture2D(lightBuffer, gTexCoord).a;

  if (specular == 0.0) {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    return;
  }

  vec3 viewNormal = texture2D(gNormal, gTexCoord).xyz;
  vec3 viewPos = texture2D(gPosition, gTexCoord).xyz;

  // Reflection vector
  vec3 reflected =
      normalize(reflect(normalize(viewPos), normalize(viewNormal)));

  // Ray cast
  vec3 hitPos = viewPos;
  float dDepth;

  vec4 coords =
      RayCast(reflected * max(minRayStep, -viewPos.z), hitPos, dDepth);

  vec2 dCoords = abs(vec2(0.5, 0.5) - coords.xy);

  float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

  vec3 metallic = vec3(texture(gMaterial, coords.xy).r);

  // Get color
  gl_FragColor =
      vec4(metallic,
           pow(specular, reflectionSpecularFalloffExponent) * screenEdgefactor *
               clamp(-reflected.z, 0.0, 1.0) *
               clamp((searchDist - length(viewPos - hitPos)) * searchDistInv,
                     0.0, 1.0) *
               coords.w);
}
