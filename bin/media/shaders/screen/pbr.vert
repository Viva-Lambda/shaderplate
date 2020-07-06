#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform sampler2D linearDepthMap;   // from GBuffer
uniform sampler2D normalMapGBuffer; // from GBuffer

uniform vec3 camPos;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

void main() {
  vec3 viewRay = texture(linearDepthMap, aTexCoord).xyz;
  float viewDist = texture(linearDepthMap, aTexCoord).a;
  FragPos = viewRay * viewDist + camPos;
  vec3 normalRayViewSpace = texture(normalMapGBuffer, aTexCoord).xyz;
  float normalDist = texture(normalMapGBuffer, aTexCoord).a;
  TexCoord = aTexCoord;
  Normal = normalRayViewSpace * normalDist + camPos;

  // classic gl pos
  gl_Position = vec4(aPos, 1);
}
