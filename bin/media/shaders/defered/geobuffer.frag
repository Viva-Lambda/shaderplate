#version 430
in vec3 FragPos;  // in world coordinates
in vec2 TexCoord; // in local coordinates if I am not mistaken
in vec3 Normal;   // in world coordinates as well

out vec3 fragPos;
out vec3 fragNorm;
out vec4 fragSpec;

uniform sampler2D diffuseMap1;
uniform sampler2D specularMap1;

void main() {
  //
  fragPos = FragPos;
  fragNorm = normalize(Normal);
  fragSpec.rgb = texture2D(diffuseMap1, TexCoord).rgb;
  vec3 spec = texture2D(specularMap1, TexCoord).rgb;

  fragSpec.a = (spec.r + spec.g + spec.b) / 3.0;
}
