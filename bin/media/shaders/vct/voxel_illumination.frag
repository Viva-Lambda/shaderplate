#version 430
//
out vec4 FragColor;

in Frag_Out {
  vec3 frag_pos;
  vec3 frag_normal;
  vec2 frag_texCoord;
  mat3 frag_tbn;
}
fs_in;

struct Ray {
  vec3 origin;
  vec3 direction; // normalized
};

vec3 at(Ray r, float dist) { return r.origin + r.direction * dist; }

struct VoxelConfig {
  // structure partly taken from
  // https://github.com/Helliaca/VXCT/blob/master/VXCT/shaders/Vxct/voxillumin.fs
  //
  float specular_offset;
  float specular_aperature; // in radians
  float specular_dist_factor;
  float diffuse_offset;
  float diffuse_aperature; // in radians
};
uniform VoxelConfig voxConf;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 attc;

uniform sampler2D diffuseMap1;
uniform sampler2D specularMap1;
uniform sampler2D normalMap1;

uniform sampler3D voxelMap;

uniform float shininess = 32.0;

uniform vec3 viewPos;

uniform float ambientCoeff; // a good value is 0.1
uniform float shininess;    // a good value is 32
uniform vec3 attC;          // x=c1, y=c2, z=c3
uniform float lightIntensity = 1.0;
uniform float occlusion = 1.0;

// functions
float computeAttenuation(vec3 att);
vec3 getSurfaceNormal();
vec3 getLightDir();
vec3 getViewDir();
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color);
vec3 getSpecColor(vec3 lightDir, vec3 normal);
vec3 getPhongColor();
vec4 getVoxelSample(float coneDiameter, vec3 hit_point);
vec2 map2Positive(vec2 v);
vec3 map2Positive(vec3 v);
vec4 map2Positive(vec4 v);

void main() {
  //
  vec3 viewDir = normalize(fs_in.frag_pos - viewPos);
  float min_occlusion = occlusion;
  vec3 phongColor = getPhongColor();
  vec3 surfNormal = getSurfaceNormal();
  vec3 outDir = normalize(reflect(viewDir, surfNormal));
  Ray r;
  r.origin fs_in.frag_pos, outDir);
  vec3 specColor = getSpecularConeColor(r);
}
vec2 map2Positive(vec2 v) { return (v + 1.0) * 0.5; }
vec3 map2Positive(vec3 v) { return (v + 1.0) * 0.5; }
vec4 map2Positive(vec4 v) { return (v + 1.0) * 0.5; }

float computeAttenuation(vec3 att) {
  float lfragdist = distance(lightPos, fs_in.FragPos);
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  return min(1 / result, 1.0);
}
vec3 getSurfaceNormal() {
  // depends on the model
  // vec3 normal1 = texture(normalMap1, TexCoord).rgb;
  vec3 normal1 = texture(heightMap1, fs_in.frag_texCoord).rgb;
  normal1 = normal1 * 2.0 - 1.0;
  return normalize(fs_in.frag_tbn * normal1);

  // return normal1;
}
vec3 getLightDir() { return normalize(lightPos - fs_in.FragPos); }
vec3 getViewDir() { return normalize(viewPos - fs_in.FragPos); }
vec3 getDiffuseColor(vec3 ldir, vec3 normal, vec3 color) {
  float costheta = dot(ldir, normal);
  // opaque surfaces
  return max(costheta, 0.0) * color;
}
vec3 getSpecColor(vec3 lightDir, vec3 normal) {
  vec3 viewDir = getViewDir();
  vec3 spec = texture(specularMap1, fs_in.frag_texCoord).rgb;
  vec3 refdir = reflect(-lightDir, normal);
  vec3 hwaydir = normalize(lightDir + viewDir);
  float specAngle = max(dot(refdir, hwaydir), 0.0);
  return pow(specAngle, shininess) * spec;
}
vec3 getPhongColor() {
  //
  vec3 viewDir = normalize(fs_in.frag_pos - viewPos);
  float min_occlusion = occlusion;

  vec3 color = texture(diffuseMap1, fs_in.frag_texCoord).rgb;
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
  vec3 total = ambient + diffuse + specular; // simple phong model
  return total;
}

vec4 getVoxelSample(vec3 hit_point) {
  vec3 hit_point_tex_space = map2Positive(hit_point); // [-1,1]->[0,1]

  float mipmapLevel = textureQueryLod(voxelMap, hit_point_tex_space).x;
  return textureLod(voxelMap, hit_point_tex_space, mipmapLevel);
}

vec3 traceSpecularCone(Ray r) {
  // specular cone color
  float maxdist = 1.0;
  float voffset = 8 / textureSize(voxelMap).r;
  float voxel_step = 1.0 / textureSize(voxelMap).r;
  vec3 origin = r.origin * getSurfaceNormal();
  Ray offr;
  offr.origin = origin;
  offr.direction = r.direction;
  vec4 color = vec4(0);
  float current_distance = voffset;

  //
  while (current_distance < maxdist && color.a < 1) {
    float cone_diameter = 2.0 * current_distance * tan(angle * 0.5);
    vec3 hit_point = at(r, current_distance);
    vec4 voxel = getVoxelSample(hit_point);
    vec3 voxel_color = voxel.rgb;
    float voxel_occ = voxel.a;
    color.xyz = color.a * color.xyz + (1 - color.xyz) * voxel_occ * voxel_color;
    color.a = color.a + (1 - color.a) * voxel_occ;

    current_distance += cone_diameter * voxConf.specular_dist_factor;
  }
  vec3 viewDir = getViewDir();
  vec3 lightDir = getLightDir();
  // vec3 refdir = reflect(-lightDir, normal); // mirror like reflection
  vec3 hwaydir = normalize(lightDir + viewDir);
  float specAngle = max(dot(r.direction, hwaydir), 0.0);
  return pow(specAngle, shininess) * spec_color;
}
