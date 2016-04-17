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
            loaded_models[1]->move( dir );
        }
        
        w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        glUseProgram(shader_forward);
        glUniformMatrix4fv(glGetUniformLocation(shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUseProgram(flat_shader_forward);
        glUniformMatrix4fv(glGetUniformLocation(flat_shader_forward, "view"), 1, GL_FALSE, glm::value_ptr(w2v_matrix));

        
        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    glm::mat4 m2w = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f)) * rot;
    loaded_models.push_back(new Model("res/models/nanosuit/nanosuit.obj", shader_forward, rot, m2w));

    // Light positions
    glm::vec3 p1 = glm::vec3(-2.f);
    glm::vec3 p2 = glm::vec3(1.f);
    m2w = glm::translate(glm::mat4(1.0f), p1);
    Model* box1 = new Model("res/models/cube/cube.obj", flat_shader_forward, glm::mat4(1.f), m2w);
    m2w = glm::translate(glm::mat4(1.0f), p2);
    Model* box2 = new Model("res/models/cube/cube.obj", flat_shader_forward, glm::mat4(1.f), m2w);
    
    loaded_models.push_back(box1);
    loaded_models.push_back(box2);
    
    // Load light sources into GPU
    box1->attach_light(new Light(p1, glm::vec3(0.1f, 0.2f, 1.0f), glm::vec3(0.8f), glm::vec3(1.f)), shader_forward);
    box2->attach_light(new Light(p2, glm::vec3(0.1f, 1.0f, 0.2f), glm::vec3(0.8f), glm::vec3(1.f)), shader_forward);
    Light::upload_all(shader_forward);

    run();

    sdl_quit(main_window, main_context);
    return 0;
}

