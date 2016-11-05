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


typedef struct {
    glm::vec3 position; // 12 +
    float brightness;   // 4 +
    glm::vec3 color;    // 12 +
    float padding;      // 4 = 40 bytes
} gpu_light;

class Light
{
public:
    static std::vector<GLuint> shader_programs; // For UBO binding on init.
    static int culled_lights;
    glm::vec3 position;
    float brightness;
    glm::vec3 color;
    bool active;
    float radius;

    Light(const glm::vec3 position,
          const float brightness, const glm::vec3 color);
    ~Light();

    static void init();
    static void cull_light_sources(Camera &camera);
    static void upload_lights();
    static void turn_off_all_lights();
    static void turn_on_all_lights();
    static void turn_on_one_light();
    static int get_num_lights() { return lights.size(); }
    static int get_culled_lights() { return culled_lights; }

private:
    static const int light_size = 40;
    static const int info_size = 4;
    static std::vector<Light*> lights;
    static gpu_light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
    static std::vector<unsigned int> free_ids;
    static GLuint ubos[2];
    static int next_to_turn_on;

    int id;

    void generate_bounding_sphere();

};
#endif
