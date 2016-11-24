


in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN_viewSpace;
in vec4 frag_pos_light_space;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec4 g_normal_shininess;
layout (location = 2) out vec4 g_albedo_specular;
layout (location = 3) out vec4 frag_pos_light_texture;

uniform float shininess;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;


void main()
{
    g_position.rgb = FragPos;

    vec3 Normal = texture(normal_map, TexCoord).rgb;
    Normal = normalize(Normal * 2.0 - 1.0);
    g_normal_shininess.rgb = normalize(TBN_viewSpace * Normal);
    g_normal_shininess.a = shininess;

    g_albedo_specular.rgb = texture(diffuse_map, TexCoord).rgb;
    g_albedo_specular.a = texture(specular_map, TexCoord).r;
}
