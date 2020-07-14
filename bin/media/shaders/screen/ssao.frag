#version 430
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseTexture;

uniform vec3 samples[64];

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);
uniform mat4 projection;

float get_occlusion(mat3 TBN, vec3 fpos) {
  //
  float occlusion = 0.0;
  for (int i = 0; i < kernelSize; i++) {
    vec3 sam = TBN * samples[i];
    sam = fpos + sam * radius;

    vec4 offs = vec4(sam, 1);
    offs = projection * offs;
    offs.xyz / offs.w;
    offs.xyz = offs.xyz * 0.5 + 0.5;

    float sampDepth = texture(gPosition, offs.xy).z;
    float rcheck = smoothstep(0.0, 1.0, radius / abs(fpos.z - sampDepth));
    occlusion += ((sampDepth >= sam.z + bias) ? 1.0 : 0.0) * rcheck;
  }
  occlusion = 1.0 - (occlusion / kernelSize);
  return occlusion;
}

void main() {
  vec3 fpos = texture(gPosition, TexCoords).rgb;
  vec3 norml = texture(gNormal, TexCoords).rgb;
  vec3 randVec = normalize(texture(noiseTexture, TexCoords * noiseScale).rgb);
  vec3 tang = normalize(randVec - norml * dot(randVec, norml));
  vec3 bitang = cross(norml, tang);
  mat3 TBN = mat3(tang, bitang, norml);
  float occlusion = get_occlusion(TBN);
  FragColor = occlusion;
}
