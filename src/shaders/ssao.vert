

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TexCoord;
//in vec3 in_Position;
//in vec2 in_TexCoord;

out vec2 TexCoord;

void main(void)
{
    TexCoord = in_TexCoord;
    gl_Position = vec4(in_Position, 1.0);
}
