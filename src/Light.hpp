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

    typedef struct {
        float radius;
        bool active;
        bool marked;
    } Light_meta;

    extern Light lights[_MAX_LIGHTS_]; // Always same order
    extern Light_meta light_meta[_MAX_LIGHTS_]; // Metadata about lights such as radius, on/off.
    extern std::vector<GLuint> shader_programs; // For UBO binding on init.
    extern int num_lights;
    extern int culled_lights;

    int create_light(glm::vec3 position, float brightness, glm::vec3 color);
    void destroy_light(int index);
    void init();
    void cull_light_sources(Camera &camera);
    void upload_lights();
    void turn_off_all_lights();
    void turn_on_all_lights();
    void turn_on_one_light();


    namespace Detail {
        const int light_size = 40;
        const int info_size = 4;
        extern Light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
        extern std::vector<int> available_indices;
        extern GLuint ubos[2];
        extern int next_to_turn_on;

        void generate_bounding_sphere(int light);
    }
}
#endif
