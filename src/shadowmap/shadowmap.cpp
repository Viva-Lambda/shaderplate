// shadow mapping from learn opengl
// license, see LICENSE
#include <custom/utils.hpp>

void renderScene(const Shader &shader, const GLuint &planeVAO);
void renderCube2();

Shader loadShadowShader() {
  fs::path vertex_path = shaderDirPath / "shadowmap" / "shadow.vert";
  fs::path fragment_path = shaderDirPath / "shadowmap" / "shadow.frag";
  Shader shadowShader(vertex_path.c_str(), fragment_path.c_str());
  return shadowShader;
}

Shader loadQuadShader() {
  //
  fs::path vertex_path = shaderDirPath / "shadowmap" / "quad.vert";
  fs::path fragment_path = shaderDirPath / "shadowmap" / "quad.frag";
  Shader quadShader(vertex_path.c_str(), fragment_path.c_str());
  return quadShader;
}

Shader loadDepthShader() {
  fs::path vertex_path = shaderDirPath / "shadowmap" / "depth.vert";
  fs::path fragment_path = shaderDirPath / "shadowmap" / "depth.frag";
  Shader depthShader(vertex_path.c_str(), fragment_path.c_str());
  return depthShader;
}

Shader loadLampShader() {
  fs::path vertex_path = shaderDirPath / "shadowmap" / "lamp.vert";
  fs::path fragment_path =
      shaderDirPath / "shadowmap" / "basic_color_light.frag";
  Shader lampShader(vertex_path.c_str(), fragment_path.c_str());
  return lampShader;
}

GLuint loadStoneTexture() {
  //
  fs::path stonePath = textureDirPath / "Stone_001_Diffuse.png";
  GLuint stone_texture = loadTexture2d(stonePath.c_str());
  return stone_texture;
}

bool isPerspective = false;

int main() {
  //
  //
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window =
      glfwCreateWindow(WINWIDTH, WINHEIGHT, "Shadow Mapping", NULL, NULL);

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

  // --------------- setup Shaders ------------------
  Shader shadowShader = loadShadowShader();
  Shader quadShader = loadQuadShader();
  Shader depthShader = loadDepthShader();
  Shader lampShader = loadLampShader();

  // --------------- setup Plane -------------------
  float planeVertices[] = {
      // positions            // normals         // texcoords
      25.0f,  -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 25.0f, 0.0f,  //
      -25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f,  //
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f,  25.0f, //
      25.0f,  -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 25.0f, 0.0f,  //
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f,  25.0f, //
      25.0f,  -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f  //
  };
  GLuint planeVAO;
  GLuint planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0],
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glBindVertexArray(0);

  // --------------- load textures ------------------
  GLuint stone_texture = loadA2DTexture();

  // --------------- setup FBO ----------------------
  const unsigned int SHADOW_MAP_WIDTH = 1024;
  const unsigned int SHADOW_MAP_HEIGHT = 1024;
  GLuint depthFBO;
  glGenFramebuffers(1, &depthFBO);

  // generate texture
  GLuint depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH,
               SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  // use nearest neighbor interpolation when zooming out
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // use nearest neighbor interpolation when zooming out
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  //
  glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ---------------- configure shaders --------------
  shadowShader.useProgram();
  shadowShader.setIntUni("diffuseMap1", 0);
  shadowShader.setIntUni("shadowMap", 1);

  quadShader.useProgram();
  quadShader.setIntUni("depthMap", 0);

  // --------------- rendering pass -----------------
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

    // render scene from light perspective
    glm::mat4 lightP, lightV;
    glm::mat4 lightSpaceMat;
    float near_plane = 1.0f;
    float far_plane = 7.5f;
    if (isPerspective) {
      lightP =
          glm::perspective(glm::radians(45.0f),
                           (float)SHADOW_MAP_WIDTH / (float)SHADOW_MAP_HEIGHT,
                           near_plane, far_plane);
    } else {
      lightP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    }
    lightV = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
    lightSpaceMat = lightP * lightV;

    depthShader.useProgram();
    depthShader.setMat4Uni("lightSpaceMat", lightSpaceMat);

    glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stone_texture);
    renderScene(depthShader, planeVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport
    glViewport(0, 0, WINWIDTH, WINHEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the scene normally with the generated shadowmap
    glViewport(0, 0, WINWIDTH, WINHEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shadowShader.useProgram();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom), float(WINWIDTH) / WINHEIGHT, 0.1f, 100.0f);
    shadowShader.setMat4Uni("projection", projection);
    shadowShader.setMat4Uni("view", view);
    shadowShader.setMat4Uni("lightSpaceMat", lightSpaceMat);
    shadowShader.setVec3Uni("viewPos", camera.pos);
    shadowShader.setVec3Uni("lightPos", lightPos);
    shadowShader.setIntUni("lightIntensity", lightIntensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stone_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderScene(shadowShader, planeVAO);
    glBindTexture(GL_TEXTURE_2D, 0);

    // render the lamp
    lampShader.useProgram();
    glm::mat4 model = glm::mat4(1);
    model = glm::scale(model, glm::vec3(0.2));
    lampShader.setMat4Uni("model", model);
    lampShader.setMat4Uni("view", view);
    lampShader.setMat4Uni("projection", view);
    lampShader.setVec3Uni("lightColor", glm::vec3(lightIntensity));
    renderCube2();

    // render the depthmap to to quad as a debugging aid

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteBuffers(1, &planeVBO);

  glfwTerminate();
  return 0;
}

void renderScene(const Shader &shader, const GLuint &planeVAO) {
  //
  glm::mat4 model = glm::mat4(1);
  shader.setMat4Uni("model", model);
  glBindVertexArray(planeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  model = glm::mat4(1);
  model = glm::translate(model, glm::vec3(0, 1.5, 0.0));
  model = glm::scale(model, glm::vec3(0.5));
  shader.setMat4Uni("model", model);
  renderCube2();

  model = glm::mat4(1);
  model = glm::translate(model, glm::vec3(2, 0.1, 1.0));
  model = glm::scale(model, glm::vec3(0.5));
  shader.setMat4Uni("model", model);
  renderCube2();

  model = glm::mat4(1);
  model = glm::translate(model, glm::vec3(-1, 0.0, 2.0));
  model = glm::rotate(model, glm::radians(60.0f),
                      glm::normalize(glm::vec3(1, 0, 1)));
  model = glm::scale(model, glm::vec3(0.25));
  shader.setMat4Uni("model", model);
  renderCube2();
}
unsigned int cubeVAONI = 0;
unsigned int cubeVBONI = 0;
void renderCube2() {
  // initialize (if necessary)
  if (cubeVAONI == 0) {
    float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
        // front face
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                            // right face
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
    };
    glGenVertexArrays(1, &cubeVAONI);
    glGenBuffers(1, &cubeVBONI);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBONI);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(cubeVAONI);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  // render Cube
  glBindVertexArray(cubeVAONI);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}
