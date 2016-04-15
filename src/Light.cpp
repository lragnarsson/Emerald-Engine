#include "Light.hpp"

std::vector<Light*> Light::lights;
std::vector<unsigned int> Light::free_ids;
GLuint Light::shader_program;

// ------------
// Construct and destruct

Light::Light(const glm::vec3 world_coord, const glm::vec3 color, GLuint shader_program)
{
  this->position = world_coord;
  this->color = color;
  this->active_light = true;
  this->shader_program = shader_program;

  // Check if there are free places in the vector for lights
  if (free_ids.empty()) {
    lights.push_back(this);
    this->id = lights.size() - 1;
  }
  else {
    this->id = free_ids.pop_back();
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
  glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].position").c_str()), 1,
  glm::value_ptr(this->position));
  glUniform3fv(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].color").c_str()), 1,
  glm::value_ptr(this->color));
  glUniform1i(glGetUniformLocation(shader_program, ("lights[" + std::to_string(this->id) + "].active_light").c_str()),
  this->active_light);
}


void Light::upload_all()
{
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
}

// ------------
// Changes color of light

void Light::set_color(glm::vec3 color)
{
  this->color = color;
  this->upload();
}

glm::vec3 Light::get_color()
{
  return this->color;
}
