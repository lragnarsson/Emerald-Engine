#version 330 core


in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D g_component;

void main()
{
    float specular = texture(g_component, TexCoord).a;
    OutColor = vec4(specular, specular, specular, 1);
}
