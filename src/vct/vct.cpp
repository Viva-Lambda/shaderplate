// license, see LICENSE
/**

  Development plan

  1. voxelize scene : a model
  2. Write it to a texture
  3. visualize voxelized scene using a quad renderer

 */
#include <custom/utils.hpp>

// --------------------- Global variables ---------------------------------
// --------------------- Debug Related ------------------------------------
GLuint debugTexture;

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
float light_near_plane = 1.0f;
float light_far_plane = 7.5f;
glm::mat4 lightSpaceMat;
glm::mat4 lightV = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
glm::mat4 lightP =
    glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, light_near_plane, light_far_plane);

// ----------------------- end light space --------------------------------

// --------------------- end of Global variables ---------------------------

// ----------------------- Textures ---------------------------------------
GLuint stoneTexture;

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

/**
  Setup shadow map fbo and texture
 */
void loadShadowTextureFbo() {
  //
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

  // -------------------------- FBO ----------------------------------
  glGenFramebuffers(1, &shadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         shadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

/**
  A plane shader to show shadows
 */
Shader loadPlaneShader() {
  fs::path vpath = shaderDirPath / "vct" / "plane.vert";
  fs::path fpath = shaderDirPath / "vct" / "plane.frag";
  Shader planeShader(vpath.c_str(), fpath.c_str());
  return planeShader;
}

void initPlaneShader(Shader planeShader, glm::mat4 viewMat, glm::mat4 modelMat,
                     glm::mat4 projection) {
  planeShader.useProgram();
  planeShader.setMat4Uni("view", viewMat);
  planeShader.setMat4Uni("model", modelMat);
  planeShader.setMat4Uni("projection", projection);
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

Shader loadQuadShader() {
  fs::path vpath = shaderDirPath / "vct" / "quad.vert";
  fs::path fpath = shaderDirPath / "vct" / "quad.frag";
  Shader quadShader(vpath.c_str(), fpath.c_str());
  return quadShader;
}

// ------------------------ end of shader ----------------------------------

// ------------------------ start drawing procedures -----------------------

/**
  The procedure which create the shadow map
 */
void renderShadowMap(Shader shadowShader, std::vector<Model> models) {
  lightSpaceMat = lightP * lightV; // create lightspace matrix to render
                                   // scene with respect to light
  shadowShader.setMat4Uni("lightSpaceMat", lightSpaceMat);

  // set viewport to the size of shadow resolution
  glViewport(0, 0, depthResolution, depthResolution);

  // bind the framebuffer to which we will draw the shadow map
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

  // clear the content of the previous framebuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // consider back face of objects when computing limits
  glCullFace(GL_FRONT);

  // drawing pass with shadow shader
  shadowShader.useProgram();
  for (unsigned int i = 0; i < models.size(); i++) {
    models[i].Draw(shadowShader);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, WINWIDTH, WINHEIGHT);
}

void renderVoxel();
void renderShadowMap();
void debugRender();
void renderNormalScene(Model backpack, Model lamp, Shader modelShader,
                       Shader lampShader, Shader planeShader);
void renderPlaneWithTexture();

// ------------------------ end drawing procedures -----------------------

unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

Model loadBackpackModel() {
  fs::path modPath = modelPath / "backpack" / "backpack.obj";
  // stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, false);
  return modelB;
}
Model loadLightModel() {
  Model modelLight(modelPath / "sphere/scene.gltf", flags, false);
  return modelLight;
}

// ------------------------- General Proceedures ------------------------

int main() {

  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Voxel Cone Tracing Example", NULL, NULL);

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

  // enable opengl state
  glEnable(GL_DEPTH_TEST);

  // load models
  Model backpack = loadBackpackModel();
  Model lamp = loadLightModel();
  showKeys();

  Shader modelShader = loadModelShader();
  Shader lampShader = loadLampShader();
  Shader quadShader = loadQuadShader();
  Shader planeShader = loadPlaneShader();
  Shader shadowShader = loadShadowShader();
  // cubeShaderInit_proc(lampShader);
  GLuint stoneTexture = loadA2DTexture();

  while (glfwWindowShouldClose(window) == 0) {
    float currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc(window);
    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. draw shadow maps = render scene from light's perspective

    //
    renderNormalScene(backpack, lamp, modelShader, lampShader, planeShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}

void debugRender() {
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, debugTexture);
  renderQuad();
}

void renderNormalScene(Model backpack, Model lamp, Shader modelShader,
                       Shader lampShader, Shader planeShader) {
  // render normal scene
  glm::mat4 projection =
      glm::perspective(glm::radians(camera.zoom),
                       (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
  glm::mat4 viewMat = camera.getViewMatrix();
  glm::vec3 viewPos = camera.pos;
  glm::mat4 modelMat(1.0f);
  initPlaneShader(planeShader, viewMat, modelMat, projection);
  renderPlane();

  // set uniforms to model shader
  modelMat = glm::mat4(1);
  modelMat = glm::translate(modelMat, glm::vec3(0, 2.0, 0.0));
  initModelShader(modelShader, viewMat, modelMat, projection, viewPos,
                  lightPos);

  // draw model
  backpack.Draw(modelShader);

  // draw the lamp
  // unbind the light vertex array object
  glm::mat4 lampModel(1.0f);
  lampModel = glm::translate(lampModel, lightPos);
  lampModel = glm::scale(lampModel, glm::vec3(0.005f));

  initLampShader(lampShader, lampModel, projection, viewMat, lightIntensity);
  // render lamp
  lamp.Draw(lampShader);
}
