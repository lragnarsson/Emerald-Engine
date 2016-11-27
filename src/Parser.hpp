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

    static void print_help(std::string exec_name);
    static void check_if_user_needs_help(int argc, char *argv[]);
private:
    static const std::vector<std::string> allowed_args;
};

#endif
