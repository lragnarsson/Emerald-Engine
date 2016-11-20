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
#include "Skydome.hpp"
#include "Utils.hpp"
#include "Error.hpp"
#include "Profiler.hpp"
#include "Terrain.hpp"


#define MAX_SSAO_SAMPLES 256

using namespace glm;

enum render_mode {
    FORWARD_MODE,
    DEFERRED_MODE,
    POSITION_MODE,
    NORMAL_MODE,
    ALBEDO_MODE,
    SPECULAR_MODE,
    SSAO_MODE
};

enum filter_type {
    GAUSSIAN_RGB_11,
    UNIFORM_RED_5,
};

typedef struct {
    GLuint x, y;
} ping_pong_shader;


class Renderer
{
public:
    bool running = false;
    bool wireframe_mode = false; // unused
    bool draw_bounding_spheres = false;
    uint objects_drawn,meshes_drawn;

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
    float get_time_diff();
    void propagate_time(bool forward);
    void increase_up_interp();
    void decrease_up_interp();
    void toggle_show_normals();
    
private:
    enum shader {
      FORWARD,
      GEOMETRY,
      GEOMETRY_NORMALS,
      DEFERRED,
      FLAT,
      FLAT_TEXTURE,
      SSAO,
      BLUR_RED_5_X,
      BLUR_RED_5_Y,
      BLUR_RGB_11_X,
      BLUR_RGB_11_Y,
      SHOW_RGB_COMPONENT,
      SHOW_ALPHA_COMPONENT,
      SHOW_SSAO,
      HDR_BLOOM,
      SHADOW_MAP,
    };

    render_mode mode;
    GLuint shaders[16];
    // Frame buffers
    GLuint g_buffer, ssao_fbo, hdr_fbo, post_proc_fbo, ping_pong_fbo_red, ping_pong_fbo_rgb, shadow_buffer;
    // Textures
    GLuint g_position, g_normal_shininess, g_albedo_specular, ssao_tex, color_tex, bright_tex, post_proc_tex,  ping_pong_tex_red, ping_pong_tex_rgb, shadow_map, frag_pos_light_space;
    GLuint quad_vao, quad_vbo;
    mat4 w2v_matrix, light_space_matrix;
    Model *sphere;
    Skydome *skydome;

    // Renderer keeps track of time so animations are time based
    unsigned last_timestamp = 0;
    unsigned time_diff = 0;
    void update_time();


    GLuint noise_texture; // Really small and tiled across the screen
    std::vector<vec3> ssao_kernel;
    std::vector<vec3> ssao_noise;
    GLfloat kernel_radius = 5; // Could be interesting to tweak this
    GLint ssao_n_samples;
    bool ssao_on;
    bool smooth_ssao;
    bool shadows_on = true;

    // Tweak bar
    TwBar* tweak_bar;
    bool use_tweak_bar = false;
    double fps;
    void count_fps();

    // Copied camera spline variables
    float cam_spline_move_para, cam_spline_look_para;
    int cam_spline_move_id, cam_spline_look_id, n_lightsources;
    vec3 cam_pos;

    // Normal visualization settings
    bool show_normals = false;
    float up_interp = 0.0; // between [0,1]

    void init_g_buffer();
    void init_hdr_fbo();
    void init_post_proc_fbo();
    void init_ssao();
    void init_ping_pong_fbos();
    void init_quad();
    void init_rgb_component_shader();
    void init_albedo_component_shader();
    void init_alpha_component_shader();
    void init_show_ssao_shader();
    void init_blur_shaders();
    void init_hdr_bloom_shader();
    void init_shadow_buffer();

    void upload_camera_uniforms(const Camera &camera);
    void draw_tweak_bar();

    void render_deferred(const Camera &camera);
    void render_forward();
    void render_flat();
    void render_bounding_spheres();
    void post_processing();
    void render_shadow_map(const Camera &camera);

    void clear_ssao();
    void ssao_pass();
    void create_ssao_samples();

    ping_pong_shader upload_filter(filter_type ft);
    void filter_pass(GLuint source_tex, GLuint target_fbo);
    void blur_red_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations);
    void blur_rgb_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations);

    void geometry_pass();
    void normal_visualization_pass();
    void render_g_position();
    void render_g_normal();
    void render_g_albedo();
    void render_g_specular();
    void render_ssao();
};

#endif
