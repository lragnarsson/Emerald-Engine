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
    static void throw_warning(const ErrorMessage& message, std::string extra_information="");

    static const ErrorMessage display_init_fail;
    static const ErrorMessage renderer_init_fail;
    static const ErrorMessage cant_open_world_file;
    static const ErrorMessage invalid_file_syntax;
    static const ErrorMessage cant_load_light;
    static const ErrorMessage cant_load_shader;
    static const ErrorMessage argument_error;
    static const ErrorMessage ssao_num_samples;
    static const ErrorMessage animation_path_time_error;
    static const ErrorMessage animation_path_vector_error;
    static const ErrorMessage model_has_no_path;
    static const ErrorMessage model_has_no_tangents;
    static const ErrorMessage cant_load_image;
    static const ErrorMessage cant_load_model;
    static const ErrorMessage camera_has_no_path;
    static const ErrorMessage camera_free_mode;
    static const ErrorMessage performance_counter_already_started;
    static const ErrorMessage performance_counter_not_started;
};

#endif
