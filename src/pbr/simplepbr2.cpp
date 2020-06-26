/*
   Simple PBR shader
 */
// license: see, LICENSE
#include <custom/utils.hpp>
// time related
float pdeltaTime = 0.0f;
float plastTime = 0.0f;
float sdeltaTime = 0.0f;
float slastTime = 0.0f;


glm::vec3 lightColor = glm::vec3(0.8f, 0.7f, 0.8f);

int ndf = 0;
int geo = 0;
bool fresnel = true;
bool hdr = true;
// function declarations

void activateTextures(GLuint tex1, GLuint tex2, GLuint tex3, GLuint tex4,
                      GLuint tex5);
void printNdfVal(int ndf) {
  switch (ndf) {
  case 0:
    std::cout << "blinn phong normal distribution" << std::endl;
    break;
  case 1:
    std::cout << "beckmann spizzichino normal distribution" << std::endl;
    break;
  case 2:
    std::cout << "Trowbridge Reitz normal distribution" << std::endl;
    break;
  case 3:
    std::cout << "pbr-book Beckmann Spizzichino normal distribution"
              << std::endl;
    break;
  default:
    std::cout << "normal distribution" << std::endl;
  }
}
void printGeoVal(int geo) {
  switch (geo) {
  case 0:
    std::cout << "implicit geometric shadowing" << std::endl;
    break;
  case 1:
    std::cout << "neumann geometric shadowing" << std::endl;
    break;
  case 2:
    std::cout << "cook torrance geometric shadowing" << std::endl;
    break;
  case 3:
    std::cout << "kelemen geometric shadowing" << std::endl;
    break;
  case 4:
    std::cout << "beckmann spizzichino geometric shadowing as in pbr-book"
              << std::endl;
    break;
  case 5:
    std::cout << "geometric shadowing as per smith's equation" << std::endl;
    break;
  case 6:
    std::cout << "schlick approximation of beckmann geometric shadowing as per "
              << "smith's equation" << std::endl;
    break;
  case 7:
    std::cout
        << "schlick approximation of smith ggx geometric shadowing as per "
        << std::endl;
    break;
  case 8:
    std::cout << "Pbr-book version of Beckmann Spizzichino geometric shadowing "
              << std::endl;
    break;
  case 9:
    std::cout << "Pbr-book version of Trowbridge Reitz geometric shadowing "
              << std::endl;
    break;
  default:
    std::cout << "geometric shadowing" << std::endl;
  }
}
void printFresnelVal(bool fre) {
  if (fre == true) {
    std::cout << "schlick fresnel equation" << std::endl;
  } else {
    std::cout << "fresnel cook torrance equation" << std::endl;
  }
}
void printHdrVal(bool hd) {
  if (hd == true) {
    std::cout << "hdr correction" << std::endl;
  } else {
    std::cout << "no hdr correction" << std::endl;
  }
}

void cubeShaderInit_proc2(Shader myShader) {
  myShader.useProgram();
  myShader.setIntUni("albedoMap", 0);
  myShader.setIntUni("normalMap", 1);
  myShader.setIntUni("metallicMap", 2);
  myShader.setIntUni("aoMap", 3);
  myShader.setIntUni("roughnessMap", 4);
  // myShader.setIntUni("ndfChoice", ndf);
  // myShader.setIntUni("geoChoice", geo);
  // myShader.setBoolUni("fresnelChoice", fresnel);
  // myShader.setBoolUni("hdrChoice", hdr);
  myShader.setVec3Uni("lightPos", lightPos);
  myShader.setVec3Uni("lightColor", lightColor);
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
  // glfwSetCursorPosCallback(window, mouse_callback);
  // glfwSetScrollCallback(window, mouse_scroll_callback);

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

  fs::path diffmapPath =
      textureDirPath / "layered" / "layered-cliff-albedo.png";
  fs::path nmapPath =
      textureDirPath / "layered" / "layered-cliff-normal-ogl.png";
  fs::path mmapPath = textureDirPath / "layered" / "layered-cliff-metallic.png";
  fs::path aomapPath = textureDirPath / "layered" / "layered-cliff-ao.png";
  fs::path romapPath =
      textureDirPath / "layered" / "layered-cliff-roughness.png";

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
  loadTexture2d(romapPath.c_str(), roMap);

  // load shaders
  // cube shader
  std::string vertFileName_t = "simplepbr2.vert"; // common
  std::string fragFileName_t = "simplepbr2.frag";
  std::string fragPath = shaderDirPath / fragFileName_t;
  std::string vertPath = shaderDirPath / vertFileName_t;

  Shader cshader(vertPath.c_str(), fragPath.c_str());

  // lamp shader
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath.c_str(), frag2Path.c_str());

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
    // lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
    // lightPos.y = 2.0 + sin(glfwGetTime());
    // lightPos.z = sin(glfwGetTime() / 2.0f) + 2.0f;
    //
    // lightColor.x = sin(glfwGetTime() / 2.0f);
    // lightColor.y = sin(glfwGetTime() * 5.0f);
    // lightColor.z = sin(glfwGetTime() * 2.0f);

    // render cube object
    glm::mat4 cubeModel1(1.0f);
    glm::mat4 cubeModel2(1.0f);
    glm::vec3 cubePos1(0.0f, 0.0f, 1.0f);
    cubeModel1 = glm::translate(cubeModel1, cubePos1);
    glm::vec3 cubePos2(2.5f, 1.0f, -5.0f);
    cubeModel2 = glm::translate(cubeModel2, cubePos2);
    // float angle = 20.0f;
    // render cube
    activateTextures(albedoMap, normalMap, metallicMap, aoMap, roMap);

    cshader.useProgram();
    cshader.setMat4Uni("view", viewMat);
    cshader.setMat4Uni("model", cubeModel1);
    cshader.setMat4Uni("projection", projection);
    cshader.setVec3Uni("lightPos", lightPos);
    cshader.setVec3Uni("viewPos", viewPos);
    cshader.setVec3Uni("lightColor", lightColor);
    // cshader.setBoolUni("fresnelChoice", fresnel);
    // cshader.setBoolUni("hdrChoice", hdr);
    // cshader.setIntUni("ndfChoice", ndf);
    // cshader.setIntUni("geoChoice", geo);

    renderCube();

    // unbind the light vertex array object
    glm::mat4 lampModel(1.0f);
    lampModel = glm::translate(lampModel, lightPos);
    lampModel = glm::scale(lampModel, glm::vec3(0.2f));
    lampShader.useProgram();
    lampShader.setMat4Uni("model", lampModel);
    lampShader.setMat4Uni("projection", projection);
    lampShader.setMat4Uni("view", viewMat);
    lampShader.setVec3Uni("lightColor", lightColor);
    // render lamp
    renderLamp();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}

void activateTextures(GLuint tex1, GLuint tex2, GLuint tex3, GLuint tex4,
                      GLuint tex5) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tex3);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, tex4);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, tex5);
}
