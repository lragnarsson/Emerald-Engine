#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>
#include <regex>

#include "Error.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "Camera.hpp"

/*
  Loader file format specification

  See scene_file.txt

*/

class Loader{
public:
    static void load_scene(std::string filepath, Camera* camera);

private:
    static std::vector<std::string> split_string(std::string input, char separator);
    static Light* load_light(std::vector<std::string> light_line);
    static void load_model(std::ifstream* read_file, int* current_line, std::vector<std::string>& model_line, bool flat);
    static void load_animation(std::vector<std::string> animation_line);
    static void init_camera_anim_path(std::vector<std::string> camera_line, Camera* camera);
};

#endif
