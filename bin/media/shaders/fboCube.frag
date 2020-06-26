#version 330

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex;

void main() {
  FragColor = texture2D(tex, TexCoord);
  // FragColor = vec4(1.0);
}
