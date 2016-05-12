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
    // DEBUG test CR_Spline
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

    Animation_Path a_path = Animation_Path(points, 10.0f);
    printf("created animation path \n");
    glm::vec3 pos;
    /*
      printf("4x3: %s\n", (glm::to_string(glm::mat4x3(p1,p2,p3,p4))).c_str());
    printf("(4x3)': %s\n", (glm::to_string(glm::transpose(glm::mat4x3(p1,p2,p3,p4)))).c_str());
    //    printf("CR_matrix: %s\n", (glm::to_string(CR_Spline::CR_matrix)).c_str());

    for  (float u = 0; u<1.05f; u += 0.1f) {
        printf("u_vec: %s\n", (glm::to_string(glm::vec4(std::pow(u,3), std::pow(u,2), u, 1))).c_str());
        pos= CR_Spline::calc_pos_on_spline(u, p1,p2,p3,p4);
        printf("u = %f, pos = %s\n",u, (glm::to_string(pos)).c_str());
        printf("pos (x,y,z): (%f,%f,%f)\n", pos.x,pos.y,pos.z);
        }*/
    for (int i = 0; i < 1000; i++) {
        pos = a_path.get_pos(0.1f);
        printf("pos (x,y,z): (%f,%f,%f)\n", pos.x,pos.y,pos.z);
    }
    
    renderer.running = true;
    while (renderer.running) {
        Model::loaded_flat_models[0]->move_to(a_path.get_pos(0.1f));
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

    TwTerminate();
    free_resources();
    return 0;
}
