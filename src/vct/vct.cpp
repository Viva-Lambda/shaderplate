// license, see LICENSE
/**

  Development plan

  1. voxelize scene : a model
  2. Write it to a texture
  3. visualize voxelized scene using a quad renderer

 */
#include "vctutils.hpp"

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
  int w, h;
  getWidthHeightViewpor(w, h);
  Model sponza = loadSponzaModel();
  Model lamp = loadLightModel();
  showKeys();

  Shader modelShader = loadModelShader();
  Shader lampShader = loadLampShader();
  Shader debugDepthShader = loadDebugDepthShader();
  Shader shadowShader = loadShadowShader();
  Shader quadShader = loadQuadShader();
  // cubeShaderInit_proc(lampShader);
  GLuint stoneTexture = loadA2DTexture();

  // setting up necessary opengl ressources in arbitrary order
  setAllUp();

  while (glfwWindowShouldClose(window) == 0) {
    float currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc2(window);
    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. draw shadow maps = render scene from light's perspective

    // consider back face of objects when computing limits
    glCullFace(GL_FRONT);
    renderShadowMap(shadowShader, sponza, lamp);
    glCullFace(GL_BACK);

    // 1.5 render debug information of depth buffer
    // renderDebugDepth(debugDepthShader);
    // renderDebugShadow(shadowShader, debugDepthShader, sponza, lamp);

    // 2. render normal scene
    renderNormalScene(sponza, lamp, modelShader, lampShader, quadShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
