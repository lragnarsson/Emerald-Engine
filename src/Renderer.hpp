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
    SSAO_MODE,
    SHADOW_MODE
};

enum filter_type {
    GAUSSIAN_RGB_11,
    UNIFORM_RED_5,
};

typedef struct {
    GLuint x, y;
} ping_pong_shader;


enum bounding_sphere_mode {
    NO_BOUNDING_SPHERES = 0,
    ONLY_MODELS,
    ALL_BOUNDING_SPHERES,
};

class Renderer
{
public:
    bool running = false;
    bool wireframe_mode = false; // unused
    bounding_sphere_mode draw_bounding_spheres = NO_BOUNDING_SPHERES;
    uint models_drawn, meshes_drawn, grass_straws_drawn;

    Renderer() {}

    void init();
    void render(const Camera &camera);
    void set_mode(render_mode mode);
    void change_bounding_sphere_mode();
    void init_uniforms(const Camera &camera);
    void set_kernel_radius(float radius) {kernel_radius = radius;}
    float get_kernel_radius() {return kernel_radius;}
    GLint get_ssao_n_samples() {return ssao_n_samples;}
    void toggle_ssao();
    void toggle_ssao_smoothing();
    void toggle_tweak_bar();
    float get_time_diff();
    void propagate_time(const bool forward);
    void increase_up_interp();
    void decrease_up_interp();
    void increase_grass_lod_distance();
    void decrease_grass_lod_distance();
    void toggle_show_normals();
    void toggle_grass_single_color() {grass_single_color_on = !grass_single_color_on;}

private:
    enum shader {
        FORWARD=0,
        GEOMETRY,
        DEFERRED,
        FLAT,
        FLAT_NO_BLOOM,
        FLAT_TEXTURE,
        FLAT_NORMALS,
        SSAO,
        BLUR_RED_5_X,
        BLUR_RED_5_Y,
        BLUR_RGB_11_X,
        BLUR_RGB_11_Y,
        SHOW_RGB_COMPONENT,
        SHOW_ALPHA_COMPONENT,
        SHOW_SSAO,
        HDR_BLOOM,
        SHADOW_BUFFER,
        GRASS_LOD1,
        GRASS_LOD2,
        GRASS_LOD3,
        GRASS_LOD1_SINGLE_COLOR,
        GRASS_LOD2_SINGLE_COLOR,
        GRASS_LOD3_SINGLE_COLOR,
        NUM_SHADERS
    };

    render_mode mode;
    GLuint shaders[NUM_SHADERS];

    // Frame buffers
    GLuint g_buffer, ssao_fbo, hdr_fbo, post_proc_fbo, ping_pong_fbo_red, ping_pong_fbo_rgb, depth_map_FBO;
    // Textures
    GLuint g_position, g_normal_shininess, g_albedo_specular, ssao_tex, color_tex, bright_tex, post_proc_tex,  ping_pong_tex_red, ping_pong_tex_rgb, depth_map_texture, light_space_texture;
    GLuint quad_vao, quad_vbo;
    mat4 w2v_matrix;
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

    // Copied camera spline variables
    float cam_spline_move_para, cam_spline_look_para;
    int cam_spline_move_id, cam_spline_look_id, n_lightsources;
    vec3 cam_pos;

    // Normal visualization settings
    const vec3 PINK = vec3(1.f, 0.f, 1.f);
    bool show_normals = false;
    float up_interp = 0.8; // between [0,1]

    // Grass settings
    float grass_lod1_distance = 40.0;
    float grass_lod2_distance = 80.0;
    float grass_lod3_distance = 320.0;
    bool grass_single_color_on = false;
    static const vec3 GRASS_LOD1_COLOR;
    static const vec3 GRASS_LOD2_COLOR;
    static const vec3 GRASS_LOD3_COLOR;

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
    void shadow_pass(const Camera &camera);
    void render_forward();
    void render_flat();
    void render_bounding_spheres();
    void post_processing();

    void clear_ssao();
    void ssao_pass();
    void create_ssao_samples();

    ping_pong_shader upload_filter(filter_type ft);
    void filter_pass(GLuint source_tex, GLuint target_fbo);
    void blur_red_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations);
    void blur_rgb_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations);

    void geometry_pass();
    void normal_visualization_pass(const vec3 cam_pos);
    void grass_generation_pass(const Camera &camera);
    void render_g_position(const Camera &camera);
    void render_g_normal(const Camera &camer);
    void render_g_albedo(const Camera &camer);
    void render_g_specular(const Camera &camer);
    void render_ssao(const Camera &camer);
    void render_shadow_map(const Camera &camera);
};

#endif
