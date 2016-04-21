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
#include "Utils.hpp"

class Renderer
{
public:
    // This is a typedef of a Renderer member function pointer with return type void and argument std::vector<Model*>.
    typedef void (Renderer::*render_fptr)(std::vector<Model*> &loaded_models,
                                          std::vector<Model*> &loaded_flat_models);

    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false; //unused

    
    // This is a function pointer to the current render function
    render_fptr render_function;

    Renderer(){}

    void init();
    void set_deferred();
    void set_forward();
    void set_g_position();
    void set_g_normal();
    void set_g_albedo();
    void set_g_specular();
    void init_uniforms(const Camera &camera);
    void upload_camera_uniforms(const Camera &camera);
    
private:
    enum shader {
        FORWARD,
        GEOMETRY,
        DEFERRED,
        FLAT
    };

    GLuint shaders[4]; 
    glm::mat4 w2v_matrix;
    
    void render_deferred(std::vector<Model*> &loaded_models,
                         std::vector<Model*> &loaded_flat_models);
    void render_forward(std::vector<Model*> &loaded_models,
                        std::vector<Model*> &loaded_flat_models);
    void render_flat(std::vector<Model*> &loaded_flat_models);
    void render_g_position(std::vector<Model*> &loaded_models,
                           std::vector<Model*> &loaded_flat_models);
    void render_g_normal(std::vector<Model*> &loaded_models,
                         std::vector<Model*> &loaded_flat_models);
    void render_g_albedo(std::vector<Model*> &loaded_models,
                         std::vector<Model*> &loaded_flat_models);
    void render_g_specular(std::vector<Model*> &loaded_models,
                           std::vector<Model*> &loaded_flat_models);
};

#endif
