#version 430

in vec2 TexCoords;

out vec4 FragColor;

// uniform sampler2D tex;

void main() {
  // FragColor = vec4(texture2D(tex, TexCoords).rgb, 1.0);
  FragColor = vec4(0.2);
}
