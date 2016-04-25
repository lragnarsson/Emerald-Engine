#ifndef ERROR_H
#define ERROR_H

#include <iostream>

struct ErrorMessage
{
  unsigned int code;
  std::string message;
};

class Error
{
public:
  static void throw_error(const ErrorMessage& message, std::string extra_information="");

  static const ErrorMessage display_init_fail;
  static const ErrorMessage cant_open_world_file;
  static const ErrorMessage invalid_file_syntax;
  static const ErrorMessage cant_load_light;
  static const ErrorMessage cant_load_shader;
  static const ErrorMessage argument_error;
};

#endif
