#include "Utils.hpp"
#include "Model.hpp"
#include "Input.hpp"
#include "Containers.hpp"
#include "Light.hpp"

GLuint shader_forward, shader_geometry, shader_deferred;
GLuint screen_width = 800;
GLuint screen_height = 640;
SDL_Window* main_window;
SDL_GLContext main_context;

Camera camera = {glm::vec3(0.f, 0.f, 3.f),    // Position
                 glm::vec3(0.f, 0.f, -1.f),   // Front
                 glm::vec3(0.0f, 1.0f, 0.0f), // Up
                 glm::vec3(1.f, 0.f, 0.f),    // Right
                 0.3f,                        // Speed
                 0.01f,                       // Rotational speed
};

glm::mat4 w2v_matrix;
glm::mat4 projection_matrix;

std::vector<Model*> loaded_models;
