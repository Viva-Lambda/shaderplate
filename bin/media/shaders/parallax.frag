#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 TbnLightPos;
in vec3 TbnViewPos;
in vec3 TbnFragPos;

out vec4 FragColor;

// phong model related
uniform float ambientCoeff = 0.1; // a good value is 0.1
uniform float shininess = 32.0;   // a good value is 32
uniform vec3 attC;                // x=c1, y=c2, z=c3
uniform float lightIntensity = 1.0;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;

// textures
// parallax mapping related
// mostly implemented from
// Oliviera (Manuel), Policarpo (Fabio) et Comba (João), « Real-Time Relief
// Mapping on Arbitrary Polygonal Surfaces », dans ACM SIGGRAPH 2005 Symposium
// on Interactive 3D Graphics and Games, Washington, DC, [s.n.], 2005, p.
// 155‑162.
uniform sampler2D heightMap;

uniform float depth_scale = 0.1;
uniform int binary_steps = 10; // 5
uniform int linear_steps = 20; // 10 in paper
//
float size = 1.0 / linear_steps;

vec2 scaled_depth = TbnViewPos.xy * depth_scale / TbnViewPos.z;

float linear_search() {
  float current_depth = 0.0;
  float best_depth = 1.0;
  for (int i = 0; i < linear_steps; i++) {
    current_depth += size;
    vec2 offset_depth = scaled_depth * current_depth;
    float depth_offset = texture2D(heightMap, TexCoord + offset_depth).x; // x
    // might be w as well it depends on how height map contains depth info
    if (best_depth > 0.990) {
      if (current_depth >= depth_offset) {
        best_depth = current_depth;
      }
    }
  }
  return best_depth; // default value for best depth
}
float binary_search(float best_depth) {
  //
  float current_depth = best_depth;
  for (int i = 0; i < binary_steps; i++) {
    size *= 0.5; // 0.5 comes from the paper
    vec2 offset_depth = scaled_depth * current_depth;
    float depth_offset = texture2D(heightMap, TexCoord + offset_depth).x;
    if (current_depth >= depth_offset) {
      best_depth = current_depth;
      current_depth -= 2 * size;
    }
    current_depth += size;
  }
  return best_depth;
}
vec2 reliefMap() {
  // relief parallax mapping
  float best_depth_linear = linear_search();
  float best_depth = binary_search(best_depth_linear);
  return (scaled_depth * best_depth) + TexCoord;
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
vec3 getSurfaceNormal(vec2 tcords) {
  vec3 normal = texture(normalMap, tcords).rgb;
  return normalize(normal * 2.0 - 1.0);
}

vec3 getLightDir() { return normalize(TbnLightPos - TbnFragPos); }
vec3 getViewDir() { return normalize(TbnViewPos - TbnFragPos); }
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
}

// vec3 getSpecColor(vec3 lightDir, vec3 normal, vec2 tcoords);

void main() {

  vec2 texCoords = reliefMap();
  if (texCoords.x > 1.0 || texCoords.x < 0.0 || texCoords.y > 1.0 ||
      texCoords.y < 0.0) {
    discard;
  }

  vec3 color = texture(diffuseMap, texCoords).rgb;
  // ambient term I_a × O_d × k_a
  vec3 ambient = color * ambientCoeff;

  // lambertian terms k_d * (N \cdot L) * I_p
  vec3 surfaceNormal = getSurfaceNormal(texCoords);
  vec3 lightDirection = getLightDir();
  vec3 diffuseColor = getDiffuseColor(lightDirection, surfaceNormal, color);

  // attenuation term f_att
  // f_att = min(\frac{1}{c_1 + c_2{\times}d_L + c_3{\times}d^2_{L}} , 1)
  float dist = distance(TbnLightPos, TbnFragPos);
  float attenuation = computeAttenuation(attC, dist);

  vec3 diffuse = attenuation * diffuseColor * lightIntensity;

  float ambientOcc = texture2D(aoMap, texCoords).r;

  // adding specular terms
  // vec3 specular = getSpecColor(lightDirection, surfaceNormal, texCoords);

  FragColor = vec4((ambient + diffuse) * ambientOcc, 1.0);
}

/*
 * Not needed since we are using layered cliff as texture
vec3 getSpecColor(vec3 lightDir, vec3 normal, vec2 texCoord) {
  vec3 viewDir = getViewDir();
  vec3 spec = texture(specularMap, texCoord).rgb;
  vec3 refdir = reflect(-lightDir, normal);
  vec3 hwaydir = normalize(lightDir + viewDir);
  float specAngle = max(dot(refdir, hwaydir), 0.0);
  // return pow(specAngle, shininess) * spec;
  return vec3(0);
}
*/
