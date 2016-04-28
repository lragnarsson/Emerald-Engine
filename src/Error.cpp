#include "Error.hpp"

using namespace std;

const ErrorMessage Error::display_init_fail = {1, "Display init failed!"};
const ErrorMessage Error::cant_open_world_file = {2, "Could not open world file!"};
const ErrorMessage Error::invalid_file_syntax = {3, "Invalid scene file syntax!"};
const ErrorMessage Error::cant_load_light = {4, "Could not load light!"};
const ErrorMessage Error::cant_load_shader = {5, "Could not load shader!"};
const ErrorMessage Error::argument_error = {6, "Argument error!"};
const ErrorMessage Error::renderer_init_fail = {7, "Renderer could not initiate framebuffer!"};


void Error::throw_error(const ErrorMessage& message, string extra_information)
{
  cerr << "Error " << message.code << ": " << message.message;
  if (extra_information != "") {
    cerr << endl << extra_information;
  }
  cerr << endl;
  exit(message.code);
}
