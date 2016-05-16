#include "Parser.hpp"

using namespace std;

string Parser::get_scene_file_from_command_line(int argc, char *argv[])
{
    string error_information = "Usage: \n";
    error_information += "  ";
    error_information += argv[0];
    error_information += " [/path/to/scene/file]";

    if (argc != 1 && argc != 2) {
        Error::throw_error(Error::argument_error, error_information);
    }

    if (argc == 2) {
        return string(argv[1]);
    }

    #ifdef _DEFAULT_SCENE_FILE_
    return string(_DEFAULT_SCENE_FILE_);
    #endif

    Error::throw_error(Error::argument_error, error_information);

    return ""; // Needed for compiler warnings, but useless.
}
