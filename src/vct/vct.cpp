// license, see LICENSE
/**

  Development plan

  1. voxelize scene : a model
  2. Write it to a texture
  3. visualize voxelized scene using a quad renderer


 */

#include <custom/utils.hpp>

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

Shader loadLampShader() {
  fs::path vpath = shaderDirPath / "highPolyModel.vert";
  fs::path fpath = shaderDirPath / "basic_color_light.frag";
  Shader modelShader(vpath.c_str(), fpath.c_str());
  return modelShader;
}

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
  // cubeShaderInit_proc(lampShader);

  while (glfwWindowShouldClose(window) == 0) {
    float currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc(window);
    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.zoom),
                         (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
    glm::mat4 viewMat = camera.getViewMatrix();
    glm::vec3 viewPos = camera.pos;
    // set uniforms
    modelShader.useProgram();
    modelShader.setMat4Uni("view", viewMat);
    modelShader.setMat4Uni("model", modelMat);
    modelShader.setMat4Uni("projection", projection);
    modelShader.setVec3Uni("viewPos", viewPos);
    modelShader.setVec3Uni("lightPos", lightPos);

    // draw model
    backpack.Draw(modelShader);

    // draw the lamp
    // unbind the light vertex array object
    glm::mat4 lampModel(1.0f);
    lampModel = glm::translate(lampModel, lightPos);
    lampModel = glm::scale(lampModel, glm::vec3(0.005f));

    lampShader.useProgram();
    lampShader.setMat4Uni("model", lampModel);
    lampShader.setMat4Uni("projection", projection);
    lampShader.setMat4Uni("view", viewMat);
    lampShader.setVec3Uni("lightColor", glm::vec3(lightIntensity));
    // render lamp
    lamp.Draw(lampShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
