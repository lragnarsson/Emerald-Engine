#include "Renderer.hpp"


void Renderer::init()
{
    shaders[FORWARD] = load_shaders("build/shaders/forward.vert", "build/shaders/forward.frag");
    shaders[GEOMETRY] = load_shaders("build/shaders/geometry.vert", "build/shaders/geometry.frag");
    shaders[DEFERRED] = load_shaders("build/shaders/deferred.vert", "build/shaders/deferred.frag");
    shaders[FLAT] = load_shaders("build/shaders/flat.vert", "build/shaders/flat.frag");
    shaders[FLAT_TEXTURE] = load_shaders("build/shaders/flat_texture.vert", "build/shaders/flat_texture.frag");
    shaders[SSAO] = load_shaders("build/shaders/identity.vert", "build/shaders/ssao.frag");
    shaders[SSAO_BLUR] = load_shaders("build/shaders/identity.vert", "build/shaders/ssao_blur.frag");
    shaders[BLUR_RED_5] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_red_5.frag");
    shaders[BLUR_RGB_5] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_rgb_5.frag");
    shaders[BLUR_RGB_11] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_rgb_11.frag");
    shaders[SHOW_RGB_COMPONENT] = load_shaders("build/shaders/identity.vert",
                                               "build/shaders/show_rgb_component.frag");
    shaders[SHOW_ALPHA_COMPONENT] = load_shaders("build/shaders/identity.vert",
                                                 "build/shaders/show_alpha_component.frag");
    shaders[SHOW_SSAO] = load_shaders("build/shaders/identity.vert",
                                      "build/shaders/show_red_component.frag");
    shaders[BLEND] = load_shaders("build/shaders/identity.vert",
                                  "build/shaders/blend.frag");


    init_g_buffer();
    init_hdr_fbo();
    init_post_proc_fbo();
    init_quad();
    init_ssao();
    init_rgb_component_shader();
    init_alpha_component_shader();
    init_blur_shaders();
    init_blend_shader();
    init_ping_pong_fbos();

    sphere = new Model("res/models/sphere/sphere.obj");
    skybox = new Model("res/models/skybox/skybox.obj");
    skybox->move_to(glm::vec3(-0.5f, -0.5f, -0.5f));
    set_mode(DEFERRED_MODE);
}

// --------------------------

void Renderer::render(const Camera &camera)
{
    upload_camera_uniforms(camera);
    switch (mode) {
    case FORWARD_MODE:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_forward();
        render_skybox(camera);
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
    case SSAO_MODE:
        render_ssao();
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
        clear_ssao();
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
    case SSAO_MODE:
        Light::shader_program = shaders[DEFERRED];
        break;
    }
    Light::upload_all();
}

// --------------------------

void Renderer::init_uniforms(const Camera &camera)
{
    glm::mat4 projection_matrix;
    w2v_matrix = glm::lookAt(camera.get_pos(), camera.get_pos() + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (int i = 0; i < 10; i ++) {
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
    w2v_matrix = glm::lookAt(camera.get_pos(), camera.get_pos() + camera.front, camera.up);
    for (int i = 0; i < 10; i ++) {
        glUseProgram(shaders[i]);
        glUniformMatrix4fv(glGetUniformLocation(shaders[i], "view"),
                           1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniform3fv(glGetUniformLocation(shaders[i], "camPos"),
                     1, glm::value_ptr(camera.get_pos()));
    }
    glUseProgram(0);
}


/* Private Renderer functions */

// --------------------------

void Renderer::render_deferred()
{
    /* GEOMETRY PASS */
    geometry_pass();

    // SSAO PASS
    if (this->ssao_on) {
        ssao_pass();
    }


    glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[DEFERRED]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_tex);

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* RENDER FLAT OBJECTS WITH DEPTH BUFFER */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_fbo);
    glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    render_flat();

    blur_rgb_texture(bright_tex, post_proc_tex, post_proc_fbo, GAUSSIAN_RGB_11, 3);

    // Show blurred bright spots:
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, post_proc_tex);*/

    // Blend blurred glow with normal color.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[BLEND]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, post_proc_tex);

    glUniform1f(glGetUniformLocation(shaders[BLEND], "alpha"), 1.0f);
    glUniform1f(glGetUniformLocation(shaders[BLEND], "beta"), 1.0f);
    glUniform1f(glGetUniformLocation(shaders[BLEND], "exposure"), 0.3f);

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // END TEMP

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

// --------------------------

void Renderer::render_forward()
{
    glUseProgram(shaders[FORWARD]);

    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[FORWARD], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));

        for (auto mesh : model->get_meshes()) {
            glActiveTexture(GL_TEXTURE0);
            GLuint diffuse_loc = glGetUniformLocation(shaders[FORWARD], "diffuse_map");
            glUniform1i(diffuse_loc, 0);
            glBindTexture(GL_TEXTURE_2D, mesh->diffuse_map->id);

            glActiveTexture(GL_TEXTURE1);
            GLuint specular_loc = glGetUniformLocation(shaders[FORWARD], "specular_map");
            glUniform1i(specular_loc, 1);
            glBindTexture(GL_TEXTURE_2D, mesh->specular_map->id);

            glActiveTexture(GL_TEXTURE2);
            GLuint normal_loc = glGetUniformLocation(shaders[FORWARD], "normal_map");
            glUniform1i(normal_loc, 2);
            glBindTexture(GL_TEXTURE_2D, mesh->normal_map->id);

            glUniform1f(glGetUniformLocation(shaders[FORWARD], "shininess"), mesh->shininess);

            /* DRAW */
            glBindVertexArray(mesh->get_VAO());
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);
    for (GLuint i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);

    render_flat();
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

        GLuint color = glGetUniformLocation(shaders[FLAT], "color");
        if (model->get_lights().size() > 0) {
            glUniform3fv(color, 1, glm::value_ptr(model->get_lights()[0]->get_color()));
        } else {
            glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1.f)));
        }

        for (auto mesh : model->get_meshes()) {
            glBindVertexArray(mesh->get_VAO());

            /* DRAW */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }
    }

    if (draw_bounding_spheres) {
        render_bounding_spheres();
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


void Renderer::toggle_ssao_smoothing()
{
    smooth_ssao = !smooth_ssao;
    if (!ssao_on) {
        clear_ssao();
    }
}


void Renderer::clear_ssao()
{
    float clearColor[1] = {1.0};

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glClearBufferfv(GL_COLOR, 0, clearColor);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::ssao_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaders[SSAO]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noise_texture);

    // Upload shader samples
    for (GLuint i = 0; i < _SSAO_N_SAMPLES_; i++) {
        GLuint sample_loc = glGetUniformLocation(shaders[SSAO], ("samples[" + std::to_string(i) + "]").c_str());
        glUniform3fv(sample_loc, 1, &ssao_kernel[i][0]);
    }
    GLuint radius_loc = glGetUniformLocation(shaders[SSAO], "kernel_radius");
    glUniform1f(radius_loc, kernel_radius);
    //    GLuint size_loc = glGetUniformLocation(shaders[SSAO], "ssao_n_samples");
    //    glUniform1i(size_loc, _SSAO_N_SAMPLES_);
    // Projection matrix should already be uploaded from init_uniforms

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (smooth_ssao) {
        blur_red_texture(ssao_tex, ssao_tex, ssao_fbo, UNIFORM_RED_5, 2);
    }

    for (GLuint i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

// --------------------------

GLuint Renderer::upload_filter(filter_type ft)
{
    GLfloat gaussian[5] = {1, 4, 6, 4, 1};
    GLfloat uniform[5] = {1, 1, 1, 1, 1};
    //GLfloat gaussian_big[11] = {0.090841, 0.090882, 0.090914, 0.090936, 0.09095, 0.090955, 0.09095, 0.090936, 0.090914, 0.090882, 0.090841};
    GLfloat gaussian_big[11] = {0.090154, 0.090606, 0.090959, 0.091212, 0.091364, 0.091414, 0.091364, 0.091212, 0.090959, 0.090606, 0.090154};
    //GLfloat gaussian_big[11] = {0.084264, 0.088139, 0.091276, 0.093585, 0.094998, 0.095474, 0.094998, 0.093585, 0.091276, 0.088139, 0.084264};

    switch (ft) {
    case GAUSSIAN_RED_5:
        glUseProgram(shaders[BLUR_RED_5]);
        glUniform1fv(glGetUniformLocation(shaders[BLUR_RED_5], "kernel"), 5, gaussian);
        glUniform1f(glGetUniformLocation(shaders[BLUR_RED_5], "magnitude"), 16);
        return shaders[BLUR_RED_5];
        break;
    case GAUSSIAN_RGB_5:
        glUseProgram(shaders[BLUR_RGB_5]);
        glUniform1fv(glGetUniformLocation(shaders[BLUR_RGB_5], "kernel"), 5, gaussian);
        glUniform1f(glGetUniformLocation(shaders[BLUR_RGB_5], "magnitude"), 16);
        return shaders[BLUR_RGB_5];
        break;
    case GAUSSIAN_RGB_11:
        glUseProgram(shaders[BLUR_RGB_11]);
        glUniform1fv(glGetUniformLocation(shaders[BLUR_RGB_11], "kernel"), 11, gaussian_big);
        glUniform1f(glGetUniformLocation(shaders[BLUR_RGB_11], "magnitude"), 1.0f);
        return shaders[BLUR_RGB_11];
        break;
    case UNIFORM_RED_5:
        glUseProgram(shaders[BLUR_RED_5]);
        glUniform1fv(glGetUniformLocation(shaders[BLUR_RED_5], "kernel"), 5, uniform);
        glUniform1f(glGetUniformLocation(shaders[BLUR_RED_5], "magnitude"), 5);
        return shaders[BLUR_RED_5];
        break;
    case UNIFORM_RGB_5:
        glUseProgram(shaders[BLUR_RGB_5]);
        glUniform1fv(glGetUniformLocation(shaders[BLUR_RGB_5], "kernel"), 5, uniform);
        glUniform1f(glGetUniformLocation(shaders[BLUR_RGB_5], "magnitude"), 5);
        return shaders[BLUR_RGB_5];
        break;
    }
}

// --------------------------

void Renderer::filter_pass(GLuint source_tex, GLuint target_fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source_tex);
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// --------------------------

void Renderer::blur_red_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations)
{
    GLuint shader = upload_filter(ft);
    glUseProgram(shader);

    for (int i=0; i<iterations; i++) {
        glUniform1i(glGetUniformLocation(shader, "horizontal"), true);
        filter_pass(source_tex, ping_pong_fbo_red);
        source_tex = fbo_tex;
        glUniform1i(glGetUniformLocation(shader, "horizontal"), false);
        filter_pass(ping_pong_tex_red, target_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

// --------------------------

void Renderer::blur_rgb_texture(GLuint source_tex, GLuint fbo_tex, GLuint target_fbo, filter_type ft, int iterations)
{
    GLuint shader = upload_filter(ft);
    glUseProgram(shader);

    for (int i=0; i<iterations; i++) {
        glUniform1i(glGetUniformLocation(shader, "horizontal"), true);
        filter_pass(source_tex, ping_pong_fbo_rgb);
        source_tex = fbo_tex;
        glUniform1i(glGetUniformLocation(shader, "horizontal"), false);
        filter_pass(ping_pong_tex_rgb, target_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

// --------------------------

void Renderer::render_bounding_spheres()
{
    // TODO: use instancing for bounding spheres!
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(shaders[FLAT]);
    Mesh* mesh = this->sphere->get_meshes()[0];
    GLuint color = glGetUniformLocation(shaders[FLAT], "color");
    glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1.f)));

    for (auto model : Model::get_loaded_flat_models()) {
        glm::mat4 bounding_scale = glm::scale(glm::mat4(1.f), glm::vec3(model->bounding_sphere_radius) / 1.5f);
        glm::mat4 bounding_move = glm::translate(glm::mat4(1.f), model->scale * model->get_center_point());

        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->move_matrix * model->rot_matrix * bounding_move * model->scale_matrix * bounding_scale));

        // DRAW
        glBindVertexArray(mesh->get_VAO());
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    for (auto model : Model::get_loaded_models()) {
        glm::mat4 bounding_scale = glm::scale(glm::mat4(1.f), glm::vec3(model->bounding_sphere_radius) / 1.5f);
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

// ------------------------

void Renderer::geometry_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaders[GEOMETRY]);

    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[GEOMETRY], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));

        for (auto mesh : model->get_meshes()) {
            glActiveTexture(GL_TEXTURE0);
            GLuint diffuse_loc = glGetUniformLocation(shaders[GEOMETRY], "diffuse_map");
            glUniform1i(diffuse_loc, 0);
            glBindTexture(GL_TEXTURE_2D, mesh->diffuse_map->id);

            glActiveTexture(GL_TEXTURE0 + 1);
            GLuint specular_loc = glGetUniformLocation(shaders[GEOMETRY], "specular_map");
            glUniform1i(specular_loc, 1);
            glBindTexture(GL_TEXTURE_2D, mesh->specular_map->id);

            glActiveTexture(GL_TEXTURE0 + 2);
            GLuint normal_loc = glGetUniformLocation(shaders[GEOMETRY], "normal_map");
            glUniform1i(normal_loc, 2);
            glBindTexture(GL_TEXTURE_2D, mesh->normal_map->id);

            glUniform1f(glGetUniformLocation(shaders[GEOMETRY], "shininess"), mesh->shininess);

            glBindVertexArray(mesh->get_VAO());

            /* DRAW GEOMETRY */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);
    for (GLuint i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_position()
{
    geometry_pass();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);


    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_normal()
{
    geometry_pass();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);


    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_albedo()
{
    geometry_pass();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);


    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}

// --------------------------

void Renderer::render_g_specular()
{
    geometry_pass();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_ALPHA_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);


    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}

// --------------------------

void Renderer::render_ssao()
{
    geometry_pass();

    ssao_pass();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_SSAO]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssao_tex);

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}


void Renderer::render_skybox(const Camera &camera)
{
    glDepthRange(0.999f, 1.f);

    glUseProgram(shaders[FLAT_TEXTURE]);
    glm::mat4 skybox_view = glm::lookAt(glm::vec3(0.f), camera.front, camera.up);
    glUniformMatrix4fv(glGetUniformLocation(shaders[FLAT_TEXTURE], "view"), 1, GL_FALSE, glm::value_ptr(skybox_view));
    glUniformMatrix4fv(glGetUniformLocation(shaders[FLAT_TEXTURE], "model"), 1, GL_FALSE, glm::value_ptr(skybox->m2w_matrix));

    Mesh* mesh = skybox->get_meshes()[0];
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_loc = glGetUniformLocation(shaders[FLAT_TEXTURE], "tex_unit");
    glUniform1i(diffuse_loc, 0);
    glBindTexture(GL_TEXTURE_2D, mesh->diffuse_map->id);

    /* DRAW */
    glBindVertexArray(mesh->get_VAO());
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDepthRange(0.f, 1.f);
    glUseProgram(0);
}

// --------------------------


void Renderer::init_ssao()
{
    ssao_on = true;
    smooth_ssao = true;
    ssao_n_samples = _SSAO_N_SAMPLES_;

    /* Create ssao kernel */
    create_ssao_samples();

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;

    /* Random rotations of the kernel */
    for (GLuint i = 0; i < 25; i++) {
        glm::vec3 noise(
                        randomFloats(generator) * 2.0 - 1.0,
                        randomFloats(generator) * 2.0 - 1.0,
                        0.0f);
        ssao_noise.push_back(noise);
    }

    glUseProgram(shaders[SSAO]);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "g_position"), 0);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "g_normal_shininess"), 1);
    glUniform1i(glGetUniformLocation(shaders[SSAO], "tex_noise"), 2);


    /*  Noise texture, really small and repeated */
    glGenTextures(1, &noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 5, 5, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* Framebuffer for SSAO shader */
    glGenFramebuffers(1, &ssao_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);


    glGenTextures(1, &ssao_tex);
    glBindTexture(GL_TEXTURE_2D, ssao_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// --------------------------

/* Intended for use as a substep when doing multipass filtering */
void Renderer::init_ping_pong_fbos()
{
    // R16f fbo
    glGenFramebuffers(1, &ping_pong_fbo_red);
    glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_red);

    glGenTextures(1, &ping_pong_tex_red);
    glBindTexture(GL_TEXTURE_2D, ping_pong_tex_red);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_tex_red, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Ping pong red framebuffer not complete!" << std::endl;


    // RGB16f fbo
    glGenFramebuffers(1, &ping_pong_fbo_rgb);
    glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_rgb);

    glGenTextures(1, &ping_pong_tex_rgb);
    glBindTexture(GL_TEXTURE_2D, ping_pong_tex_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_tex_rgb, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Ping pong rgb framebuffer not complete!" << std::endl;

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

    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_position"), 0);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_normal_shininess"), 1);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "g_albedo_specular"), 2);
    glUniform1i(glGetUniformLocation(shaders[DEFERRED], "ssao_blurred"), 3);
    glUseProgram(0);

    glDisable(GL_BLEND);
    glGenFramebuffers(1, &g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

    /* Position */
    glGenTextures(1, &g_position);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

    /* Normal buffer */
    glGenTextures(1, &g_normal_shininess);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_shininess, 0);

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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream error_msg;
        error_msg << "GL enum: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Error::throw_error(Error::renderer_init_fail, error_msg.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// -----------------

void Renderer::init_hdr_fbo()
{
    glDisable(GL_BLEND);
    glGenFramebuffers(1, &hdr_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);

    /* Lighting buffer. Contains shaded light from deferred shader. */
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);


    /* Bloom buffer */
    glGenTextures(1, &bright_tex);
    glBindTexture(GL_TEXTURE_2D, bright_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bright_tex, 0);


    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    /* Attach a depth buffer */
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream error_msg;
        error_msg << "GL enum: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Error::throw_error(Error::renderer_init_fail, error_msg.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// -----------------

void Renderer::init_post_proc_fbo()
{
    glDisable(GL_BLEND);
    glGenFramebuffers(1, &post_proc_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, post_proc_fbo);

    /* Texture for post processing. Filter result etc. */
    glGenTextures(1, &post_proc_tex);
    glBindTexture(GL_TEXTURE_2D, post_proc_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, post_proc_tex, 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream error_msg;
        error_msg << "GL enum: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Error::throw_error(Error::renderer_init_fail, error_msg.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
// -----------------

void Renderer::init_rgb_component_shader()
{
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    // Bind different rgb textures to show them
    glUniform1i(glGetUniformLocation(shaders[SHOW_RGB_COMPONENT], "input_tex"), 0);
    glUseProgram(0);
}

// -----------------

void Renderer::init_alpha_component_shader()
{
    glUseProgram(shaders[SHOW_ALPHA_COMPONENT]);
    glUniform1i(glGetUniformLocation(shaders[SHOW_ALPHA_COMPONENT], "input_tex"), 0);
    glUseProgram(0);
}

// -----------------

void Renderer::init_show_ssao_shader()
{
    glUseProgram(shaders[SHOW_SSAO]);
    glUniform1i(glGetUniformLocation(shaders[SHOW_SSAO], "input_tex"), 0);
    glUseProgram(0);
}

void Renderer::init_blur_shaders()
{
    glUseProgram(shaders[BLUR_RED_5]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RED_5], "input_tex"), 0);

    glUseProgram(shaders[BLUR_RGB_5]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RGB_5], "input_tex"), 0);

    glUseProgram(shaders[BLUR_RGB_11]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RGB_11], "input_tex"), 0);

    glUseProgram(0);
}

// -----------------


void Renderer::init_blend_shader()
{
    glUseProgram(shaders[BLEND]);
    glUniform1i(glGetUniformLocation(shaders[BLEND], "input_tex1"), 0);
    glUniform1i(glGetUniformLocation(shaders[BLEND], "input_tex2"), 1);

    glUseProgram(0);
}
// -----------------

void Renderer::draw_tweak_bar()
{
    // Draw tweak bar
    TwDraw();
}

// -----------------

void Renderer::init_tweak_bar(Camera* camera)
{
    // Initialize AntTweakBar
    TwInit(TW_OPENGL_CORE, NULL);
    // Send the new window size to AntTweakBar
    TwWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create bar
    tweak_bar = TwNewBar("emerald");
    TwDefine(" emerald label='Emerald Engine' ");
    TwDefine(" emerald refresh=1 ");

    TwAddVarRO(tweak_bar, "FPS", TW_TYPE_DOUBLE, &fps," label='FPS' help='Frames per second' ");
    TwAddVarRW(tweak_bar, "Objects drawn", TW_TYPE_INT32, &objects_drawn, " label='Objects drawn' help='Objects not removed by frustum culling.' ");
    // SSAO stuff
    TwAddVarRW(tweak_bar, "SSAO ON", TW_TYPE_BOOL8, &ssao_on, " label='SSAO ON' help='Status of SSAO' ");
    TwAddVarRW(tweak_bar, "SSAO samples", TW_TYPE_INT32, &ssao_n_samples, " label='SSAO samples' help='Defines the number of SSAO samples used.' ");
    TwAddVarRW(tweak_bar, "SSAO kernel radius", TW_TYPE_FLOAT, &kernel_radius, " label='SSAO k-radius' help='Defines the radius of SSAO samples.' ");
    TwAddVarRW(tweak_bar, "SSAO smoothing", TW_TYPE_BOOL8, &smooth_ssao, " label='SSAO smoothing' help='Blur filter for SSAO' ");

    // Camera position
    TwAddVarRW(tweak_bar, "cam-pos-x", TW_TYPE_FLOAT, &this->cam_pos.x, "label=cam-pos-x help=current-camera-x-coord");
    TwAddVarRW(tweak_bar, "cam-pos-y", TW_TYPE_FLOAT, &this->cam_pos.y, "label=cam-pos-y help=current-camera-y-coord");
    TwAddVarRW(tweak_bar, "cam-pos-z", TW_TYPE_FLOAT, &this->cam_pos.z, "label=cam-pos-z help=current-camera-z-coord");

    TwAddVarRW(tweak_bar, "look-spline", TW_TYPE_INT32, &this->cam_spline_look_id, "label=look-spline help='cam look animation path id'");
    TwAddVarRW(tweak_bar, "look-spline-para", TW_TYPE_FLOAT, &this->cam_spline_look_para, "label=look-spline-para help='time parameter along look spline'");
    TwAddVarRW(tweak_bar, "follow-spline", TW_TYPE_INT32, &this->cam_spline_move_id , "label=folow-spline help='cam move animation path id'");

    TwAddVarRW(tweak_bar, "follow-spline-para", TW_TYPE_FLOAT, &this->cam_spline_move_para, "label=follow-spline-para help='time parameter along move spline'");

    this->n_lightsources = Light::get_number_of_lightsources();
    TwAddVarRW(tweak_bar, "Number of lights", TW_TYPE_INT32, &this->n_lightsources , "label='Number of lights' help='Total number of lights in scene'");

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

void Renderer::copy_tweak_bar_cam_values(const Camera& camera)
{
    this->cam_spline_move_id = camera.get_move_id();
    this->cam_spline_look_id = camera.get_look_id();
    this->cam_spline_move_para = camera.get_spline_move_parameter();
    this->cam_spline_look_para = camera.get_spline_look_parameter();
    this->cam_pos = camera.get_pos();
}
