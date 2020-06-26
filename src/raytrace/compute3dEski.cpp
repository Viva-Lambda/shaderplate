// simple compute shader code 3d object
// license: see LICENSE
//
#include <glad/glad.h>
//
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// camera
#include <custom/camera.hpp>
#include <custom/shader.hpp>

namespace filesystem = std::filesystem;
filesystem::path current_dir(filesystem::current_path());
filesystem::path mediaDir("media");
filesystem::path textureDir("textures");
filesystem::path shaderDir("shaders");
filesystem::path shaderDirPath = current_dir / mediaDir / shaderDir;
filesystem::path textureDirPath = current_dir / mediaDir / textureDir;

// initialization code
const bool DEBUG = true;
// debug function
void printDebug(const char *mes, float arg) {
  std::cout << mes << " " << arg << std::endl;
}
void printDebug(const char *mes, glm::vec3 arg) {
  std::cout << mes << " x: " << arg.x << "  y: " << arg.y << "  z: " << arg.z
            << std::endl;
}
void printDebug(const char *mes, glm::vec4 arg) {
  std::cout << mes << " x: " << arg.x << "  y: " << arg.y << "  z: " << arg.z
            << "  w:" << arg.w << std::endl;
}
void printDebug(const char *mes, glm::mat3 arg) {
  std::cout << mes << std::endl;
  printDebug("col1: ", arg[0]);
  printDebug("col2: ", arg[1]);
  printDebug("col3: ", arg[2]);
}
void printDebug(const char *mes, glm::mat4 arg) {
  std::cout << mes << std::endl;
  printDebug("col1: ", arg[0]);
  printDebug("col2: ", arg[1]);
  printDebug("col3: ", arg[2]);
  printDebug("col3: ", arg[3]);
}

void printDebug(const char *mes, Ray arg) {
  std::cout << "debug output:" << std::endl;
  std::cout << mes << std::endl;
  printDebug("with origin: ", arg.origin);
  printDebug("with direction: ", arg.direction);
}

void printDebug(const char *mes, Segment arg) {
  std::cout << "debug output:" << std::endl;
  std::cout << mes << std::endl;
  printDebug(" with origin: ", arg.origin);
  printDebug(" with direction: ", arg.direction);
  printDebug(" with size: ", arg.size);
}
glm::vec3 interpCoord(Segment s1, Segment s2, float coord) {
  // interpolate coordinate between two rays
  glm::vec3 segment1 = s1.origin + s1.direction * s1.size;
  glm::vec3 segment2 = s2.origin + s2.direction * s2.size;
  return glm::mix(segment1, segment2, coord);
}
glm::vec3 interpCoordVec(glm::vec3 v1, glm::vec3 v2, float coord) {
  // interpolate coordinate between two rays
  return glm::mix(v1, v2, coord);
}
Ray getPixelSegment(glm::vec2 pixelScreenPos, Segment bottomLeftS,
                    Segment bottomRightS, Segment topLeftS, Segment topRightS,
                    glm::vec3 pos) {
  // get pixel segment

  glm::vec3 pixelBottom =
      interpCoord(bottomLeftS, bottomRightS, pixelScreenPos.y);
  glm::vec3 pixelTop = interpCoord(topLeftS, topRightS, pixelScreenPos.y);
  glm::vec3 pixelVec = glm::mix(pixelBottom, pixelTop, pixelScreenPos.x);
  Ray pray;
  pray.origin = pixelVec;
  pray.direction = normalize(pixelVec);
  return pray;
}
glm::vec3 getPixelVec3(glm::vec2 pixelScreenPos, glm::vec3 bottomLeftS,
                       glm::vec3 bottomRightS, glm::vec3 topLeftS,
                       glm::vec3 topRightS) {
  // get pixel vector
  glm::vec3 pixelBottom =
      interpCoordVec(bottomLeftS, bottomRightS, pixelScreenPos.y);
  glm::vec3 pixelTop = interpCoordVec(topLeftS, topRightS, pixelScreenPos.y);
  return glm::mix(pixelBottom, pixelTop, pixelScreenPos.x);
}

// Viewport values
unsigned int VIEWPORTX = 0;
unsigned int VIEWPORTY = 0;
float VIEWPORTF = 1000.0f;
float VIEWPORTN = 1.0f;
unsigned int WINWIDTH = 1280;
unsigned int WINHEIGHT = 960;

void initializeGLFWMajorMinor(unsigned int maj, unsigned int min) {
  // initialize glfw version with correct profiling etc
  // Major 4, minor 3
  if (glfwInit() == 0) {
    std::cout << "glfw not initialized correctly" << std::endl;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, maj);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, min);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
void framebuffer_size_callback(GLFWwindow *window, int newWidth,
                               int newHeight) {
  WINHEIGHT = newHeight;
  WINWIDTH = newWidth;
  glViewport(VIEWPORTX, VIEWPORTY, newWidth, newHeight);
}
void setSegmentToShader(Shader shader, Segment topLeftS, Segment bottomLeftS,
                        Segment topRightS, Segment bottomRightS) {
  // compute ray then set it to
  if (DEBUG) {
    printDebug("topLeft segment ", topLeftS);
    printDebug("bottomLeft segment ", bottomLeftS);
    printDebug("topRight segment ", topRightS);
    printDebug("bottomRight segment ", bottomRightS);
    glm::vec2 spos(0.5, 0.6);
    Ray r = getPixelSegment(spos, topLeftS, bottomLeftS, topRightS,
                            bottomRightS, glm::vec3(0.0f, 0.0f, 0.3f));
    printDebug("example pixel ray: ", r);
  }
  shader.setVec3Uni("topLeftS.origin", topLeftS.origin);
  shader.setVec3Uni("topLeftS.direction", topLeftS.direction);
  shader.setFloatUni("topLeftS.size", topLeftS.size);

  shader.setVec3Uni("bottomLeftS.origin", bottomLeftS.origin);
  shader.setVec3Uni("bottomLeftS.direction", bottomLeftS.direction);
  shader.setFloatUni("bottomLeftS.size", bottomLeftS.size);

  shader.setVec3Uni("topRightS.origin", topRightS.origin);
  shader.setVec3Uni("topRightS.direction", topRightS.direction);
  shader.setFloatUni("topRightS.size", topRightS.size);

  shader.setVec3Uni("bottomRightS.origin", bottomRightS.origin);
  shader.setVec3Uni("bottomRightS.direction", bottomRightS.direction);
  shader.setFloatUni("bottomRightS.size", bottomRightS.size);
}
void setPosToShader(Shader shader, glm::vec3 topLeftS, glm::vec3 bottomLeftS,
                    glm::vec3 topRightS, glm::vec3 bottomRightS) {
  // compute ray then set it to
  if (DEBUG) {
    printDebug("topLeft vec ", topLeftS);
    printDebug("bottomLeft vec ", bottomLeftS);
    printDebug("topRight vec ", topRightS);
    printDebug("bottomRight vec ", bottomRightS);
    glm::vec2 spos(0.5, 0.6);
    glm::vec3 v =
        getPixelVec3(spos, topLeftS, bottomLeftS, topRightS, bottomRightS);
    printDebug("example pixel ray dir: ", v);
  }
  shader.setVec3Uni("r01", topLeftS);
  shader.setVec3Uni("r00", bottomLeftS);
  shader.setVec3Uni("r11", topRightS);
  shader.setVec3Uni("r10", bottomRightS);
}

int main() {
  // context initialized
  initializeGLFWMajorMinor(4, 3);
  // let's get that window going
  GLFWwindow *window;
  window = glfwCreateWindow(WINWIDTH, WINHEIGHT, "Compute Shader 3d Window",
                            NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed creating window" << std::endl;
    return -1;
  }
  glfwMakeContextCurrent(window);
  // window resize
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // load glad
  if (gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)) == 0) {
    std::cout << "Failed to start glad" << std::endl;
    glfwTerminate();
    return -1;
  }
  glViewport(0, 0, WINWIDTH, WINHEIGHT);
  // glEnable(GL_DEPTH_TEST);
  // Texture coords
  float vertices[] = {
      // viewport position ||   texture coords
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // bottom right
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // top left
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f  // top right
  };
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(vao);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  //
  // http://antongerdelan.net/opengl/compute.html
  //
  // texture handling bit
  int texture_width = WINWIDTH;
  int texture_height = WINHEIGHT;
  GLuint texture_output;
  glGenTextures(1, &texture_output);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_output);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texture_width, texture_height, 0,
               GL_RGBA, GL_FLOAT, NULL);
  glBindImageTexture(0, texture_output, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                     GL_RGBA32F);

  // end texture handling

  // work group handling
  // work group count
  GLint work_group_count[3];
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_count[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_count[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_count[2]);
  std::cout << "total work group count x: " << work_group_count[0] << std::endl;
  std::cout << "total work group count y: " << work_group_count[1] << std::endl;
  std::cout << "total work group count z: " << work_group_count[2] << std::endl;

  // work group size
  GLint work_group_size[3];
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_group_size[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_group_size[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_group_size[2]);
  std::cout << "total work group size x: " << work_group_size[0] << std::endl;
  std::cout << "total work group size y: " << work_group_size[1] << std::endl;
  std::cout << "total work group size z: " << work_group_size[2] << std::endl;
  // global work group size is 512 * 512 == texture width * texture height
  // local work group size is 1 since 1 pixel at a time

  // work group invocation
  GLint work_group_inv;
  glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_group_inv);
  std::cout << "max work group invocation: " << work_group_inv << std::endl;
  // end of work group

  // quad shader
  // source vertex shader
  filesystem::path vpath = shaderDirPath / "compute.vert";
  // source fragment shader
  filesystem::path fpath = shaderDirPath / "compute.frag";
  // declare quad program
  Shader quadShader(vpath.c_str(), fpath.c_str());
  // end quad shader

  // compute shader part
  filesystem::path cpath = shaderDirPath / "compute3d.comp";

  // declare shaders
  Shader rayShader(cpath.c_str());
  // end compute shader

  // deal with camera and related uniforms

  Camera camera(glm::vec3(0.0f, 0.0f, -14.0f));
  glm::mat4 projMat =
      glm::perspective(glm::radians(45.0f), (float)WINWIDTH / (float)WINHEIGHT,
                       VIEWPORTN, VIEWPORTF);
  glm::mat4 viewMat = camera.getViewMatrix();
  // glm::mat4 viewMat =
  //    glm::lookAt(camera.pos, glm::vec3(0.0, 0.0, 1.0), glm::vec3(0, 1, 0));
  glm::vec3 cameraPos = camera.pos;
  // frustum rays
  // 0 0 bottom left -1 -1
  // 1 0 bottom right 1 -1
  // 0 1 top left -1 1
  // 1 1 top right 1 1
  glm::mat4 vpmat = projMat * viewMat;
  // glm::mat4 invVpMat = glm::transpose(glm::inverse(vpmat));
  glm::mat4 invVpMat = glm::inverse(vpmat);
  glm::vec4 topLeftV4 = glm::vec4(-1, 1, 0, 1) * invVpMat;
  glm::vec4 bottomLeftV4 = glm::vec4(-1, -1, 0, 1) * invVpMat;
  glm::vec4 topRightV4 = glm::vec4(1, 1, 0, 1) * invVpMat;
  glm::vec4 bottomRightV4 = glm::vec4(1, -1, 0, 1) * invVpMat;
  if (DEBUG) {
    printDebug("topLeft inverse vpmat: ", topLeftV4);
    printDebug("topLeft screen space: ", glm::vec4(-1, 1, 0, 1));
    printDebug("topRight inverse vpmat: ", topRightV4);
    printDebug("topRight screen space: ", glm::vec4(1, 1, 0, 1));
    printDebug("bottomLeft inverse vpmat: ", bottomLeftV4);
    printDebug("bottomLeft screen space: ", glm::vec4(-1, -1, 0, 1));
    printDebug("bottomRight inverse vpmat: ", bottomRightV4);
    printDebug("bottomRight screen space: ", glm::vec4(1, -1, 0, 1));
  }
  // rays
  // Segment topLeftS = camera.getSegmentToPosV4Perspective(topLeftV4);
  // Segment bottomLeftS = camera.getSegmentToPosV4Perspective(bottomLeftV4);
  // Segment topRightS = camera.getSegmentToPosV4Perspective(topRightV4);
  // Segment bottomRightS = camera.getSegmentToPosV4Perspective(bottomRightV4);

  glm::vec3 topLeftS = camera.getPosToPosV4Perspective(topLeftV4);
  glm::vec3 bottomLeftS = camera.getPosToPosV4Perspective(bottomLeftV4);
  glm::vec3 topRightS = camera.getPosToPosV4Perspective(topRightV4);
  glm::vec3 bottomRightS = camera.getPosToPosV4Perspective(bottomRightV4);

  rayShader.useProgram();

  // setSegmentToShader(rayShader, topLeftS, bottomLeftS, topRightS,
  // bottomRightS);
  setPosToShader(rayShader, topLeftS, bottomLeftS, topRightS, bottomRightS);

  // put some objects
  glm::mat4 modelMat(1.0);
  glm::vec4 scenter(0.01f, 0.01f, -2.2f, 1.0f);
  glm::vec4 modelCenter = modelMat * scenter;
  glm::vec4 vp = vpmat * scenter;
  glm::vec4 viewCenter = viewMat * modelCenter;
  glm::vec4 projCenter = projMat * viewCenter;
  // rayShader.setVec3Uni("s1.center", projCenter.x / projCenter.w,
  //                     projCenter.y / projCenter.w,
  //                     projCenter.z / projCenter.w);
  // rayShader.setVec3Uni("s1.center", projCenter.x, projCenter.y,
  // projCenter.z);
  // rayShader.setVec3Uni("s1.center", viewCenter.x, viewCenter.y,
  // viewCenter.z);
  // rayShader.setVec3Uni("s1.center", vp.x, vp.y, vp.z);

  // rayShader.setVec3Uni("s1.center", modelCenter.x, modelCenter.y,
  //                     modelCenter.z);
  rayShader.setFloatUni("s1.radius", 0.1);
  // rayShader.setIntUni("s1.indx", 1);
  rayShader.setMat4Uni("projMat", projMat);
  rayShader.setMat4Uni("viewMat", viewMat);
  rayShader.setMat4Uni("modelMat", modelMat);
  rayShader.setFloatUni("viewp.x", VIEWPORTX);
  rayShader.setFloatUni("viewp.y", VIEWPORTY);
  rayShader.setFloatUni("viewp.w", WINWIDTH);
  rayShader.setFloatUni("viewp.h", WINHEIGHT);
  rayShader.setVec3Uni("cam.pos", camera.pos);
  rayShader.setFloatUni("cam.nearZ", VIEWPORTN);
  rayShader.setFloatUni("cam.farZ", VIEWPORTF);

  if (DEBUG) {
    printDebug("viewCenter: ", viewCenter);
    printDebug("projCenter: ", projCenter);
    printDebug("modelCenter: ", modelCenter);
    printDebug("vp: ", vp);
    printDebug("camera.pos", camera.pos);
    // a pixel ray
    printDebug("viewMatrix", viewMat);
    printDebug("projMat", projMat);
    printDebug("vpmat", vpmat);
    printDebug("invpmat", invVpMat);
    printDebug("inv * vp saglamasi: ", vpmat * invVpMat);
    printDebug("window height", WINHEIGHT);
    printDebug("window width", WINWIDTH);
  }

  //
  while (glfwWindowShouldClose(window) == 0) {
    // rendering call
    // launch shaders
    rayShader.useProgram();
    glDispatchCompute((GLuint)texture_width, (GLuint)texture_height, 1);
    // end launch shaders

    // writting is finished
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // classic drawing pass
    glClear(GL_COLOR_BUFFER_BIT);
    quadShader.useProgram();
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_output);

    // drawing call
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwPollEvents();
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
    glfwSwapBuffers(window);
  }
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glfwTerminate();
  return 0;
}
