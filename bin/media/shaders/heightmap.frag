#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 TbnLightPos;
in vec3 TbnViewPos;
in vec3 TbnFragPos;

out vec4 FragColor;

uniform float ambientCoeff; // a good value is 0.1

uniform float shininess; // a good value is 32

uniform vec3 attC; // x=c1, y=c2, z=c3
uniform float lightIntensity;

// textures
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

float computeAttenuation(vec3 att, float lfragdist);
vec3 getSurfaceNormal();
vec3 getLightDir();
// vec3 getViewDir();
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color);
vec3 getSpecColor(vec3 lightDir, vec3 normal);

void main() {

  vec3 color = texture(diffuseMap, TexCoord).rgb;
  // ambient term I_a × O_d × k_a
  vec3 ambient = color * ambientCoeff;

  // lambertian terms k_d * (N \cdot L) * I_p
  vec3 surfaceNormal = getSurfaceNormal();
  vec3 lightDirection = getLightDir();
  vec3 diffuseColor = getDiffuseColor(lightDirection, surfaceNormal, color);

  // attenuation term f_att
  // f_att = min(\frac{1}{c_1 + c_2{\times}d_L + c_3{\times}d^2_{L}} , 1)
  float dist = distance(TbnLightPos, TbnFragPos);
  float attenuation = computeAttenuation(attC, dist);

  vec3 diffuse = attenuation * diffuseColor * lightIntensity;

  // adding specular terms

  FragColor = vec4(ambient + diffuse, 1.0);
}

float computeAttenuation(vec3 att, float lfragdist) {
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  result = 1 / result;
  float attenuation = min(result, 1);
  return attenuation;
}
vec3 getSurfaceNormal() {
  vec3 normal = texture(normalMap, TexCoord).rgb;
  return normalize(normal * 2.0 - 1.0);
}

vec3 getLightDir() { return normalize(TbnLightPos - TbnFragPos); }
// vec3 getViewDir() { return normalize(TbnViewPos - TbnFragPos); }
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
}
