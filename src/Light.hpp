#ifndef LIGHT_H
#define LIGHT_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Camera.hpp"
#include "Profiler.hpp"


namespace Light
{
    typedef struct {
        glm::vec3 position; // 16
        float brightness; // 4
        glm::vec3 color; // 16
        float padding;
    } Light;

    extern Light lights[_MAX_LIGHTS_]; // Always same order
    extern Light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
    extern float light_radii[_MAX_LIGHTS_];
    extern glm::vec4 light_info; // Contains num_lights + padding.
    extern GLuint ubos[2];
    extern int num_lights;
    extern int culled_lights;
    extern std::vector<GLuint> shader_programs;

    int create_light(glm::vec3 position, float brightness, glm::vec3 color);
    void destroy_light(int index);
    void init();
    void cull_light_sources(Camera &camera);
    void upload_lights();
    void generate_bounding_sphere(int light);

    //void turn_off_all_lights();
    //void turn_on_one_lightsource();

}
#endif
