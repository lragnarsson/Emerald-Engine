


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

out vec4 out_Color;

in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;

uniform float shininess;
uniform vec3 camPos;
//uniform Light lights[_MAX_LIGHTS_];


vec3 PhongShading(Light l, vec3 Normal) {
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (_ATT_CON_ + _ATT_LIN_ * distance + _ATT_QUAD_ * distance * distance);
    vec3 lightDir = normalize(l.position - FragPos);

    float d = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = d * l.color * vec3(texture(diffuse_map, TexCoord));

    vec3 reflection = normalize(reflect(-lightDir, Normal));

    vec3 viewDir = normalize(camPos - FragPos);
    float s = pow(max(dot(viewDir, reflection), 0.0), shininess);
    vec3 specular = s * l.color * vec3(texture(diffuse_map, TexCoord)) * vec3(texture(specular_map, TexCoord).r);

    return attenuation * (diffuse + specular);
}

void main(void)
{
    int i;
    vec3 result = 0.05 * vec3(texture(diffuse_map, TexCoord));
    vec3 Normal = texture(normal_map, TexCoord).rgb;
    Normal = normalize(TBN * (Normal * 2.0 - vec3(1.0)));

    for (i = 0; i < num_lights; i++) {
        result += PhongShading(lights[i], Normal);
    }

    out_Color =  vec4(result, 1.0);
}
