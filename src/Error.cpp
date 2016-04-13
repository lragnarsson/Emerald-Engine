#include "Error.hpp"

using namespace std;

const ErrorMessage Error::display_init_fail = {1, "Display init failed!"};
const ErrorMessage Error::cant_open_world_file = {2, "Can not open world file!"};
const ErrorMessage Error::invalid_file_syntax = {3, "Invalid file syntax!"};
const ErrorMessage Error::cant_load_light = {4, "Can not load light!"};

void Error::throw_error(const ErrorMessage& message, string extra_information)
{
  cerr << "Error " << message.code << ": " << message.message;
  if (extra_information != "") {
    cerr << ": " << extra_information;
  }
  cerr << endl;
  exit(message.code);
}
