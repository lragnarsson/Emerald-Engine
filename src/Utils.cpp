#include "Utils.hpp"

Light::Light(const glm::vec3 world_coord, const glm::vec3 ambient_color,
             const glm::vec3 diffuse_color, const glm::vec3 specular_color) {

    this->position = world_coord;
    this->ambient_color = ambient_color;
    this->diffuse_color = diffuse_color;
    this->specular_color = specular_color;
}

void upload_lights(const GLuint shader_program, const std::vector<Light*> &loaded_lights) {
    for (int i = 0; i < loaded_lights.size(); i++) {
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].position").c_str()), 1,
                     glm::value_ptr(loaded_lights[i]->position));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].ambientColor").c_str()), 1,
                     glm::value_ptr(loaded_lights[i]->ambient_color));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].diffuseColor").c_str()), 1,
                     glm::value_ptr(loaded_lights[i]->diffuse_color));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].specularColor").c_str()), 1,
                     glm::value_ptr(loaded_lights[i]->specular_color));
    }
    glUniform1i(glGetUniformLocation(shader_program, "nLights"), loaded_lights.size());
}

bool sdl_init(const GLuint screen_width, const GLuint screen_height,
              SDL_Window *&main_window, SDL_GLContext &main_context)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to init SDL\n";
        return false;
    }

    main_window = SDL_CreateWindow("TSBK07 Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   screen_width, screen_height, SDL_WINDOW_OPENGL);
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

    SDL_GL_SetSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    SDL_GL_SwapWindow(main_window);

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
        std::cout << "SLD Error : " << error << std::endl;

        if (line != -1) {
            std::cout << "\nLine : " << line << std::endl;
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


/* Loads, compiles and activates vertex and fragment shaders */
GLuint load_shaders(const GLchar* vertex_file_path, const GLchar* fragment_file_path)
{
    std::string vertex_source;
    std::string fragment_source;
    std::ifstream vertex_shader_file;
    std::ifstream fragment_shader_file;

    vertex_shader_file.exceptions(std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::badbit);

    /* Read shaders from files */
    try {
        vertex_shader_file.open(vertex_file_path);
        fragment_shader_file.open(fragment_file_path);

        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vertex_shader_file.rdbuf();
        fShaderStream << fragment_shader_file.rdbuf();
        vertex_shader_file.close();
        fragment_shader_file.close();

        vertex_source = vShaderStream.str();
        fragment_source = fShaderStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cerr << "ERROR: Could not read shader files." << std::endl;
    }

    const GLchar* vSource = vertex_source.c_str();
    const GLchar* fSource = fragment_source.c_str();

    GLuint vertex, fragment;
    GLint success;
    GLchar info_log[512];

    /* Compile vertex shader */
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSource, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        std::cerr << "ERROR: Vertex shader compilation failed: \n" << info_log << std::endl;
    }

    /* Compile fragment shader */
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, info_log);
        std::cerr << "ERROR: Fragment shader compilation failed: \n" << info_log << std::endl;
    }

    /* Create shader program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        std::cerr << "ERROR: Shader program linking failed: \n" << info_log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(program);

    return program;
}

