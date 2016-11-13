#include "Renderer.hpp"


void Renderer::init()
{
    shaders[FORWARD] = load_shaders("build/shaders/forward.vert", "build/shaders/forward.frag");
    shaders[GEOMETRY] = load_shaders("build/shaders/geometry.vert", "build/shaders/geometry.frag");
    shaders[GEOMETRY_NORMALS] = load_shaders("build/shaders/geometry_visualize_normals.vert",
                                             "build/shaders/geometry_visualize_normals.geom",
                                             "build/shaders/geometry_visualize_normals.frag");
    shaders[DEFERRED] = load_shaders("build/shaders/deferred.vert", "build/shaders/deferred.frag");
    shaders[FLAT] = load_shaders("build/shaders/flat.vert", "build/shaders/flat.frag");
    shaders[FLAT_TEXTURE] = load_shaders("build/shaders/flat_texture.vert", "build/shaders/flat_texture.frag");
    shaders[SSAO] = load_shaders("build/shaders/identity.vert", "build/shaders/ssao.frag");
    shaders[BLUR_RED_5_X] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_red_5_x.frag");
    shaders[BLUR_RED_5_Y] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_red_5_y.frag");
    shaders[BLUR_RGB_11_X] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_rgb_11_x.frag");
    shaders[BLUR_RGB_11_Y] = load_shaders("build/shaders/identity.vert", "build/shaders/blur_rgb_11_y.frag");
    shaders[SHOW_RGB_COMPONENT] = load_shaders("build/shaders/identity.vert",
                                               "build/shaders/show_rgb_component.frag");
    shaders[SHOW_ALPHA_COMPONENT] = load_shaders("build/shaders/identity.vert",
                                                 "build/shaders/show_alpha_component.frag");
    shaders[SHOW_SSAO] = load_shaders("build/shaders/identity.vert",
                                      "build/shaders/show_red_component.frag");
    shaders[HDR_BLOOM] = load_shaders("build/shaders/identity.vert",
                                      "build/shaders/hdr_bloom.frag");


    init_g_buffer();
    init_hdr_fbo();
    init_post_proc_fbo();
    init_quad();
    init_ssao();
    init_rgb_component_shader();
    init_alpha_component_shader();
    init_blur_shaders();
    init_hdr_bloom_shader();
    init_ping_pong_fbos();

    sphere = new Model("res/models/sphere/sphere.obj");
    skydome = new Skydome();
    skydome->init();

    set_mode(DEFERRED_MODE);

    Light::shader_programs.push_back(shaders[DEFERRED]);
    Light::shader_programs.push_back(shaders[FORWARD]);
}

// --------------------------

void Renderer::render(const Camera &camera)
{
    upload_camera_uniforms(camera);
    switch (mode) {
    case FORWARD_MODE:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_forward();
        skydome->draw(camera);
        break;
    case DEFERRED_MODE:
        render_deferred(camera);
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

    if (draw_bounding_spheres) {
        render_bounding_spheres();
    }

    if (use_tweak_bar) {
        count_fps();
        draw_tweak_bar();
    }

    update_time();
    glBindVertexArray(0);
    glUseProgram(0);
}

// -------------------------

void Renderer::update_time(){
    unsigned current_time = SDL_GetTicks();
    time_diff = current_time - last_timestamp;
    last_timestamp = current_time;
}

// --------------------------

void Renderer::set_mode(render_mode mode)
{
    this->mode = mode;
    switch (mode) {
    case FORWARD_MODE:
        break;
    case DEFERRED_MODE:
        clear_ssao();
        break;
    case POSITION_MODE:
        break;
    case NORMAL_MODE:
        break;
    case ALBEDO_MODE:
        break;
    case SPECULAR_MODE:
        break;
    case SSAO_MODE:
        break;
    }
}

// --------------------------

void Renderer::init_uniforms(const Camera &camera)
{
    mat4 projection_matrix;
    projection_matrix = perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (auto shaderProgram : shaders) {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
                           1, GL_FALSE, value_ptr(camera.get_view_matrix()));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
                           1, GL_FALSE, value_ptr(projection_matrix));
    }
    glUseProgram(0);
}

// --------------------------

void Renderer::upload_camera_uniforms(const Camera &camera)
{
    for (auto shaderProgram : shaders) {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
                           1, GL_FALSE, value_ptr(camera.get_view_matrix()));
        glUniform3fv(glGetUniformLocation(shaderProgram, "camPos"),
                     1, value_ptr(camera.get_pos()));
    }
    glUseProgram(0);
}

// ---------------------------

float Renderer::get_time_diff()
{
    return static_cast<float>(this->time_diff);
}


void Renderer::propagate_time(bool forward)
{
    float delta = forward ? this->time_diff : -(float)this->time_diff;
    skydome->propagate_time(delta);
}


void Renderer::increase_up_interp()
{
    if (this->up_interp >= 0.9f)
        this->up_interp = 1.0f;
    else
        this->up_interp += 0.1f;
}

void Renderer::decrease_up_interp()
{
    if (this->up_interp <= 0.1f)
        this->up_interp = 0.0f;
    else
        this->up_interp -= 0.1f;
}

void Renderer::toggle_show_normals()
{
    this->show_normals = !this->show_normals;
}

/* Private Renderer functions */

// --------------------------

void Renderer::render_deferred(const Camera &camera)
{
    /* GEOMETRY PASS */
    geometry_pass();

    /* VISUALIZE NORMALS: EXPERIMENTAL STUFF */
    if (this->show_normals) {
        normal_visualization_pass();
    }

    // SSAO PASS
    if (this->ssao_on) {
        ssao_pass();
    }


    glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //skydome->propagate_time(this->time_diff);
    skydome->upload_sun(shaders[DEFERRED], camera);
    Profiler::start_timer("Deferred pass");
    glUseProgram(shaders[DEFERRED]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_tex);

    // Render deferred shading stage to quad:
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Blit depth buffer from g-buffer:
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_fbo);
    glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0,SCREEN_WIDTH,
                      SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    Profiler::stop_timer("Deferred pass");

    // Draw flat objects with forward shading and skydome:
    render_flat();
    skydome->draw(camera);

    post_processing();

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
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, value_ptr(model->m2w_matrix));

        for (auto mesh : model->get_meshes()) {
            if (!mesh->draw_me) {
                continue;
            }
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

    for (auto terrain : Terrain::get_loaded_terrain()) {
        if (!terrain->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[FORWARD], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, value_ptr(terrain->m2w_matrix));

        for (auto mesh : terrain->get_meshes()) {
            if (!mesh->draw_me) {
                continue;
            }
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
    Profiler::start_timer("Flat objects pass");
    glUseProgram(shaders[FLAT]);
    for (auto model : Model::get_loaded_flat_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, value_ptr(model->m2w_matrix));

        GLuint color = glGetUniformLocation(shaders[FLAT], "color");
        glUniform3fv(color, 1,
                value_ptr(model->get_light_color()));

        for (auto mesh : model->get_meshes()) {
            glBindVertexArray(mesh->get_VAO());

            /* DRAW */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }
    }
    Profiler::stop_timer("Flat objects pass");

    if (draw_bounding_spheres) {
        render_bounding_spheres();
    }
}

// --------------------------

void Renderer::post_processing()
{
    Profiler::start_timer("Post processing");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Blur overexposed areas to cause bloom:
    blur_rgb_texture(bright_tex, post_proc_tex, post_proc_fbo, GAUSSIAN_RGB_11, 3);

    // Show bloom buffer:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[SHOW_RGB_COMPONENT]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, post_proc_tex);

    // Add bloom to original image and draw to screen quad:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[HDR_BLOOM]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, post_proc_tex);

    glUniform1f(glGetUniformLocation(shaders[HDR_BLOOM], "exposure"), 0.7f);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    Profiler::stop_timer("Post processing");
}

// --------------------------

inline GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}


void Renderer::create_ssao_samples()
{

    ssao_kernel.clear();
    /* Create a unit hemisphere with n samples */
    // random floats between 0.0 - 1.0
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    GLfloat scale;
    vec3 sample;
    for (int i = 0; i < _SSAO_N_SAMPLES_; ++i) {
        sample = vec3(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
                );
        sample = normalize(sample);
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
    Profiler::start_timer("SSAO pass");

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glViewport(0,0, SCREEN_WIDTH / _SSAO_SCALE_, SCREEN_HEIGHT / _SSAO_SCALE_);
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
        GLuint sample_loc = glGetUniformLocation(shaders[SSAO],
                ("samples[" + std::to_string(i) + "]").c_str());
        glUniform3fv(sample_loc, 1, &ssao_kernel[i][0]);
    }
    GLuint radius_loc = glGetUniformLocation(shaders[SSAO], "kernel_radius");
    glUniform1f(radius_loc, kernel_radius);
    // Projection matrix should already be uploaded from init_uniforms

    // Render quad
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (smooth_ssao) {
        blur_red_texture(ssao_tex, ssao_tex, ssao_fbo, UNIFORM_RED_5, 1);
    }

    for (GLuint i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glUseProgram(0);

    Profiler::stop_timer("SSAO pass");
}

// --------------------------

ping_pong_shader Renderer::upload_filter(filter_type ft)
{
    GLfloat uniform[5] = {1, 1, 1, 1, 1};
    GLfloat gaussian_big[11] = {0.090154, 0.090606, 0.090959, 0.091212, 0.091364,
        0.091414, 0.091364, 0.091212, 0.090959, 0.090606, 0.090154};

    ping_pong_shader shader;
    switch (ft) {
        case GAUSSIAN_RGB_11:
            shader.x = shaders[BLUR_RGB_11_X];
            glUseProgram(shader.x);
            glUniform1fv(glGetUniformLocation(shader.x, "kernel"), 11, gaussian_big);
            glUniform1f(glGetUniformLocation(shader.x, "magnitude"), 1.0f);

            shader.y = shaders[BLUR_RGB_11_Y];
            glUseProgram(shader.y);
            glUniform1fv(glGetUniformLocation(shader.y, "kernel"), 11, gaussian_big);
            glUniform1f(glGetUniformLocation(shader.y, "magnitude"), 1.0f);
            break;
        case UNIFORM_RED_5:
            shader.x = shaders[BLUR_RED_5_X];
            glUseProgram(shader.x);
            glUniform1fv(glGetUniformLocation(shader.x, "kernel"), 5, uniform);
            glUniform1f(glGetUniformLocation(shader.x, "magnitude"), 5);

            shader.y = shaders[BLUR_RED_5_Y];
            glUseProgram(shader.y);
            glUniform1fv(glGetUniformLocation(shader.y, "kernel"), 5, uniform);
            glUniform1f(glGetUniformLocation(shader.y, "magnitude"), 5);
            break;
    }
    return shader;
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

void Renderer::blur_red_texture(GLuint source_tex, GLuint fbo_tex,
        GLuint target_fbo, filter_type ft, int iterations)
{
    ping_pong_shader shader = upload_filter(ft);

    for (int i=0; i<iterations; i++) {
        glUseProgram(shader.x);
        filter_pass(source_tex, ping_pong_fbo_red);
        source_tex = fbo_tex;
        glUseProgram(shader.y);
        filter_pass(ping_pong_tex_red, target_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glUseProgram(0);
}

// --------------------------

void Renderer::blur_rgb_texture(GLuint source_tex, GLuint fbo_tex,
        GLuint target_fbo, filter_type ft, int iterations)
{
    ping_pong_shader shader = upload_filter(ft);

    for (int i=0; i<iterations; i++) {
        glUseProgram(shader.x);
        filter_pass(source_tex, ping_pong_fbo_rgb);
        source_tex = fbo_tex;
        glUseProgram(shader.y);
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
    glUniform3fv(color, 1, value_ptr(vec3(1.f)));

    for (auto model : Model::get_loaded_flat_models()) {
        mat4 bounding_scale = scale(mat4(1.f), vec3(model->bounding_sphere_radius) / 1.5f);
        mat4 bounding_move = translate(mat4(1.f), model->scale * model->get_center_point());

        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE,
                value_ptr(model->move_matrix * model->rot_matrix *
                    bounding_move * model->scale_matrix * bounding_scale));

        // DRAW
        glBindVertexArray(mesh->get_VAO());
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    for (auto model : Model::get_loaded_models()) {
        mat4 bounding_scale = scale(mat4(1.f), vec3(model->bounding_sphere_radius) / 1.5f);
        mat4 bounding_move = model->scale * translate(mat4(1.f), model->scale *
                model->get_center_point());

        GLuint m2w_location = glGetUniformLocation(shaders[FLAT], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE,
                value_ptr(model->move_matrix * model->rot_matrix *
                    bounding_move * model->scale_matrix * bounding_scale));

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
    Profiler::start_timer("Geometry pass");

    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaders[GEOMETRY]);

    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[GEOMETRY], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, value_ptr(model->m2w_matrix));

        for (auto mesh : model->get_meshes()) {
            if (!mesh->draw_me) {
                continue;
            }
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

    for (auto terrain : Terrain::get_loaded_terrain()) {
        if (!terrain->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[GEOMETRY], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, value_ptr(terrain->m2w_matrix));

        for (auto mesh : terrain->get_meshes()) {
            if (!mesh->draw_me) {
                continue;
            }
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

    Profiler::stop_timer("Geometry pass");
}


// --------------------------
void Renderer::normal_visualization_pass()
{
    Profiler::start_timer("Normal visualization pass");
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

    glUseProgram(shaders[GEOMETRY_NORMALS]);
    glUniform1f(glGetUniformLocation(shaders[GEOMETRY_NORMALS], "upInterp"), this->up_interp);

    for (auto model : Model::get_loaded_models()) {
        if (!model->draw_me) {
            continue;
        }
        GLuint m2w_location = glGetUniformLocation(shaders[GEOMETRY_NORMALS], "model");
        glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(model->m2w_matrix));

        for (auto mesh : model->get_meshes()) {
            glActiveTexture(GL_TEXTURE0);
            GLuint diffuse_loc = glGetUniformLocation(shaders[GEOMETRY_NORMALS], "diffuse_map");
            glUniform1i(diffuse_loc, 0);
            glBindTexture(GL_TEXTURE_2D, mesh->diffuse_map->id);

            glBindVertexArray(mesh->get_VAO());

            /* DRAW GEOMETRY */
            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);

    Profiler::stop_timer("Normal visualization pass");
}
// --------------------------

void Renderer::render_g_position()
{
    geometry_pass();

    if (this->show_normals) {
        normal_visualization_pass();
    }

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

    if (this->show_normals) {
        normal_visualization_pass();
    }

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

    if (this->show_normals) {
        normal_visualization_pass();
    }

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

    if (this->show_normals) {
        normal_visualization_pass();
    }

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

    if (this->show_normals) {
        normal_visualization_pass();
    }

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


// --------------------------


void Renderer::init_ssao()
{
    ssao_on = true;
    smooth_ssao = true;
    ssao_n_samples = _SSAO_N_SAMPLES_;

    /* Create ssao kernel */
    create_ssao_samples();
    // random floats between 0.0 - 1.0
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    /* Random rotations of the kernel */
    for (GLuint i = 0; i < 25; i++) {
        vec3 noise(
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH / _SSAO_SCALE_,
            SCREEN_HEIGHT / _SSAO_SCALE_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Use hardware linear interpolation.
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
    // RED fbo
    glGenFramebuffers(1, &ping_pong_fbo_red);
    glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_red);

    glGenTextures(1, &ping_pong_tex_red);
    glBindTexture(GL_TEXTURE_2D, ping_pong_tex_red);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH / _SSAO_SCALE_,
            SCREEN_HEIGHT / _SSAO_SCALE_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, ping_pong_tex_red, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Ping pong red framebuffer not complete!" << std::endl;


    // RGB16f fbo
    glGenFramebuffers(1, &ping_pong_fbo_rgb);
    glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_rgb);

    glGenTextures(1, &ping_pong_tex_rgb);
    glBindTexture(GL_TEXTURE_2D, ping_pong_tex_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            ping_pong_tex_rgb, 0);
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            g_position, 0);

    /* Normal buffer */
    glGenTextures(1, &g_normal_shininess);
    glBindTexture(GL_TEXTURE_2D, g_normal_shininess);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
            g_normal_shininess, 0);

    /* Albedo and Specular buffer*/
    glGenTextures(1, &g_albedo_specular);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
            g_albedo_specular, 0);

    /* Specify color attachments of the buffer */
    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    /* Attach a depth buffer */
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
            SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, depth_buffer);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, color_tex, 0);


    /* Bloom buffer */
    glGenTextures(1, &bright_tex);
    glBindTexture(GL_TEXTURE_2D, bright_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
            GL_TEXTURE_2D, bright_tex, 0);


    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    /* Attach a depth buffer */
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
            SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, depth_buffer);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH,SCREEN_HEIGHT,
            0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, post_proc_tex, 0);


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
    glUseProgram(shaders[BLUR_RED_5_X]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RED_5_X], "input_tex"), 0);
    glUseProgram(shaders[BLUR_RED_5_Y]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RED_5_Y], "input_tex"), 0);

    glUseProgram(shaders[BLUR_RGB_11_X]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RGB_11_X], "input_tex"), 0);
    glUseProgram(shaders[BLUR_RGB_11_Y]);
    glUniform1i(glGetUniformLocation(shaders[BLUR_RGB_11_Y], "input_tex"), 0);

    glUseProgram(0);
}

// -----------------


void Renderer::init_hdr_bloom_shader()
{
    glUseProgram(shaders[HDR_BLOOM]);
    glUniform1i(glGetUniformLocation(shaders[HDR_BLOOM], "input_tex1"), 0);
    glUniform1i(glGetUniformLocation(shaders[HDR_BLOOM], "input_tex2"), 1);

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

    TwAddVarRO(tweak_bar, "FPS", TW_TYPE_DOUBLE, &fps,
            " label='FPS' help='Frames per second' ");
    TwAddVarRW(tweak_bar, "Models drawn", TW_TYPE_INT32,&objects_drawn,
            " label='Models drawn' help='Objects not removed by frustum culling.' ");
    TwAddVarRW(tweak_bar, "Meshes drawn", TW_TYPE_INT32,&meshes_drawn,
            " label='Meshes drawn' help='Objects not removed by frustum culling.' ");
    // SSAO stuff
    TwAddVarRW(tweak_bar, "SSAO ON", TW_TYPE_BOOL8, &ssao_on,
            " label='SSAO ON' help='Status of SSAO' ");
    TwAddVarRW(tweak_bar, "SSAO samples", TW_TYPE_INT32, &ssao_n_samples,
            " label='SSAO samples' help='Defines the number of SSAO samples used.' ");
    TwAddVarRW(tweak_bar, "SSAO kernel radius", TW_TYPE_FLOAT, &kernel_radius,
            " label='SSAO k-radius' help='Defines the radius of SSAO samples.' ");
    TwAddVarRW(tweak_bar, "SSAO smoothing", TW_TYPE_BOOL8, &smooth_ssao,
            " label='SSAO smoothing' help='Blur filter for SSAO' ");

    // Camera position
    TwAddVarRW(tweak_bar, "cam-pos-x", TW_TYPE_FLOAT, &this->cam_pos.x,
            "label=cam-pos-x help=current-camera-x-coord");
    TwAddVarRW(tweak_bar, "cam-pos-y", TW_TYPE_FLOAT, &this->cam_pos.y,
            "label=cam-pos-y help=current-camera-y-coord");
    TwAddVarRW(tweak_bar, "cam-pos-z", TW_TYPE_FLOAT, &this->cam_pos.z,
            "label=cam-pos-z help=current-camera-z-coord");

    TwAddVarRW(tweak_bar, "look-spline", TW_TYPE_INT32, &this->cam_spline_look_id,
            "label=look-spline help='cam look animation path id'");
    TwAddVarRW(tweak_bar, "look-spline-para", TW_TYPE_FLOAT, &this->cam_spline_look_para,
            "label=look-spline-para help='time parameter along look spline'");
    TwAddVarRW(tweak_bar, "follow-spline", TW_TYPE_INT32, &this->cam_spline_move_id ,
            "label=folow-spline help='cam move animation path id'");

    TwAddVarRW(tweak_bar, "follow-spline-para", TW_TYPE_FLOAT, &this->cam_spline_move_para,
            "label=follow-spline-para help='time parameter along move spline'");

    this->n_lightsources = Light::get_num_lights();
    TwAddVarRW(tweak_bar, "Number of lights", TW_TYPE_INT32, &this->n_lightsources ,
            "label='Number of lights' help='Total number of lights in scene'");
    TwAddVarRW(tweak_bar, "Number of culled lights", TW_TYPE_INT32, &Light::culled_lights,
            "label='Culled lights' help='Lights with bounding sphere outside frustum.'");
    TwAddVarRW(tweak_bar, "Normal vec interp", TW_TYPE_FLOAT, &this->up_interp,
            "label='Normal vector interpolation' help='Valid range is [0,1]. 1 uses only up vector.'");
    TwAddVarRW(tweak_bar, "Show normals", TW_TYPE_BOOL8, &show_normals,
            "label='Show normals generated in geometry shader' help='Toggles the normal visualization pass'");

}

// ---------------

void Renderer::count_fps()
{
    unsigned current_time = SDL_GetTicks();
    double timediff = (current_time-last_timestamp);

    if (timediff != 0) {
        fps = 1000/timediff;
    }
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
