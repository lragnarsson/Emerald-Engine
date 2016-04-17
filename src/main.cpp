#include "main.hpp"
#include "Camera.hpp"

// --------------------------

void init_uniforms()
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

}

// --------------------------

void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void run() {
    state.running = true;
    while (state.running) {
        handle_keyboard_input(camera, state);
        handle_mouse_input(camera);
        camera.update_culling_frustum();

        w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : loaded_models) {
            model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius);
        }

        switch(state.current_render_mode) {
        case FORWARD:
            for (auto model : loaded_models) {
                model->draw_forward(shader_forward);
            }
            break;
        case DEFERRED:
            for (auto model : loaded_models) {
                model->draw_deferred(shader_forward);
            }
            break;
        }

        glBindVertexArray(0);
        SDL_GL_SwapWindow(main_window);
    }
}

// --------------------------

int main(int argc, char *argv[])
{
    if (!sdl_init(SCREEN_WIDTH, SCREEN_HEIGHT, main_window, main_context)) {
        Error::throw_error(Error::display_init_fail);
    }
    init_input();

    // Initiate shaders
    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");

    init_uniforms();

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.3f, glm::vec3(0.f, 1.f, 0.f));
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
