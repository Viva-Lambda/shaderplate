#version 430
layout(triangles) in;
// geometry shader for rendering to cubemap
// in a single pass
// 6*3
layout(triangle_strip, max_vertices = 18) out; // 6 * 3
layout(binding = 2, std430) writeonly buffer TriangleBuffer {
  mat4 triangle_vertices_normal;
  mat4 triangle_tangents;
  mat4 triangle_textures;
  int triangleId;
};
flat out int cube_face;
flat out vec4 triangle_vertices_ws[3]; // outputs triangle
                                       // vertices in world space
                                       //
in vec2 TexCoords[3];
in vec3 Normal[3];
in vec3 FragPosWS[3]; //
in vec3 TangentWS[3]; //

#define NB_FACES 6

uniform mat4 ModelViewProjection[NB_FACES];

vec3 computeTriangleNormal(vec3 p1, vec3 p2, vec3 p3) {
  // cross(p1 - p2, p1 - p3);
  return normalize(cross(p1 - p2, p1 - p3));
}

void main() {
  for (int i = 0; i < 3; i++) {
    triangle_vertices_ws[i].xyz = FragPosWS[i];
    triangle_vertices_ws[i].w = 0.0;
    triangle_vertices_normal[i].xyz = FragPosWS[i];
    triangle_vertices_normal[i].w = 0.0;
    triangle_tangents[i].xyz = TangentWS[i];
    triangle_tangents[i].w = 0.0;
    triangle_textures[i].xy = TexCoords[i];
    triangle_textures[i].zw = vec2(0.0);
  }
  triangleId = gl_PrimitiveIDIn;
  vec3 p1 = FragPosWS[0];
  vec3 p2 = FragPosWS[1];
  vec3 p3 = FragPosWS[2];

  triangle_vertices_normal[3].xyz = computeTriangleNormal(p1, p2, p3);
  triangle_vertices_normal[3].w = 0.0;

  for (int i = 0; i < NB_FACES; i++) {
    gl_ViewportIndex = i;
    cube_face = i;

    for (int k = 0; k < gl_in.length(); k++) {
      gl_Position = ModelViewProjection[i] * vec4(FragPosWS[k], 1.0);

      //
      EmitVertex();
    }
    EndPrimitive();
  }
}
