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
    float altitude, azimuth, interp, interp_night;

    vec3 zenith_color;
    vec3 horizon_color;

    void init_uniforms();
    void calculate_sky();
    void calculate_sun();


    // Constant color values:
    static const vec3 sun_dawn;
    static const vec3 sun_noon;
    static const vec3 sun_dusk;
    static const vec3 sun_midnight;

    static const vec3 zenith_dawn;
    static const vec3 horizon_dawn;

    static const vec3 zenith_noon;
    static const vec3 horizon_noon;

    static const vec3 zenith_dusk;
    static const vec3 horizon_dusk;

    static const vec3 zenith_midnight;
    static const vec3 horizon_midnight;

    static const float altitude_margin;
};

#endif
