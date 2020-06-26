/*
   Simple PBR shader
 */
// license: see, LICENSE
#include <custom/utils.hpp>

void cubeShaderInit_proc2(Shader myShader) {
  myShader.useProgram();
  myShader.setIntUni("albedoMap", 0);
  myShader.setIntUni("normalMap", 1);
  myShader.setIntUni("metallicMap", 2);
  myShader.setIntUni("aoMap", 3);
  myShader.setIntUni("aoMap", 4);
}

int main() {
  initializeGLFWMajorMinor(4, 2);
  GLFWwindow *window = glfwCreateWindow(WINWIDTH, WINHEIGHT,
                                        "Simple PBR With Texture", NULL, NULL);

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

  // layered-cliff-albedo.png
  // layered-cliff-ao.png
  // layered-cliff-height.png
  // layered-cliff-metallic.png
  // layered-cliff-normal-ogl.png
  // layered-cliff-preview.jpg
  // layered-cliff-roughness.png

  std::string albedoPath = "layered-cliff-albedo.png";
  std::string normalPath = "layered-cliff-normal-ogl.png";
  std::string metallicPath = "layered-cliff-metallic.png";
  std::string aoPath = "layered-cliff-ao.png";
  std::string roughPath = "layered-cliff-roughness.png";

  //
  // std::string albedoPath = "rustediron2_basecolor.png";

  // std::string normalPath = "rustediron2_normal.png";

  // std::string metallicPath = "rustediron2_metallic.png";
  // std::string aoPath =
  // std::string roughPath = "rustediron2_roughness.png";

  fs::path diffmapPath = textureDirPath / "layered" / albedoPath;
  fs::path nmapPath = textureDirPath / "layered" / normalPath;
  fs::path mmapPath = textureDirPath / "layered" / metallicPath;
  fs::path aomapPath = textureDirPath / "layered" / aoPath;
  fs::path romapPath = textureDirPath / "layered" / roughPath;

  GLuint albedoMap;
  glGenTextures(1, &albedoMap);
  loadTexture2d(diffmapPath.c_str(), albedoMap);

  GLuint normalMap;
  glGenTextures(1, &normalMap);
  loadTexture2d(nmapPath.c_str(), normalMap);

  GLuint metallicMap;
  glGenTextures(1, &metallicMap);
  loadTexture2d(mmapPath.c_str(), metallicMap);

  GLuint aoMap;
  glGenTextures(1, &aoMap);
  loadTexture2d(aomapPath.c_str(), aoMap);

  GLuint roMap;
  glGenTextures(1, &roMap);
  loadTexture2d(romapPath.c_str(), aoMap);

  // load shaders
  // cube shader
  std::string vertFileName_t = "simplepbr1.vert";
  std::string fragFileName_t = "simplepbr1.frag";

  fs::path vertPath_t = shaderDirPath / vertFileName_t;
  fs::path fragPath_t = shaderDirPath / fragFileName_t;

  Shader cshader(vertPath_t.c_str(), fragPath_t.c_str());

  // lamp shader
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath_t.c_str(), frag2Path.c_str());

  // let's set up some uniforms

  // init proc for uniforms that don't change over rendering
  cubeShaderInit_proc2(cshader);

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

    // lightPos change
    // lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
    // lightPos.y = sin(glfwGetTime() * 2.0f);
    // lightPos.z = sin(glfwGetTime() / 2.0f);

    // render cube object
    glm::mat4 cubeModel(1.0f);
    // float angle = 20.0f;
    // render cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, aoMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, roMap);

    cshader.useProgram();
    cshader.setMat4Uni("view", viewMat);
    cshader.setMat4Uni("model", cubeModel);
    cshader.setMat4Uni("projection", projection);
    cshader.setVec3Uni("lightPos", lightPos);
    cshader.setVec3Uni("viewPos", viewPos);

    renderCube();

    // unbind the light vertex array object
    glm::mat4 lampModel(1.0f);
    lampModel = glm::translate(lampModel, lightPos);
    lampModel = glm::scale(lampModel, glm::vec3(0.2f));
    lampShader.useProgram();
    lampShader.setMat4Uni("model", lampModel);
    lampShader.setMat4Uni("projection", projection);
    lampShader.setMat4Uni("view", viewMat);
    lampShader.setVec3Uni("lightColor", glm::vec3(1.0f));
    // render lamp
    renderLamp();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}

