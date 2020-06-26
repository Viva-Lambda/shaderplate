// license, see LICENSE
#include <custom/utils.hpp>

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
  fs::path cubeVertPath = shaderDirPath / "fbo" / "fboCube.vert";
  fs::path cubeFragPath = shaderDirPath / "fbo" / "fboCube.frag";
  Shader cubeShader(cubeVertPath.c_str(), cubeFragPath.c_str());

  // quad shader
  fs::path quadVertPath = shaderDirPath / "fbo" / "fboQuad.vert";
  // fs::path quadFragPath = shaderDirPath/ "fbo" / "fboQuad.frag";
  // fs::path quadFragPath = shaderDirPath/ "fbo" / "fboQuadInv.frag";
  // fs::path quadFragPath = shaderDirPath/ "fbo" / "fboQuadGrey.frag";
  fs::path quadFragPath = shaderDirPath / "fbo" / "fboQuadKernel.frag";
  Shader quadShader(quadVertPath.c_str(), quadFragPath.c_str());

  // -------------- vertex ------------------

  float cubeVerts[] = {// position        - texture //
                       -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //
                       0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, //
                       0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, //
                       0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, //
                       -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, //
                       -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //

                       -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, //
                       0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, //
                       0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //
                       0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //
                       -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, //
                       -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, //

                       -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, //
                       -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, //
                       -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
                       -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
                       -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, //
                       -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, //

                       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, //
                       0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, //
                       0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, //
                       0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, //
                       0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, //
                       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, //

                       -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
                       0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, //
                       0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, //
                       0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, //
                       -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, //
                       -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //

                       -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, //
                       0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, //
                       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, //
                       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, //
                       -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, //
                       -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

  float plane[] = {// positions        texture Coords
                   5.0f,  -0.5f, 5.0f,  2.0f, 0.0f, //
                   -5.0f, -0.5f, 5.0f,  0.0f, 0.0f, //
                   -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, //

                   5.0f,  -0.5f, 5.0f,  2.0f, 0.0f, //
                   -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, //
                   5.0f,  -0.5f, -5.0f, 2.0f, 2.0f};

  float screen[] = {//
                    // positions    texCoords //
                    -1.0f, 1.0f,  0.0f, 1.0f, //
                    -1.0f, -1.0f, 0.0f, 0.0f, //
                    1.0f,  -1.0f, 1.0f, 0.0f, //

                    -1.0f, 1.0f,  0.0f, 1.0f, //
                    1.0f,  -1.0f, 1.0f, 0.0f, //
                    1.0f,  1.0f,  1.0f, 1.0f};

  // ------------- vao/vbo ------------------

  // cube vao/vbo
  GLuint cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), &cubeVerts[0],
               GL_STATIC_DRAW);
  // ---
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  // plane vao/vbo
  GLuint planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plane), &plane[0], GL_STATIC_DRAW);
  // ---
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  // quad vao/vbo
  GLuint screenVAO, screenVBO;
  glGenVertexArrays(1, &screenVAO);
  glGenBuffers(1, &screenVBO);
  glBindVertexArray(screenVAO);
  glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(screen), &screen[0], GL_STATIC_DRAW);
  // ---
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  // ------------------------------------------------

  // ------------- textures -----------------
  fs::path stoneTexturePath = textureDirPath / "Stone_001_Diffuse.png";
  fs::path rustedPath = textureDirPath / "rustediron2_basecolor.png";
  GLuint stoneTexture, rustedTexture;
  stoneTexture = loadTexture2d(stoneTexturePath.c_str(), stoneTexture);
  rustedTexture = loadTexture2d(rustedPath.c_str(), rustedTexture);

  //
  cubeShader.useProgram();
  cubeShader.setIntUni("tex", 0);

  quadShader.useProgram();
  quadShader.setIntUni("quadTex", 0);

  // ------------- fbo config ---------------
  GLuint fbo; // frame buffer object
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  //
  GLuint tcbo; // texture color buffer object
  glGenTextures(1, &tcbo);
  glBindTexture(GL_TEXTURE_2D, tcbo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINWIDTH, WINHEIGHT, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tcbo, 0);
  // render buffer conf
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINWIDTH,
                        WINHEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Fbo not complete";
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ------------ rendering -----------------
  while (!glfwWindowShouldClose(window)) {
    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    //
    processInput_proc(window);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);

    //
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rendering
    cubeShader.useProgram();

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom), float(WINWIDTH) / WINHEIGHT, 0.1f, 100.0f);

    glBindVertexArray(cubeVAO);
    cubeShader.setMat4Uni("view", view);
    cubeShader.setMat4Uni("projection", projection);
    //
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    cubeShader.setMat4Uni("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stoneTexture);

    //
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    cubeShader.setMat4Uni("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_2D, 0);

    // draw plane
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rustedTexture);
    model = glm::mat4(1.0f);
    cubeShader.setMat4Uni("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    quadShader.useProgram();
    glBindVertexArray(screenVAO);
    glBindTexture(GL_TEXTURE_2D, tcbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteVertexArrays(1, &screenVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteBuffers(1, &planeVBO);
  glDeleteBuffers(1, &screenVBO);
  glfwTerminate();
  return 0;
}
