#version 430
in vec2 TexCoord;

layout(location = 0) out vec4 SampleDepth; // depth value

uniform sampler2D HiZBCopyTexture;

uniform vec2 pixelOffset;
uniform int mipmapLevel; // previous mipmap level

/**
 * adapted from
 * https://github.com/LukasBanana/ForkENGINE/blob/master/shaders/SSCTHiZMapPixelShader.glsl
 * */
void main() {
  ivec2 pixelCoord = ivec2(gl_FragCoord.xy * vec2(2));

  vec4 texels[2]; // required for downsampling

  texels[0].rg = texelFetch(HiZBCopyTexture, ivec2(pixelCoord), mipmapLevel).rg;
  texels[0].ba = texelFetch(HiZBCopyTexture,
                            pixelCoord + ivec2(pixelOffset.x, 0), mipmapLevel)
                     .rg;
  texels[1].rg = texelFetch(HiZBCopyTexture,
                            pixelCoord + ivec2(0, pixelOffset.y), mipmapLevel)
                     .rg;
  texels[1].ba =
      texelFetch(HiZBCopyTexture,
                 pixelCoord + ivec2(pixelOffset.x, pixelOffset.y), mipmapLevel)
          .rg;

  /* Down-sample texels */
  float minZ =
      min(min(texels[0].r, texels[0].b), min(texels[1].r, texels[1].b));
  float maxZ =
      max(max(texels[0].g, texels[0].a), max(texels[1].g, texels[1].a));

  SampleDepth = vec4(minZ, maxZ, 0.0, 1.0);
}
