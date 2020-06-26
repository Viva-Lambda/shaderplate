#version 330
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Light {
  //
  vec3 pos;
  vec3 color;
};

#define LIGHT_NB 4

uniform Light lights[LIGHT_NB];
uniform sampler2D diffuseTexture;

uniform vec3 viewPos; // camera,eye

void main() {
  //
  vec3 diffColor = texture2D(diffuseTexture, TexCoord).rgb;
  vec3 normal = normalize(Normal);

  vec3 ambient = vec3(0);
  // vec3 viewDir = normalize(viewPos - FragPos);

  vec3 sceneColor = vec3(0);
  for (int i = 0; i < LIGHT_NB; i++) {
    //
    vec3 lightDir = normalize(lights[i].pos - FragPos);
    float costheta = max(dot(lightDir, normal), 0.0);
    vec3 result = lights[i].color * costheta * diffColor;
    float attenuation = length(FragPos - lights[i].pos);
    result *= 1.0 / (attenuation * attenuation);
    sceneColor += result;
  }
  sceneColor += ambient;
  float brightness = dot(sceneColor, vec3(0.2126, 0.7152, 0.072));
  if (brightness > 1.0) {
    BrightColor = vec4(sceneColor, 1.0);
  } else {
    BrightColor = vec4(0, 0, 0, 1);
  }
  FragColor = vec4(sceneColor, 1.0);
}
