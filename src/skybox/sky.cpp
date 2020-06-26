// license, see LICENSE
//
#include <custom/utils.hpp>

int main() {
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window =
      glfwCreateWindow(WINWIDTH, WINHEIGHT, "Skybox Example", NULL, NULL);

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

  //------------------- shader -------------------------
  fs::path cubeVertexPath = shaderDirPath / "cubeMap.vert";
  fs::path cubeFragPath = shaderDirPath / "cubeMap.frag";
  Shader cubeShader(cubeVertexPath.c_str(), cubeFragPath.c_str());

  fs::path skyboxVertexPath = shaderDirPath / "skybox.vert";
  fs::path skyboxFragPath = shaderDirPath / "skybox.frag";
  Shader skyboxShader(skyboxVertexPath.c_str(), skyboxFragPath.c_str());

  //------------------- end shader -------------------------

  // --------------------- vertex declaration --------------

  float cubeVertices[] = {
      // positions          // normals
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, //

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f, //

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f, //
      -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f, //
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f, //
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f, //
      -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f, //
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f, //

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, //
      0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f, //
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, //
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, //
      0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f, //
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, //

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f, //

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f  //
  };

  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f,  -1.0f, //
      -1.0f, -1.0f, -1.0f, //
      1.0f,  -1.0f, -1.0f, //
      1.0f,  -1.0f, -1.0f, //
      1.0f,  1.0f,  -1.0f, //
      -1.0f, 1.0f,  -1.0f, //

      -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
      -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
      1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
      -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
  // --------------------- end vertex  ---------------------

  // VAO and VBO
  GLuint cubeVao, cubeVbo;
  glGenVertexArrays(1, &cubeVao);
  glGenBuffers(1, &cubeVbo);
  glBindVertexArray(cubeVao);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0],
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, //
                        6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, //
                        6 * sizeof(float), (void *)(3 * sizeof(float)));

  // ----- skybox VAO, VBO
  GLuint skyboxVao, skyboxVbo;
  glGenVertexArrays(1, &skyboxVao);
  glGenBuffers(1, &skyboxVbo);
  glBindVertexArray(skyboxVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  //
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  // texture
  std::vector<fs::path> sides{textureDirPath / "skybox" / "right.jpg",
                              textureDirPath / "skybox" / "left.jpg",
                              textureDirPath / "skybox" / "top.jpg",
                              textureDirPath / "skybox" / "bottom.jpg",
                              textureDirPath / "skybox" / "front.jpg",
                              textureDirPath / "skybox" / "back.jpg"};
  GLuint skyboxTexture = loadCubeMap(sides);

  // set shader uniforms
  cubeShader.useProgram();
  cubeShader.setIntUni("skybox", 0);
  skyboxShader.useProgram();
  skyboxShader.setIntUni("skybox", 0);
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
    cubeShader.useProgram();

    glm::mat4 model(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom), float(WINWIDTH) / WINHEIGHT, 0.1f, 100.0f);
    cubeShader.setMat4Uni("model", model);
    cubeShader.setMat4Uni("view", view);
    cubeShader.setMat4Uni("projection", projection);
    cubeShader.setVec3Uni("cameraPos", camera.pos);
    //
    glBindVertexArray(cubeVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    //
    glDepthFunc(GL_LEQUAL);
    skyboxShader.useProgram();
    glm::mat4 view2 = glm::mat4(glm::mat3(camera.getViewMatrix()));
    skyboxShader.setMat4Uni("view", view2);
    skyboxShader.setMat4Uni("projection", projection);
    //
    glBindVertexArray(skyboxVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    //
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &cubeVao);
  glDeleteVertexArrays(1, &skyboxVao);
  glDeleteBuffers(1, &cubeVbo);
  glDeleteBuffers(1, &skyboxVbo);

  glfwTerminate();
  return 0;
}
