#include "Renderer.hpp"


void Renderer::init()
{
    shaders[FORWARD] = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");
    shaders[GEOMETRY] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");
    shaders[DEFERRED] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");
    shaders[FLAT] = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");

    set_mode(FORWARD_MODE);
}


void Renderer::render()
{
    glClearColor(0.3, 0.3, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    glBindVertexArray(0);
    glUseProgram(0);
}

// --------------------------

void Renderer::init_g_buffer()
{
    glUseProgram(shaders[GEOMETRY]);

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


void Renderer::set_mode(render_mode mode)
{
    this->mode = mode;
    switch (mode) {
    case FORWARD_MODE:
        Light::shader_program = shaders[FORWARD];
        break;
    case DEFERRED_MODE:
        Light::shader_program = shaders[DEFERRED];
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

void Renderer::render_deferred()
{

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
        GLuint rot_location = glGetUniformLocation(shaders[FORWARD], "modelRot");
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(model->rot_matrix));

        for (auto mesh : model->get_meshes()) {
            GLuint diffuse_num = 1;
            GLuint specular_num = 1;

            for(GLuint i = 0; i < mesh.textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                if(mesh.textures[i]->type == DIFFUSE) {
                    const char* str = ("texture_Diffuse" + std::to_string(diffuse_num++)).c_str();
                    GLuint diffuse_loc = glGetUniformLocation(shaders[FORWARD], str);
                    glUniform1i(diffuse_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh.textures[i]->id);
                }
                else if(mesh.textures[i]->type == SPECULAR) {
                    const char* str2 = ("texture_Specular" + std::to_string(specular_num++)).c_str();
                    GLuint specular_loc = glGetUniformLocation(shaders[FORWARD], str2);
                    glUniform1i(specular_loc, i);
                    glBindTexture(GL_TEXTURE_2D, mesh.textures[i]->id);
                }
            }

            glUniform1f(glGetUniformLocation(shaders[FORWARD], "m.shininess"), mesh.shininess);
            glUniform3fv(glGetUniformLocation(shaders[FORWARD], "m.ambient"), 1, glm::value_ptr(mesh.ambient_color));
            glUniform3fv(glGetUniformLocation(shaders[FORWARD], "m.diffuse"), 1, glm::value_ptr(mesh.diffuse_color));
            glUniform3fv(glGetUniformLocation(shaders[FORWARD], "m.specular"), 1, glm::value_ptr(mesh.specular_color));

            glBindVertexArray(mesh.get_VAO());

            /* DRAW */
            glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);

            for (GLuint i = 0; i < mesh.textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

        }

        render_flat();
        glUseProgram(0);
    }
}


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
            glBindVertexArray(mesh.get_VAO());

            /* DRAW */
            glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }
    }
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
