#version 430

in vec2 TexCoord;

out vec4 FragColor;

// material parameters
uniform sampler2D gDepth;       // from GBuffer
uniform sampler2D gNormal;      // from GBuffer
uniform sampler2D gAlbedo;      // from GBuffer
uniform sampler2D gMaterial;    // from GBuffer

// lights
uniform vec3 lightPos; // in world space
uniform vec3 lightColor;
uniform vec3 inLightDir; // in world space

uniform vec3 viewPos; // in world space

uniform float lightIntensity = 1.0;

// some phong related uniforms
uniform float shininess = 32.0; // a good value is 32
uniform float innerCutoff = 0.91;
uniform float outerCutoff = 0.82;
uniform vec3 attC = vec3(1, 0.72, 0.04); // x=c1, y=c2, z=c3

// phong related functions
float computeAttenuation(vec3 att, vec3 fpos);

void main() {
  //
  vec3 FragPos = texture(gDepth, TexCoord).rgb;
  vec3 Normal = texture(gNormal, TexCoord).rgb;
  vec4 DiffSpecColor = texture(gAlbedo, TexCoord);
  vec3 diffuse = DiffSpecColor.rgb;

  // set some directions
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 lightDir = normalize(lightPos - FragPos);
  vec3 halfway = normalize(lightDir + viewDir);

  // set spot light related stuff
  float lightTheta = dot(lightDir, normalize(-inLightDir));
  float epsilon = innerCutoff - outerCutoff;
  float intensity = clamp((lightTheta - outerCutoff) / epsilon, 0.0, 1.0);

  vec3 ambient = texture(gIblSpecular, TexCoord).rgb;

  // compute attenuation using not dist^2 thing
  float attenuation = computeAttenuation(attC, FragPos);

  vec3 diffcolor = attenuation * diffuse * lightColor;
  float spec = pow(max(dot(Normal, halfway), 0.0), shininess);
  vec3 specColor = lightColor * spec * texture(gMaterial, TexCoord).b;

  vec3 color = ambient + (specColor + diffcolor) * attenuation * intensity;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1);
}
float computeAttenuation(vec3 att, vec3 FragPos) {
  float lfragdist = distance(lightPos, FragPos);
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  return min(1 / result, 1.0);
}
