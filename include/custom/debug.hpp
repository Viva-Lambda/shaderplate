// debug utilities
#ifndef DEBUG_HPP
#define DEBUG_HPP
#include <custom/external.hpp>

const bool DEBUG = true;

// debug function
GLenum gerror(const char *file, int line) {
  // from learnopengl.com
  GLenum errorCode = glGetError();
  if (errorCode != GL_NO_ERROR) {
    std::string error;
    switch (errorCode) {
    case GL_INVALID_ENUM:
      error = "INVALID_ENUM: enumeration parameter is not legal";
      break;
    case GL_INVALID_VALUE:
      error = "INVALID_VALUE: value parameter is not legal";
      break;
    case GL_INVALID_OPERATION:
      error = "INVALID_OPERATION: command not ready for given parameters";
      break;
    case GL_STACK_OVERFLOW:
      error = "STACK_OVERFLOW: stack push causes overflow";
      break;
    case GL_STACK_UNDERFLOW:
      error = "STACK_UNDERFLOW: stack pop at lowest point";
      break;
    case GL_OUT_OF_MEMORY:
      error = "OUT_OF_MEMORY: cannot allocate enough memory for operation";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION: fbo not complete for reading or "
              "writting";
      break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}
#define gerr() gerror(__FILE__, __LINE__)
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
#endif
