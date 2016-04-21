#include "main.hpp"
#include "Camera.hpp"




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

        
        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cull_models();
        renderer.upload_camera_uniforms(camera);
        
        // This is a call to our renderers member function pointer called render_function
        (renderer.*renderer.render_function)(loaded_models, loaded_flat_models);

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

    renderer.init();

    renderer.init_uniforms(camera);
    
    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.3f, glm::vec3(0.f, 1.f, 0.f));

    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", rot, glm::vec3(-5.f)));

    
    glm::vec3 p1 = glm::vec3(1.f);
    glm::vec3 p2 = glm::vec3(-3.f, 10.f, 2.f);
    Model* box1 = new Model("res/models/cube/cube.obj", glm::mat4(1.f), p1);
    Model* box2 = new Model("res/models/cube/cube.obj", glm::mat4(1.f), p2);
    
    loaded_flat_models.push_back(box1);
    loaded_flat_models.push_back(box2);
    
    // Load light sources into GPU
    Light light1 = Light(p1, glm::vec3(1.f));
    Light light2 = Light(p2, glm::vec3(1.f, 0.5f, 0.f));

    // attach light sources to boxes
    box1->attach_light(&light1, glm::vec3(0.0f));
    box2->attach_light(&light2, glm::vec3(0.0f));
      
    
    Light::upload_all();

    run();

    free_resources();
    return 0;
}

