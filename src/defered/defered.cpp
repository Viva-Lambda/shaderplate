// defered shading
// license, see LICENSE

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
  stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, true, false, false);
  return modelB;
}

// ----------------------- load shaders --------------------------------
Shader loadGeometryShader() {
  fs::path vpath = shaderDirPath / "defered" / "geobuffer.vert";
  fs::path fpath = shaderDirPath / "defered" / "geobuffer.frag";
  Shader geoShader(vpath.c_str(), fpath.c_str());
  return geoShader;
}
Shader loadLighteningShader() {
  //
  fs::path vpath = shaderDirPath / "defered" / "lightening.vert";
  fs::path fpath = shaderDirPath / "defered" / "lightening.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  lightShader.useProgram();
  lightShader.setIntUni("positionTexture", 0);
  lightShader.setIntUni("normalTexture", 1);
  lightShader.setIntUni("specularTexture", 2);
  return lightShader;
}
Shader loadBasicLightShader() {
  fs::path vpath = shaderDirPath / "defered" / "basic_light.vert";
  fs::path fpath = shaderDirPath / "defered" / "basic_color_light.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  return lightShader;
}

// ---------------------- frame buffers --------------------------------

/**
  Load g buffer
 */
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

int main() {
  //
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(WINWIDTH, WINHEIGHT,
                                        "Defered Shading Example", NULL, NULL);

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

  // load model
  Model lamp = loadLightModel();
  Model backpack = loadBackpackModel();

  std::vector<glm::vec3> objectPos;
  objectPos.push_back(glm::vec3(-3.0, -0.5, -3.0));
  objectPos.push_back(glm::vec3(0.0, -0.5, -3.0));
  objectPos.push_back(glm::vec3(3.0, -0.5, -3.0));
  objectPos.push_back(glm::vec3(-3.0, -0.5, 0.0));
  objectPos.push_back(glm::vec3(0.0, -0.5, 0.0));
  objectPos.push_back(glm::vec3(3.0, -0.5, 0.0));
  objectPos.push_back(glm::vec3(-3.0, -0.5, 3.0));
  objectPos.push_back(glm::vec3(0.0, -0.5, 3.0));
  objectPos.push_back(glm::vec3(3.0, -0.5, 3.0));

  //
  GLuint positionTexture = 0, normalTexture = 0, specularTexture = 0;
  GLuint gbufferFbo, depthRbo;
  setGBuffer(gbufferFbo, positionTexture, normalTexture, specularTexture,
             depthRbo);

  // load shaders
  Shader geometryShader = loadGeometryShader();
  Shader lighteningShader = loadLighteningShader();
  Shader basicLightShader = loadBasicLightShader();

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

    // 1. Geometry pass: render scene geometry color data into gbuffer

    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.zoom),
                         (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    // set shader uniforms
    geometryShader.useProgram();
    geometryShader.setMat4Uni("projection", projection);
    geometryShader.setMat4Uni("view", view);
    for (unsigned int i = 0; i < objectPos.size(); i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, objectPos[i]);
      model = glm::scale(model, glm::vec3(0.5f));
      geometryShader.setMat4Uni("model", model);
      backpack.Draw(geometryShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. lightnening pass
    // clear up everything from before and attach relevant
    // textures
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lighteningShader.useProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, specularTexture);

    // set shader uniforms
    lighteningShader.setFloatUni("lightIntensity", lightIntensity);
    lighteningShader.setVec3Uni("lightPos", spotLight.position);
    lighteningShader.setVec3Uni("inLightDir", spotLight.front);
    lighteningShader.setVec3Uni("viewPos", camera.pos);
    lighteningShader.setVec3Uni("attC", glm::vec3(1, 0, 0));

    // render the texture to screen
    renderQuad();

    // copy the content of the frame buffer holding the
    // scene geometry to default buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gbufferFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // a possible strategy to incompatible internal format
    // of FBO's seems to add another shader stage
    glBlitFramebuffer(0, 0, WINWIDTH, WINHEIGHT, 0, 0, WINWIDTH, WINHEIGHT,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    /*
    glBlitFramebuffer, glBlitNamedFramebuffer — copy a block of pixels from one
    framebuffer object to another
    C Specification
    void glBlitFramebuffer(
    GLint srcX0,
            GLint srcY0,
            GLint srcX1,
            GLint srcY1,
            GLint dstX0,
            GLint dstY0,
            GLint dstX1,
            GLint dstY1,
            GLbitfield mask,
            GLenum filter);
     */

    // lightnening is done so let's draw the light source
    basicLightShader.useProgram();
    // set uniforms
    basicLightShader.setMat4Uni("projection", projection);
    basicLightShader.setMat4Uni("view", view);

    //
    model = glm::mat4(1);
    model = glm::translate(model, spotLight.position);
    model = glm::scale(model, glm::vec3(0.1));
    basicLightShader.setMat4Uni("model", model);
    basicLightShader.setVec3Uni("lightColor", glm::vec3(lightIntensity));

    lamp.Draw(basicLightShader);

    //
    // renderSphere();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
