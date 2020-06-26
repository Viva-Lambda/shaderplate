// license: see, LICENSE
#include <custom/utils.hpp>

void cubeShaderInit_proc2(Shader myShader) {
  myShader.useProgram();
  float ambientCoeff = 0.1f;
  float shininess = 200.0f;
  glm::vec3 attc(1.0f, 0.0f, 0.0f);
  myShader.setFloatUni("ambientCoeff", ambientCoeff);
  // myShader.setFloatUni("shininess", shininess);
  myShader.setFloatUni("world_scale", 0.0);
  myShader.setVec3Uni("attC", attc);
  myShader.setIntUni("diffuseMap", 0);
  myShader.setIntUni("heightMap", 1);
  myShader.setIntUni("normalMap", 2);
}

int main() {
  initializeGLFWMajorMinor(4, 2);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Basic Phong With Height Map", NULL, NULL);

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

  // deal with textures
  // Stone_001_Diffuse.png
  // Stone_001_Normal.png
  // Stone_001_Specular.png
  // rustediron2_metallic.png
  // rustediron2_normal.png
  // rustediron2_roughness.png
  // layered-fs
  fs::path diffmapPath =
      textureDirPath / "layered" / "layered-cliff-albedo.png";
  fs::path normalMapPath =
      textureDirPath / "layered" / "layered-cliff-normal-ogl.png";
  fs::path heightMapPath =
      textureDirPath / "layered" / "layered-cliff-height.png";

  GLuint diffuseMap;
  glGenTextures(1, &diffuseMap);
  loadTexture2d(diffmapPath.c_str(), diffuseMap);
  GLuint heightMap;
  glGenTextures(1, &heightMap);
  loadTexture2d(heightMapPath.c_str(), heightMap);
  GLuint normalMap;
  glGenTextures(1, &normalMap);
  loadTexture2d(normalMapPath.c_str(), normalMap);

  // load shaders
  // cube shader
  std::string vertFileName_t = "heightmap.vert";
  std::string fragFileName_t = "heightmap.frag";

  fs::path vertPath_t = shaderDirPath / vertFileName_t;
  fs::path fragPath_t = shaderDirPath / fragFileName_t;

  Shader tangentCubeShader(vertPath_t.c_str(), fragPath_t.c_str());

  // lamp shader
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath_t.c_str(), frag2Path.c_str());

  // let's set up some uniforms

  // init proc for uniforms that don't change over rendering
  cubeShaderInit_proc2(tangentCubeShader);

  // let's deal with vertex array objects and buffers
  // render loop
  while (glfwWindowShouldClose(window) == 0) {
    float currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInput_proc(window);
    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setting model, view, projection

    glm::mat4 projection =
        glm::perspective(glm::radians(camera.zoom),
                         (float)WINWIDTH / (float)WINHEIGHT, 0.1f, 100.0f);
    glm::mat4 viewMat = camera.getViewMatrix();
    glm::vec3 viewPos = camera.pos;

    // float lightIntensity = sin(glfwGetTime() * 1.0f);
    float lightIntensity = 1.0f;

    // render cube object
    glm::mat4 cubeModel(1.0f);
    // float angle = 20.0f;
    // render cube
    tangentCubeShader.useProgram();
    tangentCubeShader.setMat4Uni("view", viewMat);
    tangentCubeShader.setMat4Uni("model", cubeModel);
    tangentCubeShader.setMat4Uni("projection", projection);
    // tangentCubeShader.setVec3Uni("viewPos", viewPos);
    tangentCubeShader.setVec3Uni("lightPos", lightPos);
    tangentCubeShader.setFloatUni("lightIntensity", lightIntensity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    renderCubeInTangentSpace();

    // unbind the light vertex array object
    glm::mat4 lampModel(1.0f);
    lampModel = glm::translate(lampModel, lightPos);
    lampModel = glm::scale(lampModel, glm::vec3(0.2f));
    lampShader.useProgram();
    lampShader.setMat4Uni("model", lampModel);
    lampShader.setMat4Uni("projection", projection);
    lampShader.setMat4Uni("view", viewMat);
    lampShader.setVec3Uni("lightColor", glm::vec3(lightIntensity));
    // render lamp
    renderLamp();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
