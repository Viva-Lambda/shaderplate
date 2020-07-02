// license, see LICENSE
// implementing
/**
 "Real-Time Global Illumination using Precomputed Light Field Probes" by M.
 McGuire, M. Mara
 */
#include <custom/utils.hpp>

// ----------------------- utility code -----------------------
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
Model loadLightModel() {
  Model modelLight(modelPath / "sphere/scene.gltf", flags, true, false, false);
  return modelLight;
}
Model loadBackpackModel() {
  fs::path modPath = modelPath / "backpack" / "backpack.obj";
  // stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, true, false, false);
  return modelB;
}

// ----------------------- load shaders --------------------------------
Shader loadEnvMapToCubemapShader() {
  fs::path vpath = shaderDirPath / "lightprobs" / "env2cube.vert";
  fs::path fpath = shaderDirPath / "lightprobs" / "env2cube.frag";
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}
Shader loadLighteningShader() {
  //
  fs::path vpath = shaderDirPath / "lightprobs" / "lightening.vert";
  fs::path fpath = shaderDirPath / "lightprobs" / "lightening.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  return lightShader;
}
Shader loadBackgroundShader() {
  fs::path vpath = shaderDirPath / "lightprobs" / "background.vert";
  fs::path fpath = shaderDirPath / "lightprobs" / "background.frag";
  Shader backgroundShader(vpath.c_str(), fpath.c_str());
  backgroundShader.setIntUni("envMap", 0);
  return backgroundShader;
}

// ---------------------- load textures -------------------------------

/**
 */
void loadHdrTexture(const char *parent, const char *tpath, GLuint &hdrTexture) {
  // load hdr environment map
  stbi_set_flip_vertically_on_load(true);
  fs::path fpath = textureDirPath / parent / tpath;
  int width, height, nbComponents;
  float *data = stbi_loadf(fpath.c_str(), &width, &height, &nbComponents, 0);
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
                 GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    std::cout << "hdr image not loaded" << std::endl;
  }
  stbi_image_free(data);
}

/**
 */
GLuint loadEnvironmentCubemap(GLuint &envCMap, unsigned int captureWidth,
                              unsigned int captureHeight) {
  glGenTextures(1, &envCMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCMap);
  for (unsigned int i = 0; i < 6; i++) {
    //
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, captureWidth,
                 captureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return envCMap;
}

// ---------------------- frame buffers --------------------------------

/**
  Set up a framebuffer object to capture shading information
 */
void setLighteningFbo(GLuint &fbo, GLuint &rbo, unsigned int captureWidth,
                      unsigned int captureHeight) {
  glGenFramebuffers(1, &fbo);
  glGenRenderbuffers(1, &rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureWidth,
                        captureHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
}

/**
  Load g buffer
 */
int main() {
  //
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Lightprobs Shading Example", NULL, NULL);

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

  // opengl global state
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // load model
  // Model backpack = loadBackpackModel();

  // --------------------- load shaders -----------------------
  Shader lighteningShader = loadLighteningShader();
  Shader envMapToCubemapShader = loadEnvMapToCubemapShader();
  Shader backgroundShader = loadBackgroundShader();

  // ---------------- Per program logic -----------------------
  unsigned int captureWidth = 512;
  unsigned int captureHeight = 512;
  GLuint captureFbo, captureRbo;
  setLighteningFbo(captureFbo, captureRbo, captureWidth, captureHeight);

  GLuint hdrTexture;
  loadHdrTexture("newport", "Newport_Loft_Ref.hdr", hdrTexture);

  GLuint envCubemap;
  loadEnvironmentCubemap(envCubemap, captureWidth, captureHeight);

  glm::mat4 captureProjection =
      glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
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

  envMapToCubemapShader.useProgram();
  envMapToCubemapShader.setMat4Uni("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);
  glViewport(0, 0, captureWidth, captureHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFbo);
  for (unsigned int i = 0; i < 6; ++i) {
    envMapToCubemapShader.setMat4Uni("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCubeD();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glm::mat4 projection =
      glm::perspective(glm::radians(camera.zoom),
                       (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
  lighteningShader.useProgram();
  lighteningShader.setMat4Uni("projection", projection);
  backgroundShader.useProgram();
  backgroundShader.setMat4Uni("projection", projection);

  // then before rendering, configure the viewport to the original framebuffer's
  // screen dimensions
  int scrWidth, scrHeight;
  glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
  glViewport(0, 0, scrWidth, scrHeight);

  // generate irradiance maps

  // generate light field probs

  // load shaders
  // set up shaders

  // render loop
  // -----------
  float lastFrame = 0.0;
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput_proc2(window);

    // render
    // ------
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. generate and trace n primary rays from each m active probes in a
    // scene storing geometry for (up to) n x m surface hits in a g-buffer
    // like structure of surfels with explicit position and normals

    // 2. shade surfel buffer with direct and indirect illumination with the
    // same routine used to shade final image pixels, i.e those directly
    // visible from the camera.

    // 3. update the texels in the octahedral representations of the m active
    // probes by blending in the updated shading, distance, and square
    // distance results for each of the n intersected surfels

    // ----- visual
    lighteningShader.useProgram();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1);
    lighteningShader.setMat4Uni("view", view);
    lighteningShader.setMat4Uni("model", model);
    lighteningShader.setVec3Uni("lightPos", spotLight.position);
    lighteningShader.setVec3Uni("lightColor", glm::vec3(300.0f));
    renderSphere();

    backgroundShader.useProgram();
    backgroundShader.setMat4Uni("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    renderCubeD();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
