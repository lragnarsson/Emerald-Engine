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
#include "Camera.hpp"


class Renderer
{
public:
    // This is a typedef of a Renderer member function pointer with return type void and argument std::vector<Model*>.
    typedef void (Renderer::*render_fptr)(std::vector<Model*> &loaded_models);

    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false; //unused

    GLuint shader_forward, shader_geometry, shader_deferred, shader_flat;
    std::vector<GLuint> current_shaders;
    std::vector<GLuint> compiled_shaders;
    
    // This is a function pointer to the current render function
    render_fptr render_function;

    Renderer(){}

    void set_deferred();
    void set_forward();
    void set_flat();
    void set_g_position();
    void set_g_normal();
    void set_g_albedo();
    void set_g_specular();
    void upload_camera_uniforms(const Camera &camera);

private:
    void render_deferred(std::vector<Model*> &loaded_models);
    void render_forward(std::vector<Model*> &loaded_models);
    void render_flat(std::vector<Model*> &loaded_models);
    void render_g_position(std::vector<Model*> &loaded_models);
    void render_g_normal(std::vector<Model*> &loaded_models);
    void render_g_albedo(std::vector<Model*> &loaded_models);
    void render_g_specular(std::vector<Model*> &loaded_models);
};

#endif
