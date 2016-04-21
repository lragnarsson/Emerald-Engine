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

private:
    enum shader {
      FORWARD,
      GEOMETRY,
      DEFERRED,
      FLAT
    };

    render_mode mode;
    GLuint shaders[4];
    glm::mat4 w2v_matrix;

    void render_deferred();
    void render_forward();
    void render_flat();
    void render_g_position();
    void render_g_normal();
    void render_g_albedo();
    void render_g_specular();
};

#endif
