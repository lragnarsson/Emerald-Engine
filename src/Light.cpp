#include "Light.hpp"


std::vector<GLuint> Light::shader_programs;
int Light::culled_lights;
std::vector<Light*> Light::lights;
gpu_light Light::gpu_lights[_MAX_LIGHTS_];
std::vector<unsigned int> Light::free_ids;
GLuint Light::ubos[2];
int Light::next_to_turn_on;


Light::Light(const glm::vec3 position,
             const float brightness, const glm::vec3 color)
{
    this->position = position;
    this->color = color;
    this->brightness = brightness;
    this->active = true;

    // Check if there are free places in the vector for lights
    if (free_ids.empty()) {
        lights.push_back(this);
        this->id = lights.size() - 1;
    } else {
        this->id = free_ids.back();
        free_ids.pop_back();
        lights[id] = this;
    }

    generate_bounding_sphere();
}


Light::~Light()
{
    this->color = glm::vec3(0);
    this->active = false;
    free_ids.push_back(this->id);
    lights[this->id] = nullptr;
}


void Light::upload_lights()
{
    int visible_lights = lights.size() - culled_lights;

    glBindBuffer(GL_UNIFORM_BUFFER, ubos[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    light_size * visible_lights, gpu_lights);

    glBindBuffer(GL_UNIFORM_BUFFER, ubos[1]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, info_size, &visible_lights);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void Light::init()
{
    for (auto shader : shader_programs) {
        GLuint light_index = glGetUniformBlockIndex(shader, "light_block");
        glUniformBlockBinding(shader, light_index, 0);
        GLuint info_index = glGetUniformBlockIndex(shader, "light_info_block");
        glUniformBlockBinding(shader, info_index, 1);
    }

    glGenBuffers(2, ubos);
    glBindBuffer(GL_UNIFORM_BUFFER, ubos[0]);
    glBufferData(GL_UNIFORM_BUFFER, light_size * _MAX_LIGHTS_,
                 NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, ubos[1]);
    glBufferData(GL_UNIFORM_BUFFER, info_size, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubos[0]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubos[1]);

    turn_on_all_lights();
}


void Light::cull_light_sources(Camera &camera)
{
    Profiler::start_timer("Cull lights");
    int num_visible = 0;
    culled_lights = 0;
    for (auto light : lights) {
        // Use lights which are turned on or inside viewing frustum:
        if (light->active &&
            camera.sphere_in_frustum(light->position,
                                     light->radius)) {
            // Move visible lights to gpu array:
            // Position in view-space:
            glm::vec3 view_space_pos = glm::vec3(camera.get_view_matrix() *
                                                 glm::vec4(light->position, 1.f));
            gpu_lights[num_visible].position = view_space_pos;
            gpu_lights[num_visible].brightness = light->brightness;
            gpu_lights[num_visible].color = light->color;
            num_visible++;
        } else {
            culled_lights++;
        }
    }
    Profiler::stop_timer("Cull lights");
}

void Light::turn_off_all_lights()
{
    for (auto light : lights) {
        light->active = false;
    }
}


void Light::turn_on_all_lights()
{
    for (auto light : lights) {
        light->active = true;
    }
}


void Light::turn_on_one_light()
{
    if (next_to_turn_on < lights.size()) {
        lights[next_to_turn_on]->active = true;
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
    float beta = glm::length(this->color) * this->brightness;

    // Solve quadratic equation to determine at what distance the light is dimmer than alpha times beta:
    this->radius = -b / (2 * c) + std::sqrt(b * b / (4 * c * c) - a / c + beta / (alpha * c));
}
