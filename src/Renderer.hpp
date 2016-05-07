#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#ifdef __linux__
#include <GL/glew.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

#include <vector>
#include <random>

#include "Camera.hpp"
#include "Model.hpp"
#include "Utils.hpp"
#include "Error.hpp"




enum render_mode {
    FORWARD_MODE,
    DEFERRED_MODE,
    POSITION_MODE,
    NORMAL_MODE,
    ALBEDO_MODE,
    SPECULAR_MODE
};


class Renderer
{
public:
    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false; //unused

    Renderer(){}

    void init();
    void render();
    void set_mode(render_mode mode);
    void init_uniforms(const Camera &camera);
    void upload_camera_uniforms(const Camera &camera);
    //unsigned long get_kernel_size() { return ssao_kernel.size(); }
    
private:
    enum shader {
      FORWARD,
      GEOMETRY,
      DEFERRED,
      FLAT,
      SSAO,
      G_COMPONENT,
      G_SPECULAR
    };

    render_mode mode;
    GLuint shaders[5];
    GLuint g_buffer, ssao_fbuffer;
    GLuint g_position_depth, g_normal, g_albedo_specular, ssao_result;
    GLuint quad_vao, quad_vbo;
    glm::mat4 w2v_matrix;

    //glm::vec3 ssao_kernel[64];

    GLuint noise_texture; // Really small and tiled across the screen
    std::vector<glm::vec3> ssao_kernel;
    std::vector<glm::vec3> ssao_noise;
    GLfloat kernel_radius = 1; // Could be interesting to tweak this
        
    void init_quad();
    void init_g_buffer();
    void init_ssao();
    
    void render_deferred();
    void render_forward();
    void render_flat();
    void render_ssao();
    // REMOVE
    void render_geometry(std::vector<Model*> models);
    // THIS?
    void render_g_position();
    void render_g_normal();
    void render_g_albedo();
    void render_g_specular();
};

#endif
