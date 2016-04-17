#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include "Containers.hpp"

void init_input();
/* Added temporary dir vector as a way to get more keyboard input for debugging.
   Use arrow keys to change the dir vector. */
void handle_keyboard_input(Camera &camera, bool &loop, glm::vec3 &dir);
void handle_mouse_input(Camera &camera);

#endif
