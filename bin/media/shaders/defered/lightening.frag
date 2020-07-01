#version 430
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform vec3 lightPos;
uniform vec3 inLightDir;
uniform vec3 viewPos;
uniform float lightIntensity = 1.0;

// some phong related uniforms
uniform float ambientCoeff = 0.01; // a good value is 0.1
uniform float shininess = 32.0;    // a good value is 32
uniform float innerCutoff = 0.91;
uniform float outerCutoff = 0.82;
uniform vec3 attC; // x=c1, y=c2, z=c3

// phong related functions
float computeAttenuation(vec3 att, vec3 fpos);

void main() {
  //
  vec3 FragPos = texture2D(positionTexture, TexCoords).rgb;
  vec3 Normal = texture2D(normalTexture, TexCoords).rgb;
  vec4 DiffSpecColor = texture2D(specularTexture, TexCoords);
  vec3 diffuse = DiffSpecColor.rgb;
  float specular = DiffSpecColor.a;

  // set some directions
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 lightDir = normalize(lightPos - FragPos);
  vec3 halfway = normalize(lightDir + viewDir);

  // set spot light related stuff
  float lightTheta = dot(lightDir, normalize(-inLightDir));
  float epsilon = innerCutoff - outerCutoff;
  float intensity = clamp((lightTheta - outerCutoff) / epsilon, 0.0, 1.0);

  vec3 ambient = diffuse * ambientCoeff;

  // compute attenuation using not dist^2 thing
  float attenuation = computeAttenuation(attC, FragPos);

  vec3 diffcolor = attenuation * diffuse * vec3(lightIntensity);
  float spec = pow(max(dot(Normal, halfway), 0.0), shininess);
  vec3 specColor = vec3(lightIntensity) * spec * specular;

  vec3 result = ambient + (specColor + diffcolor) * attenuation * intensity;
  FragColor = vec4(result, 1);
}
float computeAttenuation(vec3 att, vec3 FragPos) {
  float lfragdist = distance(lightPos, FragPos);
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  return min(1 / result, 1.0);
}
