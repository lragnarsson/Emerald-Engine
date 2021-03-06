

layout (location = 0) in vec3 in_Position;

out vec3 sky_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void)
{
    sky_pos = vec3(model * vec4(in_Position, 1));
    gl_Position = projection * view * model *  vec4(in_Position, 1.0);
}
