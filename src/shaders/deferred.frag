#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
};

//-------------------------

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;

uniform vec3 camPos;

uniform Material m;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.008;
const float ATT_QUAD = 0.005;

const int MAX_LIGHTS = 20;
uniform Light lights[MAX_LIGHTS];


void main()
{
    vec3 position = texture(g_position, TexCoord).rgb;
    vec3 normal = texture(g_normal, TexCoord).rgb;
    vec3 albedo = texture(g_albedo_specular, TexCoord).rgb;
    float specular = texture(g_albedo_specular, TexCoord).a;

    vec3 view_direction = normalize(camPos - position);

    // Ambient
    vec3 light = 0.1 * albedo;

    for(int i=0; i < MAX_LIGHTS; i++) {
        float distance = length(lights[i].position - position);
        float attenuation = 5.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
        vec3 light_dir = normalize(lights[i].position - position);

        // Diffuse
        float d = max(dot(normalize(normal), light_dir), 0.0);
        vec3 diffuse_light = d * lights[i].color *albedo;

        // Specular
        vec3 reflection = normalize(reflect(-light_dir, normal));
        float s = pow(max(dot(view_direction, reflection), 0.0), 80);
        vec3 specular_light = s * lights[i].color * specular;

        light += attenuation * (diffuse_light + specular_light);
    }

    OutColor = vec4(light, 1.0);
}
