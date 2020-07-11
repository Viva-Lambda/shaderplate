#version 430

// adapted from
/*
 *https://github.com/LukasBanana/ForkENGINE/blob/master/shaders/SSCTReflectionPixelShader.glsl
 * */

in vec2 TexCoord;
in mat4 invViewMat;         // vertex
in vec3 viewDirInViewSpace; // view direction, ie camera front

out vec4 FragColor;

// ---------------------------- uniforms -----------------------------------
// ------------------------- from other passes -----------------------------

uniform sampler2D gDepth;
uniform sampler2D lightBuffer; // convolved color buffer - all mip levels
uniform sampler2D gNormal;     // normal buffer - from g-buffer in camera space
uniform sampler2D gMaterial;   // specular buffer - from g-buffer (rgb = ior,
uniform sampler2D visibilityBuffer;
uniform sampler2D HizBuffer;

uniform int mipCount; // total mipmap count for hiz buffer
uniform mat4 projection;
uniform mat4 view;
uniform vec2 nearFar; // near and far plane for viewing frustum
uniform float hiz_start_level = 2.0;
uniform vec2 resolution; // hiz buffer resolution

/**Find intersection point of ray */
vec3 hiz_trace(vec3 point, vec3 dir) {
  const float root = mipCount - 1.0;
  float level = hiz_start_level;

  float iterations = 0.0;

  vec2 crossStep, crossOffset;
  vec2 crossEpsilon = 0.5 / resolution;

  crossStep.x = (v.x >= 0) ? 1.f : −1. f;
  crossStep.y = (v.y >= 0) ? 1.f : −1. f;
  crossOffset.xy = crossStep.xy * crossEpsilon;
  crossStep.xy = clamp(crossStep.xy, 0.0, 1.0);

  //
}
