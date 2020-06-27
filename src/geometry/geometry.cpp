// simple usage of geometry shader
// based on learnopengl
#include <custom/utils.hpp>

Shader loadBasicShader() {
  fs::path bvertpath = shaderDirPath / "geometry" / "basic.vert";
  fs::path bfragpath = shaderDirPath / "geometry" / "basic.frag";
  Shader shdr(bvertpath.c_str(), bfragpath.c_str());
  return shdr;
}

Shader loadGeometryShader() {
  // load geometry shader
  fs::path bvertpath = shaderDirPath / "geometry" / "geometry.vert";
  fs::path bfragpath = shaderDirPath / "geometry" / "geometry.frag";
  fs::path bgeopath = shaderDirPath / "geometry" / "geometry.geom";
  std::vector<fs::path> shaderPaths{bvertpath, bfragpath, bgeopath};
  std::vector<std::string> shaderTypes{
      std::string("VERTEX"), std::string("FRAGMENT"), std::string("GEOMETRY")};
  Shader shdr(shaderPaths, shaderTypes);
  return shdr;
}

Shader loadLampShader() {
  fs::path bvertpath = shaderDirPath / "geometry" / "basic.vert";
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / "geometry" / frag2FileName;
  Shader lampShader(bvertpath.c_str(), frag2Path.c_str());
  return lampShader;
}
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

unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

int main() {
  //
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Basic Phong With Geometry Shader", NULL, NULL);

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

  // deal with global opengl state
  glEnable(GL_DEPTH_TEST);

  Shader basicShader = loadBasicShader();
  Shader geometryShader = loadGeometryShader();
  Shader lampShader = loadLampShader();

  // -------------------- load models -----------------------------
  stbi_set_flip_vertically_on_load(true);
  fs::path modPath = modelPath / "backpack" / "backpack.obj";
  Model whaleModel(modPath.c_str(), flags, false);
  Model modelLight(modelPath / "sphere/scene.gltf", flags, false);

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
    basicShader.useProgram();
    basicShader.setMat4Uni("view", viewMat);
    basicShader.setMat4Uni("model", modelMat);
    basicShader.setMat4Uni("projection", projection);
    basicShader.setVec3Uni("lightPos", lightPos);
    basicShader.setVec3Uni("viewPos", viewPos);

    //
    whaleModel.Draw(basicShader);

    // redraw model with geometry shader
    geometryShader.useProgram();
    geometryShader.setMat4Uni("view", viewMat);
    geometryShader.setMat4Uni("model", modelMat);
    geometryShader.setMat4Uni("projection", projection);
    geometryShader.setVec3Uni("lightPos", lightPos);
    geometryShader.setVec3Uni("viewPos", viewPos);

    whaleModel.Draw(geometryShader);

    // draw the lamp
    // unbind the light vertex array object
    glm::mat4 lampModel(1.0f);
    lampModel = glm::translate(lampModel, lightPos);
    lampModel = glm::scale(lampModel, glm::vec3(0.005f));
    lampShader.useProgram();
    lampShader.setMat4Uni("model", lampModel);
    lampShader.setMat4Uni("projection", projection);
    lampShader.setMat4Uni("view", viewMat);
    lampShader.setVec3Uni("lightColor", glm::vec3(1.0));
    // render lamp
    modelLight.Draw(lampShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
