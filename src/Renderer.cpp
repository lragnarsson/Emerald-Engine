#include "Renderer.hpp"


void Renderer::set_deferred()
{
    this->render_function = &Renderer::render_deferred;
    this->current_shaders.clear();
    current_shaders.push_back(shader_deferred);
    Light::shader_program = shader_deferred;
}

void Renderer::set_forward()
{
    this->render_function = &Renderer::render_forward;
    this->current_shaders.clear();
    current_shaders.push_back(shader_forward);
    Light::shader_program = shader_forward;
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


// --------------------------

void Renderer::render_deferred(std::vector<Model*> &loaded_models)
{
    glUseProgram(shader_deferred);
    for (auto model : loaded_models) {
        model->draw_deferred(current_shaders[0]);
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::render_forward(std::vector<Model*> &loaded_models)
{
    glUseProgram(shader_forward);
    for (auto model : loaded_models) {
        model->draw_forward(current_shaders[0]);
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_position(std::vector<Model*> &loaded_models)
{

}

// --------------------------

void Renderer::render_g_normal(std::vector<Model*> &loaded_models)
{

}

// --------------------------

void Renderer::render_g_albedo(std::vector<Model*> &loaded_models)
{

}

// --------------------------

void Renderer::render_g_specular(std::vector<Model*> &loaded_models)
{

}
