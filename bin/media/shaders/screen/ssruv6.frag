#version 430

// -------- path tracing in view space ----------------------------------
layout(binding = 0) uniform sampler2D gPosition; // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;   // normals in viewspace
// local coloring + prefiltered environment maps
layout(binding = 2) uniform sampler2D lightBuffer; // brdf contains f_r

// material are needed to determine reflection angle based on
// roughness
layout(binding = 3) uniform sampler2D gMaterial;

uniform float bounceNb = 3.0;
