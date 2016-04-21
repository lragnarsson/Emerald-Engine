#version 330

in vec3 in_Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void)
{
    gl_Position = projection * view * model *  vec4(in_Position, 1.0);
}
