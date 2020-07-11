// screen space ray tracing implementation effort
#include <custom/utils.hpp>

float deltaTime2 = 0.05;
void moveCamera2(GLFWwindow *window) {
  //
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.processKeyboard(FORWARD, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.processKeyboard(LEFT, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.processKeyboard(BACKWARD, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.processKeyboard(RIGHT, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    camera.processKeyBoardRotate(LEFT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    camera.processKeyBoardRotate(RIGHT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    camera.processKeyBoardRotate(FORWARD, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    camera.processKeyBoardRotate(BACKWARD, 0.7f);
  }
}
void moveLight2(GLFWwindow *window) {
  // move light
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
    spotLight.position.y += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    spotLight.position.y -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    spotLight.position.x += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    spotLight.position.x -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    spotLight.position.z -= deltaTime2; // the axis are inverse
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    spotLight.position.z += deltaTime2;
  }
}

void rotateLight2(GLFWwindow *window) {
  // move light
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_LEFT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_RIGHT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_FORWARD, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_BACKWARD, 0.7f);
  }
}
void processInput_proc2(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  moveCamera2(window);
  moveLight2(window);
  rotateLight2(window);
  captureScreen(window);
}

// ----------------------- load models ---------------------------------
unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

Model loadBackpackModel() {
  fs::path modPath = modelPath / "backpack" / "backpack.obj";
  // stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, true, false, false);
  return modelB;
}

// ---------------------- load textures --------------------------------

Shader loadEquirectangulareToCubemapShader() {
  fs::path vpath = shaderDirPath / "screen" / "env2cube.vert"; // DONE
  fs::path fpath = shaderDirPath / "screen" / "env2cube.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}
Shader loadIrradianceShader() {
  fs::path vpath = shaderDirPath / "screen" / "env2cube.vert";   // DONE
  fs::path fpath = shaderDirPath / "screen" / "irradiance.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}
Shader loadPrefilterShader() {
  fs::path vpath = shaderDirPath / "screen" / "env2cube.vert";  // DONE
  fs::path fpath = shaderDirPath / "screen" / "prefilter.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}
Shader loadBrdfShader() {
  fs::path vpath = shaderDirPath / "screen" / "tquad.vert"; // DONE
  fs::path fpath = shaderDirPath / "screen" / "brdf.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}
Shader loadPbrShader() {
  fs::path vpath = shaderDirPath / "screen" / "pbr.vert"; // DONE
  fs::path fpath = shaderDirPath / "screen" / "pbr.frag"; // DONE
  Shader pbrShader(vpath.c_str(), fpath.c_str());
  pbrShader.useProgram();
  pbrShader.setIntUni("gDepth", 0);
  pbrShader.setIntUni("gNormal", 1);
  pbrShader.setIntUni("gAlbedo", 2);
  pbrShader.setIntUni("gMaterial", 3);
  pbrShader.setIntUni("gAmbient", 4);

  return pbrShader;
}
Shader loadLampShader() {
  fs::path vpath = shaderDirPath / "screen" / "basic_light.vert";
  fs::path fpath = shaderDirPath / "screen" / "basic_color_light.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  return lightShader;
}

Shader loadBackgroundShader() {
  fs::path vpath = shaderDirPath / "screen" / "background.vert";
  fs::path fpath = shaderDirPath / "screen" / "background.frag";
  Shader backgroundShader(vpath.c_str(), fpath.c_str());
  return backgroundShader;
}

Shader loadGeometryShader() {
  //
  fs::path vpath = shaderDirPath / "screen" / "geobuffer.vert";
  fs::path fpath = shaderDirPath / "screen" / "geobuffer.frag";
  Shader gShader(vpath.c_str(), fpath.c_str());
  gShader.useProgram();
  gShader.setIntUni("albedoMap", 0);
  gShader.setIntUni("normalMap", 1);
  gShader.setIntUni("roughnessMap", 2);
  gShader.setIntUni("metallicMap", 3);
  gShader.setIntUni("aoMap", 4);
  gShader.setIntUni("irradianceMap", 5);
  gShader.setIntUni("prefilterMap", 6);
  gShader.setIntUni("brdfLUT", 7);

  return gShader;
}

Shader loadRayConeShader() {
  fs::path vpath = shaderDirPath / "screen" / "conetrace.vert";
  fs::path fpath = shaderDirPath / "screen" / "conetrace.frag";
  Shader coneShader(vpath.c_str(), fpath.c_str());
  coneShader.useProgram();
  coneShader.setIntUni("gDepth", 0);
  coneShader.setIntUni("lightBuffer", 1);
  coneShader.setIntUni("gNormal", 2);
  coneShader.setIntUni("gMaterial", 3);
  return coneShader;
}

Shader loadHizShader() {
  fs::path vpath = shaderDirPath / "screen" / "tquad.vert";
  fs::path fpath = shaderDirPath / "screen" / "hizbuffer.frag";
  Shader s(vpath.c_str(), fpath.c_str());
  s.useProgram();
  s.setIntUni("HiZBCopyTexture", 0);
  return s;
}
Shader loadVisibilityShader() {
  fs::path vpath = shaderDirPath / "screen" / "tquad.vert";
  fs::path fpath = shaderDirPath / "screen" / "visibility.frag";
  Shader s(vpath.c_str(), fpath.c_str());
  s.useProgram();
  s.setIntUni("HiZBuffer", 0);
  s.setIntUni("visibilityMap", 1);
  return s;
}

void setFboTexture(GLuint &tex,
                   GLenum informat,     // GL_RGB16F
                   GLenum outformat,    // GL_RGBA
                   GLenum bufferType,   // GL_FLOAT
                   unsigned int width,  // viewport width
                   unsigned int height, // viewport height
                   unsigned int &attachmentNb) {
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, informat, width, height, 0, outformat,
               bufferType, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNb,
                         GL_TEXTURE_2D, tex, 0);
  attachmentNb++;
}
void loadEnvironmentCubemap(GLuint &envCubemap, unsigned int captureWidth,
                            unsigned int captureHeight) {
  glGenTextures(1, &envCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, captureWidth,
                 captureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void setGBufferDepthRbo(GLuint &rbo) {
  //
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINWIDTH,
                        WINHEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
}

float lerp(float n, float n2, float f) { return n + f * (n2 - n); }

void genCaptureFboRbo(GLuint &captureFBO, GLuint &captureRBO,
                      GLuint captureWidth, GLuint captureHeight) {
  glGenFramebuffers(1, &captureFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glGenRenderbuffers(1, &captureRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureWidth,
                        captureHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, captureRBO);
}

std::vector<glm::mat4> getCaptureView() {
  std::vector<glm::mat4> captureViews{
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f))};
  return captureViews;
}

void equirectToCubemapTransform(Shader equirectangularToCubemapShader,
                                glm::mat4 captureProjection,
                                GLuint &environmentHdrMap, GLuint captureWidth,
                                GLuint captureHeight,
                                std::vector<glm::mat4> captureViews,
                                GLuint &envCubemap, GLuint &captureFBO) {
  //
  equirectangularToCubemapShader.useProgram();
  equirectangularToCubemapShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, environmentHdrMap);

  glViewport(0, 0, captureWidth, captureHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  std::vector<VertexAttrib> env2cubeVa{{0, 3}};
  for (unsigned int i = 0; i < 6; ++i) {
    equirectangularToCubemapShader.setMat4Uni("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCubeD();
  }
}

void genIrradianceCubeMap(GLuint lmapResolutionWidth,
                          GLuint lmapResolutionHeight,
                          GLuint &irradianceCubemap, GLuint &captureFBO,
                          GLuint &captureRBO) {
  loadEnvironmentCubemap(irradianceCubemap, lmapResolutionWidth,
                         lmapResolutionHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                        lmapResolutionWidth, lmapResolutionHeight);
}

void computeIrradianceMap(Shader irradianceShader, glm::mat4 captureProjection,
                          GLuint &envCubemap, GLuint lmapResolutionWidth,
                          GLuint lmapResolutionHeight, GLuint &captureFBO,
                          std::vector<glm::mat4> captureViews,
                          GLuint &irradianceCubemap) {

  irradianceShader.useProgram();
  irradianceShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glViewport(0, 0, lmapResolutionWidth, lmapResolutionHeight);
  std::vector<VertexAttrib> env2cubeVa{{0, 3}};
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    irradianceShader.setMat4Uni("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           irradianceCubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCubeD();
  }
}

void genPrefilterMap(GLuint &prefilterMap, GLuint prefilterMapWidth,
                     GLuint prefilterMapHeight) {
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                 prefilterMapWidth, prefilterMapHeight, 0, GL_RGB, GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter
                                            // to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the
  // required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void computePrefilterMap(Shader prefilterShader, Shader geometryShader,
                         glm::mat4 captureProjection, GLuint captureWidth,
                         GLuint &envCubemap, GLuint &captureFBO,
                         GLuint &captureRBO,
                         std::vector<glm::mat4> captureViews,
                         GLuint &prefilterMap, GLuint prefilterMapWidth,
                         GLuint prefilterMapHeight) {
  prefilterShader.useProgram();
  prefilterShader.setMat4Uni("projection", captureProjection);
  prefilterShader.setFloatUni("cubemapResolution", (float)captureWidth);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  std::vector<VertexAttrib> env2cubeVa{{0, 3}};
  int maxMipLevels = 5;
  geometryShader.useProgram();
  geometryShader.setFloatUni("maxMipLevels", (float)maxMipLevels);
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    GLuint mipWidth = prefilterMapWidth * std::pow(0.5, mip);
    GLuint mipHeight = prefilterMapHeight * std::pow(0.5, mip);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth,
                          mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader.useProgram();
    prefilterShader.setFloatUni("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4Uni("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderCubeD();
    }
  }
}

void attachBrdfToCapture(GLuint &captureFBO, GLuint &captureRBO,
                         GLuint &brdfLutTexture, GLuint captureWidth,
                         GLuint captureHeight) {
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureWidth,
                        captureHeight);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         brdfLutTexture, 0);
}

void genTextures1(GLuint &metallicMap, GLuint &baseColorMap, GLuint &normalMap,
                  GLuint &roughnessMap, GLuint &aoMap,
                  GLuint &environmentHdrMap) {
  metallicMap = loadTexture2d("paintedmetal", "paintedmetal_metallic.jpg");
  gerr();

  baseColorMap = loadTexture2d("paintedmetal", "paintedmetal_basecolor.jpg");
  gerr();

  normalMap = loadTexture2d("paintedmetal", "paintedmetal_normal.jpg");
  gerr();

  roughnessMap = loadTexture2d("paintedmetal", "paintedmetal_roughness.jpg");
  gerr();

  aoMap = loadTexture2d("paintedmetal", "paintedmetal_ao.jpg");
  gerr();

  loadHdrTexture("newport", "Newport_Loft_Ref.hdr", environmentHdrMap);
  gerr();
}

void activateTextures1(GLuint &metallicMap, GLuint &baseColorMap,
                       GLuint &normalMap, GLuint &roughnessMap, GLuint &aoMap) {
  // activate textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, baseColorMap);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normalMap);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, roughnessMap);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, metallicMap);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, aoMap);
  gerr();
}

void drawPaintedMetalSphere(Shader geometryShader, glm::mat4 view,
                            glm::mat4 model, GLuint &metallicMap,
                            GLuint &baseColorMap, GLuint &normalMap,
                            GLuint &roughnessMap, GLuint &aoMap,
                            GLuint &irradianceCubemap, GLuint &prefilterMap,
                            GLuint &brdfLutTexture) {
  // activate textures
  activateTextures1(metallicMap, baseColorMap, normalMap, roughnessMap, aoMap);
  gerr();
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemap);

  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, brdfLutTexture);

  geometryShader.useProgram();
  geometryShader.setMat4Uni("model", model);
  geometryShader.setMat4Uni("view", view);
  geometryShader.setFloatUni("fresnel", 0.04); // 0.4
  geometryShader.setVec3Uni("viewPos", camera.pos);
  geometryShader.setVec3Uni("lightPos", spotLight.position);
  renderSphere();
}

void genTextures2(GLuint &metallicMap2, GLuint &baseColorMap2,
                  GLuint &normalMap2, GLuint &roughnessMap2) {
  metallicMap2 = loadTexture2d("gold", "lightgold_metallic.png");
  gerr();

  baseColorMap2 = loadTexture2d("gold", "lightgold_albedo.png");
  gerr();

  normalMap2 = loadTexture2d("gold", "lightgold_normal-ogl.png");
  gerr();

  roughnessMap2 = loadTexture2d("gold", "lightgold_roughness.png");
  gerr();
}

void activateTextures2(GLuint &metallicMap2, GLuint &baseColorMap2,
                       GLuint &normalMap2, GLuint &roughnessMap2) {
  // activate textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, baseColorMap2);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normalMap2);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, roughnessMap2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, metallicMap2);
  gerr();
}

void drawGoldSphere(Shader geometryShader, glm::mat4 view, glm::mat4 model,
                    GLuint &metallicMap2, GLuint &baseColorMap2,
                    GLuint &normalMap2, GLuint &roughnessMap2,
                    GLuint &irradianceCubemap, GLuint &prefilterMap,
                    GLuint &brdfLutTexture) {
  // activate textures
  activateTextures2(metallicMap2, baseColorMap2, normalMap2, roughnessMap2);
  gerr();
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemap);

  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, brdfLutTexture);

  geometryShader.useProgram();
  geometryShader.setMat4Uni("model", model);
  geometryShader.setMat4Uni("view", view);
  geometryShader.setFloatUni("fresnel", 0.27035); // 0.4
  geometryShader.setVec3Uni("viewPos", camera.pos);
  geometryShader.setVec3Uni("lightPos", spotLight.position);
  renderSphere();
  // renderCubeInTangentSpace();
}

void drawScene(Shader geometryShader, glm::mat4 view, glm::mat4 model,
               GLuint &metallicMap2, GLuint &baseColorMap2, GLuint &normalMap2,
               GLuint &roughnessMap2, GLuint &metallicMap, GLuint &baseColorMap,
               GLuint &normalMap, GLuint &roughnessMap, GLuint &aoMap,
               GLuint &irradianceCubemap, GLuint &prefilterMap,
               GLuint &brdfLutTexture) {
  glm::vec3 objectPos = glm::vec3(3.0, -0.5, -3.0);
  glm::vec3 objectPos2 = glm::vec3(0.5, -0.5, -5.0);
  model = glm::translate(model, objectPos);
  model = glm::scale(model, glm::vec3(1.0f));

  gerr();
  drawPaintedMetalSphere(geometryShader, view, model, metallicMap, baseColorMap,
                         normalMap, roughnessMap, aoMap, irradianceCubemap,
                         prefilterMap, brdfLutTexture);
  model = glm::mat4(1);
  model = glm::translate(model, objectPos2);
  model = glm::scale(model, glm::vec3(1.5f));
  drawGoldSphere(geometryShader, view, model, metallicMap2, baseColorMap2,
                 normalMap2, roughnessMap2, irradianceCubemap, prefilterMap,
                 brdfLutTexture);
}
void drawSceneDepth(Shader hizShader, glm::mat4 view, glm::mat4 model,
                    GLuint &mipRbo, int mw, int mh, GLuint &HiZBufferTexture,
                    unsigned int level, GLuint &HiZBCopyTexture,
                    glm::vec2 offs) {

  // read from here and write to attached texture of hizFBO
  glBindRenderbuffer(GL_RENDERBUFFER, mipRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mw, mh);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         HiZBufferTexture, // write to this texture
                         level );
  gerrf();
  gerr();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, HiZBCopyTexture);

  hizShader.useProgram();
  hizShader.setIntUni("mipmapLevel", level-1);
  hizShader.setVec2Uni("pixelOffset", offs);
  renderQuad();
  gerr();
}

struct MipMapInfo {
  const int width, height;
  const unsigned int level;
  MipMapInfo(int w, int h, unsigned int l) : width(w), height(h), level(l) {}
};

void genHiZTexture(GLuint &hizTex, std::vector<MipMapInfo> &ms) {
  glGenTextures(1, &hizTex);
  float sceneAspectRatio = (float)WINWIDTH / (float)WINHEIGHT;
  glBindTexture(GL_TEXTURE_2D, hizTex);
  float imarr[WINWIDTH * WINHEIGHT * 3];
  std::fill(std::begin(imarr), std::end(imarr), 1.0f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINWIDTH, WINHEIGHT, 0, GL_RGB,
               GL_FLOAT, &imarr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  ms.clear();
  int mipw = WINWIDTH;
  int height = WINHEIGHT;
  unsigned int level = 0;
  while (mipw >= 1 && height >= 1) {
    MipMapInfo msize(mipw, height, level);
    ms.push_back(msize);
    mipw = static_cast<int>(mipw / 2);
    height = static_cast<int>(mipw / sceneAspectRatio);
    level++;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level+1);
  for (unsigned int i = 0; i < ms.size(); i++) {
    MipMapInfo msize = ms[i];
    std::vector<float> iarr(msize.width * msize.height * 3, 1.0f);
    glTexImage2D(GL_TEXTURE_2D, msize.level, GL_RGB16F, msize.width,
                 msize.height, 0, GL_RGB, GL_FLOAT, &iarr[0]);
  }
}
void genHiZCopy(GLuint &hizTexCopy, std::vector<MipMapInfo> &ms) {
  glGenTextures(1, &hizTexCopy);
  glBindTexture(GL_TEXTURE_2D, hizTexCopy);
  float imarr[WINWIDTH * WINHEIGHT * 3];
  std::fill(std::begin(imarr), std::end(imarr), 1.0f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINWIDTH, WINHEIGHT, 0, GL_RGB,
               GL_FLOAT, &imarr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, ms.size());
  for (unsigned int i = 0; i < ms.size(); i++) {
    MipMapInfo msize = ms[i];
    std::vector<float> iarr(msize.width * msize.height * 3, 1.0f);
    glTexImage2D(GL_TEXTURE_2D, msize.level, GL_RGB16F, msize.width,
                 msize.height, 0, GL_RGB, GL_FLOAT, &iarr[0]);
  }
}

int main() {
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Screen Space Shading Example", NULL, NULL);

  if (window == NULL) {
    std::cout << "Loading GLFW window had failed" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  //
  // dealing with mouse actions
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, mouse_scroll_callback);

  // deal with input method
  // glfw should capture cursor movement as well
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // load opengl function
  if (gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)) == 0) {
    std::cout << "Failed to start glad" << std::endl;
    glfwTerminate();
    return -1;
  }

  // set default view port
  glViewport(0, 0, WINWIDTH, WINHEIGHT);

  // define opengl states
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL); // skyboxing
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glDisable(GL_BLEND);

  // scene description:
  // painted metal cube near a gold sphere
  GLuint metallicMap = 0;
  GLuint baseColorMap = 0;
  GLuint normalMap = 0;
  GLuint roughnessMap = 0;
  GLuint aoMap = 0;
  GLuint environmentHdrMap = 0;
  genTextures1(metallicMap, baseColorMap, normalMap, roughnessMap, aoMap,
               environmentHdrMap);

  GLuint metallicMap2 = 0, baseColorMap2 = 0, normalMap2 = 0, roughnessMap2 = 0;
  genTextures2(metallicMap2, baseColorMap2, normalMap2, roughnessMap2);

  // with a sphere controllable spotlight inside a newport environment map

  // High level overview of the pipeline

  // 1. generate a prefiltered cubemap to be used as a fallback texture before
  // rendering DONE

  // start rendering

  // 2. geometry buffer pass

  // 3. lightening buffer pass

  // 4. screen space tracing pass

  // 5. screen blur pass

  // 6. cone tracing pass

  // ------------------------- A. set up  -----------------------------------
  // set up required buffers for prefiltering environment map

  GLuint captureFBO, captureRBO;
  unsigned int captureWidth = 512, captureHeight = 512; // usually good values

  genCaptureFboRbo(captureFBO, captureRBO, captureWidth, captureHeight);
  gerr();

  // load environment map
  GLuint envCubemap;
  loadEnvironmentCubemap(envCubemap, captureWidth, captureHeight);
  gerr();

  // pbr: set up projection and view matrices for capturing data onto the 6
  Shader pbrShader = loadPbrShader();
  Shader geometryShader = loadGeometryShader();
  gerr();
  // cubemap face directions
  // ----------------------------------------------------------------------------------------------
  glm::mat4 captureProjection =
      glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  std::vector<glm::mat4> captureViews = getCaptureView();
  gerr();

  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  // ----------------------------------------------------------------------
  Shader equirectangularToCubemapShader = loadEquirectangulareToCubemapShader();
  gerr();
  equirectToCubemapTransform(equirectangularToCubemapShader, captureProjection,
                             environmentHdrMap, captureWidth, captureHeight,
                             captureViews, envCubemap, captureFBO);
  gerr();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // generate mipmaps from first mip face (combatting visible
  // dots artifact)
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  gerr();

  unsigned int lmapResolutionWidth = 32, lmapResolutionHeight = 32;
  GLuint irradianceCubemap;
  genIrradianceCubeMap(lmapResolutionWidth, lmapResolutionHeight,
                       irradianceCubemap, captureFBO, captureRBO);
  gerr();

  // pbr: solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  // -----------------------------------------------------------------------------
  Shader irradianceShader = loadIrradianceShader();
  gerr();
  computeIrradianceMap(irradianceShader, captureProjection, envCubemap,
                       lmapResolutionWidth, lmapResolutionHeight, captureFBO,
                       captureViews, irradianceCubemap);
  gerr();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter
  // scale.
  // --------------------------------------------------------------------------------
  GLuint prefilterMap;
  GLuint prefilterMapWidth = 128, prefilterMapHeight = 128;
  genPrefilterMap(prefilterMap, prefilterMapWidth, prefilterMapHeight);
  gerr();

  // pbr: run a quasi monte-carlo simulation on the environment lighting to
  // create a prefilter (cube)map.
  // ----------------------------------------------------------------------------------------------------
  Shader prefilterShader = loadPrefilterShader();
  gerr();
  computePrefilterMap(prefilterShader, geometryShader, captureProjection,
                      captureWidth, envCubemap, captureFBO, captureRBO,
                      captureViews, prefilterMap, prefilterMapWidth,
                      prefilterMapHeight);
  gerr();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------
  GLuint brdfLutTexture =
      loadHdrTexture(captureWidth, captureHeight, GL_RG16F, GL_RG);
  gerr();

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  attachBrdfToCapture(captureFBO, captureRBO, brdfLutTexture, captureWidth,
                      captureHeight);
  gerr();

  glViewport(0, 0, captureWidth, captureHeight);
  Shader brdfShader = loadBrdfShader();
  gerr();
  std::vector<VertexAttrib> brdfVa{{0, 3}, {1, 2}};
  brdfShader.useProgram();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();
  gerr();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // -------------------------------------------------------------------------

  //--------------------------------- B. rendering ---------------------------

  // hiz buffer
  GLuint hizFBO, mipRbo;
  glGenFramebuffers(1, &hizFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, hizFBO);

  glGenRenderbuffers(1, &mipRbo);
  glBindRenderbuffer(GL_RENDERBUFFER, mipRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WINWIDTH,
                        WINHEIGHT);

  // get number of mipmap levels required for texture
  // generate HiZ texture
  GLuint HiZBufferTexture = 0;
  GLuint HiZBCopyTexture = 0;
  std::vector<MipMapInfo> mipmaps;

  genHiZTexture(HiZBufferTexture, mipmaps);
  genHiZCopy(HiZBCopyTexture, mipmaps);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Shader hizShader = loadHizShader();
  Shader visibilityShader = loadVisibilityShader();

  // -----------------------------------------------------------------------

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set up required buffers for rendering
  GLuint geometry_fbo, depthRbo; // required for g buffer pass
  glGenFramebuffers(1, &geometry_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);

  gerr();

  GLuint attachmentNb = 0;

  // stores fragement distance to camera
  GLuint gDepth;
  setFboTexture(gDepth, GL_RGB16F, GL_RGB, GL_FLOAT, WINWIDTH, WINHEIGHT,
                attachmentNb);

  // stores fragment normal in view space
  GLuint gNormal;
  setFboTexture(gNormal, GL_RGBA16F, GL_RGBA, GL_FLOAT, WINWIDTH, WINHEIGHT,
                attachmentNb);

  // stores color values for diffuse etc
  GLuint gAlbedo;
  setFboTexture(gAlbedo, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, WINWIDTH,
                WINHEIGHT, attachmentNb);

  // stores material information
  GLuint gMaterial;
  setFboTexture(gMaterial, GL_RGB16F, GL_RGBA, GL_FLOAT, WINWIDTH, WINHEIGHT,
                attachmentNb);

  // stores ambient light information
  GLuint gAmbient;
  setFboTexture(gAmbient, GL_RGB16F, GL_RGBA, GL_FLOAT, WINWIDTH, WINHEIGHT,
                attachmentNb);

  // stores screen space depth information
  GLuint gSDepth;
  setFboTexture(gSDepth, GL_RGB16F, GL_RGB, GL_FLOAT, WINWIDTH, WINHEIGHT,
                attachmentNb);

  // setting color attachments
  GLuint attachments[attachmentNb];
  for (unsigned int i = 0; i < attachmentNb; i++) {
    attachments[i] = GL_COLOR_ATTACHMENT0 + i;
  }

  glDrawBuffers(attachmentNb, attachments);
  gerr();

  setGBufferDepthRbo(depthRbo);
  gerrf();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // implementing the fbo for lightening
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GLuint lightFBO;
  glGenFramebuffers(1, &lightFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);

  gerr();
  GLuint lightTexture;
  GLuint lightAttachment = 0;
  setFboTexture(lightTexture, GL_RGBA16F, GL_RGBA, GL_FLOAT, WINWIDTH,
                WINHEIGHT, lightAttachment);
  gerrf();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ----------------- needed shaders are ------------------------------
  gerr();

  // lightening shader

  // screen space ray tracing + cone tracing shader
  Shader rayConeShader = loadRayConeShader();
  gerr();

  int srcw, srch;
  glfwGetFramebufferSize(window, &srcw, &srch);
  gerr();
  glViewport(0, 0, srcw, srch);
  // initialize static shader uniforms before rendering
  // --------------------------------------------------
  float nearPlane = 0.1f;
  float farPlane = 100.0f;
  glm::mat4 projection =
      glm::perspective(glm::radians(camera.zoom),
                       (float)WINWIDTH / (float)WINHEIGHT, nearPlane, farPlane);
  pbrShader.useProgram();
  pbrShader.setMat4Uni("projection", projection);

  Shader lampShader = loadLampShader();
  gerr();
  lampShader.useProgram();
  lampShader.setMat4Uni("projection", projection);

  Shader backgroundShader = loadBackgroundShader();
  backgroundShader.useProgram();
  backgroundShader.setMat4Uni("projection", projection);
  backgroundShader.setIntUni("envMap", 0);
  gerr();

  geometryShader.useProgram();
  geometryShader.setMat4Uni("projection", projection);
  gerr();

  // geometryShader.setIntUni();
  std::vector<VertexAttrib> geoVa{{0, 3}, {1, 3}, {2, 2}};
  std::vector<VertexAttrib> backVa{{0, 3}};
  std::vector<VertexAttrib> lampVa{{0, 3}, {1, 3}, {2, 2}};
  std::vector<VertexAttrib> pbrVa{{0, 3}, {1, 2}};
  std::vector<VertexAttrib> coneVa{{0, 3}, {1, 2}};

  while (!glfwWindowShouldClose(window)) {
    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc2(window);
    //
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    
    // 1. geometry pass: render scene geometry color data into geometry buffer
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
      // set shader uniforms
      drawScene(geometryShader, view, model, metallicMap2, baseColorMap2,
                normalMap2, roughnessMap2, metallicMap, baseColorMap, normalMap,
                roughnessMap, aoMap, irradianceCubemap, prefilterMap,
                brdfLutTexture);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 2. Hi-Z buffer pass: generate hierarchical depth buffer
    {
      glClear(GL_COLOR_BUFFER_BIT);
      glm::vec2 offs = glm::vec2(0);
      offs.x = WINWIDTH % 2 == 0 ? 1 : 2;
      offs.y = WINHEIGHT % 2 == 0 ? 1 : 2;

      glBindFramebuffer(GL_FRAMEBUFFER, hizFBO);
      // first copy from screen space depth texture of gBuffer
      drawSceneDepth(hizShader, model, view, mipRbo, WINWIDTH, WINHEIGHT,
                     HiZBufferTexture, 0, gSDepth, offs);
      //
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glCopyImageSubData(HiZBufferTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                         HiZBCopyTexture, GL_TEXTURE_2D, 0, 0, 0, 0, WINWIDTH,
                         WINHEIGHT,
                         0); // copy previously written mipmap to current
      gerr();
      glBindFramebuffer(GL_FRAMEBUFFER, hizFBO);

      // now sample previous mipmaps from hiz buffer and hiz copy buffer
      for (unsigned int i = 1; i < mipmaps.size(); i++) {
        MipMapInfo minfo = mipmaps[i];
        int mw = minfo.width;
        int mh = minfo.height;
        offs.x = mw % 2 == 0 ? 1 : 2;
        offs.y = mh % 2 == 0 ? 1 : 2;
        auto level = minfo.level;
        glViewport(0, 0, mw, mh);
        glCopyImageSubData(HiZBufferTexture, GL_TEXTURE_2D, level, 0, 0, 0,
                           HiZBCopyTexture, GL_TEXTURE_2D, level, 0, 0, 0, mw,
                           mh,
                           0); // copy previously written mipmap to current
        gerr();

        drawSceneDepth(hizShader, model, view, mipRbo, mw, mh, HiZBufferTexture,
                       level, HiZBCopyTexture, offs);
        gerr();
      }
      glViewport(0, 0, WINWIDTH, WINHEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 3. lightening pass: render lightening to be refined later on
    {
      glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
      glClear(GL_COLOR_BUFFER_BIT);

      // activate textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gDepth);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, gNormal);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, gAlbedo);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, gMaterial);

      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, gAmbient);

      pbrShader.useProgram();

      pbrShader.setVec3Uni("ViewPosVS",
                           glm::vec3(view * glm::vec4(camera.pos, 1.0)));
      pbrShader.setVec3Uni(
          "LightPosVS", glm::vec3(view * glm::vec4(spotLight.position, 1.0)));

      pbrShader.setVec3Uni("lightColor", glm::vec3(30.0));
      // if doing phong lightening
      // pbrShader.setVec3Uni("inLightDir", spotLight.front);

      renderQuad();
      gerr();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. cone tracing pass:
    {
      glClear(GL_COLOR_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gDepth);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, lightTexture);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, gNormal);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, gMaterial);

      rayConeShader.useProgram();
      glm::mat4 view = camera.getViewMatrix();

      rayConeShader.setMat4Uni("view", view);
      rayConeShader.setMat4Uni("projection", projection);
      rayConeShader.setVec3Uni("viewDir", camera.front);
      rayConeShader.setVec3Uni("viewPos", camera.pos);
      rayConeShader.setFloatUni("csNearPlaneZ", -nearPlane);
      rayConeShader.setFloatUni("csDepthThickness", 10.0);
      rayConeShader.setFloatUni("traceStride", 1.0);
      rayConeShader.setFloatUni("csMaxSteps", 15.0);
      rayConeShader.setFloatUni("csMaxDistance", 20.0);
      rayConeShader.setFloatUni("jitter", 0.1);
      rayConeShader.setFloatUni("coneAngleZeta", 0.244);
      rayConeShader.setFloatUni("max_shine", 25.0);
      rayConeShader.setFloatUni("cone_trace_iteration", 6.0);
      rayConeShader.setFloatUni("fadeStart", 0.4);
      rayConeShader.setFloatUni("fadeEnd", 1.0);

      renderQuad();
    }

    // 3.5 draw light source
    glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WINWIDTH, WINHEIGHT, 0, 0, WINWIDTH, WINHEIGHT,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    gerr();
    lampShader.useProgram();
    lampShader.setVec3Uni("lightColor", glm::vec3(30.0f));

    model = glm::mat4(1.0f);
    model = glm::translate(model, spotLight.position);
    model = glm::scale(model, glm::vec3(0.2f));
    lampShader.setMat4Uni("model", model);
    lampShader.setMat4Uni("view", view);
    renderSphere();
    gerr();

    // 4. background

    // glm::mat4 view = camera.getViewMatrix();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    backgroundShader.useProgram();
    backgroundShader.setMat4Uni("view", view);
    backgroundShader.setMat4Uni("projection", projection);
    renderCubeD();
    gerr();
    // swap buffer vs
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
