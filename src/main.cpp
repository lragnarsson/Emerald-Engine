#include "main.hpp"


void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void cull_models()
{
    // TODO: Run in parallel
    uint i = 0;
    for (auto model : Model::get_loaded_models()) {
        bool draw_me = camera.sphere_in_frustum(model->get_center_point_world(), model->bounding_sphere_radius * model->scale);
        model->draw_me = draw_me;
       if (draw_me)
           i++;
    }
    for (auto model : Model::get_loaded_flat_models()) {
        bool draw_me = camera.sphere_in_frustum(model->get_center_point_world(), model->bounding_sphere_radius * model->scale);
        model->draw_me = draw_me;
        if (draw_me)
            i++;
    }
    renderer.objects_drawn = i;
}

// --------------------------

void animate_models()
{
    float elapsed_time = 0.1f; // Should be calculated properly so it depends on FPS
    // TODO: Run in parallel
    for (auto model : Model::get_loaded_models()) {
        if (model->has_animation_path()) {
            model->move_along_path(elapsed_time);
        }
    }
    for (auto model : Model::get_loaded_flat_models()) {
        if (model->has_animation_path()) {
            model->move_along_path(elapsed_time);
        }
    }
}

// --------------------------

// Assumes that the scene is loaded
// If there are any animation paths, the camera will follow the last animation path
void init_camera_anim_path()
{
    int number_of_anim_paths = Animation_Path::get_number_of_animation_paths();
    int end_id = number_of_anim_paths - 1;

    // There is only one animation path, use this for move and look path
    if (end_id == 0) {
        camera.attach_move_animation_path(end_id, 0.f);
        camera.attach_look_animation_path(end_id, 10.f);
    }
    if (number_of_anim_paths > 1) {
        camera.attach_move_animation_path(end_id, 0.f);
        camera.attach_look_animation_path(end_id - 1, 5.f);
    }
}


void run()
{
    renderer.running = true;
    while (renderer.running) {
        handle_keyboard_input(camera, renderer);
        handle_mouse_input(camera);
        camera.update_culling_frustum();


        if (!camera.can_move_free()) {
            camera.move_along_path(0.1f);
        }
        if (!camera.can_look_free()) {
            camera.move_look_point_along_path(0.1f);
        }
        renderer.copy_tweak_bar_cam_values(camera);
        
        animate_models();
        cull_models();

        renderer.render(camera);

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
    renderer.init_tweak_bar(&camera);

    Loader::load_scene(Parser::get_scene_file_from_command_line(argc, argv));

    init_camera_anim_path();
    
    Light::upload_all();


    run();

    TwTerminate();
    free_resources();
    return 0;
}
