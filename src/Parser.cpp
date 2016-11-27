#include "Parser.hpp"

using namespace std;

string Parser::get_scene_file_from_command_line(int argc, char *argv[])
{
    string error_information = "Usage: \n";
    error_information += "  ";
    error_information += argv[0];
    error_information += " [/path/to/scene/file]";

    if (argc > 4) {
        Error::throw_error(Error::argument_error, error_information);
    }
    
    for ( int i = 0; i < argc; i++ ){
        if ( string(argv[i]) == "--scene-file" or string(argv[i]) == "-sf" ){
            return string(argv[i+1]);
        }

    }

    #ifdef _DEFAULT_SCENE_FILE_
    return string(_DEFAULT_SCENE_FILE_);
    #endif

    Error::throw_error(Error::argument_error, error_information);

    return ""; // Needed for compiler warnings, but useless.
}

// ----------------

display_mode Parser::get_display_mode_from_command_line(int argc, char *argv[]){

    for (int i = 0; i < argc; i++) {
        if ( string(argv[i]) == "--fullscreen" or string(argv[i]) == "-f" ) {
            return FULLSCREEN;
        }
    }
    return WINDOWED;
}
