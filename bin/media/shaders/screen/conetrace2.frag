#version 430
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D gDepth;
uniform sampler2D lightBuffer; // convolved color buffer - all mip levels
uniform sampler2D gNormal;     // normal buffer - from g-buffer in camera space
uniform sampler2D gMaterial;   // specular buffer - from g-buffer (rgb = ior,

// implementing from
// publica.fraunhofer.de/documents/N-336466.html
//

/**
 * Project point to screen space algorithm 2 p. 27
 * */
vec3 project_to_screen_space(vec3 PointVS, mat4 projection) {
  vec4 PointPS = projection * vec4(PointVS, 1.0);
  vec3 PointSS = PointPS.xyz / PointPS.w;
  PointSS.xy = PointSS.xy * vec2(0.5, -0.5) + 0.5;
  return PointSS;
}
vec3 unproject_from_screen_space(vec3 PointSS, mat4 projection) {
  mat4 invproj = inverse(projection);
  PointSS.xy = (PointSS.xy - 0.5) * vec2(2.0, -2.0);
  vec3 PointPS = invproj * PointSS;
  vec3 PointVS = PointPS.xyz / PointPS.w;
  return PointVS;
}

vec3 get_reflection_ray_direction(ivec2 PixelCoordinates, float pixelDepth,
                                  vec3 NormalVS, mat4 projection,
                                  float texSize) {
  vec3 PointSS = vec3(PixelCoordinates.x, PixelCoordinates.y, pixelDepth);
  vec3 PointVS = unproject_from_screen_space(PointSS, projection);
  vec3 ViewDirection = normalize(PointVS);
  vec3 ReflectRayDirection = reflect(ViewDirection, NormalVS);
  float epsilon = 1.0 / texSize;
  vec3 ScreenSpaceOffset = project_to_screen_space(
      PointVS + ReflectRayDirection * epsilon, projection);
  vec3 ReflectionVectorSS = ScreenSpaceOffset - PointSS;
  return ReflectionVectorSS;
}
