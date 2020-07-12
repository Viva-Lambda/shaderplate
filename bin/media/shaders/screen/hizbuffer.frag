#version 430
layout(location = 0) out vec4 SampleDepth;      // depth value
layout(location = 1) out vec3 SampleVisibility; // visibility value

uniform sampler2D HiZBCopyTexture;
uniform sampler2D visibilityMap;
uniform sampler2D gSDepth;

uniform vec2 pixelOffset;
uniform int mipmapLevel; // previous mipmap level
uniform vec2 nearFar;
in vec2 TexCoords;

/**Linearize depth value
 * to camera space
 * */
float linearize(float zval) {
  return (2 * nearFar.x) /
         (nearFar.y + nearFar.x - (zval * (nearFar.y - nearFar.x)));
}

/**
 * adapted from
 * https://github.com/LukasBanana/ForkENGINE/blob/master/shaders/SSCTHiZMapPixelShader.glsl
 * */

vec2 get_depth_sample(ivec2 pixelCoord) {

  vec4 texels[2]; // required for downsampling

  texels[0].rg = texelFetch(HiZBCopyTexture, ivec2(pixelCoord), mipmapLevel).rg;
  texels[0].ba = texelFetch(HiZBCopyTexture,
                            pixelCoord + ivec2(pixelOffset.x, 0), mipmapLevel)
                     .rg;
  texels[1].rg = texelFetch(HiZBCopyTexture,
                            pixelCoord + ivec2(0, pixelOffset.y), mipmapLevel)
                     .rg;
  texels[1].ba =
      texelFetch(HiZBCopyTexture,
                 pixelCoord + ivec2(pixelOffset.x, pixelOffset.y), mipmapLevel)
          .rg;

  /* Down-sample texels */
  float minZ =
      min(min(texels[0].r, texels[0].b), min(texels[1].r, texels[1].b));
  float maxZ =
      max(max(texels[0].g, texels[0].a), max(texels[1].g, texels[1].a));

  vec2 hizValue = vec2(minZ, maxZ);
  return hizValue;
}

float get_visibility_sample(float minz, float maxz, ivec2 pixelCoord) {
  vec4 finez;
  finez.x = linearize(texelFetch(HiZBCopyTexture, pixelCoord, mipmapLevel).r);
  finez.y = linearize(
      texelFetch(HiZBCopyTexture, pixelCoord + ivec2(1, 0), mipmapLevel).r);
  finez.z = linearize(
      texelFetch(HiZBCopyTexture, pixelCoord + ivec2(0, 1), mipmapLevel).r);
  finez.w = linearize(
      texelFetch(HiZBCopyTexture, pixelCoord + ivec2(1, 1), mipmapLevel).r);

  // coarse volume pre divide
  float coarseVolume = 1.0f / (maxz - minz);

  vec4 vis;
  vis.x = linearize(texelFetch(visibilityMap, pixelCoord, mipmapLevel).r);
  vis.y = linearize(
      texelFetch(visibilityMap, pixelCoord + ivec2(1, 0), mipmapLevel).r);
  vis.z = linearize(
      texelFetch(visibilityMap, pixelCoord + ivec2(0, 1), mipmapLevel).r);
  vis.w = linearize(
      texelFetch(visibilityMap, pixelCoord + ivec2(1, 1), mipmapLevel).r);

  // calculate visibility with respect to coarse depth
  vec4 integration = finez.xyzw * abs(coarseVolume) * vis.xyzw;

  // 0.25 is the weight we are attributing to each
  // pixel. Since we are using 4 pixels, the weight
  // is homogeneously distributed.
  float integ = dot(vec4(0.25), integration);
  return integ;
}
vec2 clipToScreenSpace(vec4 pointInClipSpace) {
  return (pointInClipSpace.xy + 1.0) / 2.0 *
         vec2(textureSize(gSDepth, mipmapLevel));
}

void main() {
  vec3 pixelCS = texture(gSDepth, TexCoords).rgb;

  ivec2 pixelCoord = ivec2(clipToScreenSpace(vec4(pixelCS, 1)));
  vec2 dsample = get_depth_sample(pixelCoord);
  float visibility = get_visibility_sample(dsample.x, dsample.y, pixelCoord);
  SampleDepth = vec4(dsample, 0, 1);
  SampleVisibility = vec3(visibility);
}
