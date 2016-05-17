

struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
};

//-------------------------

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D g_position;
uniform sampler2D g_normal_shininess;
uniform sampler2D g_albedo_specular;
uniform sampler2D ssao_blurred;

uniform vec3 camPos;

uniform Light lights[_MAX_LIGHTS_];

void main()
{
    vec3 position = texture(g_position, TexCoord).rgb;
    vec3 normal = texture(g_normal_shininess, TexCoord).rgb;
    float shininess = texture(g_normal_shininess, TexCoord).a;
    vec3 albedo = texture(g_albedo_specular, TexCoord).rgb;
    float specular = texture(g_albedo_specular, TexCoord).a;
    float occlusion = texture(ssao_blurred, TexCoord).r; // Only red
    vec3 view_direction = normalize(camPos - position);

    // Ambient
    vec3 light = 0.05 * occlusion * albedo;

    for(int i=0; i < _MAX_LIGHTS_; i++) {
        if (lights[i].active_light)
            {
                float distance = length(lights[i].position - position);
                float attenuation = 1.0 / (_ATT_CON_ + _ATT_LIN_ * distance + _ATT_QUAD_ * distance * distance);
                vec3 light_dir = normalize(lights[i].position - position);
                
                // Diffuse
                float d = max(dot(normalize(normal), light_dir), 0.0);
                vec3 diffuse_light = occlusion * d * lights[i].color * albedo;

                // Specular
                vec3 reflection = normalize(reflect(-light_dir, normal));
                float s = specular * pow(max(dot(view_direction, reflection), 0.0), shininess);
                vec3 specular_light = s * lights[i].color * albedo;

                light += attenuation * (diffuse_light + specular_light);
            }
    }

    OutColor = vec4(light, 1.0);
}
