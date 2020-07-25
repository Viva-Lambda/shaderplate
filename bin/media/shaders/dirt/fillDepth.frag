#version 430
layout(location = 0) out vec4 FragDepthBound;

flat in int cube_face;
flat in vec4 triangle_vertices_ws[3]; // outputs triangle

// https://github.com/kvarcg/publications/blob/master/DIRT%20Deferred%20Image-based%20Tracing%20-%20HPG%202016/Shaders%20Only/Build/1.%20Fill%20Depth.frag
//

// --------------------------- constants --------------------------------
#define NB_FACES 6
#define EPSILON 0.0000001
#define LTF 0
#define RTF 1
#define LBF 2
#define RBF 3
#define LTN 4
#define RTN 5
#define LBN 6
#define RBN 7
#define NEAR 0
#define FAR 1
#define RIGHT 2
#define LEFT 3
#define TOP 4
#define BOTTOM 5

uniform vec2 NearFar[NB_FACES];
uniform mat4 ModelView[NB_FACES];
uniform vec4 Viewports[NB_FACES];
uniform vec3 FrustumCorners[NB_FACES * 8];
uniform int cube_index;

// https://github.com/kvarcg/publications/blob/master/DIRT%20Deferred%20Image-based%20Tracing%20-%20HPG%202016/Shaders%20Only/Include/clip_primitives.h

vec2 clip(int cface, vec3 p1, vec3 p2, vec3 p3) {
  //
  bvec2 clipped = bvec2(false);

  vec3 bounding_pointsWS[8];
  int points_index = cface * 8;
  bounding_pointsWS[LTF] = vec3(FrustumCorners[points_index + LTF]);
  bounding_pointsWS[RTF] = vec3(FrustumCorners[points_index + RTF]);
  bounding_pointsWS[LBF] = vec3(FrustumCorners[points_index + LBF]);
  bounding_pointsWS[RBF] = vec3(FrustumCorners[points_index + RBF]);
  bounding_pointsWS[LTN] = vec3(FrustumCorners[points_index + LTN]);
  bounding_pointsWS[RTN] = vec3(FrustumCorners[points_index + RTN]);
  bounding_pointsWS[LBN] = vec3(FrustumCorners[points_index + LBN]);
  bounding_pointsWS[RBN] = vec3(FrustumCorners[points_index + RBN]);

  // store pixel position
  vec4 f_viewport = vec4(0);
  f_viewport.xy = floor(gl_FragCoord.xy);
  f_viewport.zw = f_viewport.xy + vec2(1);

  vec2 fragCoord_leftBottom =
      vec2(f_viewport.x, f_viewport.y) / Viewports[cface].zw;
  vec2 fragCoord_rightTop =
      vec2(f_viewport.z, f_viewport.w) / Viewports[cube_index].zw;

  // and store them
  float top_a = fragCoord_rightTop.y;
  float right_a = fragCoord_rightTop.x;
  float bottom_a = fragCoord_leftBottom.y;
  float left_a = fragCoord_leftBottom.x;

  // calculate pixel frustum points by interpolating using the external frustum
  // positions
  vec3 pixel_pos_wcs[8];
  vec3 vertical1;
  vec3 vertical2;
  vertical1 = mix(bounding_pointsWS[LBF], bounding_pointsWS[LTF], top_a);
  vertical2 = mix(bounding_pointsWS[RBF], bounding_pointsWS[RTF], top_a);
  pixel_pos_wcs[LTF] = mix(vertical1, vertical2, left_a);
  pixel_pos_wcs[RTF] = mix(vertical1, vertical2, right_a);
  vertical1 = mix(bounding_pointsWS[LBF], bounding_pointsWS[LTF], bottom_a);
  vertical2 = mix(bounding_pointsWS[RBF], bounding_pointsWS[RTF], bottom_a);
  pixel_pos_wcs[LBF] = mix(vertical1, vertical2, left_a);
  pixel_pos_wcs[RBF] = mix(vertical1, vertical2, right_a);
  vertical1 = mix(bounding_pointsWS[LBN], bounding_pointsWS[LTN], top_a);
  vertical2 = mix(bounding_pointsWS[RBN], bounding_pointsWS[RTN], top_a);
  pixel_pos_wcs[LTN] = mix(vertical1, vertical2, left_a);
  pixel_pos_wcs[RTN] = mix(vertical1, vertical2, right_a);
  vertical1 = mix(bounding_pointsWS[LBN], bounding_pointsWS[LTN], bottom_a);
  vertical2 = mix(bounding_pointsWS[RBN], bounding_pointsWS[RTN], bottom_a);
  pixel_pos_wcs[LBN] = mix(vertical1, vertical2, left_a);
  pixel_pos_wcs[RBN] = mix(vertical1, vertical2, right_a);

  // create four rays from near to far
  vec3 ray_origin[4] = {pixel_pos_wcs[LTN], pixel_pos_wcs[RTN],
                        pixel_pos_wcs[LBN], pixel_pos_wcs[RBN]};

  vec3 ray_line[4] = {pixel_pos_wcs[LTF] - pixel_pos_wcs[LTN],
                      pixel_pos_wcs[RTF] - pixel_pos_wcs[RTN],
                      pixel_pos_wcs[LBF] - pixel_pos_wcs[LBN],
                      pixel_pos_wcs[RBF] - pixel_pos_wcs[RBN]};

  // find the triangle (plane) normal
  vec3 triangle_normal = cross(p2.xyz - p1.xyz, p3.xyz - p1.xyz);
  triangle_normal = normalize(triangle_normal);
  float triangle_d = 0.0;
  for (int i = 0; i < 3; ++i)
    triangle_d -= triangle_normal[i] * p1[i];

  float near = NearFar[cube_index].x;
  float far = NearFar[cube_index].y;
  float Zmin = far;
  float Zmax = near;

  // perform ray-plane intersections for each ray
  // and store the min and max extents in eye space
  for (int i = 0; i < 4; ++i) {
    vec3 raystart = ray_origin[i];
    vec3 ray_dir = normalize(ray_line[i]);
    float td = -((dot(raystart, triangle_normal)) + triangle_d);
    float d = dot(ray_dir, triangle_normal);
    if (abs(d) < 0.001)
      continue;
    td = td / d;
    if (td > 0 && td < length(ray_line[i])) {
      vec3 intersectpoint_wcs = raystart + td * ray_dir;
      vec3 intersectpoint =
          vec3(ModelView[cube_index] * vec4(intersectpoint_wcs, 1)).xyz;
      Zmin = max(near, min(Zmin, -intersectpoint.z));
      Zmax = min(far, max(Zmax, -intersectpoint.z));
      if (abs(Zmin - -intersectpoint.z) < EPSILON) {
        clipped.x = true;
      }
      if (abs(Zmax - -intersectpoint.z) < EPSILON) {
        clipped.y = true;
      }
    }
  }

  if (!clipped.x) {
    Zmin = near;
  }
  if (!clipped.y) {
    Zmax = far;
  }
  return vec2(Zmin, Zmax);
}

void main() {
  vec3 p1 = triangle_vertices_ws[0].xyz;
  vec3 p2 = triangle_vertices_ws[1].xyz;
  vec3 p3 = triangle_vertices_ws[2].xyz;
  vec2 depth_bound = clip(cube_face, p1, p2, p3);
  FragDepthBound.xy = vec2(-depth_bound.x, depth_bound.y);
  FragDepthBound.zw = vec2(gl_FragCoord.z, cube_face);
}
