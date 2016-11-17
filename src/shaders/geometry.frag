


in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN_viewSpace;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec4 g_normal_shininess;
layout (location = 2) out vec4 g_albedo_specular;

uniform float shininess;
uniform vec3 tmp_foo;

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

    g_albedo_specular.rgb = tmp_foo; //texture(diffuse_map, TexCoord).rgb;
    g_albedo_specular.a = texture(specular_map, TexCoord).r;
}
