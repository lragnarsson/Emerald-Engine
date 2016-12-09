in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec4 g_normal_shininess;
layout (location = 2) out vec4 g_albedo_specular;

uniform vec3 color;
uniform float shininess;

const float SPECULAR = 1.0f;

void main()
{
    g_position.rgb = FragPos;

    g_normal_shininess.rgb = Normal;
    g_normal_shininess.a = shininess;
    g_albedo_specular.rgb = color;
    g_albedo_specular.a = SPECULAR;
}
