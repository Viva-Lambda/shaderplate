#version 430
layout(location = 0) out vec4 topFaceView;
layout(location = 1) out vec4 bottomFaceView;
layout(location = 2) out vec4 rightFaceView;
layout(location = 3) out vec4 leftFaceView;
layout(location = 4) out vec4 frontFaceView;
layout(location = 5) out vec4 backFaceView;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPosVS;

uniform int cubeFace;

void main() {
  switch (cubeFace) {
  case 0:
    topFaceView = vec4(FragPosVS, 1.0);
    break;
  case 1:
    bottomFaceView = vec4(FragPosVS, 1.0);
    break;
  case 2:
    rightFaceView = vec4(FragPosVS, 1.0);
    break;
  case 3:
    leftFaceView = vec4(FragPosVS, 1.0);
    break;
  case 4:
    frontFaceView = vec4(FragPosVS, 1.0);
    break;
  case 5:
    backFaceView = vec4(FragPosVS, 1.0);
    break;
  }
}
