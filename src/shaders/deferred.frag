

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
uniform sampler2D shadow_map;

uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform bool sun_up;
uniform mat4 light_space_matrix;
// camera position is always 0,0,0 in view space


// ------------------
// Is this fragment in shadow or not?
float shadow_calculation(vec4 frag_pos_light_space, float bias)
{
    // perform perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // Transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    // Get depth of current fragment from light's perspective
    float current_depth = proj_coords.z;

    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    int core_size = 2;
    float shadow = 0;

    for(int x = -core_size; x <= core_size; ++x)
    {
        for(int y = -core_size; y <= core_size; ++y)
        {
            float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += current_depth - bias < pcf_depth ? 1.0 : 0.2;
        }
    }
    shadow /= (2 * core_size) * (2 * core_size);

    // If outside of shadow frustum, just put light there
    if(proj_coords.z > 1.0)
        shadow = 1.0;

    return shadow;
}

// ------------------

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

    // Complete shadow if sun is behind fragment:
    float shadow = 0.0;
    float sun_dot = dot(sun_direction, normal);
    if (sun_dot > 0) {
        float shadow_bias = max(0.012 * (1.0 - sun_dot), 0.005);
        shadow = shadow_calculation(light_space_matrix * vec4(position, 1.f), shadow_bias);
    }

    // Ambient
    vec3 light = 0.1 * occlusion * albedo;

    // Point lights:
    for(int i=0; i < num_lights; i++) {
        vec3 light_dir = normalize(lights[i].position - position);

        float distance = length(lights[i].position - position);
        float attenuation = 1.0 / (_ATT_CON_ + _ATT_LIN_ * distance + _ATT_QUAD_ * distance * distance);
        // Diffuse
        float d = max(dot(normal, light_dir), 0.0);
        vec3 diffuse_light = occlusion * d * lights[i].color * albedo;

        // Blinn-Phon Specular
        vec3 halfway_dir = normalize(light_dir + view_direction);
        float s = pow(max(dot(normal, halfway_dir), 0.0), shininess);

        vec3 specular_light = s * lights[i].color * albedo;

        light += attenuation * (diffuse_light + specular_light);
    }

    // Directional light source (sun):
    if (sun_up) {
        // Correct for sun color:
        albedo = albedo * albedo * (vec3(1) - 0.8 * normalize(sun_color));
        float d = max(sun_dot, 0.0);
        vec3 diffuse_light = occlusion * d * sun_color * albedo;
        vec3 halfway_dir = normalize(sun_direction + view_direction);
        float s = pow(max(dot(normal, halfway_dir), 0.0), shininess);
        vec3 specular_light = s * sun_color * albedo * albedo;
        light += shadow * diffuse_light + specular_light;
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
