#include "Input.hpp"

void init_input()
{
  // Mouse init
  SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  // SSAO controls
  printf("Toggle SSAO with m \nDecrease/Increse number of ssao samples: u/i\nDecrease/Increase SSAO kernel radius: j/k\n");
}

void handle_keyboard_input(Camera &camera, Renderer &renderer)
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
            renderer.running = false;
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                renderer.running = false;
                break;
            case SDLK_1:
                renderer.set_mode(FORWARD_MODE);
                break;
            case SDLK_2:
                renderer.set_mode(DEFERRED_MODE);
                break;
            case SDLK_3:
                renderer.set_mode(POSITION_MODE);
                break;
            case SDLK_4:
                renderer.set_mode(NORMAL_MODE);
                break;
            case SDLK_5:
                renderer.set_mode(ALBEDO_MODE);
                break;
            case SDLK_6:
                renderer.set_mode(SPECULAR_MODE);
                break;
            case SDLK_b:
                renderer.draw_bounding_spheres = !renderer.draw_bounding_spheres;
                break;
            case SDLK_k:
                renderer.set_kernel_radius(renderer.get_kernel_radius() + 0.1f);
                printf("SSAO kernel radius = %f\n", renderer.get_kernel_radius());
                break;
            case SDLK_j:
                if (renderer.get_kernel_radius() > 0.2f) {
                    renderer.set_kernel_radius(renderer.get_kernel_radius() - 0.1f);
                }
                printf("SSAO kernel radius = %f\n", renderer.get_kernel_radius());
                break;
            case SDLK_i:
                if (renderer.get_ssao_n_samples() < MAX_SSAO_SAMPLES)
                    renderer.set_ssao_n_samples(renderer.get_ssao_n_samples() + 1);
                printf("SSAO number of samples: %d\n", renderer.get_ssao_n_samples());
                break;
            case SDLK_u:
                if (renderer.get_ssao_n_samples() > 1)
                    renderer.set_ssao_n_samples(renderer.get_ssao_n_samples() - 1);
                printf("SSAO number of samples: %d\n", renderer.get_ssao_n_samples());
                break;
            case SDLK_m:
                printf("SSAO is now %s\n", renderer.toggle_ssao() ? "ON" : "OFF");
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

  camera.front = glm::rotate(camera.front, -dy*camera.rot_speed, camera.right);    // pitch
  camera.front = glm::normalize(glm::rotateY(camera.front, -dx*camera.rot_speed)); // yaw
  camera.right = glm::cross(camera.front, camera.up);
}
