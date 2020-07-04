#version 430

/**
 * taken from L. Hermanns, Screen space cone tracing for glossy reflections,
 * Bachelor Thesis, Technische Universität Darmstadt, Darmstadt, 2015. URL :
 * http://publica.fraunhofer.de/documents/N-336466.html.
 */

uniform float linear_march_count = 100.0;
uniform float linear_march_step = 1.2;
uniform float bineary_search_count = 32.0;
uniform float bineary_search_decrease = 0.5;
uniform float depth_delta_epsilon = 0.01;
uniform vec3 invalid_ray = vec3(-1);

uniform sampler2D depthBuffer;

vec3 LinearRayMarch(vec3 ray, vec3 dir, float stride) {
  vec3 prevRay = ray;
  for (int i = 0; i < linear_march_count; ++i) {
    // Sample depth value at current ray position
    float depth = textureLod(depthBuffer, ray.xy, 0.0);
    // Check if ray steps through the depth buffer
    if (ray.z > depth) {
      // Intersection found -> now do a binary search
      return BinarySearch((prevRay + ray) * 0.5, dir * stride);
    }
    // Store previous position and step to the next sample position
    prevRay = ray;
    ray += dir * stride;
    // Increase stride to accelerate ray marching
    stride *= linear_march_step;
  }
  return invalid_ray; // No intersection found
}
vec3 BinarySearch(vec3 ray, vec3 dir) {
  for (int i = 0; i < bineary_search_count; ++i) {
    // Sample depth value at current ray position
    float depth = textureLod(depthBuffer, ray.xy, 0.0);
    // Check if the ’depth delta ’ is smaller than our epsilon
    float depthDelta = depth - ray.z;
    if (abs(depthDelta) < depth_delta_epsilon)
      break; // Final intersection found -> break iteration
    // Move ray forwards if we are in front of geometry ,
    // and move ray backwards if we are behind geometry .
    if (depthDelta > 0.0)
      ray += dir;
    else
      ray -= dir;
    // Decrease direction vector for further refinement
    dir *= bineary_search_decrease;
  }
  return ray; // Intersection already found , but we could not refine it to the
              // minimum
}
