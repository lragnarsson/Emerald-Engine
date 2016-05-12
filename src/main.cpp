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
        model->draw_me = camera.sphere_in_frustum(model->get_center_point_world(), model->bounding_sphere_radius * model->scale);
    }
    for (auto model : Model::get_loaded_flat_models()) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point_world(), model->bounding_sphere_radius * model->scale);
    }
}

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
    
void create_animation_paths()
{
    // This is just debug. Should be done in load_scene
    std::vector<glm::vec3> points;

    glm::vec3 p1 = glm::vec3(3.0f, 10.0f, 1.0f);
    glm::vec3 p2 = glm::vec3(-2.0f, 8.0f, -10.0f);
    glm::vec3 p3 = glm::vec3(-2.0f, 7.0f, -7.0f);
    glm::vec3 p4 = glm::vec3(-5.0f, 7.0f, -10.f);
    glm::vec3 p5 = glm::vec3(-1.0f, 10.0f, -8.0f);
    glm::vec3 p6 = glm::vec3(-3.0f, 5.0f, -1.0f);

    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    points.push_back(p6);

    Animation_Path* a_path = new Animation_Path(points, 10.0f);

    // Attaches some lightsources to the animation path
    std::vector<Model*> models = Model::get_loaded_flat_models();
    models[0]->attach_animation_path(a_path, 0.0f);
    models[1]->attach_animation_path(a_path, 1.0f);
    models[2]->attach_animation_path(a_path, 2.0f);
    models[3]->attach_animation_path(a_path, 3.0f);
    models[4]->attach_animation_path(a_path, 4.0f);
    models[5]->attach_animation_path(a_path, 5.0f);
    
    // END DEBUG
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
        animate_models();
        
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

    load_scene(get_scene_file_from_command_line(argc, argv));

    create_animation_paths();
    
    Light::upload_all();

    
    run();

    TwTerminate();
    free_resources();
    return 0;
}
