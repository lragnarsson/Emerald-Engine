#version 330 core


in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D g_component;

void main()
{
    OutColor = vec4(texture(g_component, TexCoord).rgb, 1);
}
