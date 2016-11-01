

struct Light {
    vec3 viewSpacePosition;
    vec3 color;
    bool active_light;
};

//-------------------------

in vec2 TexCoord;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D g_position;
uniform sampler2D g_normal_shininess;
uniform sampler2D g_albedo_specular;
uniform sampler2D ssao_blurred;

// camera position is always 0,0,0 in view space
// uniform vec3 camPos; 

uniform Light lights[_MAX_LIGHTS_];


void main()
{
    const vec3 eye_colors = vec3(0.2126, 0.7152, 0.0722);
    vec3 viewSpacePosition = texture(g_position, TexCoord).rgb;
    vec3 normal = texture(g_normal_shininess, TexCoord).rgb;
    float shininess = texture(g_normal_shininess, TexCoord).a;
    vec3 albedo = texture(g_albedo_specular, TexCoord).rgb;
    float specular = texture(g_albedo_specular, TexCoord).a;
    float occlusion = texture(ssao_blurred, TexCoord).r; // Only red
    vec3 view_direction = normalize(- viewSpacePosition);

    // Ambient
    vec3 light = 0.03 * occlusion * albedo;

    for(int i=0; i < _MAX_LIGHTS_; i++) {
        if (lights[i].active_light)
            {
                float distance = length(lights[i].viewSpacePosition - viewSpacePosition);
                float attenuation = 1.0 / (_ATT_CON_ + _ATT_LIN_ * distance + _ATT_QUAD_ * distance * distance);
                vec3 light_dir = normalize(lights[i].viewSpacePosition - viewSpacePosition);

                // Diffuse
                float d = max(dot(normalize(normal), light_dir), 0.0);
                vec3 diffuse_light = occlusion * d * lights[i].color * albedo;

                // Blinn-Phon Specular
                vec3 halfway_dir = normalize(light_dir + view_direction);
                float s = pow(max(dot(normal, halfway_dir), 0.0), shininess);

                vec3 specular_light = s * lights[i].color * albedo; 

                light += attenuation * (diffuse_light + specular_light);
            }
    }

    OutColor = vec4(light, 1.0);

    /* Filter out parts above 1 in brightness */
    // Human eye is more sensitive to red and green than blue. learnopengl.com
    float brightness = dot(OutColor.rgb, eye_colors);
    if (brightness > 1.0)
        BrightColor = vec4(OutColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0);
}
