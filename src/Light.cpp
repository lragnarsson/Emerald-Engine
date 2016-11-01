#include "Light.hpp"


namespace Light
{
    Light lights[_MAX_LIGHTS_]; // Always same order
    Light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
    float light_radii[_MAX_LIGHTS_];
    GLuint ubos[2]; // Light ubo, light_info ubo
    std::vector<GLuint> shader_programs;
    int num_lights;
    int culled_lights;
    const int light_size = 40;
    const int info_size = 4;


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
            light_radii[index] = light_radii[num_lights-1];
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
    }


    void cull_light_sources(Camera &camera)
    {
        Profiler::start_timer("Cull lights");
        int num_visible = 0;
        culled_lights = 0;
        for (int i=0; i<num_lights; i++) {
            if (camera.sphere_in_frustum(lights[i].position,
                                         light_radii[i])) {
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
        light_radii[light] = -b / (2 * c) + std::sqrt(b * b / (4 * c * c) - a / c + beta / (alpha * c));
    }


/*void Light::turn_off_all_lights()
    {
        for (int id = 0; id < lights.size(); id++) {
            // id not in free_ids
            if (std::find(free_ids.begin(), free_ids.end(), id) == free_ids.end()) {
                lights[id]->active_light = false;
            }
        }
        upload_all();
    }



    void Light::turn_on_one_lightsource()
    {
        if (std::find(free_ids.begin(), free_ids.end(), next_to_turn_on) == free_ids.end()) {
            lights[next_to_turn_on]->active_light = true;
            lights[next_to_turn_on]->upload();
        }
        if (++next_to_turn_on == lights.size()) {
            next_to_turn_on = 0;
        }
    }
*/
}
