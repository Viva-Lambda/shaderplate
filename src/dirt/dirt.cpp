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

Shader loadMultiVCubemapShader() {
  fs::path vpath = shaderDirPath / "dirt" / "multiCube.vert"; // DONE
  fs::path gpath = shaderDirPath / "dirt" / "multiCube.geom"; // DONE
  fs::path fpath = shaderDirPath / "dirt" / "multiCube.frag"; // DONE
  Shader envShader(vpath.c_str(), fpath.c_str());
  envShader.shaderName = "multiview";
  return envShader;
}
Shader loadFillDepthShader() {
  std::vector<fs::path> paths = {
      shaderDirPath / "dirt" / "fillDepth.vert",
      shaderDirPath / "dirt" / "fillDepth.geom",  // DONE
      shaderDirPath / "dirt" / "fillDepth.frag"}; // DONE
  std::vector<std::string> strs = {"VERTEX", "GEOMETRY", "FRAGMENT"};
  Shader envShader(paths, strs);
  envShader.shaderName = "fillDepthShader";
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
               GL_FLOAT, NULL);
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
    float nyaw, npitch;
    switch (i) {
    case 0:
      nyaw = 0.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 1:
      nyaw = 90.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 2:
      nyaw = 180.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 3:
      nyaw = 270.0;
      camera.setYaw(nyaw);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 4:
      npitch = -90.0;
      camera.setPitch(npitch);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    case 5:
      npitch = 90.0;
      camera.setPitch(npitch);
      sceneViewMats[i] = camera.getViewMatrix();
      break;
    }
  }
  camera.setZoom(45.0f); // default value
}

void getFrustumCorners(glm::mat4 invVP, std::vector<glm::vec4> &corners) {
  //
  glm::vec4 localCorner[8] = {
      glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
      glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),  glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
      glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),  glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
      glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),   glm::vec4(1.0f, 1.0f, -1.0f, 1.0f)};
  for (GLuint i = 0; i < 8; i++) {
    corners[i] = invVP * localCorner[i];
    corners[i] /= corners[i].w;
  }
}
void getViewPort(glm::mat4 projection, glm::vec4 &viewport, float cubeSize) {
  glm::vec4 localCorners[4] = {
      glm::vec4(0.0, 0.0, 1.0, 1.0), // bottom left
      glm::vec4(1.0, 0.0, 1.0, 1.0), // bottom right
      glm::vec4(0.0, 1.0, 1.0, 1.0), // top left
      glm::vec4(1.0, 1.0, 1.0, 1.0)  // top right
  };
  for (GLuint i = 0; i < 4; i++) {
    glm::vec4 viewp = glm::vec4(0, 0, cubeSize, cubeSize);
    glm::vec2 nearFar = glm::vec2(cubeSize);
    glm::vec2 localClip = getClipFromFrag(localCorners[i], viewp, nearFar);
  }
}

void genCubemapFboTexture(GLuint &cubemapTex, GLuint &cubemapFbo,
                          GLuint &cubemapRbo, GLuint ww, GLuint wh) {
  //
  glGenFramebuffers(1, &cubemapFbo);
  glGenRenderbuffers(1, &cubemapRbo);
  glGenTextures(1, &cubemapTex);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubemapFbo);
  gerrf();
  gerr();

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
  gerr();

  // setup texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  gerr();
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gerr();
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  gerr();

  // generate faces of cubemap
  // glTexStorage2D(GL_TEXTURE_CUBE_MAP, 0, GL_RGBA16F, ww, wh);
  gerr();

  for (GLuint i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, // level
                 GL_RGBA16F,                            // internal format
                 ww, wh,                                // width, height
                 0,                                     // border
                 GL_RGBA,                               // format
                 GL_FLOAT,                              // data type
                 NULL);
    gerr();
  }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  gerr();

  // bind texture to frame buffer
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemapTex,
                       0); // mipmap level
  gerr();

  // setting color attachments
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};

  glDrawBuffers(1, attachments);

  // create and bind render buffer
  glBindRenderbuffer(GL_RENDERBUFFER, cubemapRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ww, wh);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, cubemapRbo);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  gerrf();
}

struct CubemapInfo {
  glm::vec2 NearFar;
  int cube_index;
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec4 viewport;
  std::vector<glm::vec4> frustum_corners;
};
CubemapInfo getCubeInfo(std::vector<glm::mat4> sceneViewMats,
                        glm::mat4 projection, float cubeSize, int cubeIndex) {
  CubemapInfo cinfo;
  glm::mat4 view = sceneViewMats[cubeIndex];
  glm::mat4 invVP = glm::inverse(projection * view);
  glm::vec2 nfar = glm::vec2(0.1,     // near
                             cubeSize // far
                             );
  std::vector<glm::vec4> corners(8);
  getFrustumCorners(invVP, corners);
  cinfo.view = view;
  cinfo.projection = projection;
  cinfo.viewport = glm::vec4(0, 0, cubeSize, cubeSize);
  cinfo.frustum_corners = corners;
  cinfo.cube_index = cubeIndex;
  cinfo.NearFar = nfar;
  return cinfo;
}

void drawMultiVScene(Shader &fillDepthShader, const GLuint SCENE_MAT_NB,
                     const std::vector<glm::mat4> &sceneViewMats,
                     const float CAMERA_FOV, glm::mat4 &model, GLuint CUBESIZE,
                     GLuint &cubemapTex, const glm::vec2 &nearFar) {
  fillDepthShader.useProgram();
  glm::mat4 projection = glm::perspective(glm::radians(CAMERA_FOV),
                                          (float)WINWIDTH / (float)WINHEIGHT,
                                          nearFar.x, nearFar.y);

  for (GLuint i = 0; i < SCENE_MAT_NB; i++) {
    CubemapInfo cinfo =
        getCubeInfo(sceneViewMats, projection, CUBESIZE, int(i));

    glm::mat4 MVP = cinfo.projection * cinfo.view * model;
    glm::mat4 MV = cinfo.view * model;
    fillDepthShader.setMat4Uni("model", model); // vertex shader
    fillDepthShader.setMat4Uni("ModelViewProjection[" + std::to_string(i) + "]",
                               MVP);
    fillDepthShader.setMat4Uni("ModelView" + std::to_string(i) + "]", MV);
    fillDepthShader.setVec4Uni("Viewports" + std::to_string(i) + "]",
                               cinfo.viewport);
    fillDepthShader.setVec2Uni("NearFar[" + std::to_string(i) + "]",
                               cinfo.NearFar);
    fillDepthShader.setIntUni("cube_index", cinfo.cube_index);
    for (GLuint c = 0; c < 8; c++) {
      glm::vec4 corner = cinfo.frustum_corners[c];
      glm::vec3 fcorner = glm::vec3(corner.x, corner.y, corner.z);
      fillDepthShader.setVec3Uni(
          "FrustumCorners[" + std::to_string(i * c) + "]", fcorner);
    }
    gerrf();
    gerr();
    renderSphere();
  }
}
void drawScene() {}

struct TriangleBuffer {
  glm::mat4 triangle_vertices_normal;
  glm::mat4 triangle_tangents;
  glm::mat2 triangle_textures;
};

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

  // ------------------- make ssbo --------------------
  GLuint triangleBlock;
  glGenBuffers(1, &triangleBlock);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleBuffer), NULL,
               GL_STATIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleBlock);

  // ------------------------------------------------------------------------

  GLuint LOW_WINHEIGHT = 128;
  GLuint LOW_WINWIDTH = (GLuint)ASPECT_RATIO * LOW_WINHEIGHT;
  GLuint LOW_CUBESIZE =
      LOW_WINWIDTH > LOW_WINHEIGHT ? LOW_WINWIDTH : LOW_WINHEIGHT;
  GLuint CUBESIZE = WINWIDTH > WINHEIGHT ? WINWIDTH : WINHEIGHT;

  const float SCENE_FAR_PLANE = 1000.0f;
  float nearPlane = 0.1f;
  float farPlane = SCENE_FAR_PLANE;
  glm::vec2 nearFar(nearPlane, farPlane);
  const float CAMERA_FOV = 90.0;
  const GLuint SCENE_MAT_NB = 6;

  // scene objects
  glm::vec3 objectPos = glm::vec3(3.0, -0.5, -3.0);
  glm::vec3 objectPos2 = glm::vec3(0.5, -0.5, -5.0);
  glm::mat4 model1 = glm::mat4(1);
  glm::mat4 model2 = glm::mat4(1);

  model1 = glm::translate(model1, objectPos);
  model1 = glm::scale(model1, glm::vec3(1.0f));

  model2 = glm::translate(model2, objectPos2);
  model2 = glm::scale(model2, glm::vec3(1.0f));

  std::vector<glm::mat4> sceneViewMats(SCENE_MAT_NB);
  // generate scene view matrix

  // scene cubemap properties

  // get cubemap render fbo and texture
  GLuint cubemapFbo, cubemapRbo, cubemapDepthTex;
  genCubemapFboTexture(cubemapDepthTex, cubemapFbo, cubemapRbo, CUBESIZE,
                       CUBESIZE);

  // shader multiview cubemap
  Shader fillDepthShader = loadFillDepthShader();

  while (!glfwWindowShouldClose(window)) {
    //
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc2(window);
    //
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // --------------------------------------------------------------------
    // shading stages are here

    // ----------------------- A. Build Stage -----------------------------
    {
      // get scene view matrix for setting up cubemap view of the scene
      getSceneViewMats(sceneViewMats, CAMERA_FOV);
      glBindFramebuffer(GL_FRAMEBUFFER, cubemapFbo);
      gerrf();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // draw gbuffer on steroids

      // draw scene
      drawMultiVScene(fillDepthShader, SCENE_MAT_NB, sceneViewMats, CAMERA_FOV,
                      model1, CUBESIZE, cubemapDepthTex, nearFar);
      drawMultiVScene(fillDepthShader, SCENE_MAT_NB, sceneViewMats, CAMERA_FOV,
                      model2, CUBESIZE, cubemapDepthTex, nearFar);

      //
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, LOW_WINWIDTH, LOW_WINHEIGHT);
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
    glViewport(0, 0, WINWIDTH, WINHEIGHT);

    drawScene();

    // ----------------------- B. Traversal Stage -------------------------

    // 4. Trace Pass:
    // requires: screen quad
    // resolution: lower than normal window width/height
    glViewport(0, 0, LOW_WINWIDTH, LOW_WINHEIGHT);

    renderQuad();

    // 5. Fetch Pass:
    // requires: geometry of objects
    // resolution: full screen
    glViewport(0, 0, WINWIDTH, WINHEIGHT);
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
