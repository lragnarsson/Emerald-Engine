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

/*
  Loader file format specification

  See scene_file.txt

*/

class Loader{
public:
    static void load_scene(std::string filepath);

private:
    static std::vector<std::string> split_string(std::string input, char separator);
    static Light* load_light(std::vector<std::string> light_line);
    static void load_model(std::ifstream* read_file, int* current_line, std::vector<std::string>& model_line, bool flat);
    static void load_animation(std::vector<std::string> animation_line);
    static void replace_in_string(std::string& str, const std::string& from, const std::string& to);
};

#endif
