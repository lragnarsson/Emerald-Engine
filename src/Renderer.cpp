#include "Renderer.hpp"


void Renderer::init()
{
    shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_geometry = load_shaders("src/shaders/geometry.vert", "src/shaders/geometry.frag");
    /*shader_deferred = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_flat = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_pos = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_normal = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_albedo = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shader_specular = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");*/


    set_forward();
    init_g_buffer();
}


void Renderer::set_deferred()
{
    this->render_function = &Renderer::render_deferred;
    this->current_shaders.clear();
    current_shaders.push_back(shader_geometry);
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
    this->current_shaders.clear();
    current_shaders.push_back(shader_pos);
}


void Renderer::set_g_normal()
{
    this->render_function = &Renderer::render_g_normal;
    this->current_shaders.clear();
    current_shaders.push_back(shader_normal);
}


void Renderer::set_g_albedo()
{
    this->render_function = &Renderer::render_g_albedo;
    this->current_shaders.clear();
    current_shaders.push_back(shader_albedo);
}


void Renderer::set_g_specular()
{
    this->render_function = &Renderer::render_g_specular;
    this->current_shaders.clear();
    current_shaders.push_back(shader_specular);
}

// --------------------------

void Renderer::init_g_buffer()
{
    glUseProgram(shader_geometry);

    glDisable(GL_BLEND);
    glGenFramebuffers(1, &g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

    /* Position buffer */
    glGenTextures(1, &g_position);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

    /* Normal buffer */
    glGenTextures(1, &g_normal);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

    /* Normal and Specular buffer*/
    glGenTextures(1, &g_albedo_specular);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_specular, 0);

    /* Specify color attachments of the buffer */
    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0,
                             GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    /* Attach a depth buffer */
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);


    std::cout << "normal buffer: " << g_normal << std::endl;
    std::cout << "pos buffer: " << g_position << std::endl;
    std::cout << "as buffer: " << g_albedo_specular << std::endl;
    std::cout << "depth buffer: " << depth_buffer << std::endl;


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream error_msg;
        error_msg << "GL enum: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Error::throw_error(Error::renderer_init_fail, error_msg.str());
        }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// --------------------------

void Renderer::render_deferred(std::vector<Model*> &loaded_models)
{
    glUseProgram(shader_geometry);
    for (auto model : loaded_models) {
        if (model->draw_me) {
            model->draw_geometry(shader_geometry);
        }
    }
    glUseProgram(shader_deferred);

    glUseProgram(0);
}

// --------------------------

void Renderer::render_forward(std::vector<Model*> &loaded_models)
{
    glUseProgram(shader_forward);
    for (auto model : loaded_models) {
        if (model->draw_me) {
            model->draw_forward(current_shaders[0]);
        }
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_position(std::vector<Model*> &loaded_models)
{
    glUseProgram(shader_pos);
    for (auto model : loaded_models) {
        if (model->draw_me) {
            model->draw_forward(shader_pos);
        }
    }
    glUseProgram(0);
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
