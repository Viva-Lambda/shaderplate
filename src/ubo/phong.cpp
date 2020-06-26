// license: see, LICENSE
#include <glad/glad.h>
//
#include <GLFW/glfw3.h>

#include <custom/camera.hpp>
#include <custom/shader.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <custom/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fs = std::filesystem;

fs::path current_dir = fs::current_path();
fs::path shaderDirPath = current_dir / "media" / "shaders";
fs::path textureDirPath = current_dir / "media" / "textures";

// initialization code

const unsigned int WINWIDTH = 800;
const unsigned int WINHEIGHT = 600;

// camera related

float lastX = WINWIDTH / 2.0f;
float lastY = WINHEIGHT / 2.0f;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 0.3f));

// time related
float deltaTime = 0.0f;
float lastTime = 0.0f;

bool inTangent = false;

glm::vec3 lightPos = glm::vec3(0.2f, 1.0f, 0.5f);
// function declarations

static void glfwErrorCallBack(int id, const char *desc);
void initializeGLFWMajorMinor(unsigned int maj, unsigned int min);
void framebuffer_size_callback(GLFWwindow *window, int newWidth, int newHeight);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow *window, double xpos, double ypos);
GLuint loadTexture2d_proc(const char *texturePath, GLuint tex);
void processInput_proc(GLFWwindow *window);
void cubeShaderInit_proc(Shader myShader);
void renderCube();
void renderCubeInTangentSpace();
void renderLamp();
void renderTriangleInTangentSpace(float vert[15], float normal[3]);
glm::vec3 getTangent(glm::vec2 deltaUV2, glm::vec2 deltaUV1, glm::vec3 edge1,
                     glm::vec3 edge2);
glm::vec3 getBiTangent(glm::vec2 deltaUV2, glm::vec2 deltaUV1, glm::vec3 edge1,
                       glm::vec3 edge2);

int main() {
  initializeGLFWMajorMinor(4, 3);
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
  loadTexture2d_proc(diffmapPath.c_str(), diffuseMap);
  GLuint specularMap;
  glGenTextures(1, &specularMap);
  loadTexture2d_proc(specularMapPath.c_str(), specularMap);
  GLuint normalMap;
  glGenTextures(1, &normalMap);
  loadTexture2d_proc(normalMapPath.c_str(), normalMap);

  // load shaders
  // cube shader
  std::string vertFileName_t = "phongUbo.vert";
  std::string fragFileName_t = "phongUbo.frag";

  fs::path vertPath_t = shaderDirPath / vertFileName_t;
  fs::path fragPath_t = shaderDirPath / fragFileName_t;

  Shader tangentCubeShader(vertPath_t.c_str(), fragPath_t.c_str());

  // lamp shader
  fs::path frag2FileName("lightUbo.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath_t.c_str(), frag2Path.c_str());

  // let's set up some uniforms
  // mats block
  GLuint matblock;
  glGenBuffers(1, &matblock);
  glBindBuffer(GL_UNIFORM_BUFFER, matblock);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0); // unbind
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, matblock);

  // lights block
  GLuint lightsB;
  glGenBuffers(1, &lightsB);
  glBindBuffer(GL_UNIFORM_BUFFER, lightsB);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(float) + sizeof(glm::vec4), NULL,
               GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0); // unbind
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightsB);

  // init proc for uniforms that don't change over rendering
  cubeShaderInit_proc(tangentCubeShader);

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
    glBindBuffer(GL_UNIFORM_BUFFER, matblock);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                    glm::value_ptr(viewMat));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                    glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // ----------- lights block ----------------
    glm::vec3 lightColor(lightIntensity);
    lightColor.z = sin(glfwGetTime() * 1.3f);
    float coeff = sin(glfwGetTime() * 1.1);

    glBindBuffer(GL_UNIFORM_BUFFER, lightsB);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4),
                    glm::value_ptr(lightColor));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(float),
                    &coeff);
    glBindBuffer(GL_UNIFORM_BUFFER, lightsB);

    tangentCubeShader.useProgram();
    // tangentCubeShader.setMat4Uni("view", viewMat);
    tangentCubeShader.setMat4Uni("model", cubeModel);
    // tangentCubeShader.setMat4Uni("projection", projection);
    tangentCubeShader.setVec3Uni("viewPos", viewPos);
    tangentCubeShader.setVec3Uni("lightPos", lightPos);
    // tangentCubeShader.setFloatUni("lightIntensity", lightIntensity);
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
    // lampShader.setMat4Uni("projection", projection);
    // lampShader.setMat4Uni("view", viewMat);
    // lampShader.setVec3Uni("lightColor", );
    // render lamp
    renderLamp();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
void framebuffer_size_callback(GLFWwindow *window, int newWidth,
                               int newHeight) {
  glViewport(0, 0, newWidth, newHeight);
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xoffset;
  lastY = yoffset;

  camera.processMouseMovement(xoffset, yoffset);
}
void mouse_scroll_callback(GLFWwindow *window, double xpos, double ypos) {
  camera.processMouseScroll(ypos);
}
void processInput_proc(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.processKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.processKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.processKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.processKeyboard(RIGHT, deltaTime);
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
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
    lightPos.y += 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    lightPos.y -= 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    lightPos.x += 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    lightPos.x -= 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    lightPos.z -= 0.05f; // the axis are inverse
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    lightPos.z += 0.05f;
  }
}

static void glfwErrorCallBack(int id, const char *desc) {
  std::cout << desc << std::endl;
}
void initializeGLFWMajorMinor(unsigned int maj, unsigned int min) {
  // initialize glfw version with correct profiling etc
  glfwSetErrorCallback(glfwErrorCallBack);
  if (glfwInit() == 0) {
    std::cout << "glfw not initialized correctly" << std::endl;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, maj);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, min);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
GLuint loadTexture2d_proc(const char *texturePath, GLuint tex) {
  // create and load, bind texture to gl

  int width, height, nbChannels;
  unsigned char *data = stbi_load(texturePath, &width, &height, &nbChannels, 0);
  if (data) {
    GLenum format;
    if (nbChannels == 1) {
      format = GL_RED;
    } else if (nbChannels == 3) {
      format = GL_RGB;
    } else if (nbChannels == 4) {
      format = GL_RGBA;
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // use nearest neighbor interpolation when zooming out
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // use nearest neighbor interpolation when zooming out
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  return tex;
}
glm::vec3 getTangent(glm::vec2 deltaUV2, glm::vec2 deltaUV1, glm::vec3 edge1,
                     glm::vec3 edge2) {
  // Compute tangent
  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  glm::vec3 tangent;
  tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent = glm::normalize(tangent);
  return tangent;
}
glm::vec3 getBiTangent(glm::vec2 deltaUV2, glm::vec2 deltaUV1, glm::vec3 edge1,
                       glm::vec3 edge2) {
  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  glm::vec3 bitangent;
  bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent = glm::normalize(bitangent);
  return bitangent;
}
void cubeShaderInit_proc(Shader myShader) {
  myShader.useProgram();
  float ambientCoeff = 0.1f;
  float shininess = 200.0f;
  glm::vec3 attc(1.0f, 0.0f, 0.0f);
  myShader.setFloatUni("ambientCoeff", ambientCoeff);
  myShader.setFloatUni("shininess", shininess);
  myShader.setVec3Uni("attC", attc);
  myShader.setIntUni("diffuseMap", 0);
  myShader.setIntUni("specularMap", 1);
  myShader.setIntUni("normalMap", 2);
}
void renderTriangleInTangentSpace(float vert[15], float normal[3]) {
  GLuint triVBO, triVAO;
  glGenBuffers(1, &triVBO);
  glGenVertexArrays(1, &triVAO);

  // triangle points
  glm::vec3 p1(vert[0], vert[1], vert[2]);
  glm::vec2 tex1(vert[3], vert[4]);
  glm::vec3 p2(vert[5], vert[6], vert[7]);
  glm::vec2 tex2(vert[8], vert[9]);
  glm::vec3 p3(vert[10], vert[11], vert[12]);
  glm::vec2 tex3(vert[13], vert[14]);

  // there are two triangles in a square
  glm::vec3 snormal(normal[0], normal[1], normal[2]);

  // first triangle
  glm::vec3 edge1 = p2 - p1;
  glm::vec3 edge2 = p3 - p1;
  glm::vec2 deltaUV1 = tex2 - tex1;
  glm::vec2 deltaUV2 = tex3 - tex1;

  glm::vec3 tan = getTangent(deltaUV2, deltaUV1, edge1, edge2);
  glm::vec3 bitan = getBiTangent(deltaUV2, deltaUV1, edge1, edge2);

  float trivert[] = {
      p1.x,   p1.y,  p1.z,  snormal.x, snormal.y, snormal.z, tex1.x,
      tex1.y, tan.x, tan.y, tan.z,     bitan.x,   bitan.y,   bitan.z,
      p2.x,   p2.y,  p2.z,  snormal.x, snormal.y, snormal.z, tex2.x,
      tex2.y, tan.x, tan.y, tan.z,     bitan.x,   bitan.y,   bitan.z,
      p3.x,   p3.y,  p3.z,  snormal.x, snormal.y, snormal.z, tex3.x,
      tex3.y, tan.x, tan.y, tan.z,     bitan.x,   bitan.y,   bitan.z,
  };
  glBindVertexArray(triVAO);
  glBindBuffer(GL_ARRAY_BUFFER, triVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(trivert), &trivert, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); // location
  // specify attributes
  GLsizei fsize = 14 * sizeof(float);
  glVertexAttribPointer(0, // location ==  aPos
                        3, // vec3
                        GL_FLOAT, GL_FALSE, fsize, (void *)0);
  glEnableVertexAttribArray(1); // location
  glVertexAttribPointer(1,      // location ==  aNormal
                        3,      // vec3
                        GL_FLOAT, GL_FALSE, fsize, (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2); // location
  glVertexAttribPointer(2,      // location ==  aTexCoord
                        2,      // vec2
                        GL_FLOAT, GL_FALSE, fsize, (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(3); // location
  glVertexAttribPointer(3,      // location ==  aTan
                        3,      // vec3
                        GL_FLOAT, GL_FALSE, fsize, (void *)(8 * sizeof(float)));
  glEnableVertexAttribArray(4); // location
  glVertexAttribPointer(4,      // location ==  aBiTan
                        3,      // vec3
                        GL_FLOAT, GL_FALSE, fsize,
                        (void *)(11 * sizeof(float)));
  glBindVertexArray(triVAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &triVAO);
  glDeleteBuffers(1, &triVBO);
}
void renderTriangle(float vert[15], float normal[3]) {
  GLuint triVBO, triVAO;
  glGenBuffers(1, &triVBO);
  glGenVertexArrays(1, &triVAO);

  // there are two triangles in a square
  float trivert[] = {
      vert[0],   vert[1],   vert[2],   normal[0], normal[1], normal[2],
      vert[3],   vert[4],   vert[5],   vert[6],   vert[7],   normal[0],
      normal[1], normal[2], vert[8],   vert[9],   vert[10],  vert[11],
      vert[12],  normal[0], normal[1], normal[2], vert[13],  vert[14],
  };
  glBindVertexArray(triVAO);
  glBindBuffer(GL_ARRAY_BUFFER, triVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(trivert), &trivert, GL_STATIC_DRAW);
  // specify attributes
  GLsizei fsize = 8 * sizeof(float);
  glVertexAttribPointer(0, // location ==  aPos
                        3, // vec3
                        GL_FLOAT, GL_FALSE, fsize, (void *)0);
  glEnableVertexAttribArray(0); // location
  glVertexAttribPointer(1,      // location ==  aNormal
                        3,      // vec3
                        GL_FLOAT, GL_FALSE, fsize, (void *)3);
  glEnableVertexAttribArray(1); // location
  glVertexAttribPointer(2,      // location ==  aTexCoord
                        2,      // vec2
                        GL_FLOAT, GL_FALSE, fsize, (void *)6);
  glEnableVertexAttribArray(2); // location
  glBindVertexArray(triVAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &triVAO);
  glDeleteBuffers(1, &triVBO);
}
void renderLamp() {
  GLuint vbo, lightVao;
  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &lightVao); // separate object to isolate lamp from
  float vert[] = {-0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f};

  glBindVertexArray(lightVao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vert), &vert, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(lightVao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &lightVao);
  glDeleteBuffers(1, &vbo);
}
void renderCubeInTangentSpace() {
  /*
     Draw cube
   */
  float s1n[] = {0.0f, 0.0f, -1.0f};
  float s2n[] = {0.0f, 0.0f, 1.0f};
  float s3n[] = {-1.0f, 0.0f, 0.0f};
  float s4n[] = {1.0f, 0.0f, 0.0f};
  float s5n[] = {0.0f, -1.0f, 0.0f};
  float s6n[] = {0.0f, 1.0f, 0.0f};

  // positions        // texture coords
  float t1[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  0.5f, -0.5f, -0.5f,
      1.0f,  0.0f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
  };
  renderTriangleInTangentSpace(t1, s1n);
  float tt1[] = {
      0.5f, 0.5f, -0.5f, 1.0f,  1.0f,  -0.5f, 0.5f, -0.5f,
      0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,
  };
  renderTriangleInTangentSpace(tt1, s1n);

  float t2[] = {
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f,
      1.0f,  0.0f,  0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
  };

  renderTriangleInTangentSpace(t2, s2n);
  float tt2[] = {
      0.5f, 0.5f, 0.5f,  1.0f,  1.0f, -0.5f, 0.5f, 0.5f,
      0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f,  0.0f,
  };

  renderTriangleInTangentSpace(tt2, s2n);

  float t3[] = {
      -0.5f, 0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, 0.5f, -0.5f,
      1.0f,  1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,
  };
  renderTriangleInTangentSpace(t3, s3n);

  float tt3[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,
      0.0f,  0.0f,  -0.5f, 0.5f, 0.5f, 1.0f,  0.0f,
  };
  renderTriangleInTangentSpace(tt3, s3n);

  float t4[] = {
      0.5f, 0.5f, 0.5f, 1.0f,  0.0f,  0.5f, 0.5f, -0.5f,
      1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  };

  renderTriangleInTangentSpace(t4, s4n);
  float tt4[] = {
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f,
      0.0f, 0.0f,  0.5f,  0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangleInTangentSpace(tt4, s4n);

  float t5[] = {
      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.5f, -0.5f, -0.5f,
      1.0f,  1.0f,  0.5f,  -0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangleInTangentSpace(t5, s5n);

  float tt5[] = {
      0.5f, -0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
      0.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,
  };

  renderTriangleInTangentSpace(tt5, s5n);

  float t6[] = {
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f,
      1.0f,  1.0f, 0.5f,  0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangleInTangentSpace(t6, s6n);

  float tt6[] = {0.5f, 0.5f, 0.5f,  1.0f, 0.0f,  -0.5f, 0.5f, 0.5f,
                 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f,  1.0f};

  renderTriangleInTangentSpace(tt6, s6n);
}
void renderCube() {
  /*
     Draw cube
   */
  float s1n[] = {0.0f, 0.0f, -1.0f};
  float s2n[] = {0.0f, 0.0f, 1.0f};
  float s3n[] = {-1.0f, 0.0f, 0.0f};
  float s4n[] = {1.0f, 0.0f, 0.0f};
  float s5n[] = {0.0f, -1.0f, 0.0f};
  float s6n[] = {0.0f, 1.0f, 0.0f};

  // positions        // texture coords
  float t1[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  0.5f, -0.5f, -0.5f,
      1.0f,  0.0f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
  };
  renderTriangle(t1, s1n);
  float tt1[] = {
      0.5f, 0.5f, -0.5f, 1.0f,  1.0f,  -0.5f, 0.5f, -0.5f,
      0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,
  };
  renderTriangle(tt1, s1n);

  float t2[] = {
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f,
      1.0f,  0.0f,  0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
  };

  renderTriangle(t2, s2n);
  float tt2[] = {
      0.5f, 0.5f, 0.5f,  1.0f,  1.0f, -0.5f, 0.5f, 0.5f,
      0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f,  0.0f,
  };

  renderTriangle(tt2, s2n);

  float t3[] = {
      -0.5f, 0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, 0.5f, -0.5f,
      1.0f,  1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,
  };
  renderTriangle(t3, s3n);

  float tt3[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,
      0.0f,  0.0f,  -0.5f, 0.5f, 0.5f, 1.0f,  0.0f,
  };
  renderTriangle(tt3, s3n);

  float t4[] = {
      0.5f, 0.5f, 0.5f, 1.0f,  0.0f,  0.5f, 0.5f, -0.5f,
      1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  };

  renderTriangle(t4, s4n);
  float tt4[] = {
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f,
      0.0f, 0.0f,  0.5f,  0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangle(tt4, s4n);

  float t5[] = {
      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.5f, -0.5f, -0.5f,
      1.0f,  1.0f,  0.5f,  -0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangle(t5, s5n);

  float tt5[] = {
      0.5f, -0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
      0.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,
  };

  renderTriangle(tt5, s5n);

  float t6[] = {
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f,
      1.0f,  1.0f, 0.5f,  0.5f, 0.5f, 1.0f, 0.0f,
  };

  renderTriangle(t6, s6n);

  float tt6[] = {0.5f, 0.5f, 0.5f,  1.0f, 0.0f,  -0.5f, 0.5f, 0.5f,
                 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f,  1.0f};

  renderTriangle(tt6, s6n);
}
