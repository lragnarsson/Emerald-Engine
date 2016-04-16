#include "Utils.hpp"
#include "Model.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Light.hpp"

GLuint shader_forward, shader_geometry, shader_deferred;

SDL_Window* main_window;
SDL_GLContext main_context;

Camera* camera;

glm::mat4 w2v_matrix;
glm::mat4 projection_matrix;

std::vector<Model*> loaded_models;
