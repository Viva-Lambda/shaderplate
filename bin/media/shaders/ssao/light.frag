#version 430

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

uniform vec3 lightPos;

uniform vec3 lightColor;

uniform vec3 attc = vec3(1, 0, 0);
uniform float shininess = 32.0;
uniform float ambientCoeff = 0.2;

void main() {
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
  float ao = texture(ssao, TexCoords).r; // this is the
  // whole point to calculate this automatically with no map etc
  // and blinn phong
  vec3 ambient = vec3(ambientCoeff * Diffuse * ao);
  vec3 viewDir = normalize(-FragPos); // view position is 0

  vec3 lightDir = normalize(lightPos - FragPos);
  float diffuse = max(dot(Normal, lightDir), 0);

  vec3 hdir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(Normal, hdir), 0), shininess);
  vec3 specular = lightColor * spec;

  float dist = length(lightPos - FragPos);
  float atten = 1 / (attc.x + dist * attc.y + dist * dist * attc.z);
  FragColor = vec4(ambient + atten * (diffuse + specular), 1);
}
