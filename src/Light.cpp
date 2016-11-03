#include "Light.hpp"


namespace Light
{
    Light lights[_MAX_LIGHTS_]; // Always same order
    Light_meta light_meta[_MAX_LIGHTS_]; // Metadata about lights such as radius, on/off.
    std::vector<GLuint> shader_programs; // For UBO binding on init.
    int num_lights;
    int culled_lights;


    int create_light(glm::vec3 position, float brightness,
                     glm::vec3 color)
    {
        int index;
        int available = Detail::available_indices.size();
        if (available > 0) {
            index = Detail::available_indices[available - 1];
            Detail::available_indices.pop_back();
        }
        else if (num_lights < _MAX_LIGHTS_) {
            index = num_lights;
            num_lights++;
        } else { // Reached max number of lights
            return -1;
        }
        lights[index].position = position;
        lights[index].brightness = brightness;
        lights[index].color = color;
        Detail::generate_bounding_sphere(index);
        return index;
    }


    void destroy_light(int index)
    {
        if (index >= 0 && index < num_lights) {
            light_meta[index].active = false;
            light_meta[index].marked = true;
            if (index == num_lights - 1) {
                num_lights--;
            }
        }
    }


    void upload_lights()
    {
        int visible_lights = num_lights - culled_lights;

        glBindBuffer(GL_UNIFORM_BUFFER, Detail::ubos[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0,
                        Detail::light_size * visible_lights,
                        Detail::gpu_lights);

        glBindBuffer(GL_UNIFORM_BUFFER, Detail::ubos[1]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0,
                        Detail::info_size, &visible_lights);

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

        glGenBuffers(2, Detail::ubos);
        glBindBuffer(GL_UNIFORM_BUFFER, Detail::ubos[0]);
        glBufferData(GL_UNIFORM_BUFFER,
                     Detail::light_size * _MAX_LIGHTS_,
                     NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_UNIFORM_BUFFER, Detail::ubos[1]);
        glBufferData(GL_UNIFORM_BUFFER, Detail::info_size,
                     NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, Detail::ubos[0]);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, Detail::ubos[1]);

        turn_on_all_lights();
    }


    void cull_light_sources(Camera &camera)
    {
        Profiler::start_timer("Cull lights");
        int num_visible = 0;
        culled_lights = 0;
        for (int i=0; i<num_lights; i++) {
            // Use lights which are turned on or inside viewing frustum:
            if (!light_meta[i].marked &&
                light_meta[i].active &&
                camera.sphere_in_frustum(lights[i].position,
                                         light_meta[i].radius)) {
                // Move visible lights to gpu array:
                // Position in view-space:
                glm::vec3 view_space_pos = glm::vec3(camera.get_view_matrix() *
                                                     glm::vec4(lights[i].position, 1.f));
                Detail::gpu_lights[num_visible].position = view_space_pos;
                Detail::gpu_lights[num_visible].brightness = lights[i].brightness;
                Detail::gpu_lights[num_visible].color = lights[i].color;
                num_visible++;
            } else {
                culled_lights++;
            }
        }
        Profiler::stop_timer("Cull lights");
    }

    void turn_off_all_lights()
    {
        for (int i = 0; i < num_lights; i++) {
            light_meta[i].active = false;
        }
    }


    void turn_on_all_lights()
    {
        for (int i = 0; i < num_lights; i++) {
            light_meta[i].active = true;
        }
    }


    void turn_on_one_light()
    {
        if (Detail::next_to_turn_on < num_lights) {
            light_meta[Detail::next_to_turn_on].active = true;
        }
        if (++Detail::next_to_turn_on == num_lights) {
            Detail::next_to_turn_on = 0;
        }
    }


    namespace Detail {
        Light gpu_lights[_MAX_LIGHTS_]; // Sorted, pushed to GPU
        std::vector<int> available_indices;
        GLuint ubos[2]; // Light ubo, light_info ubo
        int next_to_turn_on;

        void generate_bounding_sphere(int light)
        {
            float a = _ATT_CON_;
            float b = _ATT_LIN_;
            float c = _ATT_QUAD_;
            float alpha = 0.02; // 2 percent
            float beta = glm::length(lights[light].color) * lights[light].brightness;

            // Solve quadratic equation to determine at what distance the light is dimmer than alpha times beta:
            light_meta[light].radius = -b / (2 * c) + std::sqrt(b * b / (4 * c * c) - a / c + beta / (alpha * c));
        }
    }
}
