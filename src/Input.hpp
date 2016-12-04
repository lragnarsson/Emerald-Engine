#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include <AntTweakBar.h>
#include "Renderer.hpp"
#include "Camera.hpp"
#include "Profiler.hpp"


void init_input();
void handle_keyboard_input(Camera &camera, Renderer &renderer);
void handle_mouse_input(Camera &camera, Renderer &renderer);

#endif
