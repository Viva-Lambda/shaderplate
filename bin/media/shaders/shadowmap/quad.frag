#version 330

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D depthMap;
uniform float nearPlane = 0.1;
uniform float farPlane = 100.0;
uniform bool isPerspective = false;

float get_linear_depth(float depth) {
  //
  float z = depth * 2.0 - 1.0; // [-1, 1] -> [0, 1]
  return (2.0 * nearPlane * farPlane) /
         (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main() {
  float depth = texture2D(depthMap, TexCoord).r;
  // debugging part
  if (isPerspective) {
    FragColor = vec4(vec3(get_linear_depth(depth) / farPlane), 1.0); //
    // perspective projection
  } else {
    FragColor = vec4(vec3(depth), 1.0); // ortographic projection
  }
}
