// simple texture viewing code
#include <custom/singulars.hpp>
#include <custom/utils.hpp>
// initialization code

int main() {
  // let's initialize
  initializeGLFWMajorMinor(4, 3);

  // let's get that window going
  GLFWwindow *window;
  window = glfwCreateWindow(WINWIDTH, WINHEIGHT, "Texture Window", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create a glfw window" << std::endl;
    glfwTerminate();
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

  // set default viewport
  glViewport(0, 0, WINWIDTH, WINHEIGHT); // viewport equal to window

  /*
  float textureCoords[] = {
      // texture coords are between 0 - 1
      0.0f, 0.0f, // bottom left
      1.0f, 0.0f, // bottom right
      0.5f, 1.0f // top middle
  };
  */

  // Texture coords
  float vertices[] = {
      // viewport position || colors           ||   texture coords
      1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };

  // let us load shaders
  fs::path fragFileName("texture.frag");
  fs::path fragPath = shaderDirPath / fragFileName;
  fs::path vertFileName("texture.vert");
  fs::path vertPath = shaderDirPath / vertFileName;
  Shader myShader(vertPath.c_str(), fragPath.c_str());

  // indices
  GLuint indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  // VAO, VBO, EBO related
  GLuint vao, vbo, ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao); // bind the array

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // let's add data to buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // let's add element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // let's specify the attributes

  // position attribute
  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE,
      8 * sizeof(float), // stride value =: when does the next value comes
      // relating to this attribute
      (void *)0);
  glEnableVertexAttribArray(0);

  // color attribute
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
      (void *)(3 * sizeof(float)) // where does the first value related
      // to this attribute in the vertices array start
      );
  glEnableVertexAttribArray(1);

  // texture coord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // ------ Load and Generate Texture ----------
  fs::path imname("Stone_001_Diffuse.png");
  fs::path impath = textureDirPath / imname;

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  texture = loadTexture2d(impath.c_str());

  // -------- main loop -----------
  while (glfwWindowShouldClose(window) == 0) {
    // process inputs
    processInput_proc(window);

    // render stuff
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // use shader program
    myShader.useProgram();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // unbind array
    glBindVertexArray(0);

    // swap buffers etc
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);

  glfwTerminate();
  return 0;
}
