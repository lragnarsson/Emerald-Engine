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


typedef void (*render_fptr)();

class Renderer
{
public:
    typedef void (Renderer::*render_fptr)(std::vector<Model*> &loaded_models);

    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false; //unused

    GLuint shader_forward, shader_geometry, shader_deferred;
    std::vector<GLuint> current_shaders;

    render_fptr render_function;

    Renderer(){}

    void set_deferred();
    void set_forward();
    void set_g_position();
    void set_g_normal();
    void set_g_albedo();
    void set_g_specular();

private:
    void render_deferred(std::vector<Model*> &loaded_models);
    void render_forward(std::vector<Model*> &loaded_models);
    void render_g_position(std::vector<Model*> &loaded_models);
    void render_g_normal(std::vector<Model*> &loaded_models);
    void render_g_albedo(std::vector<Model*> &loaded_models);
    void render_g_specular(std::vector<Model*> &loaded_models);
};

#endif
