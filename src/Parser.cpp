#include "Parser.hpp"

using namespace std;

const vector<string> Parser::allowed_args = {
        "--help",
        "-h",
        "--scene-file",
        "-sf",
        "--fullscreen",
        "-f"
    };

// -----------------

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
    string exec_name = string(argv[0]);
    // Incorect nr args
    if (argc > 4){
        print_help(exec_name);
    }

    bool includes_scene_file = false;
    // Check if user specificly asks for help
    for (int i = 1; i < argc; i++) {
        if ( string(argv[i]) == "-h" or string(argv[i]) == "--help" )
            print_help(exec_name);

        // If scene file isn't given, we should have at maximum 2 args
        if ( string(argv[i]) == "--scene-file" or string(argv[i]) == "-sf" ){
            includes_scene_file = true;
            // Next arg can't be any allowed argument
            for ( int it = 0; it < Parser::allowed_args.size(); it++ ){
                if ( i+1 < argc ){
                    if (string(argv[i+1]) == Parser::allowed_args[it])
                        print_help(exec_name);
                    }
                else {
                    print_help(exec_name);
                }
            }
        }
    }

    if (!includes_scene_file and argc > 2)
        print_help(exec_name);
    // If argv does NOT include scene file argument, then
    //  args allowed are the only things to be inside argv
    if (!includes_scene_file){
        for ( int i = 1; i < argc; i++ ){
            bool found_arg = false;
            for ( int it = 1; it < Parser::allowed_args.size(); it++ ){
                if ( argv[i] == allowed_args[it] )
                    found_arg = true;
            }
            if (found_arg == false)
                print_help(exec_name);
        }
    }
}

string Parser::get_scene_file_from_command_line(int argc, char *argv[])
{
    for ( int i = 1; i < argc; i++ ){
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

    for (int i = 1; i < argc; i++) {
        if ( string(argv[i]) == "--fullscreen" or string(argv[i]) == "-f" ) {
            return FULLSCREEN;
        }
    }
    return WINDOWED;
}
