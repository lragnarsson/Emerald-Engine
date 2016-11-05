#ifndef SKYDOME_H
#define SKYDOME_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>

#include "Model.hpp"
#include "Utils.hpp"
#include "Error.hpp"

using namespace glm;


class Skydome
{
public:
    float time_of_day, time_scale;

    void init();
    void draw(const Camera &camera);
    void upload_sun(const GLuint shader, const Camera &camera);
    void propagate_time(float elapsed_time);
    void reset_time();

private:
    vec3 sun_direction;
    vec3 sun_color;
    Model *sphere; // actual dome
    GLuint shader;

    vec3 current_zenith_color;
    vec3 current_horizon_color;

    vec3 zenith_dawn = {0.53f, 0.81f, 0.98f};
    vec3 horizon_dawn = {0.53f, 0.81f, 0.98f};

    vec3 zenith_noon = {0.1f, 0.4f, 1.f};
    vec3 horizon_noon = {0.3f, 0.5f, 0.98f};

    vec3 zenith_dusk = {0.53f, 0.81f, 0.98f};
    vec3 horizon_dusk = {0.53f, 0.81f, 0.98f};

    void init_uniforms();
    void calculate_sky();
    void calculate_sun();
};

#endif
