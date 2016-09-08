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
    SPECULAR_MODE,
    SSAO_MODE
};


class Renderer
{
public:
    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false;
    uint objects_drawn;

    Renderer() {}

    void init();
    void init_tweak_bar(Camera* camera);
    void render(const Camera &camera);
    void set_mode(render_mode mode);
    void init_uniforms(const Camera &camera);
    void set_kernel_radius(float radius) {kernel_radius = radius;}
    float get_kernel_radius() {return kernel_radius;}
    GLint get_ssao_n_samples() {return ssao_n_samples;}
    void toggle_ssao();
    void toggle_ssao_smoothing();
    void toggle_tweak_bar();
    void copy_tweak_bar_cam_values(const Camera& camera);

private:
    enum shader {
      FORWARD,
      GEOMETRY,
      DEFERRED,
      FLAT,
      FLAT_TEXTURE,
      SSAO,
      SSAO_BLUR, // Remove later. Maybe rename to BROKEN_BLUR_XY
      BLUR_RED_X,
      BLUR_RED_Y,
      BLUR_RGB_X,
      BLUR_RGB_Y,
      SHOW_RGB_COMPONENT,
      SHOW_ALPHA_COMPONENT,
      SHOW_SSAO
    };

    render_mode mode;
    GLuint shaders[14];
    GLuint g_buffer, ssao_fbo, ping_pong_fbo_red, ping_pong_fbo_rgb;
    GLuint g_position, g_normal_shininess, g_albedo_specular, ssao_tex, ping_pong_tex_red, ping_pong_tex_rgb;
    GLuint quad_vao, quad_vbo;
    glm::mat4 w2v_matrix;
    Model *sphere, *skybox;


    GLuint noise_texture; // Really small and tiled across the screen
    std::vector<glm::vec3> ssao_kernel;
    std::vector<glm::vec3> ssao_noise;
    GLfloat kernel_radius = 5; // Could be interesting to tweak this
    GLint ssao_n_samples;
    bool ssao_on;
    bool smooth_ssao;

    // Tweak bar
    TwBar* tweak_bar;
    bool use_tweak_bar = false;
    double fps;
    unsigned last_time;
    void count_fps();
    // Copied camera spline variables
    float cam_spline_move_para, cam_spline_look_para;
    int cam_spline_move_id, cam_spline_look_id, n_lightsources;
    glm::vec3 cam_pos;

    void init_g_buffer();
    void init_ssao();
    void init_ping_pong_fbos();
    void init_quad();
    void init_rgb_component_shader();
    void init_albedo_component_shader();
    void init_alpha_component_shader();
    void init_show_ssao_shader();
    void init_blur_shaders();

    void upload_camera_uniforms(const Camera &camera);
    void draw_tweak_bar();

    void render_deferred();
    void render_forward();
    void render_flat();
    void render_bounding_spheres();

    void clear_ssao();
    void ssao_pass();
    void create_ssao_samples();

    void filter_pass(GLuint source_tex, GLuint target_fbo, GLuint shader);
    void blur_red_texture(GLuint source_tex, GLuint target_fbo, int iterations);
    void blur_rgb_texture(GLuint source_tex, GLuint target_fbo, int iterations);

    void geometry_pass();
    void render_g_position();
    void render_g_normal();
    void render_g_albedo();
    void render_g_specular();
    void render_ssao();
    void render_skybox(const Camera &camera);
};

#endif
