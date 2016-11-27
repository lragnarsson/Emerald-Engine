#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>

#include "Error.hpp"

const std::string SCENE_DIRECTORY = "res/scenes/";

enum display_mode {
    WINDOWED,
    FULLSCREEN
};

class Parser {
public:
    static std::string get_scene_file_from_command_line(int argc, char *argv[]);
    static display_mode get_display_mode_from_command_line(int argc, char *argv[]);
private:

};

#endif
