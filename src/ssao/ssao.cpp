// license, see LICENSE
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

Shader loadGeometryShader() {
  fs::path vpath = shaderDirPath / "ssao" / "geometry.vert";
  fs::path fpath = shaderDirPath / "ssao" / "geometry.frag";
  Shader geometryShader(vpath.c_str(), fpath.c_str());
  return geometryShader;
}
Shader loadLightShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "light.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  return lightShader;
}
Shader loadSsaoShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "ssao.frag";
  Shader ssaoShader(vpath.c_str(), fpath.c_str());
  return ssaoShader;
};
Shader loadSsaoBlurShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "ssaoblur.frag";
  Shader blurShader(vpath.c_str(), fpath.c_str());
  return blurShader;
}

void setGBufferTexture(GLuint &tex, GLenum informat, GLenum bufferType,
                       unsigned int attachmentNb) {
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, informat, WINWIDTH, WINHEIGHT, 0, GL_RGBA,
               bufferType, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNb,
                         GL_TEXTURE_2D, tex, 0);
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
void setGBuffer(GLuint &gbufferFbo, GLuint &posT, GLuint &normT, GLuint &specT,
                GLuint &depthRbo) {
  //
  glGenFramebuffers(1, &gbufferFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
  setGBufferTexture(posT, GL_RGBA16F, GL_FLOAT, 0);       // hdr
  setGBufferTexture(normT, GL_RGBA16F, GL_FLOAT, 1);      // hdr
  setGBufferTexture(specT, GL_RGBA, GL_UNSIGNED_BYTE, 2); // ldf
  GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                           GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);
  setGBufferDepthRbo(depthRbo);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float lerp(float n, float n2, float f) { return n + f * (n2 - n); }

int main() {
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

  // load shaders
  Shader geometryShader = loadGeometryShader();
  Shader lightShader = loadLightShader();
  Shader ssaoShader = loadSsaoShader();
  Shader ssaoBlurShader = loadSsaoBlurShader();

  Model backpack = loadBackpackModel();

  //
  GLuint geometry_fbo, depthRbo;
  GLuint positionTexture = 0, normalTexture = 0, albedoTexture = 0;
  setGBuffer(geometry_fbo, positionTexture, normalTexture, albedoTexture,
             depthRbo);

  // create ssao fbo
  GLuint ssaoFBO, ssaoBlurFBO;
  glGenFramebuffers(1, &ssaoFBO);
  glGenFramebuffers(1, &ssaoBlurFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
  // create ssaoFbo textures
  GLuint ssaoColorTexture = 0, ssaoBlurColorTexture = 0;
  glGenTextures(1, &ssaoColorTexture);
  glBindTexture(GL_TEXTURE_2D, ssaoColorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WINWIDTH, WINHEIGHT, 0, GL_RED,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssaoColorTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "SSAO Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
  glGenTextures(1, &ssaoBlurColorTexture);
  glBindTexture(GL_TEXTURE_2D, ssaoBlurColorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WINWIDTH, WINHEIGHT, 0, GL_RED,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssaoBlurColorTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "SSAO Blur Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  std::uniform_real_distribution<GLfloat> randFloats(0.0, 1.0);
  std::default_random_engine gen;
  std::vector<glm::vec3> ssaoKernel;
  for (unsigned int i = 0; i < 64; i++) {
    glm::vec3 sample(randFloats(gen) * 2.0 - 1.0, randFloats(gen) * 2.0 - 1.0,
                     randFloats(gen));
    sample = glm::normalize(sample);
    float scale = float(i) / 64.0;
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }

  // generate noise
  std::vector<glm::vec3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++) {
    //
    glm::vec3 noise(randFloats(gen) * 2.0 - 1.0, randFloats(gen) * 2.0 - 1.0,
                    randFloats(gen));
    ssaoNoise.push_back(noise);
  }
  GLuint noiseTexture = 0;
  glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT,
               &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // light etc
  glm::vec3 lightPos = glm::vec3(2, 4, -2);
  glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

  // shader confs
  lightShader.useProgram();
  lightShader.setIntUni("gPosition", 0);
  lightShader.setIntUni("gNormal", 1);
  lightShader.setIntUni("gAlbedo", 2);
  lightShader.setIntUni("ssao", 3);

  //
  ssaoShader.useProgram();
  ssaoShader.setIntUni("gPosition", 0);
  ssaoShader.setIntUni("gNormal", 1);
  ssaoShader.setIntUni("noiseTexture", 2);

  ssaoBlurShader.useProgram();
  ssaoBlurShader.setIntUni("ssaoInput", 0);

  // rendering pass
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

    // 1. geometry pass
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 proj =
        glm::perspective(glm::radians(camera.zoom),
                         (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 50.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0);

    geometryShader.useProgram();
    geometryShader.setMat4Uni("projection", proj);
    geometryShader.setMat4Uni("view", view);

    model = glm::translate(model, glm::vec3(0, 7, 0));
    model = glm::scale(model, glm::vec3(7.5, 7.5, 7.5));
    geometryShader.setMat4Uni("model", model);
    geometryShader.setBoolUni("invertedNormals", true);
    renderCubeD();
    geometryShader.setBoolUni("invertedNormals", false);
    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(1));
    backpack.Draw(geometryShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. ssao texture generation
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoShader.useProgram();
    for (unsigned int i = 0; i < 64; i++) {
      ssaoShader.setVec3Uni("samples[" + std::to_string(i) + "]",
                            ssaoKernel[i]);
    }

    ssaoShader.setMat4Uni("projection", proj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. blur ssao texture to remove noise

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurShader.useProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorTexture);
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. light pass with ao computed
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightShader.useProgram();
    glm::vec3 lposView =
        glm::vec3(camera.getViewMatrix() * glm::vec4(spotLight.position, 1));
    lightShader.setVec3Uni("lightPos", lposView);
    lightShader.setVec3Uni("lightColor", glm::vec3(1));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssaoColorTexture);
    renderQuad();

    // terminate

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
