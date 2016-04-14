#include "Light.hpp"

std::vector<Light*> Light::lights;


Light::Light(const glm::vec3 world_coord, const glm::vec3 color)
{
  this->position = world_coord;
  this->color = color;
  this->active_light = true;
  lights.push_back(this);
  this->id = lights.size() - 1;
}

void Light::upload(const GLuint shader_program)
{
  glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].position").c_str()), 1,
  glm::value_ptr(this->position));
  glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].color").c_str()), 1,
  glm::value_ptr(this->color));
  glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].active_light").c_str()),
  this->active_light);
}


void Light::upload_all(const GLuint shader_program)
{
  for (int i = 0; i < lights.size(); i++) {
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].position").c_str()), 1,
    glm::value_ptr(lights[i]->position));
    glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].color").c_str()), 1,
    glm::value_ptr(lights[i]->color));
    glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(i) + "].active_light").c_str()),
    lights[i]->active_light);
  }
}
