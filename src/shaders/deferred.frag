

struct Light {
    vec3 position;
    float brightness;
    vec3 color;
    float padding;
};

layout (std140) uniform light_block {
    Light lights[_MAX_LIGHTS_];
};

layout (std140) uniform light_info_block {
    int num_lights;
};


//-------------------------

in vec2 TexCoord;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D g_position;
uniform sampler2D g_normal_shininess;
uniform sampler2D g_albedo_specular;
uniform sampler2D ssao_blurred;

uniform vec3 sun_direction;
uniform vec3 sun_color;
// camera position is always 0,0,0 in view space


void main()
{
    const vec3 eye_colors = vec3(0.2126, 0.7152, 0.0722);
    vec3 position = texture(g_position, TexCoord).rgb;
    vec3 normal = texture(g_normal_shininess, TexCoord).rgb;
    float shininess = texture(g_normal_shininess, TexCoord).a;
    vec3 albedo = texture(g_albedo_specular, TexCoord).rgb;
    float specular = texture(g_albedo_specular, TexCoord).a;
    float occlusion = texture(ssao_blurred, TexCoord).r; // Only red
    vec3 view_direction = normalize(- position);

    // Ambient
    vec3 light = 0.03 * occlusion * albedo;

    // Point lights:
    for(int i=0; i < num_lights; i++) {
        float distance = length(lights[i].position - position);
        float attenuation = 1.0 / (_ATT_CON_ + _ATT_LIN_ * distance + _ATT_QUAD_ * distance * distance);
        vec3 light_dir = normalize(lights[i].position - position);

        // Diffuse
        float d = max(dot(normalize(normal), light_dir), 0.0);
        vec3 diffuse_light = occlusion * d * lights[i].color * albedo;

        // Blinn-Phon Specular
        vec3 halfway_dir = normalize(light_dir + view_direction);
        float s = pow(max(dot(normal, halfway_dir), 0.0), shininess);

        vec3 specular_light = s * lights[i].color * albedo;

        light += attenuation * (diffuse_light + specular_light);
    }

    // Directional light source (sun):
    float d = max(dot(normalize(normal), sun_direction), 0.0);
    vec3 diffuse_light = occlusion * d * sun_color * albedo;
    vec3 halfway_dir = normalize(sun_direction + view_direction);
    float s = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    vec3 specular_light = s * sun_color * albedo;
    light += diffuse_light + specular_light;


    OutColor = vec4(light, 1.0);

    /* Filter out parts above 1 in brightness */
    // Human eye is more sensitive to red and green than blue. learnopengl.com
    float brightness = dot(OutColor.rgb, eye_colors);
    if (brightness > 1.0)
        BrightColor = vec4(OutColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0);
}
