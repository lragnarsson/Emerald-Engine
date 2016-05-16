

layout (location = 0) in vec3 in_Position;
layout (location = 2) in vec2 in_TexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void)
{
    TexCoord = in_TexCoord;
    gl_Position = projection * view * model *  vec4(in_Position, 1.0);
}
