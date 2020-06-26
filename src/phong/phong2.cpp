// license: see, LICENSE

#include <custom/utils.hpp>

int main() {
  initializeGLFWMajorMinor(4, 2);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Basic Phong With Specular Map", NULL, NULL);

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

  fs::path diffmapPath = textureDirPath / "Stone_001_Diffuse.png";
  fs::path specularMapPath = textureDirPath / "Stone_001_Specular.png";
  fs::path normalMapPath = textureDirPath / "Stone_001_Normal.png";

  GLuint diffuseMap;
  glGenTextures(1, &diffuseMap);
  loadTexture2d(diffmapPath.c_str(), diffuseMap);
  GLuint specularMap;
  glGenTextures(1, &specularMap);
  loadTexture2d(specularMapPath.c_str(), specularMap);
  GLuint normalMap;
  glGenTextures(1, &normalMap);
  loadTexture2d(normalMapPath.c_str(), normalMap);

  // load shaders
  // cube shader
  std::string vertFileName_t = "phong.vert";
  std::string fragFileName_t = "phong.frag";

  fs::path vertPath_t = shaderDirPath / vertFileName_t;
  fs::path fragPath_t = shaderDirPath / fragFileName_t;

  Shader tangentCubeShader(vertPath_t.c_str(), fragPath_t.c_str());

  // lamp shader
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath_t.c_str(), frag2Path.c_str());

  // let's set up some uniforms

  // cube shader
  //

  // init proc for uniforms that don't change over rendering
  cubeShaderInit_proc(tangentCubeShader);

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

    // isik kaynagi
    lightPos.x = 1.0f + sin(currentTime) * 2.0f;
    lightPos.y = sin(currentTime / 2.0f) * 1.0f;
    lightPos.z = sin(currentTime / 5.0f) * 3.0f;
    // float angle = 20.0f;
    // render cube
    tangentCubeShader.useProgram();
    tangentCubeShader.setMat4Uni("view", viewMat);
    tangentCubeShader.setMat4Uni("model", cubeModel);
    tangentCubeShader.setMat4Uni("projection", projection);
    tangentCubeShader.setVec3Uni("viewPos", viewPos);
    tangentCubeShader.setVec3Uni("lightPos", lightPos);
    tangentCubeShader.setFloatUni("lightIntensity", lightIntensity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
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
