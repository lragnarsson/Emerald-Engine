


in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN;

layout (location = 0) out vec4 g_position_depth;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_specular;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;


void main()
{
    g_position_depth.rgb = FragPos;

    vec3 Normal = texture(normal_map, TexCoord).rgb;
    Normal = normalize(Normal * 2.0 - 1.0);
    g_normal = normalize(TBN * Normal);

    g_albedo_specular.rgb = texture(diffuse_map, TexCoord).rgb;
    g_albedo_specular.a = texture(specular_map, TexCoord).r;
}
