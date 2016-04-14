#include "Input.hpp"

void init_input()
{
  // Mouse init
  SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void handle_keyboard_input(Camera &camera, bool &loop, glm::vec3 &dir)
{
    SDL_Event event;
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    if(keystate[SDL_GetScancodeFromKey(SDLK_w)]) {
        camera.position += camera.speed * camera.front;
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_s)]) {
        camera.position -= camera.speed * camera.front;
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_a)]) {
        camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_d)]) {
        camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;
    }

    
    if(keystate[SDL_GetScancodeFromKey(SDLK_UP)]) {
        dir = dir + glm::vec3(0.f, 0.f, 1.f);
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
        dir = dir + glm::vec3(1.f, 0.f, 0.f);
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
        dir = dir + glm::vec3(-1.f, 0.f, 0.f);
    }
    if(keystate[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
        dir = dir + glm::vec3(0.f, 0.f, -1.f);
    }
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            loop = false;
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                loop = false;
                break;
            }
        }
    }
}

void handle_mouse_input(Camera &camera)
{
  int dx,dy;
  unsigned int button_state;

  button_state = SDL_GetRelativeMouseState(&dx, &dy);

  camera.front = glm::rotate(camera.front, -dy*camera.rot_speed, camera.right); // pitch
  camera.front = glm::rotateY(camera.front, -dx*camera.rot_speed); // yaw
  camera.right = glm::cross(camera.front, camera.up);
}
