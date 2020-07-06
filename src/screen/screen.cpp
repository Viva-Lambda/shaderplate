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
  fs::path vpath = shaderDirPath / "screen" / "brdf.vert"; // DONE
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
  pbrShader.setIntUni("albedoMap", 0);
  pbrShader.setIntUni("normalMapGBuffer", 1);
  pbrShader.setIntUni("materialBuffer", 2);
  pbrShader.setIntUni("aoMap", 3);
  pbrShader.setIntUni("irradianceMap", 4);
  pbrShader.setIntUni("prefilterMap", 5);
  pbrShader.setIntUni("brdfLUT", 6);
  pbrShader.setIntUni("linearDepthMap", 7);
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
  backgroundShader.setIntUni("envMap", 0);
  return backgroundShader;
}

Shader loadGeometryShader() {
  //
  fs::path vpath = shaderDirPath / "screen" / "geobuffer.vert";
  fs::path fpath = shaderDirPath / "screen" / "geobuffer.frag";
  Shader gShader(vpath.c_str(), fpath.c_str());
  gShader.useProgram();
  gShader.setIntUni("normalMap", 0);
  gShader.setIntUni("roughnessMap", 1);
  gShader.setIntUni("metallicMap", 2);
  return gShader;
}

Shader loadRayConeShader() {
  fs::path vpath = shaderDirPath / "screen" / "conetrace.vert";
  fs::path fpath = shaderDirPath / "screen" / "conetrace.frag";
  Shader coneShader(vpath.c_str(), fpath.c_str());
  return coneShader;
}

void setGBufferTexture(GLuint &tex, GLenum informat, GLenum bufferType,
                       unsigned int attachmentNb) {
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, informat, WINWIDTH, WINHEIGHT, 0, GL_RGBA,
               bufferType, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNb,
                         GL_TEXTURE_2D, tex, 0);
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

void computePrefilterMap(Shader prefilterShader, Shader pbrShader,
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
  int maxMipLevels = 5;
  pbrShader.useProgram();
  pbrShader.setFloatUni("maxMipLevels", (float)maxMipLevels);
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    GLuint mipWidth = prefilterMapWidth * std::pow(0.5, mip);
    GLuint mipHeight = prefilterMapHeight * std::pow(0.5, mip);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth,
                          mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
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
  // rusted metal sphere on a mirror like platform
  GLuint metallicMap = 0;
  metallicMap = loadTexture2d("paintedmetal", "paintedmetal_metallic.jpg");
  gerr();

  GLuint baseColorMap = 0;
  baseColorMap = loadTexture2d("paintedmetal", "paintedmetal_basecolor.jpg");
  gerr();

  GLuint normalMap = 0;
  normalMap = loadTexture2d("paintedmetal", "paintedmetal_normal.jpg");
  gerr();

  GLuint roughnessMap = 0;
  roughnessMap = loadTexture2d("paintedmetal", "paintedmetal_roughness.jpg");
  gerr();

  GLuint aoMap = 0;
  aoMap = loadTexture2d("paintedmetal", "paintedmetal_ao.jpg");
  gerr();

  GLuint environmentHdrMap = 0;
  loadHdrTexture("newport", "Newport_Loft_Ref.hdr", environmentHdrMap);
  gerr();

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
  computePrefilterMap(prefilterShader, pbrShader, captureProjection,
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
  brdfShader.useProgram();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();
  gerr();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // -------------------------------------------------------------------------

  //--------------------------------- B. rendering ---------------------------
  // set up required buffers for rendering
  GLuint geometry_fbo, depthRbo; // required for g buffer pass
  glGenFramebuffers(1, &geometry_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);
  gerr();

  GLuint attachmentNb = 0;

  // stores normal of value of vertex in view space
  GLuint normalBuffer = 0;
  setGBufferTexture(normalBuffer, // buffer id
                    GL_RGBA16F,   // buffer stored value precision
                    GL_FLOAT,     // buffer stored value type
                    0             // attachment index
                    );
  gerr();

  // stores material parameters of the vertex
  // such as roughness, reflectance value at zero incidence, maybe pdf value
  // etc
  GLuint materialBuffer = 0;
  setGBufferTexture(materialBuffer, // buffer id
                    GL_RGBA16F,     // buffer stored value precision
                    GL_FLOAT,       // buffer stored value type
                    1               // attachment index
                    );
  gerr();

  // stores color values for the given vertex such as diffuse/albedo values
  // GLuint albedoBuffer = 0;
  // setGBufferTexture(albedoBuffer,     // buffer id
  //                  GL_RGBA,          // buffer stored value precision
  //                  GL_UNSIGNED_BYTE, // buffer stored value type
  //                  attachmentNb      // attachment index
  //                  );
  // attachmentNb += 1;

  // stores depth values for given vertex, ie the distance between the camera
  // and the vertex
  GLuint linearDepthBuffer = 0;
  setGBufferTexture(linearDepthBuffer, // buffer id
                    GL_RGBA16F,        // buffer stored value precision
                    GL_FLOAT,          // buffer stored value type
                    2                  // attachment index
                    );
  gerr();

  // a fall back texture with precomputed brdf and environment cubemap
  // the values inside this cubemap is captured using a captureFBO
  // then rendered with a shader before drawing starts
  // the values inside this texture is provided using the following pipeline

  // float lod             = getMipLevelFromRoughness(roughness);
  // vec3 prefilteredColor = textureCubeLod(PrefilteredEnvMap, refVec, lod);
  // vec2 envBRDF = texture2D(BRDFIntegrationMap, vec2(NdotV, roughness)).xy;
  // vec3 indirectSpecular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

  GLuint envCubemapFallbackTexture = envCubemap;
  GLuint irradianceCubemapFallbackTexture = irradianceCubemap;
  GLuint prefilteredCubemapFallbackTexture = prefilterMap;
  GLuint brdfLutFallbackTexture = brdfLutTexture;

  // setting color attachments
  GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                           GL_COLOR_ATTACHMENT2};

  glDrawBuffers(3, attachments);
  gerr();

  setGBufferDepthRbo(depthRbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Geometry Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ----------------- needed shaders are ------------------------------
  // geometry shader
  Shader geometryShader = loadGeometryShader();
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
  glm::mat4 projection =
      glm::perspective(glm::radians(camera.zoom),
                       (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
  pbrShader.useProgram();
  pbrShader.setMat4Uni("projection", projection);

  Shader lampShader = loadLampShader();
  gerr();
  lampShader.useProgram();
  lampShader.setMat4Uni("projection", projection);

  Shader backgroundShader = loadBackgroundShader();
  backgroundShader.useProgram();
  backgroundShader.setMat4Uni("projection", projection);

  geometryShader.useProgram();
  geometryShader.setMat4Uni("projection", projection);

  while (!glfwWindowShouldClose(window)) {

    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc2(window);

    //
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. geometry pass: render scene geometry color data into geometry buffer
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {

      glm::mat4 view = camera.getViewMatrix();
      glm::mat4 model = glm::mat4(1.0f);

      // set shader uniforms
      geometryShader.setMat4Uni("view", view);
      model = glm::mat4(1.0f);
      glm::vec3 objectPos = glm::vec3(3.0, -0.5, -3.0);
      model = glm::translate(model, objectPos);
      model = glm::scale(model, glm::vec3(0.5f));
      geometryShader.setMat4Uni("model", model);

      // activate and bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, normalMap);
      gerr();

      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, roughnessMap);
      gerr();

      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, metallicMap);
      renderCubeInTangentSpace();
      gerr();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2. lightening pass: render lightening to be refined later on
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      pbrShader.useProgram();
      glm::mat4 model = glm::mat4(1);
      glm::mat4 view = camera.getViewMatrix();
      pbrShader.setVec3Uni("camPos", camera.pos);

      // bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, baseColorMap);
      gerr();

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, normalBuffer);
      gerr();

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, materialBuffer);
      gerr();

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, aoMap);
      gerr();

      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemap);
      gerr();

      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
      gerr();

      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_CUBE_MAP, brdfLutTexture);
      gerr();

      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, linearDepthBuffer);

      // model = glm::mat4(1.0f);
      // model = glm::translate(model, glm::vec3(-5.0, 0.0, 2.0));
      // pbrShader.setMat4Uni("model", model);
      renderQuad();
      gerr();

      // render light
      // glBindTexture(GL_TEXTURE_2D, 0);
      pbrShader.useProgram();
      pbrShader.setVec3Uni("lightPosition", spotLight.position);
      pbrShader.setVec3Uni("lightColor", glm::vec3(300.0));
      lampShader.useProgram();
      lampShader.setVec3Uni("lightColor", glm::vec3(300.0f));

      model = glm::mat4(1.0f);
      model = glm::translate(model, spotLight.position);
      model = glm::scale(model, glm::vec3(0.2f));
      lampShader.setMat4Uni("model", model);
      lampShader.setMat4Uni("view", view);
      // renderSphere();
      gerr();
    }

    // 3. raytrace, cone trace scene
    rayConeShader.useProgram();

    // 4. background

    //  glm::mat4 view = camera.getViewMatrix();
    //  backgroundShader.useProgram();
    //  backgroundShader.setMat4Uni("view", view);
    //  glActiveTexture(GL_TEXTURE0);
    //  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    //  renderCubeD();
    //  gerr();
    // swap buffer vs
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
