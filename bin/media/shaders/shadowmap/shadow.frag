#version 330

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D diffuseMap1;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float lightIntensity = 0.8;
uniform float ambientCoeff = 0.1; // a good value is 0.1

uniform float shininess = 32.0; // a good value is 32

uniform vec3 attC; // x=c1, y=c2, z=c3

float getShadow(vec4 lightSpacePos) {
  // Now whenever we sample outside the depth map's [0,1] coordinate range,
  // the texture function will always return a depth of 1.0, producing a
  // shadow value of 0.0. The result now looks more plausible:
  //
  vec3 projC = lightSpacePos.xyz / lightSpacePos.w;

  projC = projC * 0.5 + 0.5;
  float closestDepth = texture2D(shadowMap, projC.xy).r; // is r in code
  float currentDepth = projC.z; // depth from the perspective of light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  // pcf
  // Another (partial) solution to these jagged edges is called PCF, or
  // percentage-closer filtering, which is a term that hosts many different
  // filtering functions that produce softer shadows, making them appear less
  // blocky or hard. The idea is to sample more than once from the depth map,
  // each time with slightly different texture coordinates. For each
  // individual sample we check whether it is in shadow or not. All the
  // sub-results are then combined and averaged and we get a nice soft looking
  // shadow.
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for (int x = -1; x <= 1; x++) {
    //
    for (int y = -1; y <= 1; y++) {
      //
      float pcfDepth =
          texture2D(shadowMap, projC.xy + vec2(x, y) + texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  // Here textureSize returns a vec2 of the width and height of the given
  // sampler texture at mipmap level 0. 1 divided over this returns the size of
  // a single texel that we use to offset the texture coordinates, making sure
  // each new sample samples a different depth value. Here we sample 9 values
  // around the projected coordinate's x and y value, test for shadow
  // occlusion, and finally average the results by the total number of samples
  // taken.

  //
  shadow /= 4.0;
  if (projC.z > 1.0) {
    shadow = 0.0;
  }
  return shadow;
}

void main() {
  //
  vec3 color = texture2D(diffuseMap1, TexCoords).rgb;
  vec3 normal = normalize(Normal);
  vec3 lightColor = vec3(lightIntensity);

  // ambient
  vec3 ambient = ambientCoeff * color;

  vec3 lightDir = normalize(lightPos - FragPos);

  float costheta = max(dot(lightDir, normal), 0.0);

  vec3 diffuse = costheta * lightColor;
  // specular
  vec3 viewDir = normalize(viewPos - FragPos);

  vec3 refdir = reflect(-lightDir, normal);
  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfway), 0.0), shininess);
  vec3 specular = spec * lightColor;

  // compute shadows
  float shadow = getShadow(FragPosLightSpace);
  vec3 res = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
  FragColor = vec4(res, 1.0);
}
