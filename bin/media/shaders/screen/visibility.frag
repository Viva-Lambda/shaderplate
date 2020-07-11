#version 430 core
layout(location = 0) out vec3 visibility;

in vec2 TexCoords;

uniform sampler2D HiZBuffer;
uniform sampler2D visibilityMap;

uniform vec2 nearFar;
uniform int miplevel;

void main() {
  ivec2 sscoord = ivec2(gl_FragCoord.xy) * 2;
  int mprev = miplevel - 1;
  vec4 finez;
  finez.x = linearize(texelFetch(HiZBuffer, sscoord, mprev).r);
  finez.y = linearize(texelFetch(HiZBuffer, sscoord + ivec2(1, 0), mprev).r);
  finez.z = linearize(texelFetch(HiZBuffer, sscoord + ivec2(0, 1), mprev).r);
  finez.w = linearize(texelFetch(HiZBuffer, sscoord + ivec2(1, 1), mprev).r);

  // min and max depth value from current hiz buffer
  float minz = linearize(texelFetch(HiZBuffer, sscoord, miplevel).r);
  float maxz = linearize(texelFetch(HiZBuffer, sscoord, miplevel).y);

  // coarse volume pre divide
  float coarseVolume = 1.0f / (maxz - minz);

  vec4 vis;
  vis.x = linearize(texelFetch(HiZBuffer, sscoord, mprev).r);
  vis.y = linearize(texelFetch(HiZBuffer, sscoord + ivec2(1, 0), mprev).r);
  vis.z = linearize(texelFetch(HiZBuffer, sscoord + ivec2(0, 1), mprev).r);
  vis.w = linearize(texelFetch(HiZBuffer, sscoord + ivec2(1, 1), mprev).r);

  // calculate visibility with respect to coarse depth
  vec4 integration = finez.xyzw * abs(coarseVolume) * vis.xyzw;

  // 0.25 is the weight we are attributing to each
  // pixel. Since we are using 4 pixels, the weight 
  // is homogeneously distributed.
  float integ = dot(0.25, integration.xyzw);
  visibility = vec3(integ);
}
