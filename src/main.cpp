#include "main.hpp"

// --------------------------

void init_uniforms()
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(45.0f, (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    
    glUseProgram(shader_forward);
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_forward, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    
    glUseProgram(flat_shader_forward);
    glUniformMatrix4fv(glGetUniformLocation(flat_shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
    glUniformMatrix4fv(glGetUniformLocation(flat_shader_forward, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
}

// --------------------------

void run() {
    bool loop = true;
    glm::vec3 dir = glm::vec3(0.0f);
    
    while (loop) {
        dir = glm::vec3(0.0f);
        handle_keyboard_input(camera, loop, dir);
        handle_mouse_input(camera);

        /* Temporary way to move the blue box around */
        if (glm::length(dir)  > 0.1f) {
            
            loaded_models[1]->rotate(glm::vec3(0.f, 0.f, 1.f), 0.1f);
        }
        
        w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        glUseProgram(shader_forward);
        glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniform3fv(glGetUniformLocation(shader_forward, "camPos"), 1, glm::value_ptr(camera.position));
        
        glUseProgram(flat_shader_forward);
        glUniformMatrix4fv(glGetUniformLocation(flat_shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        
        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Positions might have changed for lights */
        Light::upload_all(shader_forward);
        for (auto model : loaded_models) {
            model->draw(model->shader_programs[0]);
        }

        glBindVertexArray(0);
        SDL_GL_SwapWindow(main_window);
    }
}

// --------------------------

int main(int argc, char *argv[])
{
    if (!sdl_init(screen_width, screen_height, main_window, main_context)) {
        return 1;
    }
    init_input();
    
    // Initiate shaders
    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    flat_shader_forward = load_shaders("src/shaders/flatColorShader.vert", "src/shaders/flatColorShader.frag");
    
    
    init_uniforms();

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.1f, glm::vec3(1.f));
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", shader_forward, rot, glm::vec3(-2.0f, -5.0f, -2.0f)));

    // Light positions
    glm::vec3 p1 = glm::vec3(-1.f);
    glm::vec3 p2 = glm::vec3(1.f);
    Model* box1 = new Model("res/models/cube/cube.obj", flat_shader_forward, glm::mat4(1.f), p1);
    Model* box2 = new Model("res/models/cube/cube.obj", flat_shader_forward, glm::mat4(1.f), p2);
    
    loaded_models.push_back(box1);
    loaded_models.push_back(box2);

    box1->shader_programs.push_back(shader_forward);
    box2->shader_programs.push_back(shader_forward);
    // Load light sources into GPU
    box1->attach_light(new Light(p1, glm::vec3(0.1f, 0.2f, 1.0f), glm::vec3(0.8f), glm::vec3(1.f)), glm::vec3(0.0f, 10.0f, 0.0f));
    box2->attach_light(new Light(p2, glm::vec3(0.1f, 1.0f, 0.2f), glm::vec3(0.8f), glm::vec3(1.f)), glm::vec3(0.0f));
    Light::upload_all(shader_forward);

    run();

    sdl_quit(main_window, main_context);
    return 0;
}

