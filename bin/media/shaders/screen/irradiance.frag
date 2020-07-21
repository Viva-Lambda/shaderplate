#version 430
in vec3 FragPos;
out vec4 FragColor;
uniform samplerCube envMap;

uniform float sampleSize = 10.0;
uniform float sampleStep = 0.1;

const float PI = 3.14159265;

// from github/D-K-E/raytracing-gl

float rand(vec2 co) {
  // random gen
  float a = 12.9898;
  float b = 78.233;
  float c = 43758.5453;
  float dt = dot(co.xy, vec2(a, b));
  float sn = mod(dt, PI);
  return fract(sin(sn) * c);
}
float random_double() {
  // random double
  return rand(vec2(0.0, 1.0));
}
float random_double(float mi, float mx) {
  // random double
  return rand(vec2(mi, mx));
}
int random_int(int mi, int mx) { return int(random_double(mi, mx)); }
vec3 random_vec() {
  // random vector
  return vec3(random_double(), random_double(), random_double());
}
vec3 random_vec(float mi, float ma) {
  // random vector in given seed
  return vec3(random_double(mi, ma), random_double(mi, ma),
              random_double(mi, ma));
}

void main() {
  vec3 normal = normalize(FragPos);
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = cross(up, normal);
  up = cross(normal, right);

  vec3 irradiance = vec3(0);
  for (float i = 0.0; i < sampleSize; i += sampleStep) {
    float phi = random_double(0.0, 2.0 * PI);
    float theta = random_double(0.0, 0.5 * PI);
    vec3 tansample =
        vec3(1.0 * sin(theta) * cos(phi), // x = r * sin(theta)*cos(theta)
             1.0 * sin(theta) * sin(phi), // y = r *sin(theta) * sin(phi)
             cos(theta)                   // z
             );
    vec3 svec = tansample.x * right + tansample.y * up + tansample.z * normal;
    irradiance += texture(envMap, svec).rgb * sin(theta) * cos(theta);
  }
  irradiance = PI * irradiance * (1.0 / sampleSize);

  FragColor = vec4(irradiance, 1.0);
}
