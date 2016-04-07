#include "Input.hpp"

void handle_keyboard_input(Camera &camera, bool &loop)
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
