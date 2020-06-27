// license, see LICENSE
//
#include <custom/utils.hpp>
// bloom related
bool isBloom = true;
bool bkeyPressed = true;
float exposure = 1.0f;

void configureFbo(GLuint fbo, GLuint rbo, GLuint colorbs[2],
                  unsigned int buffer_nb = 2) {
  // hdr config for fbo
}

void configureBlurFbo(GLuint fbos[2], GLuint colorbs[2],
                      unsigned int buffer_nb = 2) {
  //
}

int main() {
  //
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window =
      glfwCreateWindow(WINWIDTH, WINHEIGHT, "FBO Example", NULL, NULL);

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

  //--------------- shaders -----------------
  // cube & plane shader
  fs::path cubeVertPath = shaderDirPath / "bloom" / "bloomCube.vert";
  fs::path cubeFragPath = shaderDirPath / "bloom" / "bloomCube.frag";
  Shader cubeShader(cubeVertPath.c_str(), cubeFragPath.c_str());

  // light shader
  fs::path lightFragPath = shaderDirPath / "bloom" / "bloomLight.frag";
  Shader lightShader(cubeVertPath.c_str(), lightFragPath.c_str());

  // blur shader
  fs::path blurVertPath = shaderDirPath / "bloom" / "bloomBlur.vert";
  fs::path blurFragPath = shaderDirPath / "bloom" / "bloomBlur.frag";
  Shader blurShader(blurVertPath.c_str(), blurFragPath.c_str());

  // quad shader
  fs::path quadVertPath = shaderDirPath / "bloom" / "bloomQuad.vert";
  fs::path quadFragPath = shaderDirPath / "bloom" / "bloomQuad.frag";
  Shader quadShader(quadVertPath.c_str(), quadFragPath.c_str());

  // ------------- textures -----------------
  fs::path stoneTexturePath =
      textureDirPath / "layered" / "layered-cliff-albedo.png";
  fs::path rustedPath = textureDirPath / "rusted" / "rustediron2_basecolor.png";
  GLuint stoneTexture, rustedTexture;
  glGenTextures(1, &stoneTexture);
  glGenTextures(1, &rustedTexture);
  stoneTexture = loadTexture2d(stoneTexturePath.c_str(), stoneTexture, false);
  rustedTexture = loadTexture2d(rustedPath.c_str(), rustedTexture, false);

  // ------------- fbo config ---------------
  GLuint hdrFbo; // frame buffer object
  unsigned int buffer_nb = 2;
  GLuint hdrColorbs[buffer_nb];
  // ------------- fbo config ---------------
  glGenFramebuffers(1, &hdrFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
  //
  glGenTextures(buffer_nb, hdrColorbs);
  for (unsigned int i = 0; i < buffer_nb; i++) {
    //
    glBindTexture(GL_TEXTURE_2D, hdrColorbs[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINWIDTH, WINHEIGHT, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, hdrColorbs[i], 0);
  }
  // rbo declaration
  GLuint rbo;

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINWIDTH,
                        WINHEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
  GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Fbo not complete";
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // render buffer conf
  unsigned int blur_buffer_nb = 2;
  GLuint blurFbo[blur_buffer_nb];
  GLuint blurColorbs[blur_buffer_nb];
  glGenFramebuffers(blur_buffer_nb, blurFbo);
  glGenTextures(blur_buffer_nb, blurColorbs);
  for (unsigned int i = 0; i < buffer_nb; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);
    glBindTexture(GL_TEXTURE_2D, blurColorbs[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINWIDTH, WINHEIGHT, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, //+ i,
                           GL_TEXTURE_2D, blurColorbs[i], 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "ERROR::FRAMEBUFFER:: Blur FBO not complete";
    }
  }

  // lights
  std::vector<glm::vec3> lightPositions;
  lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
  lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
  lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
  lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
  // colors
  std::vector<glm::vec3> lightColors;
  lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
  lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
  lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
  lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
  //
  // cube, light, blur, quad
  cubeShader.useProgram();
  cubeShader.setIntUni("diffuseTexture", 0);

  blurShader.useProgram();
  blurShader.setIntUni("img", 0);

  quadShader.useProgram();
  quadShader.setIntUni("scene", 0);
  quadShader.setIntUni("bloomBlur", 1);

  // ------------ rendering -----------------
  while (!glfwWindowShouldClose(window)) {
    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    //
    processInput_proc(window);

    //
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom), float(WINWIDTH) / WINHEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1);

    cubeShader.useProgram();
    cubeShader.setMat4Uni("view", view);
    cubeShader.setMat4Uni("projection", projection);
    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stoneTexture);
    for (unsigned int i = 0; i < lightPositions.size(); i++) {
      cubeShader.setVec3Uni("lights[" + std::to_string(i) + "].pos",
                            lightPositions[i]);
      cubeShader.setVec3Uni("lights[" + std::to_string(i) + "].color",
                            lightColors[i]);
    }
    cubeShader.setVec3Uni("viewPos", camera.pos);

    // platform
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(12.5f, 0.5f, 0.0f));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    //
    glBindTexture(GL_TEXTURE_2D, rustedTexture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f),
                        glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
    model = glm::rotate(model, glm::radians(23.0f),
                        glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(1.25));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
    model = glm::rotate(model, glm::radians(124.0f),
                        glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    cubeShader.setMat4Uni("model", model);
    renderCubeD();

    // light shader
    lightShader.useProgram();
    lightShader.setMat4Uni("projection", projection);
    lightShader.setMat4Uni("view", view);
    //
    for (unsigned int i = 0; i < lightPositions.size(); i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(lightPositions[i]));
      model = glm::scale(model, glm::vec3(0.25f));
      lightShader.setMat4Uni("model", model);
      lightShader.setVec3Uni("lightColor", lightColors[i]);
      renderCubeD();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // blur pass
    bool hor = true;
    bool first_pass = true;
    unsigned int pass_nb = 10;
    blurShader.useProgram();
    for (unsigned int i = 0; i < pass_nb; i++) {
      //
      glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[hor]);
      blurShader.setBoolUni("horizontal", hor);
      glBindTexture(GL_TEXTURE_2D,
                    first_pass ? hdrColorbs[1] : blurColorbs[!hor]);
      renderQuad();
      hor = not hor;
      if (first_pass) {
        first_pass = false;
      }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quadShader.useProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorbs[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurColorbs[!hor]);

    quadShader.setBoolUni("bloomCheck", isBloom);
    quadShader.setFloatUni("exposure", exposure);
    renderQuad();
    std::cout << "bloom: " << (isBloom ? "on" : "off") << " "
              << "exposure: " << exposure << std::endl;

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
