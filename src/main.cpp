#include "main.hpp"
#include "Camera.hpp"


// --------------------------

void init_uniforms()
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (int i = 0; i < renderer.current_shaders.size(); i ++) {
        glUniformMatrix4fv(glGetUniformLocation(renderer.current_shaders[i], "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniformMatrix4fv(glGetUniformLocation(renderer.current_shaders[i], "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    }

}

// --------------------------

void init_renderer()
{
    renderer.shader_deferred = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    renderer.shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");

    renderer.set_forward();
}

// --------------------------

void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void cull_models()
{
    // TODO: Run in parallel
    for (auto model : loaded_models) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius);
    }
}

// --------------------------

void run()
{
    renderer.running = true;
    while (renderer.running) {
        handle_keyboard_input(camera, renderer);
        handle_mouse_input(camera);
        camera.update_culling_frustum();

        glUseProgram(renderer.current_shaders[0]);
        w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        glUniformMatrix4fv(glGetUniformLocation(renderer.current_shaders[0], "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cull_models();

        (renderer.*renderer.render_function)(loaded_models);

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

    init_renderer();

    init_uniforms();

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.3f, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 m2w = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f)) * rot;
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", renderer.current_shaders[0], rot, m2w));

    // Load light sources into GPU
    Light light1 = Light(glm::vec3(-5.f, 5.f, -10.f), glm::vec3(1.f));
    Light light2 = Light(glm::vec3(1.f, 5.f, 5.f), glm::vec3(1.f));
    Light::upload_all();

    run();

    free_resources();
    return 0;
}

