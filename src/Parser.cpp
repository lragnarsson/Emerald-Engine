#include "Parser.hpp"

using namespace std;

void Parser::print_help(string exec_name){
    string help_text = "Usage: \n";
    help_text += "  ";
    help_text += exec_name;
    help_text += " [--scene-file /path/to/scene/file] [--fullscreen] \n";
    help_text += "\n";
    help_text += "-f, --fullscreen      Run Emerald in full screen. \n";
    help_text += "-sf, --scene-file     Run Emerald with specified scene file, otherwise default. \n";
    help_text += "-h, --help            Print this help text. \n";

    cout << help_text;
    exit(1);
}

void Parser::check_if_user_needs_help(int argc, char *argv[]){

    // Incorect nr args
    if (argc > 4){
        print_help(string(argv[0]));
    }

    // Check if user specificly asks for help
    for (int i = 0; i < argc; i++) {
        if ( string(argv[i]) == "-h" or string(argv[i]) == "--help" ){
            print_help(string(argv[0]));
        }
    }

}

string Parser::get_scene_file_from_command_line(int argc, char *argv[])
{
    for ( int i = 0; i < argc; i++ ){
        if ( string(argv[i]) == "--scene-file" or string(argv[i]) == "-sf" ){
            return string(argv[i+1]);
        }

    }

    #ifdef _DEFAULT_SCENE_FILE_
    return string(_DEFAULT_SCENE_FILE_);
    #endif

    Error::throw_error(Error::argument_error, "No scene file specified.");

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
