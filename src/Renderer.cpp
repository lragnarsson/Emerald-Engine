#include "Renderer.hpp"


void Renderer::init()
{
    shaders[FORWARD] = load_shaders("build/shaders/forward.vert", "build/shaders/forward.frag");
    shaders[GEOMETRY] = load_shaders("build/shaders/geometry.vert", "build/shaders/geometry.frag");
    shaders[DEFERRED] = load_shaders("build/shaders/deferred.vert", "build/shaders/deferred.frag");
    shaders[FLAT] = load_shaders("build/shaders/flat.vert", "build/shaders/flat.frag");
    shaders[SSAO] = load_shaders("build/shaders/ssao.vert", "build/shaders/ssao.frag");

    init_g_buffer();
    init_quad();
    init_ssao();
    init_tweak_bar();

    sphere = new Model("res/models/sphere/sphere.obj");

    set_mode(DEFERRED_MODE);
}

// --------------------------

void Renderer::render()
{
    switch (mode) {
    case FORWARD_MODE:
        render_forward();
        break;
    case DEFERRED_MODE:
        render_deferred();
        break;
    case POSITION_MODE:
        render_g_position();
        break;
    case NORMAL_MODE:
        render_g_normal();
        break;
    case ALBEDO_MODE:
        render_g_albedo();
        break;
    case SPECULAR_MODE:
        render_g_specular();
        break;
    }

    if (use_tweak_bar) {
        count_fps();
        draw_tweak_bar();
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

// --------------------------

void Renderer::set_mode(render_mode mode)
{
    this->mode = mode;
    switch (mode) {
    case FORWARD_MODE:
        Light::shader_program = shaders[FORWARD];
        break;
    case DEFERRED_MODE:
        Light::shader_program = shaders[DEFERRED];
        glClearColor(0, 0, 0, 1.0);
        break;
    case POSITION_MODE:
        Light::shader_program = shaders[DEFERRED];
        break;
    case NORMAL_MODE:
        Light::shader_program = shaders[DEFERRED];
        break;
    case ALBEDO_MODE:
        Light::shader_program = shaders[DEFERRED];
        break;
    case SPECULAR_MODE:
        Light::shader_program = shaders[DEFERRED];
        break;
    }
    Light::upload_all();
}

// --------------------------

void Renderer::init_uniforms(const Camera &camera)
{
    glm::mat4 projection_matrix;
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (int i = 0; i < 5; i ++) {
        glUseProgram(shaders[i]);
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "view"),
                           1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "projection"),
                           1, GL_FALSE, glm::value_ptr(projection_matrix));
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::upload_camera_uniforms(const Camera &camera)
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    for (int i = 0; i < 5; i ++) {
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

void Renderer::render_deferred()
{
    /* GEOMETRY PASS */
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[GEOMETRY]);

    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[GEOMETRY], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));
        GLuint rot_location = glGetUniformLocation(shaders[GEOMETRY], "modelRot");
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(model->rot_matrix));

        for (auto mesh : model->get_meshes()) {
            GLuint diffuse_num = 1;
            GLuint specular_num = 1;

            for(GLuint i = 0; i < mesh->textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                if(mesh->textures[i]->type == DIFFUSE) {
                    const char* str = ("texture_Diffuse" + std::to_string(diffuse_num++)).c_str();
                    GLuint diffuse_loc = glGetUniformLocation(shaders[GEOMETRY], str);
                    glUniform1i(diffuse_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
                }
                else if(mesh->textures[i]->type == SPECULAR) {
                    const char* str2 = ("texture_Specular" + std::to_string(specular_num++)).c_str();
                    GLuint specular_loc = glGetUniformLocation(shaders[GEOMETRY], str2);
                    glUniform1i(specular_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
                }
            }

            glBindVertexArray(mesh->get_VAO());

            /* DRAW GEOMETRY */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);

            for (GLuint i = 0; i < mesh->textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    if (this->ssao_on) {
        render_ssao();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[DEFERRED]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position_depth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    // NEW SSAO texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_result);

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* RENDER FLAT OBJECTS WITH DEPTH BUFFER */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    render_flat();

    if (draw_bounding_spheres) {
        render_bounding_spheres();
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// --------------------------

void Renderer::render_forward()
{
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaders[FORWARD]);
    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[FORWARD], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));
        GLuint rot_location = glGetUniformLocation(shaders[FORWARD], "modelRot");
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(model->rot_matrix));

        for (auto mesh : model->get_meshes()) {
            GLuint diffuse_num = 1;
            GLuint specular_num = 1;

            for(GLuint i = 0; i < mesh->textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                if(mesh->textures[i]->type == DIFFUSE) {
                    const char* str = ("texture_Diffuse" + std::to_string(diffuse_num++)).c_str();
                    GLuint diffuse_loc = glGetUniformLocation(shaders[FORWARD], str);
                    glUniform1i(diffuse_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
                }
                else if(mesh->textures[i]->type == SPECULAR) {
                    const char* str2 = ("texture_Specular" + std::to_string(specular_num++)).c_str();
                    GLuint specular_loc = glGetUniformLocation(shaders[FORWARD], str2);
                    glUniform1i(specular_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
                }
            }

            glUniform1f(glGetUniformLocation(shaders[FORWARD], "shininess"), mesh->shininess);

            /* DRAW */
            glBindVertexArray(mesh->get_VAO());
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            for (GLuint i = 0; i < mesh->textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
    render_flat();
    if (draw_bounding_spheres) {
        render_bounding_spheres();
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::render_flat()
{
    glUseProgram(shaders[FLAT]);
    for (auto model : Model::get_loaded_flat_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));
        GLuint rot_location = glGetUniformLocation(shaders[FLAT], "modelRot");
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(model->rot_matrix));
        if (model->get_lights().size() > 0) {
            GLuint color = glGetUniformLocation(shaders[FLAT], "color");
            glUniform3fv(color, 1, glm::value_ptr(model->get_lights()[0]->get_color()));
        }
        for (auto mesh : model->get_meshes()) {
            glBindVertexArray(mesh->get_VAO());

            /* DRAW */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }
    }
}

inline GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}

void Renderer::create_ssao_samples()
{

    ssao_kernel.clear();
    /* Create a unit hemisphere with n samples */
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;

    GLfloat scale;
    glm::vec3 sample;
    for (int i = 0; i < _SSAO_N_SAMPLES_; ++i) {
        sample = glm::vec3(
                           randomFloats(generator) * 2.0 - 1.0,
                           randomFloats(generator) * 2.0 - 1.0,
                           randomFloats(generator)
                           );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        scale = GLfloat(i) / _SSAO_N_SAMPLES_;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssao_kernel.push_back(sample);
    }
}

void Renderer::toggle_ssao()
{
    ssao_on = !ssao_on;
    if (!ssao_on) {
        clear_ssao();
    }
}


void Renderer::clear_ssao()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbuffer);
    float clearColor[1] = {1.0};
    glClearBufferfv(GL_COLOR, 0, clearColor);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::render_ssao()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaders[SSAO]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position_depth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noise_texture);

    // Upload shader samples
    for (GLuint i = 0; i < _SSAO_N_SAMPLES_; i++) {
        GLuint sample_loc = glGetUniformLocation(shaders[SSAO], ("samples[" + std::to_string(i) + "]").c_str());
        glUniform3fv(sample_loc, 1, &ssao_kernel[i][0]);
    }
    GLuint radius_loc = glGetUniformLocation(shaders[SSAO], "kernel_radius");
    glUniform1f(radius_loc, kernel_radius);
    GLuint size_loc = glGetUniformLocation(shaders[SSAO], "ssao_n_samples");
    glUniform1i(size_loc, _SSAO_N_SAMPLES_);
    // Projection matrix should already be uploaded from init_uniforms

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// --------------------------

void Renderer::render_bounding_spheres()
{
    // TODO: use instancing for bounding spheres!
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(shaders[FLAT]);
    Mesh* mesh = this->sphere->get_meshes()[0];

    for (auto model : Model::get_loaded_flat_models()) {
        glm::mat4 bounding_scale = glm::scale(glm::mat4(1.f), 0.6667f * glm::vec3(model->bounding_sphere_radius));
        glm::mat4 bounding_move = glm::translate(glm::mat4(1.f), model->scale * model->get_center_point());

        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->move_matrix * model->rot_matrix * bounding_move * model->scale_matrix * bounding_scale));

        /* DRAW */
        glBindVertexArray(mesh->get_VAO());
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    for (auto model : Model::get_loaded_models()) {
        glm::mat4 bounding_scale = glm::scale(glm::mat4(1.f), 0.6667f * glm::vec3(model->bounding_sphere_radius));
        glm::mat4 bounding_move = model->scale * glm::translate(glm::mat4(1.f), model->scale * model->get_center_point());

        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->move_matrix * model->rot_matrix * bounding_move * model->scale_matrix * bounding_scale));

        // DRAW
        glBindVertexArray(mesh->get_VAO());
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


// --------------------------

void Renderer::render_g_position()
{

}

// --------------------------

void Renderer::render_g_normal()
{

}

// --------------------------

void Renderer::render_g_albedo()
{

}

// --------------------------

void Renderer::render_g_specular()
{

}

// --------------------------


void Renderer::init_ssao()
{
    printf("Init_SSAO\n");
    ssao_on = true;
    ssao_n_samples = _SSAO_N_SAMPLES_;

    /* Create ssao kernel */
    create_ssao_samples();

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;

    /* Random rotations of the kernel */
    for (GLuint i = 0; i < 16; i++) {
        glm::vec3 noise(
                        randomFloats(generator) * 2.0 - 1.0,
                        randomFloats(generator) * 2.0 - 1.0,
                        0.0f);
        ssao_noise.push_back(noise);
    }

    glUseProgram(shaders[SSAO]);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "g_position_depth"), 0);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "g_normal"), 1);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "tex_noise"), 2);


    /*  Noise texture, really small and repeated */
    glGenTextures(1, &noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* Framebuffer for SSAO shader */
    glGenFramebuffers(1, &ssao_fbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbuffer);

    glGenTextures(1, &ssao_result);
    glBindTexture(GL_TEXTURE_2D, ssao_result);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_result, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// --------------------------

void Renderer::init_quad()
{
    GLfloat vertices[] = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

// --------------------------

void Renderer::init_g_buffer()
{
    glUseProgram(shaders[DEFERRED]);

    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_position_depth"), 0);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_normal"), 1);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_albedo_specular"), 2);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "ssao_result"), 3);

    glDisable(GL_BLEND);
    glGenFramebuffers(1, &g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

    /* Position and LINEAR depth buffer */
    glGenTextures(1, &g_position_depth);
    glBindTexture(GL_TEXTURE_2D, g_position_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position_depth, 0);

    /* Normal buffer */
    glGenTextures(1, &g_normal);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

    /* Albedo and Specular buffer*/
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream error_msg;
        error_msg << "GL enum: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Error::throw_error(Error::renderer_init_fail, error_msg.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// -----------------

void Renderer::draw_tweak_bar()
{
    // Draw tweak bar
    TwDraw();
}

// -----------------

void Renderer::init_tweak_bar()
{
    // Initialize AntTweakBar
    TwInit(TW_OPENGL_CORE, NULL);
    // Send the new window size to AntTweakBar
    TwWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create bar
    tweak_bar = TwNewBar("Emeralds tweakbar");

    // FPS counter
    TwAddVarRO(tweak_bar, "FPS", TW_TYPE_DOUBLE, &fps," label='FPS' help='Frames per second' ");
    // SSAO stuff
    TwAddVarRW(tweak_bar, "SSAO samples", TW_TYPE_INT32, &ssao_n_samples, " label='Number of SSAO samples' help='Defines the number of SSAO samples used.' ");
    TwAddVarRW(tweak_bar, "SSAO kernel radius", TW_TYPE_FLOAT, &kernel_radius, " label='SSAO kernel radius' help='Defines the radius of SSAO samples.' ");
    TwAddVarRW(tweak_bar, "SSAO ON", TW_TYPE_BOOL8, &ssao_on, " label='SSAO ON' help='Status of SSAO' ");
}

// ---------------

void Renderer::count_fps()
{
    unsigned current_time = SDL_GetTicks();
    double timediff = (current_time-last_time);

    if (timediff != 0) {
      fps = 1000/timediff;
    }

    last_time = current_time;
}

// ----------------

void Renderer::toggle_tweak_bar()
{
    use_tweak_bar = !use_tweak_bar;
}
