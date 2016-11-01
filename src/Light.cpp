#include "Light.hpp"
#include <iostream>


std::vector<Light*> Light::lights;
std::vector<unsigned int> Light::free_ids;
GLuint Light::shader_program;
uint Light::culled_number = 0;
unsigned int Light::next_to_turn_on = 0;

// ------------
// Construct and destruct

Light::Light(const glm::vec3 world_coord, const glm::vec3 color)
{
    this->position = world_coord;
    this->color = 1.f * color;
    this->active_light = true;

    // Check if there are free places in the vector for lights
    if (free_ids.empty()) {
        lights.push_back(this);
        this->id = lights.size() - 1;
    }
    else {

        this->id = free_ids.back();
        free_ids.pop_back();
        lights[id] = this;
    }
    generate_bounding_sphere();
}

Light::~Light()
{
    this->color = glm::vec3(0);
    this->active_light = false;
    free_ids.push_back(this->id);
    lights[this->id] = nullptr;
}

// ------------
// Uploads to GPU
// Position is converted to view-space. Lights should be uploaded every frame.

void Light::upload(const Camera &camera)
{
    glUseProgram(shader_program);
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].viewSpacePosition").c_str()), 1,
                 glm::value_ptr(glm::vec3(camera.get_view_matrix() * glm::vec4(this->position, 1.0f))));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].color").c_str()), 1,
                 glm::value_ptr(this->color));
    glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].active_light").c_str()),
                this->active_light && this->inside_frustum);
    glUseProgram(0);
}

// Position is converted to view-space. Lights should be uploaded every frame.
void Light::upload_all(const Camera &camera)
{
    glUseProgram(shader_program);
    for (int i = 0; i < lights.size(); i++) {
        if (lights[i] != nullptr) {
            glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].viewSpacePosition").c_str()), 1,
                         glm::value_ptr(glm::vec3(camera.get_view_matrix() * glm::vec4(lights[i]->position, 1.0f))));
            glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].color").c_str()), 1,
                         glm::value_ptr(lights[i]->color));
            glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].active_light").c_str()),
                        lights[i]->active_light && lights[i]->inside_frustum);
        }
    }
    glUseProgram(0);
}


glm::vec3 Light::get_color()
{
    return this->color;
}


void Light::move_to(glm::vec3 world_coord)
{
    this->position = world_coord;
}

glm::vec3 Light::get_pos()
{
    return this->position;
}

// ------------
// Changes color of light

void Light::set_color(glm::vec3 color)
{
    this->color = color;
}


void Light::turn_off_all_lights()
{
    for (int id = 0; id < lights.size(); id++) {
        // id not in free_ids
        if (std::find(free_ids.begin(), free_ids.end(), id) == free_ids.end()) {
            lights[id]->active_light = false;
        }
    }
}

void Light::cull_light_sources(Camera &camera)
{
    Profiler::start_timer("Light culling");
    int i = 0;
    for (auto light : lights) {
        if (camera.sphere_in_frustum(light->position, light->bounding_sphere_radius)) {
            light->inside_frustum = true;
        } else {
            light->inside_frustum = false;
            i++;
        }
    }
    Light::culled_number = i;
    Profiler::stop_timer("Light culling");
}

void Light::turn_on_one_lightsource()
{
    if (std::find(free_ids.begin(), free_ids.end(), next_to_turn_on) == free_ids.end()) {
        lights[next_to_turn_on]->active_light = true;
    }
    if (++next_to_turn_on == lights.size()) {
        next_to_turn_on = 0;
    }
}

void Light::generate_bounding_sphere()
{
    float a = _ATT_CON_;
    float b = _ATT_LIN_;
    float c = _ATT_QUAD_;
    float alpha = 0.02; // 2 percent
    float beta = glm::length(this->color);

    // Solve quadratic equation to determine at what distance the light is dimmer than alpha times beta:
    this->bounding_sphere_radius = -b / ( 2 * c) + std::sqrt(b * b / (4 * c * c) - a / c + beta / (alpha * c));
}
