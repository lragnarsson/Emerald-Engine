#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include "Camera.hpp"

void init_input();
void handle_keyboard_input(Camera &camera, bool &loop);
void handle_mouse_input(Camera &camera);

#endif
