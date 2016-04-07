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

// --------------------------

void run_game() {
    bool loop = true;

    while (loop) {
        handle_keyboard_input(camera, loop);
        handle_mouse_input(camera);

        w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
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

// --------------------------

int main(int argc, char *argv[])
{

    if (!sdl_init(screen_width, screen_height, main_window, main_context)) {
        return 1;
    }
    init_input();

    // Initiate shaders
    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.1f, glm::vec3(1.f));
    glm::mat4 m2w = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f)) * rot;
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", shader_forward, rot, m2w));

    // Load light sources into GPU
    new Light(glm::vec3(-2.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f));
    new Light(glm::vec3(1.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f));
    Light::upload_all(shader_forward);

    // Create and upload view uniforms:
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(45.0f, (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    GLint view = glGetUniformLocation(shader_forward, "view");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    GLint projection = glGetUniformLocation(shader_forward, "projection");
    glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));


    run_game();

    sdl_quit(main_window, main_context);
    return 0;
}
