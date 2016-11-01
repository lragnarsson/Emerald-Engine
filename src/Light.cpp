#include "Light.hpp"


namespace Light
{
    Light lights[_MAX_LIGHTS_]; // Always same order
    Light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
    Light_meta light_metas[_MAX_LIGHTS_]; // Metadata about lights such as radius, on/off.
    std::vector<GLuint> shader_programs; // For UBO binding on init.
    GLuint ubos[2]; // Light ubo, light_info ubo
    int num_lights;
    int culled_lights;
    int next_to_turn_on;


    int create_light(glm::vec3 position, float brightness,
                     glm::vec3 color)
    {
        if (num_lights < _MAX_LIGHTS_) {
            lights[num_lights].position = position;
            lights[num_lights].brightness = brightness;
            lights[num_lights].color = color;
            generate_bounding_sphere(num_lights);
            num_lights++;
            return num_lights - 1;
        } else { // Reached max number of lights
            return -1;
        }
    }


    void destroy_light(int index)
    {
        // Take the last light and move it to index. Decrease num lights.
        if (index >= 0 && index < num_lights) {
            lights[index].position = lights[num_lights-1].position;
            lights[index].brightness = lights[num_lights-1].brightness;
            lights[index].color = lights[num_lights-1].color;
            light_metas[index].radius = light_metas[num_lights-1].radius;
            num_lights--;
        }
    }


    void upload_lights()
    {
        int visible_lights = num_lights - culled_lights;

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0,
                        light_size * visible_lights, gpu_lights);

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[1]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0,
                        info_size, &visible_lights);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }


    void init()
    {
        for (auto shader : shader_programs) {
            GLuint light_index = glGetUniformBlockIndex(shader, "light_block");
            glUniformBlockBinding(shader, light_index, 0);
            GLuint info_index = glGetUniformBlockIndex(shader, "light_info_block");
            glUniformBlockBinding(shader, info_index, 1);
        }

        glGenBuffers(2, ubos);
        glBindBuffer(GL_UNIFORM_BUFFER, ubos[0]);
        glBufferData(GL_UNIFORM_BUFFER,
                     light_size * _MAX_LIGHTS_,
                     NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[1]);
        glBufferData(GL_UNIFORM_BUFFER, info_size,
                     NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubos[0]);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubos[1]);

        turn_on_all_lights();
    }


    void cull_light_sources(Camera &camera)
    {
        Profiler::start_timer("Cull lights");
        int num_visible = 0;
        culled_lights = 0;
        for (int i=0; i<num_lights; i++) {
            // Use lights which are turned on or inside viewing frustum:
            if (light_metas[i].active &&
                camera.sphere_in_frustum(lights[i].position,
                                         light_metas[i].radius)) {
                // Move visible lights to gpu array:
                gpu_lights[num_visible].position = lights[i].position;
                gpu_lights[num_visible].brightness = lights[i].brightness;
                gpu_lights[num_visible].color = lights[i].color;
                num_visible++;
            } else {
                culled_lights++;
            }
        }
        upload_lights();
        Profiler::stop_timer("Cull lights");
    }


    void generate_bounding_sphere(int light)
    {
        float a = _ATT_CON_;
        float b = _ATT_LIN_;
        float c = _ATT_QUAD_;
        float alpha = 0.02; // 2 percent
        float beta = glm::length(lights[light].color) * lights[light].brightness;

        // Solve quadratic equation to determine at what distance the light is dimmer than alpha times beta:
        light_metas[light].radius = -b / (2 * c) + std::sqrt(b * b / (4 * c * c) - a / c + beta / (alpha * c));
    }


    void turn_off_all_lights()
    {
        for (int i = 0; i < num_lights; i++) {
            light_metas[i].active = false;
        }
    }

    void turn_on_all_lights()
    {
        for (int i = 0; i < num_lights; i++) {
            light_metas[i].active = true;
        }
    }


    void turn_on_one_light()
    {
        if (next_to_turn_on < num_lights) {
            light_metas[next_to_turn_on].active = true;
        }
        if (++next_to_turn_on == num_lights) {
            next_to_turn_on = 0;
        }
    }
}
