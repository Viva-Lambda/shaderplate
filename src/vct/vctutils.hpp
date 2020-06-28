#ifndef VCTUTILS_HPP
#define VCTUTILS_HPP

#include <custom/utils.hpp>
// --------------------- Global variables ---------------------------------
float deltaTime2 = 1;
// --------------------- Debug Related ------------------------------------
GLuint debugTexture;

// normal scene related
// two textures are necessary one for having 3d storage
// the other for having 2d storage
GLuint sceneTextureArray, sceneTexture;
GLuint sceneFBO; // fbo of the scene

// voxel related
GLuint voxel2DTexture; // used for debugging purposes
int voxelResolution = 64;
GLuint mipmapLevels = static_cast<GLuint>(glm::log2((float)voxelResolution));

GLuint voxel3DTexture; // for storing voxel values

// ---------------------- shadow map related ------------------------------
GLuint shadowFBO, shadowMap;
int depthResolution = 512;
GLuint planeVAO;

// ---------------------- end shadow map related --------------------------

// -------------------------- light space ---------------------------------
float light_near_plane = 0.01f;
float light_far_plane = 1000.0f;
glm::mat4 lightSpaceMat;
glm::mat4 lightV = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
glm::mat4 lightP = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f,
                              light_near_plane, light_far_plane);

// ----------------------- end light space --------------------------------

// --------------------- end of Global variables ---------------------------
// --------------------- utils ----------------------

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
    lightPos.y += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    lightPos.y -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    lightPos.x += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    lightPos.x -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    lightPos.z -= deltaTime2; // the axis are inverse
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    lightPos.z += deltaTime2;
  }
}
void processInput_proc2(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  moveCamera2(window);
  moveLight2(window);
  captureScreen(window);
}

// ----------------------- Textures ---------------------------------------
/**
  Create a debugging texture
 */
void loadDebugTexture(int w, int h, GLenum informat) {

  glGenTextures(1, &debugTexture);
  glBindTexture(GL_TEXTURE_2D, debugTexture);
  glTexStorage2D(GL_TEXTURE_2D, 1, informat, w, h);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/**
  This function is here as a debugging aid.
  It helps to visualize the contents of the voxelization procedure
 */
void loadVoxel2DTexture() {
  if (voxel2DTexture != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &voxel2DTexture);
  }

  glGenTextures(1, &voxel2DTexture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, voxel2DTexture);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, voxelResolution,
                 voxelResolution, 3);
  /*
    void glTexStorage3D(GLenum target,
        GLsizei levels,
        GLenum internalformat,
        GLsizei width,
        GLsizei height,
        GLsizei depth);

   */
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/**
  Initializer function for 3d voxel that would store the necessary values.
 */
void loadVoxel3DTexture() {
  if (voxel3DTexture != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &voxel3DTexture);
  }

  glGenTextures(1, &voxel3DTexture);
  glBindTexture(GL_TEXTURE_3D, voxel3DTexture);
  glTexStorage3D(GL_TEXTURE_3D, mipmapLevels + 1, GL_R32UI, voxelResolution,
                 voxelResolution, voxelResolution);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void setVoxelUp() {
  loadVoxel2DTexture();
  loadVoxel3DTexture();
}

void loadScene2DTextureArray() {
  if (sceneTextureArray != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &sceneTextureArray);
  }
  // get current viewport size in order to set it back after loading
  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, currentViewport);
  int width = currentViewport[2];
  int height = currentViewport[3];

  glGenTextures(1, &sceneTextureArray);
  glBindTexture(GL_TEXTURE_2D_ARRAY, sceneTextureArray);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, width, height, 5);
  // 5 is passed to fbo as well, I might refactor it to another global
  /*
    void glTexStorage3D(GLenum target,
        GLsizei levels,
        GLenum internalformat,
        GLsizei width,
        GLsizei height,
        GLsizei depth);

   */
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void loadScene2DTexture() {
  if (sceneTexture != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &sceneTexture);
  }
  // get current viewport size in order to set it back after loading
  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, currentViewport);
  int width = currentViewport[2];
  int height = currentViewport[3];

  glGenTextures(1, &sceneTexture);
  glBindTexture(GL_TEXTURE_2D, sceneTexture);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void loadSceneFBO() {
  if (sceneFBO == 0) {
    // if it is not generated yet
    glGenFramebuffers(1, &sceneFBO);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            sceneTextureArray, 0, 0);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                            sceneTextureArray, 0, 1);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                            sceneTextureArray, 0, 2);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
                            sceneTextureArray, 0, 3);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4,
                            sceneTextureArray, 0, 4);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sceneTexture, 0);

  GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                          GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                          GL_COLOR_ATTACHMENT4};
  glDrawBuffers(5, DrawBuffers);
  glReadBuffer(GL_NONE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setSceneUp() {
  loadScene2DTexture();
  loadScene2DTextureArray();
  loadSceneFBO();
}

/**
  Setup shadow map fbo and texture
 */
void loadShadowTexture() {
  if (shadowMap != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &shadowMap);
  }

  // ----------------------------- Texture ----------------------------------
  glGenTextures(1, &shadowMap);

  glBindTexture(GL_TEXTURE_2D, shadowMap);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, depthResolution,
                 depthResolution);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_REF_TO_TEXTURE);
}
void loadShadowFbo() {
  // -------------------------- FBO ----------------------------------
  glGenFramebuffers(1, &shadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         shadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
  Setting up ressources with respect to shadow
 */
void setShadowUp() {
  loadShadowTexture();
  loadShadowFbo();
}

void bindShadowFboToDebugTexture() {
  if (debugTexture != 0) {
    // texture not empty let us delete it
    glDeleteTextures(1, &debugTexture);
  }
  loadDebugTexture(depthResolution, depthResolution, GL_DEPTH_COMPONENT32F);
  glGenFramebuffers(1, &shadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         debugTexture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ------------------ end of Textures --------------------------------------

// ------------------------ shaders ----------------------------------------

void cubeShaderInit_proc2(Shader myShader) {
  myShader.useProgram();
  float ambientCoeff = 0.1f;
  float shininess = 20.0f;
  glm::vec3 attc(1.0f, 0.0f, 0.0f);
  myShader.setFloatUni("ambientCoeff", ambientCoeff);
  myShader.setFloatUni("shininess", shininess);
  myShader.setFloatUni("lightIntensity", lightIntensity);
  myShader.setVec3Uni("attC", attc);
}

Shader loadModelShader() {
  //
  fs::path vpath = shaderDirPath / "highPolyModel.vert";
  fs::path fpath = shaderDirPath / "highPolyModelSimple.frag";
  Shader modelShader(vpath.c_str(), fpath.c_str());
  cubeShaderInit_proc2(modelShader);
  return modelShader;
}
void initModelShader(Shader modelShader, glm::mat4 viewMat, glm::mat4 modelMat,
                     glm::mat4 projection, glm::vec3 viewPos,
                     glm::vec3 lightPos) {
  modelShader.useProgram();
  modelShader.setMat4Uni("view", viewMat);
  modelShader.setMat4Uni("model", modelMat);
  modelShader.setMat4Uni("projection", projection);
  modelShader.setVec3Uni("viewPos", viewPos);
  modelShader.setVec3Uni("lightPos", lightPos);
}

Shader loadLampShader() {
  fs::path vpath = shaderDirPath / "highPolyModel.vert";
  fs::path fpath = shaderDirPath / "basic_color_light.frag";
  Shader modelShader(vpath.c_str(), fpath.c_str());
  return modelShader;
}

void initLampShader(Shader lampShader, glm::mat4 lampModel,
                    glm::mat4 projection, glm::mat4 viewMat,
                    float lightIntensity) {
  //
  lampShader.useProgram();
  lampShader.setMat4Uni("model", lampModel);
  lampShader.setMat4Uni("projection", projection);
  lampShader.setMat4Uni("view", viewMat);
  lampShader.setVec3Uni("lightColor", glm::vec3(lightIntensity));
}

Shader loadVoxelizationShader() {
  // load geometry shader
  fs::path bvertpath = shaderDirPath / "vct" / "voxelization.vert";
  fs::path bfragpath = shaderDirPath / "vct" / "voxelization.frag";
  fs::path bgeopath = shaderDirPath / "vct" / "voxelization.geom";
  std::vector<fs::path> shaderPaths{bvertpath, bfragpath, bgeopath};
  std::vector<std::string> shaderTypes{
      std::string("VERTEX"), std::string("FRAGMENT"), std::string("GEOMETRY")};
  Shader shdr(shaderPaths, shaderTypes);
  return shdr;
}

Shader loadShadowShader() {
  fs::path vpath = shaderDirPath / "vct" / "shadow.vert";
  fs::path fpath = shaderDirPath / "vct" / "shadow.frag";
  Shader shadowShader(vpath.c_str(), fpath.c_str());
  return shadowShader;
}

void initShadowShader(Shader shadowShader) {
  // shadowShader.setMat4Uni("lightSpaceMat", )
  shadowShader.useProgram();
  shadowShader.setIntUni("diffuseMap1", 0);
  shadowShader.setIntUni("shadowMap", 1);
}

Shader loadDebugDepthShader() {
  fs::path vpath = shaderDirPath / "vct" / "debug_depth.vert";
  fs::path fpath = shaderDirPath / "vct" / "debug_depth.frag";
  Shader depthShader(vpath.c_str(), fpath.c_str());
  return depthShader;
}

// ------------------------ end of shader ----------------------------------

// ------------------------ start drawing procedures -----------------------

/**
  The procedure which create the shadow map
 */
void renderShadowMap(Shader shadowShader, Model sponza, Model lamp) {
  lightSpaceMat = lightP * lightV; // create lightspace matrix to render
                                   // scene with respect to light

  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, currentViewport);
  int width = currentViewport[2];
  int height = currentViewport[3];

  // set viewport to the size of shadow resolution
  glViewport(0, 0, depthResolution, depthResolution);

  // bind the framebuffer to which we will draw the shadow map
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

  // clear the content of the previous framebuffer
  glClear(GL_DEPTH_BUFFER_BIT);

  // consider back face of objects when computing limits
  glCullFace(GL_FRONT);

  // drawing pass with shadow shader
  shadowShader.useProgram();
  shadowShader.setMat4Uni("lightSpaceMat", lightSpaceMat);

  glm::mat4 modelMat(1.0f);
  shadowShader.setMat4Uni("model", modelMat);

  sponza.Draw(shadowShader);
  lamp.Draw(shadowShader);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(currentViewport[0], currentViewport[1], width, height);
}

void renderDebugDepth(Shader debugDepthShader) {
  debugDepthShader.useProgram();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, shadowMap);
  renderQuad();
}

void renderVoxel();
void renderShadowMap();
void debugRender();
void renderNormalScene(Model sponza, Model lamp, Shader modelShader,
                       Shader lampShader);
void renderPlaneWithTexture();

// ------------------------ end drawing procedures -----------------------

unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

Model loadSponzaModel() {
  fs::path modPath = modelPath / "sponza" / "sponza.obj";
  // stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, true, false, true);
  return modelB;
}
Model loadLightModel() {
  Model modelLight(modelPath / "sphere/scene.gltf", flags, false);
  return modelLight;
}

void renderNormalScene(Model sponza, Model lamp, Shader modelShader,
                       Shader lampShader) {
  // render normal scene
  glm::mat4 projection =
      glm::perspective(glm::radians(camera.zoom),
                       (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 5000.0f);
  glm::mat4 viewMat = camera.getViewMatrix();
  glm::vec3 viewPos = camera.pos;
  glm::mat4 modelMat(1.0f);
  // initPlaneShader(planeShader, viewMat, modelMat, projection);
  // renderPlane();

  // set uniforms to model shader
  initModelShader(modelShader, viewMat, modelMat, projection, viewPos,
                  lightPos);

  // draw model
  sponza.Draw(modelShader);

  // draw the lamp
  // unbind the light vertex array object
  glm::mat4 lampModel(1.0f);
  lampModel = glm::translate(lampModel, lightPos);
  lampModel = glm::scale(lampModel, glm::vec3(0.5f));

  initLampShader(lampShader, lampModel, projection, viewMat, lightIntensity);
  // render lamp
  lamp.Draw(lampShader);
}
#endif
