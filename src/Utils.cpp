#include "Utils.hpp"


bool sdl_init(const GLuint screen_width, const GLuint screen_height,
              display_mode d_mode,
              SDL_Window *&main_window, SDL_GLContext &main_context)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to init SDL\n";
        return false;
    }

    if ( d_mode == WINDOWED){
        main_window = SDL_CreateWindow("TSBK07 Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                screen_width, screen_height, SDL_WINDOW_OPENGL);
    }
    else {
        main_window = SDL_CreateWindow("TSBK07 Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                screen_width, screen_height, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
    }
    if (!main_window) {
        std::cerr << "Unable to create window\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    main_context = SDL_GL_CreateContext(main_window);

#ifndef __APPLE__
    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
#endif

    glEnable(GL_DEPTH_TEST);
    SDL_GL_SwapWindow(main_window);

    if ( SDL_GL_SetSwapInterval(0) != 0 ) {
        printf("WARNING: Unable to disable vsync, %s\n", SDL_GetError());
    }

    return true;
}


void sdl_quit(SDL_Window* main_window, SDL_GLContext &main_context)
{
    SDL_GL_DeleteContext(main_context);
    SDL_DestroyWindow(main_window);
    SDL_Quit();
}


void sdl_check_error(const int line = -1)
{
    std::string error = SDL_GetError();

    if (error != "") {
        std::cerr << "SDL Error : " << error << std::endl;

        if (line != -1) {
            std::cerr << "\nLine : " << line << std::endl;
        }

        SDL_ClearError();
    }
}


void sdl_print_attributes()
{
    int value = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
    std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
    std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}



/* Loads, compiles and activates vertex, geometry and fragment shaders */
GLuint load_shaders(const GLchar* vertex_file_path, const GLchar* geometry_file_path,
                    const GLchar* fragment_file_path)
{
    std::string vertex_source = read_shader_file(vertex_file_path);
    std::string geometry_source = read_shader_file(geometry_file_path);
    std::string fragment_source = read_shader_file(fragment_file_path);

    GLuint vertex = compile_shader((GLchar*) vertex_source.c_str(), GL_VERTEX_SHADER, vertex_file_path);
    GLuint geometry = compile_shader((GLchar*) geometry_source.c_str(), GL_GEOMETRY_SHADER, geometry_file_path);
    GLuint fragment = compile_shader((GLchar*) fragment_source.c_str(), GL_FRAGMENT_SHADER, fragment_file_path);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, geometry);
    glAttachShader(program, fragment);

    glLinkProgram(program);

    GLint success;
    GLchar info_log[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        Error::throw_error(Error::cant_load_shader, "Linking failed: " + std::string(info_log));
    }

    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);

    glUseProgram(program);

    return program;
}


/* Loads, compiles and activates vertex, geometry and fragment shaders */
GLuint load_shaders(const GLchar* vertex_file_path, const GLchar* fragment_file_path)
{
    std::string vertex_source = read_shader_file(vertex_file_path);
    std::string fragment_source = read_shader_file(fragment_file_path);

    GLuint vertex = compile_shader((GLchar*) vertex_source.c_str(), GL_VERTEX_SHADER, vertex_file_path);
    GLuint fragment = compile_shader((GLchar*) fragment_source.c_str(), GL_FRAGMENT_SHADER, fragment_file_path);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);

    GLint success;
    GLchar info_log[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        Error::throw_error(Error::cant_load_shader, "Linking failed for "
                           + std::string(vertex_file_path) + ": " + std::string(info_log));
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(program);

    return program;
}


std::string read_shader_file(const char *file_path) {
    std::string shader_source;
    std::ifstream shader_file;

    shader_file.exceptions(std::ifstream::badbit);

    try {
        shader_file.open(file_path);

        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();
        shader_file.close();

        shader_source = shader_stream.str();
    }
    catch (std::ifstream::failure e) {
        Error::throw_error(Error::cant_load_shader, "Could not read file: " + std::string(e.what()));
    }

    return shader_source;
}


GLuint compile_shader(const GLchar *shader_source, const GLenum shader_type,
                      const char *file_path)
{
    GLint success;
    GLchar info_log[512];

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        Error::throw_error(Error::cant_load_shader, (std::string) file_path +
                           " compilation failed: " + std::string(info_log));
    }
    return shader;
}
