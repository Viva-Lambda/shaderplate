// license, see LICENSE
//
#include <custom/utils.hpp>

Shader loadPbrShader() {
  // load pbr shader
  fs::path pbrVertPath = shaderDirPath / "ibl" / "iblPbr.vert";
  fs::path pbrFragPath = shaderDirPath / "ibl" / "iblPbr.frag";
  Shader pbrShader(pbrVertPath.c_str(), pbrFragPath.c_str());

  pbrShader.useProgram();
  pbrShader.setIntUni("irradianceMap", 0);
  pbrShader.setIntUni("prefilterMap", 1);
  pbrShader.setIntUni("brdfLUT", 2);
  pbrShader.setIntUni("albedoMap", 3);
  pbrShader.setIntUni("normalMap", 4);
  pbrShader.setIntUni("metallicMap", 5);
  pbrShader.setIntUni("roughnessMap", 6);
  pbrShader.setIntUni("aoMap", 7);
  return pbrShader;
}
Shader loadCubemapShader() {
  fs::path cubemapVertPath = shaderDirPath / "ibl" / "iblCubemap.vert";
  fs::path cubemapEnvFragPath = shaderDirPath / "ibl" / "iblCubemapEnv.frag";
  Shader envToCubeMapShader(cubemapVertPath.c_str(),
                            cubemapEnvFragPath.c_str());

  return envToCubeMapShader;
}
Shader loadIrradianceShader() {
  // irradiance shader
  fs::path cubemapVertPath = shaderDirPath / "ibl" / "iblCubemap.vert";
  fs::path convIrFragPath = shaderDirPath / "ibl" / "iblConvolution.frag";
  Shader irradianceShader(cubemapVertPath.c_str(), convIrFragPath.c_str());
  return irradianceShader;
}

Shader loadPrefilterShader() {
  fs::path cubemapVertPath = shaderDirPath / "ibl" / "iblCubemap.vert";
  fs::path prefilterFragPath = shaderDirPath / "ibl" / "iblPrefilter.frag";
  Shader prefilterShader(cubemapVertPath.c_str(), prefilterFragPath.c_str());
  return prefilterShader;
}
Shader loadBrdfShader() {
  fs::path brdfVertPath = shaderDirPath / "ibl" / "iblBrdf.vert";
  fs::path brdfFragPath = shaderDirPath / "ibl" / "iblBrdf.frag";
  Shader brdfShader(brdfVertPath.c_str(), brdfFragPath.c_str());
  return brdfShader;
}
Shader loadBackgroundShader() {
  fs::path bgVertPath = shaderDirPath / "ibl" / "iblBackground.vert";
  fs::path bgFragPath = shaderDirPath / "ibl" / "iblBackground.frag";
  Shader backgroundShader(bgVertPath.c_str(), bgFragPath.c_str());
  backgroundShader.useProgram();
  backgroundShader.setIntUni("environmentMap", 0);
  return backgroundShader;
}

GLuint loadEnvironmentMap() {
  GLuint envCMap;
  glGenTextures(1, &envCMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);
  for (unsigned int i = 0; i < 6; i++) {
    //
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0,
                 GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return envCMap;
}
GLuint loadCubemapTexture(int w, int h) {
  //
  GLuint cube;
  glGenTextures(1, &cube);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube);
  for (unsigned int i = 0; i < 6; i++) {
    //
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, w, h, 0,
                 GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return cube;
}

void bindFboRbo(GLuint fbo, GLuint rbo, int w, int h) {
  //
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
}

void renderCubeToFbo(Shader shdr, GLuint texture,
                     std::vector<glm::mat4> views) {
  //
  for (unsigned int i = 0; i < 6; i++) {
    //
    shdr.setMat4Uni("view", views[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderCube();
  }
}

int main() {
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(WINWIDTH, WINHEIGHT,
                                        "IBL With Specular Map", NULL, NULL);

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

  glEnable(GL_DEPTH_TEST);
  //
  glDepthFunc(GL_LEQUAL);
  //
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // -------------------------- shaders ------------------------

  // pbr shader

  Shader pbrShader = loadPbrShader();
  // cubemap shader
  Shader envToCubeMapShader = loadCubemapShader();
  // irradiance shader
  Shader irradianceShader = loadIrradianceShader();
  // prefilter Shader
  Shader prefilterShader = loadPbrShader();
  // brdf/bsdf shader
  Shader brdfShader = loadBrdfShader();
  // background
  Shader backgroundShader = loadBackgroundShader();

  // -------------------- set shaders ----------------------------

  // ----------------------- textures --------------------------
  // iron textures
  GLuint ironAlbedo = loadTexture2d("rusted", "rustediron2_basecolor.png");

  GLuint ironNormal = loadTexture2d("rusted", "rustediron2_normal.png");

  GLuint ironMetallic = loadTexture2d("rusted", "rustediron2_metallic.png");
  GLuint ironRoughness = loadTexture2d("rusted", "rustediron2_roughness.png");

  // gold textures
  GLuint goldAlbedo = loadTexture2d("gold", "lightgold_albedo.png");
  GLuint goldNormal = loadTexture2d("gold", "lightgold_normal-ogl.png");
  GLuint goldMetallic = loadTexture2d("gold", "lightgold_metallic.png");
  GLuint goldRoughness = loadTexture2d("gold", "lightgold_roughness.png");

  // ----------------------- end textures ----------------------------

  //
  std::vector<glm::vec3> lightPoss = {
      glm::vec3(-10.0f, 10.0f, 10.0f),  //
      glm::vec3(10.0f, 10.0f, 10.0f),   //
      glm::vec3(-10.0f, -10.0f, 10.0f), //
      glm::vec3(10.0f, -10.0f, 10.0f),
  };
  std::vector<glm::vec3> lightColors = {
      glm::vec3(300.0f), //
      glm::vec3(300.0f), //
      glm::vec3(300.0f), //
      glm::vec3(300.0f),
  };

  // capture fbo
  GLuint captureFBO;
  GLuint captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, captureRBO);

  // ---------------------- load pbr env map ------------------------------
  GLuint hdrTexture = loadHdrTexture("newport", "Newport_Loft_Env.hdr");

  // ---------- pbr: cubemap to be rendered and attached to framebuffer ------
  GLuint envCMap = loadEnvironmentMap();

  //
  glm::mat4 captureProjection =
      glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  std::vector<glm::mat4> captureView = {
      //
      glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)),
  };

  // pbr: convert hdr equirectangular environment map
  envToCubeMapShader.useProgram();
  envToCubeMapShader.setIntUni("equirectangularMap", 0);
  envToCubeMapShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  glViewport(0, 0, 512, 512); // WINWIDTH, WINHEIGHT
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  renderCubeToFbo(envToCubeMapShader, envCMap, captureView);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // ---------------- irradiance map ----------------

  GLuint irradianceMap = loadCubemapTexture(32, 32);
  bindFboRbo(captureFBO, captureRBO, 32, 32);

  // cook-torrance integral solution
  // ----------------------------------
  irradianceShader.useProgram();
  irradianceShader.setIntUni("environmentMap", 0);
  irradianceShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);

  // arrange viewport
  glViewport(0, 0, 32, 32);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  renderCubeToFbo(irradianceShader, irradianceMap, captureView);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ---------------- prefilter map ----------------

  GLuint prefilterMap = loadCubemapTexture(128, 128);

  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // cook-torrance integral solution
  // ----------------------------------
  prefilterShader.useProgram();
  prefilterShader.setIntUni("environmentMap", 0);
  prefilterShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);

  // arrange viewport
  // glViewport(0, 0, 32, 32);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    unsigned int mipWidth = 128 * std::pow(0.5, mip);
    unsigned int mipHeight = 128 * std::pow(0.5, mip);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth,
                          mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader.setFloatUni("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4Uni("view", captureView[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderCube();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ------------------- pbr: brdf lut 2d -----------------------
  GLuint brdfLutTexture = loadHdrTexture(512, 512);
  // bind the texture
  bindFboRbo(captureFBO, captureRBO, 512, 512);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         brdfLutTexture, 0);
  glViewport(0, 0, 512, 512);
  brdfShader.useProgram();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //
  glm::mat4 projection = glm::perspective(
      glm::radians(camera.zoom), float(WINWIDTH) / WINHEIGHT, 0.1f, 100.0f);

  // pbr shader
  pbrShader.useProgram();
  pbrShader.setMat4Uni("projection", projection);
  backgroundShader.useProgram();
  backgroundShader.setMat4Uni("projection", projection);

  int srcw, srch;
  glfwGetFramebufferSize(window, &srcw, &srch);
  glViewport(0, 0, srcw, srch);
  while (!glfwWindowShouldClose(window)) {

    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc(window);

    //
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pbrShader.useProgram();
    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = camera.getViewMatrix();
    pbrShader.setMat4Uni("model", model);
    pbrShader.setMat4Uni("view", view);

    // set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLutTexture);

    //
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ironAlbedo);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ironNormal);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ironMetallic);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ironRoughness);
    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(-5.0, 0, 2.0));
    pbrShader.setMat4Uni("model", model);
    renderSphere();

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, goldAlbedo);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, goldNormal);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, goldMetallic);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, goldRoughness);
    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(-2.0, 0, 2.0));
    pbrShader.setMat4Uni("model", model);
    renderSphere();

    // ----------------- render light sources ------------------------

    for (unsigned int i = 0; i < lightPoss.size(); i++) {
      glm::vec3 newPos =
          lightPoss[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0, 0);
      pbrShader.setVec3Uni("lightPositions[" + std::to_string(i) + "]", newPos);

      pbrShader.setVec3Uni("lightColors[" + std::to_string(i) + "]",
                           lightColors[i]);
      model = glm::mat4(1);
      model = glm::translate(model, newPos);
      model = glm::scale(model, glm::vec3(0.5f));
      pbrShader.setMat4Uni("model", model);
      renderSphere();
    }
    // lightPos
    // dynamic light
    pbrShader.setVec3Uni("lightPositions[" + std::to_string(4) + "]", lightPos);

    pbrShader.setVec3Uni("lightColors[" + std::to_string(4) + "]",
                         glm::vec3(1.0));
    model = glm::mat4(1);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.5f));
    pbrShader.setMat4Uni("model", model);
    renderSphere();

    // ----------- background -----------------
    backgroundShader.useProgram();
    backgroundShader.setMat4Uni("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);
    renderCube();

    // swap buffer vs
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
