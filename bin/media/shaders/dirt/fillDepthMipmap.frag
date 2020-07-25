#version 440
layout(location = 0) out vec2 FragPosDepth;
/**
 * Adapted from
 * https://github.com/kvarcg/publications/blob/master/DIRT%20Deferred%20Image-based%20Tracing%20-%20HPG%202016/Shaders%20Only/Build/1.1.%20Fill%20Depth%20Mipmap.frag
 * */

uniform samplerCube cubeDepth;
uniform int level;
in vec2 TexCoords;

void main() {
  ivec2 coords = ivec2(gl_FragCoord.xy) * 2;
  int frag_z = gl_FragCoord.z;
  ivec3 coordz = 
  vec2 t_minmax[4];
  t_minmax[0] =
      texelFetch(tex_depth_bounds, ivec3(coords.xy, uniform_cube_index), 0).xy;
  t_minmax[1] =
      texelFetch(tex_depth_bounds,
                 ivec3(coords.xy + ivec2(1, 0), uniform_cube_index), 0)
          .xy;
  t_minmax[2] =
      texelFetch(tex_depth_bounds,
                 ivec3(coords.xy + ivec2(1, 1), uniform_cube_index), 0)
          .xy;
  t_minmax[3] =
      texelFetch(tex_depth_bounds,
                 ivec3(coords.xy + ivec2(0, 1), uniform_cube_index), 0)
          .xy;

  float minZ =
      max(max(t_minmax[0].x, t_minmax[1].x), max(t_minmax[2].x, t_minmax[3].x));
  float maxZ =
      max(max(t_minmax[0].y, t_minmax[1].y), max(t_minmax[2].y, t_minmax[3].y));

  out_frag_mipmap = vec2(minZ, maxZ);
}
