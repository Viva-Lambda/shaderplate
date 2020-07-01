#version 430
in vec3 FragPos;  // in world coordinates
in vec2 TexCoord; // in local coordinates if I am not mistaken
in vec3 Normal;   // in world coordinates as well
in mat3 TBN;

out vec3 fragPos;
out vec3 fragNorm;
out vec4 fragSpec;

uniform sampler2D diffuseMap1;
uniform sampler2D specularMap1;
uniform sampler2D heightMap1;

vec3 getNormal() {
  vec3 normal1 = texture(heightMap1, TexCoord).rgb;
  normal1 = normal1 * 2.0 - 1.0;
  return normalize(TBN * normal1);
}

void main() {
  //
  fragPos = FragPos;
  fragNorm = getNormal();
  fragSpec.rgb = texture2D(diffuseMap1, TexCoord).rgb;
  vec3 spec = texture2D(specularMap1, TexCoord).rgb;

  fragSpec.a = (spec.r + spec.g + spec.b) / 3.0;
}
