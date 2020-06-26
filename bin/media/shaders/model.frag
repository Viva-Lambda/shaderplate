#version 330

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 TbnLightPos;
in vec3 TbnViewPos;
in vec3 TbnFragPos;

out vec4 FragColor;

uniform float ambientCoeff; // a good value is 0.1
uniform float shininess;    // a good value is 32
uniform vec3 attC;          // x=c1, y=c2, z=c3
uniform float lightIntensity = 1.0;

// textures
uniform sampler2D diffuseMap1;
uniform sampler2D specularMap1;
uniform sampler2D normalMap1;

float computeAttenuation(vec3 att);
vec3 getSurfaceNormal();
vec3 getLightDir();
vec3 getViewDir();
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color);
vec3 getSpecColor(vec3 lightDir, vec3 normal);

void main() {

  vec3 color = texture(diffuseMap1, TexCoord).rgb;
  // ambient term I_a × O_d × k_a
  vec3 ambient = color * ambientCoeff;

  // lambertian terms k_d * (N \cdot L) * I_p
  vec3 surfaceNormal = getSurfaceNormal();
  vec3 lightDirection = getLightDir();
  vec3 diffuseColor = getDiffuseColor(lightDirection, surfaceNormal, color);

  // attenuation term f_att
  // f_att = min(\frac{1}{c_1 + c_2{\times}d_L + c_3{\times}d^2_{L}} , 1)
  float attenuation = computeAttenuation(attC);

  vec3 diffuse = attenuation * diffuseColor * lightIntensity;

  // adding specular terms
  vec3 specular = getSpecColor(lightDirection, surfaceNormal);
  float gamma = 2.2;
  vec3 total = ambient + diffuse + specular;
  // vec3 total = ambient + diffuse;
  // vec3 total = diffuse + specular;

  // FragColor = vec4(pow(total, vec3(1.0 / gamma)), 1.0);
  // FragColor = vec4(pow(diffuse, vec3(1.0 / gamma)), 1.0);
  FragColor = vec4(total, 1);
  // FragColor = vec4(1.0);
}

float computeAttenuation(vec3 att) {
  float lfragdist = distance(TbnLightPos, TbnFragPos);
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  return min(1 / result, 1.0);
}
vec3 getSurfaceNormal() {
  vec3 normal1 = texture(normalMap1, TexCoord).rgb;
  normal1 = normalize(normal1 * 2.0 - 1.0);

  return normal1;
}

vec3 getLightDir() { return normalize(TbnLightPos - TbnFragPos); }
vec3 getViewDir() { return normalize(TbnViewPos - TbnFragPos); }
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
}
vec3 getSpecColor(vec3 lightDir, vec3 normal) {
  vec3 viewDir = getViewDir();
  vec3 spec = texture(specularMap1, TexCoord).rgb;
  vec3 refdir = reflect(-lightDir, normal);
  vec3 hwaydir = normalize(lightDir + viewDir);
  float specAngle = max(dot(refdir, hwaydir), 0.0);
  return pow(specAngle, shininess) * spec;
}
