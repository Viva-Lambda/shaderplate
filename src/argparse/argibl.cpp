// license: see, LICENSE
#include <argparse/argparse.hpp>
#include <custom/utils.hpp>
#include <stdexcept>

unsigned int flags1 = aiProcessPreset_TargetRealtime_MaxQuality;
unsigned int flags2 = aiProcessPreset_TargetRealtime_Quality;
unsigned int flags3 = aiProcessPreset_TargetRealtime_Fast;

namespace fs = std::filesystem;

unsigned int min_obj_nb = 2;
unsigned int max_obj_nb = 12;
std::vector<std::string> drawables{
    "MODEL", "SPHERE", "CUBE", "PYRAMID"
};

int main(int argc, char *argv[]) {
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window =
      glfwCreateWindow(WINWIDTH, WINHEIGHT, winTitle, NULL, NULL);

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
  //
  // start shader

  fs::path vertPath_t = shaderDirPath / shaderVertPath;
  fs::path fragPath_t = shaderDirPath / shaderFragPath;

  Shader mShader(vertPath_t.c_str(), fragPath_t.c_str());

  // lamp shader
  fs::path frag2FileName("basic_color_light.frag");
  fs::path frag2Path = shaderDirPath / frag2FileName;
  Shader lampShader(vertPath_t.c_str(), frag2Path.c_str());

  // backpack model
  fs::path modPath = modelPath / mPath;
  stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, false);
  shaderInit(mShader);

  Model modelLight(modelPath / "sphere/scene.gltf", flags, false);

  //
  showKeys();

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
    mShader.useProgram();
    mShader.setMat4Uni("view", viewMat);
    mShader.setMat4Uni("model", modelMat);
    mShader.setMat4Uni("projection", projection);
    mShader.setVec3Uni("viewPos", viewPos);
    mShader.setVec3Uni("lightPos", lightPos);

    // draw model
    modelB.Draw(mShader);

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
    modelLight.Draw(lampShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
}
