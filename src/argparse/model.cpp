// license: see, LICENSE
#include <argparse/argparse.hpp>
#include <custom/mutils.hpp>
#include <stdexcept>

void cubeShaderInit_proc2(Shader myShader) {
  myShader.useProgram();
  float ambientCoeff = 0.1f;
  float shininess = 20.0f;
  glm::vec3 attc(1.0f, 0.0f, 0.0f);
  myShader.setFloatUni("ambientCoeff", ambientCoeff);
  myShader.setFloatUni("shininess", shininess);
  myShader.setFloatUni("lightIntensity", lightIntensity);
  myShader.setVec3Uni("attC", attc);
}

unsigned int flags1 = aiProcessPreset_TargetRealtime_MaxQuality;
unsigned int flags2 = aiProcessPreset_TargetRealtime_Quality;
unsigned int flags3 = aiProcessPreset_TargetRealtime_Fast;

int main(int argc, char *argv[]) {

  argparse::ArgumentParser program("Model Loader");

  program.add_argument("window-title")
      .help("Title of the Window to be displayed with GLFW");
  program.add_argument("vertex-shader-path")
      .help("Path to vertex shader of model");
  program.add_argument("fragment-shader-path")
      .help("Path to fragment shader of model");
  program.add_argument("model-path").help("Path to model");
  program.add_argument("flag-choice").help("Choose your flag {0,1,2}");

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    //
    std::cout << "Parsing Failed" << std::endl;
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }

  std::string window_title = program.get<std::string>("window-title");
  std::cout << window_title << std::endl;
  std::string vertex_path = program.get<std::string>("vertex-shader-path");
  std::cout << vertex_path << std::endl;
  std::string fragment_path = program.get<std::string>("fragment-shader-path");
  std::cout << fragment_path << std::endl;

  auto model_path = program.get<std::string>("model-path");
  std::cout << model_path << std::endl;

  std::string flag_choice = program.get<std::string>("flag-choice");
  std::cout << flag_choice << std::endl;

  // return launch("Model Phong Shader", "highPolyModel.vert",
  //              "highPolyModelSimple.frag", "ebru/kedi.x3d", flags1,
  //              cubeShaderInit_proc2);
  unsigned int flag;
  int user_flag = std::stoi(flag_choice);
  if (user_flag == 0) {
    flag = flags1;
  } else if (user_flag == 1) {
    flag = flags2;
  } else if (user_flag == 2) {
    flag = flags3;
  } else {
    throw std::invalid_argument("flag choice should be either 0,1,2");
  }
  return launch(window_title.c_str(), vertex_path.c_str(),
                fragment_path.c_str(), model_path.c_str(), flag,
                cubeShaderInit_proc2);
}
