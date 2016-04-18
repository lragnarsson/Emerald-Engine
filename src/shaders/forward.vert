#version 330

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

out vec3 Normal;
out vec2 TexCoord;
out vec3 ViewPos;
out vec3 FragPos;
//out vec3 light;
//out vec3 color;

uniform mat4 model;
uniform mat4 modelRot;
uniform mat4 view;
//uniform vec3 lightPos;
uniform mat4 projection;

void main(void)
{
    FragPos = vec3(model * vec4(in_Position, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    //    light = lightPos;
    Normal = mat3(modelRot) * in_Normal;
    TexCoord = in_TexCoord;
}
