// screen space ray tracing implementation effort
#include <custom/utils.hpp>

float deltaTime2 = 0.05;
void moveCamera2(GLFWwindow *window) {
  //
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.processKeyboard(FORWARD, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.processKeyboard(LEFT, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.processKeyboard(BACKWARD, deltaTime2);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.processKeyboard(RIGHT, deltaTime2);
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
}
void moveLight2(GLFWwindow *window) {
  // move light
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
    spotLight.position.y += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    spotLight.position.y -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    spotLight.position.x += deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    spotLight.position.x -= deltaTime2;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    spotLight.position.z -= deltaTime2; // the axis are inverse
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    spotLight.position.z += deltaTime2;
  }
}

void rotateLight2(GLFWwindow *window) {
  // move light
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_LEFT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_RIGHT, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_FORWARD, 0.7f);
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    spotLight.processKeyBoardRotate(L_BACKWARD, 0.7f);
  }
}
void processInput_proc2(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  moveCamera2(window);
  moveLight2(window);
  rotateLight2(window);
  captureScreen(window);
}

// ----------------------- load models ---------------------------------
unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

Model loadBackpackModel() {
  fs::path modPath = modelPath / "backpack" / "backpack.obj";
  // stbi_set_flip_vertically_on_load(true);
  Model modelB(modPath, flags, true, false, false);
  return modelB;
}

// ---------------------- load textures --------------------------------

Shader loadGeometryShader() {
  fs::path vpath = shaderDirPath / "ssao" / "geometry.vert";
  fs::path fpath = shaderDirPath / "ssao" / "geometry.frag";
  Shader geometryShader(vpath.c_str(), fpath.c_str());
  return geometryShader;
}
Shader loadLightShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "light.frag";
  Shader lightShader(vpath.c_str(), fpath.c_str());
  return lightShader;
}
Shader loadSsaoShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "ssao.frag";
  Shader ssaoShader(vpath.c_str(), fpath.c_str());
  return ssaoShader;
};
Shader loadSsaoBlurShader() {
  fs::path vpath = shaderDirPath / "ssao" / "light.vert";
  fs::path fpath = shaderDirPath / "ssao" / "ssaoblur.frag";
  Shader blurShader(vpath.c_str(), fpath.c_str());
  return blurShader;
}

void setGBufferTexture(GLuint &tex, GLenum informat, GLenum bufferType,
                       unsigned int attachmentNb) {
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, informat, WINWIDTH, WINHEIGHT, 0, GL_RGBA,
               bufferType, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNb,
                         GL_TEXTURE_2D, tex, 0);
}
void loadEnvironmentCubemap(GLuint &envCubemap, GLenum informat,
                            unsigned int captureWidth,
                            unsigned int captureHeight,
                            unsigned int attachmentNb) {
  glGenTextures(1, &envCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, informat, captureWidth,
                 captureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNb,
                         GL_TEXTURE_2D, tex, 0);
}

void setGBufferDepthRbo(GLuint &rbo) {
  //
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINWIDTH,
                        WINHEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
}

float lerp(float n, float n2, float f) { return n + f * (n2 - n); }

int main() {
  initializeGLFWMajorMinor(4, 3);
  GLFWwindow *window = glfwCreateWindow(
      WINWIDTH, WINHEIGHT, "Lightprobs Shading Example", NULL, NULL);

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

  // define opengl states
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL); // skyboxing
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // set up required buffers
  GLuint geometry_fbo, depthRbo; // required for g buffer pass
  glGenFramebuffers(1, &geometry_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, geometry_fbo);
  setGBufferDepthRbo(depthRbo);

  // stores normal of value of vertex in view space
  GLuint normalBuffer = 0;
  setGBufferTexture(normalBuffer, // buffer id
                    GL_RGBA16F,   // buffer stored value precision
                    GL_FLOAT,     // buffer stored value type
                    0             // attachment index
                    );

  // stores material parameters of the vertex
  // such as roughness, reflectance value at zero incidence, maybe pdf value
  // etc
  GLuint materialBuffer = 0;
  setGBufferTexture(materialBuffer, // buffer id
                    GL_RGBA16F,     // buffer stored value precision
                    GL_FLOAT,       // buffer stored value type
                    1               // attachment index
                    );

  // stores color values for the given vertex such as diffuse/albedo values
  GLuint albedoBuffer = 0;
  setGBufferTexture(albedoBuffer,     // buffer id
                    GL_RGBA,          // buffer stored value precision
                    GL_UNSIGNED_BYTE, // buffer stored value type
                    1                 // attachment index
                    );

  // stores depth values for given vertex, ie the distance between the camera
  // and the vertex
  GLuint linearDepthTexture = 0;
  setGBufferTexture(linearDepthTexture, // buffer id
                    GL_RGBA16F,         // buffer stored value precision
                    GL_FLOAT,           // buffer stored value type
                    1                   // attachment index
                    );

  // a fall back texture with precomputed brdf and environment cubemap
  // the values inside this cubemap is captured using a captureFBO
  // then rendered with a shader
  // the values inside this texture is provided using the following pipeline

  // float lod             = getMipLevelFromRoughness(roughness);
  // vec3 prefilteredColor = textureCubeLod(PrefilteredEnvMap, refVec, lod);
  // vec2 envBRDF = texture2D(BRDFIntegrationMap, vec2(NdotV, roughness)).xy;
  // vec3 indirectSpecular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

  GLuint envCubemapFallbackTexture = 0;

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Geometry Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // ----------------- needed shaders are ------------------------------
  // geometry shader

  // lightening shader

  // screen space tracing shader

  // screen blur shader

  // cone tracing shader

  // scene description:
  // rusted metal sphere on a mirror like platform
  GLuint metallicMap = 0;
  metallicMap = loadTexture2d("rusted", "rustediron2_metallic.png");

  GLuint baseColorMap = 0;
  baseColorMap = loadTexture2d("rusted", "rustediron2_basecolor.png");

  GLuint normalMap = 0;
  normalMap = loadTexture2d("rusted", "rustediron2_normal.png");

  GLuint roughnessMap = 0;
  roughnessMap = loadTexture2d("rusted", "rustediron2_roughness.png");

  // with a sphere controllable spotlight
}
