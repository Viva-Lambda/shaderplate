# shaderplate

Simple boilerplate for testing shaders.

Best way to learn what's going on is to look at the shader code and source
code. However there is a doxygen generated site as well.


## Requirements

- Assimp for loading models. This project assumes that `libassimp.so.4` file
  is under `libs/assimp/` directory and `include/assimp` contains all the
  necessary headers from assimp project.

- C++17 as the executables use `<filesystem>` header heavily.

- I am using clang-9 as compiler since it supports the `filesystem` but if you
  would like to use another compiler, it should be okay. The code should be
  fairly portable, apart from the `filesystem`.

- GLFW for window management.

- `glad` as well for function pointers but this one is included.

## Motivation

This project can be used as an example for learning  different topics related
to OpenGL:

Here is a list of covered topics roughly ordered from simple to advanced:

- Rendering a full screen quad: `texture.cpp`

- Rendering a cube with phong lightening: `phong/phong.cpp`

- Rendering a cube with moving lights phong lightening: `phong/phong2.cpp`

- Rendering a cube without specular lights with phong lightening:
  `phong/phongNoSpec.cpp`

- Rendering a cube with heightmap and phong lightening: `heightmap/phong2.cpp`

- Rendering a cube with phong lightening using Uniform Buffer Objects:
  `ubo/phong.cpp`

- Rendering a cube with phong lightening using Shader Storage Buffer Objects:
  `ssbo/phong.cpp`

- Rendering a cube with phong lightening using Parallax Relief Mapping:
  `parallax/phong.cpp`

- Rendering a cube with phong lightening using Parallax Relief Mapping a
  variation:
  `parallax/phong2.cpp`

- Rendering a cube with PBR materials: `pbr/simplepbr.cpp`

- Rendering a model with PBR materials: `model/pbr.cpp`

- Rendering a model with Geometry Shader: `geometry/geometry.cpp`

- Simple Image Processing using Frame Buffer Object:
  `fbo/fbo.cpp`

- Rendering a cube inside a skybox/cubemap: `skybox/sky.cpp`

- Rendering bloom effect (buggy) : `bloom/bloom.cpp`

- Image Based Lightening with textured spheres (buggy): `ibl/ibl.cpp`

- Global Illumination with Precomputed Lightfield Probes (ongoing):
  `lightprobs/lightprobs.cpp`

