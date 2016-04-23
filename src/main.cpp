#include "main.hpp"
#include "Camera.hpp"


void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void cull_models()
{
    // TODO: Run in parallel
    for (auto model : Model::get_loaded_models()) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius);
    }
    for (auto model : Model::get_loaded_flat_models()) {
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

        cull_models();
        renderer.upload_camera_uniforms(camera);

        renderer.render();
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

    load_scene("scene_file.txt");

    Light::upload_all();

    run();

    free_resources();
    return 0;
}
