#include "Utils.hpp"
#include "Model.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Error.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "Parser.hpp"
#include "CR_Spline.hpp"
#include "Animation_Path.hpp"
#include "Profiler.hpp"
#include "Terrain.hpp"

SDL_Window* main_window;
SDL_GLContext main_context;

Camera camera = Camera(glm::vec3(40.f, 26.f, 50.f),    // Position
                       glm::vec3(0.f, 0.f, -1.f),   // Front
                       glm::vec3(0.0f, 1.0f, 0.0f), // Up
                       glm::vec3(1.f, 0.f, 0.f),    // Right
                       0.01f,                        // Speed
                       0.0001f);                      // Rotational speed

glm::mat4 w2v_matrix;
glm::mat4 projection_matrix;

Renderer renderer;

#ifdef PROFILING
Profiler perf_monitor;
#endif
