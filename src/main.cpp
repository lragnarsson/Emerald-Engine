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
// --- TEST CODE ---------
Light* light1;
Light* light2;
// --- END TEST CODE -----

void run()
{
  bool loop = true;

  // --- TEST CODE ---------
  glm::vec3 current_color;
  bool change_lights = true;
  int i = 0;
  // --- END TEST CODE -----

  while (loop) {
    handle_keyboard_input(camera, loop);
    handle_mouse_input(camera);
    camera.update_culling_frustum();

    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

    glClearColor(0.3, 0.3, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- TEST CODE ---------
    if (change_lights) {
      current_color[0] = current_color[0] + 0.01f;
      current_color[1] = current_color[1] + 0.01f;
      current_color[2] = current_color[2] + 0.01f;
      light1->set_color(current_color);
      light2->set_color(current_color);
    }
    else {
      current_color[0] = current_color[0] - 0.01f;
      current_color[1] = current_color[1] - 0.01f;
      current_color[2] = current_color[2] - 0.01f;
      light1->set_color(current_color);
      light2->set_color(current_color);
    }

    if (i > 100) {
      change_lights = !change_lights;
      i = 0;
    }
    i++;
    // --- END TEST CODE -----

    for (auto model : loaded_models) {
      model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius);
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
  light1 = new Light(glm::vec3(-2.f), glm::vec3(0.5f), shader_forward);
  light2 = new Light(glm::vec3(1.f), glm::vec3(0.5f), shader_forward);
  new Light(glm::vec3(-5.f, 5.f, -10.f), glm::vec3(1.f), shader_forward);
  Light::upload_all();

  run();

  free_resources();
  return 0;
}
