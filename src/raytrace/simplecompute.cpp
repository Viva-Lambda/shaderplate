// simple compute shader code
// license: see LICENSE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace filesystem = std::filesystem;
filesystem::path current_dir(filesystem::current_path());
filesystem::path mediaDir("media");
filesystem::path textureDir("textures");
filesystem::path shaderDir("shaders");
filesystem::path shaderDirPath = current_dir / mediaDir / shaderDir;
filesystem::path textureDirPath = current_dir / mediaDir / textureDir;

// initialization code

const unsigned int WINWIDTH = 1280;
const unsigned int WINHEIGHT = 960;

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
  glViewport(0, 0, newWidth, newHeight);
}
std::string loadStringFromPath(const char *stringPath) {
  std::ifstream shdrFileStream;
  shdrFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::string shaderCodeStr;
  try {
    shdrFileStream.open(stringPath);
    std::stringstream shaderSStream;
    shaderSStream << shdrFileStream.rdbuf();
    shdrFileStream.close();
    shaderCodeStr = shaderSStream.str();
  } catch (std::ifstream::failure e) {
    //
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }
  return shaderCodeStr;
}
void checkShaderCompilation(GLuint shader, const char *shaderType) {
  // check the shader compilation
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == 0) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
}
void checkShaderProgramCompilation(GLuint program) {
  // check the shader compilation
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::"
              << "PROGRAM"
              << "::LINK_FAILED\n"
              << infoLog << std::endl;
  }
}

int main() {
  // context initialized
  initializeGLFWMajorMinor(4, 3);
  // let's get that window going
  GLFWwindow *window;
  window = glfwCreateWindow(WINWIDTH, WINHEIGHT, "Compute Shader Window", NULL,
                            NULL);
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
  int texture_width = 512;
  int texture_height = 512;
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

  // compute shader part
  filesystem::path compute_shader_path = shaderDirPath / "compute.comp";

  // declare shaders
  GLuint rayShader = glCreateShader(GL_COMPUTE_SHADER);
  std::string shaderSourceString =
      loadStringFromPath(compute_shader_path.c_str());
  const char *shaderSource = shaderSourceString.c_str();
  glShaderSource(rayShader, 1, &shaderSource, NULL);
  glCompileShader(rayShader);
  checkShaderCompilation(rayShader, "COMPUTE");

  GLuint rayProgram = glCreateProgram();
  glAttachShader(rayProgram, rayShader);
  glLinkProgram(rayProgram);
  checkShaderProgramCompilation(rayProgram);

  // quad shader
  // source vertex shader
  filesystem::path vertex_shader_path = shaderDirPath / "compute.vert";
  GLuint quadVshader = glCreateShader(GL_VERTEX_SHADER);
  std::string vertex_shader_source_str =
      loadStringFromPath(vertex_shader_path.c_str());
  const char *vertex_shader = vertex_shader_source_str.c_str();
  glShaderSource(quadVshader, 1, &vertex_shader, NULL);
  glCompileShader(quadVshader);
  checkShaderCompilation(quadVshader, "VERTEX");
  // source fragment shader
  filesystem::path fragment_shader_path = shaderDirPath / "compute.frag";
  GLuint quadFshader = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fragment_shader_source_str =
      loadStringFromPath(fragment_shader_path.c_str());

  const char *frag_shader = fragment_shader_source_str.c_str();
  glShaderSource(quadFshader, 1, &frag_shader, NULL);
  glCompileShader(quadFshader);
  checkShaderCompilation(quadFshader, "FRAGMENT");
  // declare quad program
  GLuint quadProgram = glCreateProgram();
  glAttachShader(quadProgram, quadVshader);
  glAttachShader(quadProgram, quadFshader);
  glLinkProgram(quadProgram);
  checkShaderProgramCompilation(quadProgram);
  // glUseProgram(quadProgram);
  // int uniLocation = glGetUniformLocation(quadProgram, "myTexture");
  // glUniform1i(uniLocation, texture_output);

  while (glfwWindowShouldClose(window) == 0) {
    // rendering call
    // launch shaders
    glUseProgram(rayProgram);
    glDispatchCompute((GLuint)texture_width, (GLuint)texture_height, 1);
    // end launch shaders

    // writting is finished
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // classic drawing pass
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(quadProgram);
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
