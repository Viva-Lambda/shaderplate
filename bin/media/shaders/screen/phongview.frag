#version 430
// layout(location = 0) out vec3 FragColor;
out vec3 FragColor;

in vec2 TexCoord;
in vec3 ViewRay;

// material parameters
uniform sampler2D gDepth;    // from GBuffer
uniform sampler2D gNormal;   // from GBuffer
uniform sampler2D gAlbedo;   // from GBuffer
uniform sampler2D gMaterial; // from GBuffer
uniform sampler2D gAmbient; // from GBuffer
// lights
uniform vec3 lightPos; // in world space
uniform vec3 lightColor;
uniform vec3 inLightDir; // in world space

uniform vec3 viewPos; // in world space
uniform mat4 view;

// some phong related uniforms
uniform float shininess = 32.0; // a good value is 32
uniform float innerCutoff = 0.91;
uniform float outerCutoff = 0.82;
uniform vec3 attC = vec3(1, 0.72, 0.04); // x=c1, y=c2, z=c3

// phong related functions
float computeAttenuation(vec3 att, vec3 fpos);

void main() {
  //
  vec3 FragPosVS = texture(gDepth, TexCoord).xyz;

  vec3 NormalVS = texture(gNormal, TexCoord).xyz; // view space
  vec4 DiffSpecColor = texture(gAlbedo, TexCoord);
  vec3 diffuse = DiffSpecColor.rgb;
  vec3 ViewPosVS = vec3(view * vec4(viewPos, 1)); // view space

  vec3 LightPosVS = vec3(view * vec4(lightPos, 1));

  // set some directions
  vec3 viewDirVS = normalize(ViewPosVS - FragPosVS);
  vec3 lightDirVS = normalize(LightPosVS - FragPosVS);
  vec3 halfway = normalize(lightDirVS + viewDirVS);

  // set spot light related stuff
  vec3 InLightDirVS = vec3(view * vec4(inLightDir, 1));
  float lightTheta = dot(lightDirVS, normalize(-InLightDirVS));
  float epsilon = innerCutoff - outerCutoff;
  float intensity = clamp((lightTheta - outerCutoff) / epsilon, 0.0, 1.0);

  vec3 ambient = texture(gAmbient, TexCoord).rgb;

  // compute attenuation using not dist^2 thing
  float attenuation = computeAttenuation(attC, FragPosVS);

  vec3 diffcolor = attenuation * diffuse * lightColor;
  float spec = pow(max(dot(NormalVS, halfway), 0.0), shininess);
  vec3 specColor = lightColor * spec * texture(gMaterial, TexCoord).g;

  vec3 color = ambient + (specColor + diffcolor) * attenuation * intensity;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec3(color);
}
float computeAttenuation(vec3 att, vec3 FragPos) {
  float lfragdist = distance(lightPos, FragPos);
  float distSqr = lfragdist * lfragdist;
  float att1 = lfragdist * att.y;
  float att2 = distSqr * att.z;
  float result = att.x + att2 + att1;
  return min(1 / result, 1.0);
}
