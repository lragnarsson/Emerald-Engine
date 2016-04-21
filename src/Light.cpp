#include "Light.hpp"

std::vector<Light*> Light::lights;
std::vector<unsigned int> Light::free_ids;
GLuint Light::shader_program;

// ------------
// Construct and destruct

Light::Light(const glm::vec3 world_coord, const glm::vec3 color)
{
    this->position = world_coord;
    this->color = color;
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
}

Light::~Light()
{
    this->color = glm::vec3(0);
    this->active_light = false;
    this->upload();
    free_ids.push_back(this->id);
    lights[this->id] = nullptr;
}

// ------------
// Uploads to GPU

void Light::upload()
{
    glUseProgram(shader_program);
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].position").c_str()), 1,
                 glm::value_ptr(this->position));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].color").c_str()), 1,
                 glm::value_ptr(this->color));
    glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].active_light").c_str()),
                this->active_light);
    glUseProgram(0);
}


void Light::upload_all()
{
    glUseProgram(shader_program);
    for (int i = 0; i < lights.size(); i++) {
        if (lights[i] != nullptr) {
            glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].position").c_str()), 1,
                         glm::value_ptr(lights[i]->position));
            glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].color").c_str()), 1,
                         glm::value_ptr(lights[i]->color));
            glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].active_light").c_str()),
                        lights[i]->active_light);
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

void Light::upload_pos()
{
    glUseProgram(shader_program);
    const char* name = ("lights[" + std::to_string(this->id) + "].position").c_str();
    GLuint pos_loc = glGetUniformLocation(shader_program, name);
    glUniform3fv(pos_loc, 1, glm::value_ptr(this->position));
    glUseProgram(0);
}

// ------------
// Changes color of light

void Light::set_color(glm::vec3 color)
{
    this->color = color;
    this->upload();
}
