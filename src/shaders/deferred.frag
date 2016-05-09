


struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
};

//-------------------------

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D g_position_depth;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;
uniform sampler2D ssao_result;

uniform vec3 camPos;
const float shininess = 86.0;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.001;
const float ATT_QUAD = 0.0008;

uniform Light lights[_MAX_LIGHTS_];

const float NEAR = 0.1f;
const float FAR = 100.0f;


void main()
{
    vec3 position = texture(g_position_depth, TexCoord).rgb;
    // new
    float depth = texture(g_position_depth, TexCoord).a;
    //
    vec3 normal = texture(g_normal, TexCoord).rgb;
    vec3 albedo = texture(g_albedo_specular, TexCoord).rgb;
    float specular = texture(g_albedo_specular, TexCoord).a;
    float occlusion = texture(ssao_result, TexCoord).r; // Only red 
    
    vec3 view_direction = normalize(camPos - position);

    // Ambient
    vec3 light = occlusion * albedo;

    for(int i=0; i < _MAX_LIGHTS_; i++) {
        float distance = length(lights[i].position - position);
        float attenuation = 1.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
        vec3 light_dir = normalize(lights[i].position - position);

        // Diffuse
        float d = max(dot(normalize(normal), light_dir), 0.0);
        vec3 diffuse_light = d * lights[i].color * albedo;

        // Specular
        vec3 reflection = normalize(reflect(-light_dir, normal));
        float s = pow(max(dot(view_direction, reflection), 0.0), shininess);
        vec3 specular_light = s * lights[i].color * specular;

        light += attenuation * (diffuse_light + specular_light);
    }
    
    OutColor = vec4(light, 1);
}
