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


class Light
{
public:
    static void upload_all();
    static GLuint shader_program;

    float bounding_sphere_radius = -1.f;

    Light(const glm::vec3 world_coord, const glm::vec3 color);
    ~Light();

    void upload();
    glm::vec3 get_color();
    void set_color(glm::vec3 color);
    glm::vec3 get_pos();
    void move_to(glm::vec3 world_coord); // does not upload data
    bool is_active() {return this->active_light;}
    static uint* get_number_of_culled_lightsources() {return &culled_number;}
    static void cull_light_sources(Camera &camera);
    static int get_number_of_lightsources() {return lights.size();}
    static void turn_off_all_lights();
    static void turn_on_one_lightsource();

private:
    unsigned int id;
    glm::vec3 position, color;
    GLboolean active_light, inside_frustum;

    static unsigned int next_to_turn_on, culled_number;
    static std::vector<Light*> lights;
    static std::vector<unsigned int> free_ids;

    void generate_bounding_sphere();
};



#endif
