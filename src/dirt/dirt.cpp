// implementing dirt from
// Kostas Vardis Phd with help from
// https://github.com/kvarcg/publications/tree/master/DIRT%20Deferred%20Image-based%20Tracing%20-%20HPG%202016/Shaders%20Only
#include <custom/utils.hpp>

// --------------------------- processing input etc -------------------------
// pitch : up/down
// yaw : left/right
// front: target
// back: -target.z

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

// --------------------------- texture related --------------------------
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

// ---------------------- Shaders -------------------------------------
Shader loadEquirectangulareToCubemapShader() {
  fs::path vpath = shaderDirPath / "dirt" / "env2cube.vert"; // DONE
  fs::path fpath = shaderDirPath / "dirt" / "env2cube.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.shaderName = "envShader";
  envShader.useProgram();
  envShader.setIntUni("envMap", 0);
  return envShader;
}

// hiz buffer related

struct MipMapInfo {
  const int width, height;
  const unsigned int level;
  MipMapInfo(int w, int h, unsigned int l) : width(w), height(h), level(l) {}
};
void genHiZTexture(GLuint &hizTex, std::vector<MipMapInfo> &ms, GLuint ww,
                   GLuint wh) {
  glGenTextures(1, &hizTex);
  glBindTexture(GL_TEXTURE_2D, hizTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINWIDTH, WINHEIGHT, 0, GL_RGB,
               GL_FLOAT, &imarr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glGenerateMipmap(GL_TEXTURE_2D);
}
void genHiZCopy(GLuint &hizTexCopy, std::vector<MipMapInfo> &ms) {
  glGenTextures(1, &hizTexCopy);
  glBindTexture(GL_TEXTURE_2D, hizTexCopy);
  float imarr[WINWIDTH * WINHEIGHT * 3];
  std::fill(std::begin(imarr), std::end(imarr), 1.0f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINWIDTH, WINHEIGHT, 0, GL_RGB,
               GL_FLOAT, &imarr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  for (unsigned int i = 0; i < ms.size(); i++) {
    MipMapInfo msize = ms[i];
    std::vector<float> iarr(msize.width * msize.height * 3, 1.0f);
    glTexImage2D(GL_TEXTURE_2D, msize.level, GL_RGB16F, msize.width,
                 msize.height, 0, GL_RGB, GL_FLOAT, &iarr[0]);
  }
}

void getSceneViewMats(std::vector<glm::mat4> &sceneViewMats,
                      const float CAMERA_FOV) {
  camera.setZoom(CAMERA_FOV);
  sceneViewMats.clear();
  for (GLuint i = 0; i < 6; i++) {
    switch (i) {
    case 0:
      float nyaw = 0.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 1:
      float nyaw = 90.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 2:
      float nyaw = 180.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 3:
      float nyaw = 270.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 4:
      float npitch = -90.0;
      camera.setPitch(npitch);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 5:
      float npitch = 90.0;
      camera.setPitch(npitch);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    }
  }
  camera.setZoom(45.0f); // default value
}

void genCubemapFboTexture(GLuint &cubemapTex, GLuint &cubemapFbo,
                          GLuint &cubemapRbo, GLuint ww, GLuint wh) {
  //
  glGenFramebuffers(1, &cubemapFbo);
  glGenRenderbuffers(1, &cubemapRbo);

  glBindFramebuffer(GL_FRAMEBUFFER, cubemapFbo);

  glGenTextures(1, &cubemapTex);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);

  // generate faces of cubemap
  for (GLuint i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, ww, wh, 0,
                 GL_RGBA, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // bind texture to frame buffer
  glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemapTex, 0);

  // setting color attachments
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};

  glDrawBuffers(1, attachments);
  gerr();

  // create and bind render buffer
  glBindRenderbuffer(GL_RENDERBUFFER, cubemapRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ww, wh);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, cubemapRbo);
}

void drawScene();

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

  // setting up textures
  // --------------------

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

  // ------------------------------------------------------------------------

  GLuint LOW_WINHEIGHT = 128;
  GLuint LOW_WINWIDTH = (GLuint)ASPECT_RATIO * LOW_WINHEIGHT;

  const float SCENE_FAR_PLANE = 1000.0f;
  float nearPlane = 0.1f;
  float farPlane = SCENE_FAR_PLANE;
  glm::vec2 nearFar(nearPlane, farPlane);
  const float CAMERA_FOV = 90.0;

  std::vector<glm::mat4> sceneViewMats(6);
  // generate scene view matrix

  // get cubemap render fbo and texture
  GLuint cubemapFbo, cubemapRbo, cubemapTex;
  genCubemapFboTexture(cubemapTex, cubemapFbo, cubemapRbo, WINWIDTH, WINHEIGHT);

  while (!glfwWindowShouldClose(window)) {
    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc2(window);
    //
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // --------------------------------------------------------------------
    // shading stages are here

    // ----------------------- A. Build Stage -----------------------------
    // get scene view matrix for setting up cubemap view of the scene
    getSceneViewMats(sceneViewMats, CAMERA_FOV);

    glViewport(0, 0 LOW_WINWIDTH, LOW_WINHEIGHT);
    // 1. Fill depth pass
    // requires: geometry of objects
    // resolution: lower than normal window width/height

    drawScene();

    // 1.1 Mipmap depth pass
    // requires: full screen quad
    // resolution: lower than normal window width/height

    renderQuad();

    // 2. Fill primitives pass
    // requires: geometry of objects
    // resolution: lower than normal window width/height

    drawScene();

    // 3. Direct illumination pass creating a geometry buffer
    // requires: geometry of objects
    // resolution: full screen resolution
    glViewPort(0, 0, WINWIDTH, WINHEIGHT);

    drawScene();

    // ----------------------- B. Traversal Stage -------------------------

    // 4. Trace Pass:
    // requires: screen quad
    // resolution: lower than normal window width/height
    glViewport(0, 0 LOW_WINWIDTH, LOW_WINHEIGHT);

    renderQuad();

    // 5. Fetch Pass:
    // requires: geometry of objects
    // resolution: full screen
    glViewPort(0, 0, WINWIDTH, WINHEIGHT);
    drawScene();

    // 6. Shade/Resolve Pass:
    // requires: full screen quad
    // resolution: full screen
    renderQuad();

    // --------------------------------------------------------------------
    // swap buffer vs
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
