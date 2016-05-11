#include "main.hpp"


void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void cull_models()
{
    // TODO: Run in parallel
    for (auto model : Model::get_loaded_models()) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius * model->scale);
    }
    for (auto model : Model::get_loaded_flat_models()) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius * model->scale);
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

    bool tweak_bar = true;
    renderer.init(tweak_bar);

    renderer.init_uniforms(camera);

    load_scene(get_scene_file_from_command_line(argc, argv));


    Light::upload_all();

    run();

    free_resources();
    TwTerminate();
    return 0;
}
