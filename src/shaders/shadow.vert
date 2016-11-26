
layout (location = 0) in vec3 in_position;

out vec4 frag_pos_light_space;

uniform mat4 model;
uniform mat4 light_space_matrix;

void main()
{
    gl_Position = light_space_matrix * model * vec4(in_position, 1.0f); // Shadow map
}

