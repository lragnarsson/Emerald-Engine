#version 330 core
layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_specular;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    g_position = FragPos;
    g_normal = normalize(Normal);
    g_albedo_specular.rgb = texture(texture_diffuse1, TexCoord).rgb;
    g_albedo_specular.a = texture(texture_specular1, TexCoord).r;
}
