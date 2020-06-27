#version 430
// voxelization
layout(triangles) in; // triple vertex input
layout(triangle_strip, max_vertices = 3) out;

in Vertex_Out {
  vec3 Normal;
  vec4 FragPos;
  vec2 TexCoord;
  mat3 TBN;
}
vs_in;

out Frag_Out {
  vec3 frag_pos;
  vec3 frag_normal;
  vec2 frag_texCoord;
  mat3 frag_tbn;
}
fs_out;

/**
 * get face normal of triangle using given vertices*/
vec3 getFaceNormal() {
  return abs(cross(vs_in.FragPos[1] - vs_in.FragPos[0],
                   vs_in.FragPos[2] - vs_in.FragPos[0]));
}

/**
 * get dominant axis using face normal
 *
 * @param face normal of the input triangle
 * */
float getDominantAxis(vec3 faceNormal) {
  return max(faceNormal.x, max(faceNormal.y, faceNormal.z));
}

/**
 * rotate vertex position to dominant axis
 * @param ndx is the index of vertex
 * @param axis is the dominant axis
 * @param normal face normal
 * */
void rotateToDominantAxis(in uint ndx, float daxis, vec3 fnormal) {
  //
  fs_out.frag_pos = vs_in.FragPos[ndx];
  fs_out.frag_normal = vs_in.Normal[ndx];
  fs_out.frag_texCoord = vs_in.TexCoord;
  fs_out.frag_tbn = vs_in.TBN;
  if (daxis == fnormal.x) {
    gl_Position = vec4(frag_pos.zyx, 1);
  } else if (daxis == fnormal.y) {
    gl_Position = vec4(frag_pos.xzy, 1);
  } else if (daxis == fnormal.z) {
    gl_Position = vec4(frag_pos.xyz, 1);
  }
  EmitVertex();
}

void main() {
  //
  vec3 face_normal = getFaceNormal();
  float dominant_axis = getDominantAxis(face_normal);
  for (uint i = 0; i < 3; i++) {
    rotateToDominantAxis(i, dominant_axis, face_normal);
  }
  EndPrimitive();
}
