#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <vector>
#include "Model.hpp"
#include "Light.hpp"
#include "Utils.hpp"
#include "Error.hpp"


const GLuint SCREEN_WIDTH = 800;
const GLuint SCREEN_HEIGHT = 640;


class Renderer
{
public:
    // This is a typedef of a Renderer member function pointer with return type void and argument std::vector<Model*>. Type name is rander_fptr.
    typedef void (Renderer::*render_fptr)(std::vector<Model*> &loaded_models);

    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false; //unused

    GLuint shader_forward, shader_geometry, shader_deferred,
        shader_flat, shader_pos, shader_normal,
        shader_albedo, shader_specular;
    GLuint g_buffer;
    GLuint g_position, g_normal, g_albedo_specular;

    std::vector<GLuint> current_shaders;

    // This is a function pointer to the current render function
    render_fptr render_function;

    Renderer() {};

    void init();

    void set_deferred();
    void set_forward();
    void set_g_position();
    void set_g_normal();
    void set_g_albedo();
    void set_g_specular();

private:
    void init_g_buffer();
    void render_deferred(std::vector<Model*> &loaded_models);
    void render_forward(std::vector<Model*> &loaded_models);
    void render_g_position(std::vector<Model*> &loaded_models);
    void render_g_normal(std::vector<Model*> &loaded_models);
    void render_g_albedo(std::vector<Model*> &loaded_models);
    void render_g_specular(std::vector<Model*> &loaded_models);
};

#endif
