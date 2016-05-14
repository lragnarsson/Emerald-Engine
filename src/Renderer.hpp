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
#include <SDL2/SDL.h>
#include <AntTweakBar.h>

#include "Camera.hpp"
#include "Model.hpp"
#include "Utils.hpp"
#include "Error.hpp"


#define MAX_SSAO_SAMPLES 256

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
    uint objects_drawn;

    Renderer() {}

    void init();
    void render();
    void set_mode(render_mode mode);
    void init_uniforms(const Camera &camera);
    void upload_camera_uniforms(const Camera &camera);
    void set_kernel_radius(float radius) {kernel_radius = radius;}
    float get_kernel_radius() {return kernel_radius;}
    GLint get_ssao_n_samples() {return ssao_n_samples;}
    void toggle_ssao();
    void toggle_tweak_bar();

private:
    enum shader {
      FORWARD,
      GEOMETRY,
      DEFERRED,
      FLAT,
      SSAO,
      SSAO_BLUR,
      G_COMPONENT,
      G_SPECULAR
    };

    render_mode mode;
    GLuint shaders[6];
    GLuint g_buffer, ssao_fbuffer, ssao_blur_fbo;
    GLuint g_position, g_normal, g_albedo_specular, ssao_result, ssao_blurred;
    GLuint quad_vao, quad_vbo;
    glm::mat4 w2v_matrix;
    Model* sphere;

    //glm::vec3 ssao_kernel[64];

    GLuint noise_texture; // Really small and tiled across the screen
    std::vector<glm::vec3> ssao_kernel;
    std::vector<glm::vec3> ssao_noise;
    GLfloat kernel_radius = 1; // Could be interesting to tweak this
    GLint ssao_n_samples = 64;
    bool ssao_on;

    // Tweak bar
    TwBar* tweak_bar;
    bool use_tweak_bar = true;
    double fps;
    unsigned last_time;
    void count_fps();

    void init_quad();
    void init_g_buffer();
    void init_ssao();

    void init_tweak_bar();
    void draw_tweak_bar();

    void render_deferred();
    void render_forward();
    void render_flat();
    void render_bounding_spheres();

    void clear_ssao();
    void render_ssao();
    void create_ssao_samples();

    void render_geometry(std::vector<Model*> models);
    void render_g_position();
    void render_g_normal();
    void render_g_albedo();
    void render_g_specular();
};

#endif
