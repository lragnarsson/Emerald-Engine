#include "Renderer.hpp"


void Renderer::init()
{
    shaders[FORWARD] = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag"); 
    shaders[GEOMETRY] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");
    shaders[DEFERRED] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");
    shaders[FLAT] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");

    set_forward();
}

void Renderer::set_deferred()
{
    this->render_function = &Renderer::render_deferred;
    Light::shader_program = shaders[DEFERRED];
}

void Renderer::set_forward()
{
    this->render_function = &Renderer::render_forward;
    Light::shader_program = shaders[FORWARD];
}

void Renderer::set_g_position()
{
    this->render_function = &Renderer::render_g_position;
}

void Renderer::set_g_normal()
{
    this->render_function = &Renderer::render_g_normal;
}

void Renderer::set_g_albedo()
{
    this->render_function = &Renderer::render_g_albedo;
}

void Renderer::set_g_specular()
{
    this->render_function = &Renderer::render_g_specular;
}


void Renderer::init_uniforms(const Camera &camera)
{
    glm::mat4 projection_matrix;
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (int i = 0; i < 4; i ++) {
        glUseProgram(shaders[i]);
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "view"),
                           1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "projection"),
                           1, GL_FALSE, glm::value_ptr(projection_matrix));
    }
    glUseProgram(0);
}

/* Uploads view matrix and camPos */
void Renderer::upload_camera_uniforms(const Camera &camera)
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    for (int i = 0; i < 4; i ++) {
        glUseProgram(shaders[i]);
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "view"),
                           1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniform3fv(glGetUniformLocation(shaders[i], "camPos"),
                     1, glm::value_ptr(camera.position));
    }
    glUseProgram(0);
}

/* Private Renderer functions */
// --------------------------

void Renderer::render_deferred(std::vector<Model*> &loaded_models,
                               std::vector<Model*> &loaded_flat_models)
{
    glUseProgram(shaders[GEOMETRY]);
    for (auto model : loaded_models) {
        model->draw_deferred(shaders[GEOMETRY]);
    }

    glUseProgram(shaders[DEFERRED]);
    for (auto model : loaded_models) {
        model->draw_deferred(shaders[DEFERRED]);
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::render_forward(std::vector<Model*> &loaded_models,
                              std::vector<Model*> &loaded_flat_models)
{
    glUseProgram(shaders[FORWARD]);
    for (auto model : loaded_models) {
        model->draw_forward(shaders[FORWARD]);
    }
    render_flat(loaded_flat_models);
    glUseProgram(0);
}

void Renderer::render_flat(std::vector<Model*> &loaded_flat_models)
{
    glUseProgram(shaders[FLAT]);
    for (auto model : loaded_flat_models) {
        model->draw_forward(shaders[FLAT]);
    }
}

// --------------------------

void Renderer::render_g_position(std::vector<Model*> &loaded_models,
                                 std::vector<Model*> &loaded_flat_models)
{

}

// --------------------------

void Renderer::render_g_normal(std::vector<Model*> &loaded_models,
                               std::vector<Model*> &loaded_flat_models)
{

}

// --------------------------

void Renderer::render_g_albedo(std::vector<Model*> &loaded_models,
                               std::vector<Model*> &loaded_flat_models)
{

}

// --------------------------

void Renderer::render_g_specular(std::vector<Model*> &loaded_models,
                                 std::vector<Model*> &loaded_flat_models)
{

}
