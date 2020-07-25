#version 440
layout(location = 0) out vec2 FragPosDepth;
/**
 * Adapted from
 * https://github.com/kvarcg/publications/blob/master/DIRT%20Deferred%20Image-based%20Tracing%20-%20HPG%202016/Shaders%20Only/Build/1.1.%20Fill%20Depth%20Mipmap.frag
 * */

uniform sampler2DArray cubeDepth;
// uniform int level; // needed if we are changing the viewport size
uniform int cube_face;
in vec2 TexCoords;

void main() {
  vec3 coords = vec3(TexCoords, float(cube_face));
  vec4 xs = textureGather(cubeDepth, coords, 0);
  vec4 ys = textureGather(cubeDepth, coords, 1);
  float minZ = max(max(xs.x, xs.y), max(xs.z, xs.w));
  float maxZ = max(max(ys.x, ys.y), max(ys.z, ys.w));
  FragPosDepth = vec2(minZ, maxZ);
}
