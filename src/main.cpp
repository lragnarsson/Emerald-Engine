#include "main.hpp"
#include "Camera.hpp"

// --------------------------

void init_uniforms()
{
    camera = new Camera(glm::vec3(0.f, 0.f, 3.f),    // Position
                        glm::vec3(0.f, 0.f, -1.f),   // Front
                        glm::vec3(0.0f, 1.0f, 0.0f), // Up
                        glm::vec3(1.f, 0.f, 0.f),    // Right
                        0.3f,                        // Speed
                        0.01f);                      // Rotational speed

    w2v_matrix = glm::lookAt(camera->position, camera->position + camera->front, camera->up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

}

// --------------------------

void free_resources()
{
    sdl_quit(main_window, main_context);
    delete camera;
}

// --------------------------

void run()
{
    bool loop = true;

    while (loop) {
        handle_keyboard_input(camera, loop);
        handle_mouse_input(camera);
        camera->update_culling_frustum();

        w2v_matrix = glm::lookAt(camera->position, camera->position + camera->front, camera->up);
        glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : loaded_models) {
            model->draw_me = camera->sphere_in_frustum(model->bounding_sphere_center, model->bounding_sphere_radius);
        }

        for (auto model : loaded_models) {
            if (model->draw_me) {
                model->draw(shader_forward);
            }
        }

        glBindVertexArray(0);
        SDL_GL_SwapWindow(main_window);
    }
}

// --------------------------

int main(int argc, char *argv[])
{

    if (!sdl_init(SCREEN_WIDTH, SCREEN_HEIGHT, main_window, main_context)) {
        return 1;
    }
    init_input();

    // Initiate shaders
    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");

    init_uniforms();

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.1f, glm::vec3(1.f));
    glm::mat4 m2w = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f)) * rot;
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", shader_forward, rot, m2w));

    // Load light sources into GPU
    new Light(glm::vec3(-2.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f));
    new Light(glm::vec3(1.f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f));
    Light::upload_all(shader_forward);

    run();

    free_resources();
    return 0;
}

