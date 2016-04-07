#include "Utils.hpp"
#include "Model.hpp"

GLuint shader_forward, shader_geometry, shader_deferred;
GLuint screen_width = 800;
GLuint screen_height = 640;
SDL_Window* main_window;
SDL_GLContext main_context;

glm::vec3 camera_pos = glm::vec3(0.f, 0.f, 3.f);
glm::vec3 camera_front = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 camera_rot;

float camera_speed = 0.1f;
float camera_rot_speed = 0.01f;

glm::mat4 w2v_matrix;
glm::mat4 projection_matrix;


std::vector<Model*> loaded_models;
std::vector<Light*> loaded_lights;


void run_game();

int main(int argc, char *argv[])
{

    if (!sdl_init(screen_width, screen_height, main_window, main_context)) {
        return 1;
    }

    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.1f, glm::vec3(1.f));
    glm::mat4 m2w = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f)) * rot;
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", shader_forward, rot, m2w));

    loaded_lights.push_back(new Light(glm::vec3(-2.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f)));
    loaded_lights.push_back(new Light(glm::vec3(1.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f)));
    upload_lights(shader_forward, loaded_lights);

    // Create and upload view uniforms:
    w2v_matrix = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
    projection_matrix = glm::perspective(45.0f, (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    GLint view = glGetUniformLocation(shader_forward, "view");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    GLint projection = glGetUniformLocation(shader_forward, "projection");
    glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));


    run_game();

    sdl_quit(main_window, main_context);
    return 0;
}


void run_game() {
    bool loop = true;

    while (loop) {
        SDL_Event event;
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, camera_up));

        if(keystate[SDL_GetScancodeFromKey(SDLK_w)]) {
            camera_pos += camera_speed * camera_front;
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_s)]) {
            camera_pos -= camera_speed * camera_front;
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_a)]) {
            camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_d)]) {
            camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_q)]) {
            camera_rot = glm::rotate(glm::mat4(1.0f),
                                 camera_rot_speed,
                                 glm::vec3(0.f, 1.f, 0.f) //camera_up
                                 );
            camera_front = glm::vec3(camera_rot * glm::vec4(camera_front, 1.f));
            camera_up = glm::vec3(camera_rot * glm::vec4(camera_up, 1.f));
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_e)]) {
            camera_rot = glm::rotate(glm::mat4(1.0f),
                                 -camera_rot_speed,
                                 glm::vec3(0.f, 1.f, 0.f) //camera_up
                                 );
            camera_front = glm::vec3(camera_rot * glm::vec4(camera_front, 1.f));
            camera_up = glm::vec3(camera_rot * glm::vec4(camera_up, 1.f));
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_f)]) {
            camera_right = glm::normalize(glm::cross(camera_front, camera_up));
            camera_rot = glm::rotate(glm::mat4(1.0f),
                                 -camera_rot_speed,
                                 camera_right
                                 );
            camera_front = glm::vec3(camera_rot * glm::vec4(camera_front, 1.f));
            camera_up = glm::vec3(camera_rot * glm::vec4(camera_up, 1.f));
        }
        if(keystate[SDL_GetScancodeFromKey(SDLK_r)]) {
            camera_right = glm::normalize(glm::cross(camera_front, camera_up));
            camera_rot = glm::rotate(glm::mat4(1.0f),
                                 camera_rot_speed,
                                 camera_right
                                 );
            camera_front = glm::vec3(camera_rot * glm::vec4(camera_front, 1.f));
            camera_up = glm::vec3(camera_rot * glm::vec4(camera_up, 1.f));
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

        w2v_matrix = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
        GLint view = glGetUniformLocation(shader_forward, "view");
        glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : loaded_models) {
            model->draw(shader_forward);
        }

        glBindVertexArray(0);

        SDL_GL_SwapWindow(main_window);
    }
}
