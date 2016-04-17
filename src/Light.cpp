#include "Light.hpp"

std::vector<Light*> Light::lights;


Light::Light(const glm::vec3 world_coord, const glm::vec3 ambient_color,
             const glm::vec3 diffuse_color, const glm::vec3 specular_color)
{
    this->position = world_coord;
    this->ambient_color = ambient_color;
    this->diffuse_color = diffuse_color;
    this->specular_color = specular_color;
    lights.push_back(this);
    this->id = lights.size() - 1;
}

void Light::upload(const GLuint shader_program)
{
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].position").c_str()), 1,
                 glm::value_ptr(lights[this->id]->position));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].ambientColor").c_str()), 1,
                 glm::value_ptr(lights[this->id]->ambient_color));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].diffuseColor").c_str()), 1,
                 glm::value_ptr(lights[this->id]->diffuse_color));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].specularColor").c_str()), 1,
                     glm::value_ptr(lights[this->id]->specular_color));

    glUniform1i(glGetUniformLocation(shader_program, "nLights"), lights.size());
}


void Light::upload_all(const GLuint shader_program)
{
    glUseProgram(shader_program);
    for (int i = 0; i < lights.size(); i++) {
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].position").c_str()), 1,
                     glm::value_ptr(lights[i]->position));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].ambientColor").c_str()), 1,
                     glm::value_ptr(lights[i]->ambient_color));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].diffuseColor").c_str()), 1,
                     glm::value_ptr(lights[i]->diffuse_color));
        glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].specularColor").c_str()), 1,
                     glm::value_ptr(lights[i]->specular_color));
    }
    glUniform1i(glGetUniformLocation(shader_program, "nLights"), lights.size());
    glUseProgram(0);
}

glm::vec3 Light::get_color() {
    return this->ambient_color; // this should be changed soon Hans-Filip
}

void Light::move_to(glm::vec3 world_coord) {
    this->position = world_coord;
    
}

void Light::upload_pos(const GLuint shader_program) {
    glUseProgram(shader_program);
    const char* name = ("lights[" + std::to_string(this->id) + "].position").c_str();
    GLuint pos_loc = glGetUniformLocation(shader_program, name);                                
    glUniform3fv(pos_loc, 1, glm::value_ptr(this->position));
    glUseProgram(0);
}
