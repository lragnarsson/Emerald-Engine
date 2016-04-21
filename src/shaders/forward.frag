#version 330

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

out vec4 out_Color;
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D texture_Diffuse1;
uniform sampler2D texture_Specular1;

uniform vec3 camPos;

uniform Material m;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.008;
const float ATT_QUAD = 0.005;

const int MAX_LIGHTS = 20;
uniform Light lights[MAX_LIGHTS];


vec3 PhongShading(Light l) {
    float distance = length(l.position - FragPos);
    float attenuation = 5.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
    vec3 lightDir = normalize(l.position - FragPos);

    vec3 ambient =  l.color * m.ambient * vec3(texture(texture_Diffuse1, TexCoord));

    float d = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = d * l.color * m.diffuse * vec3(texture(texture_Diffuse1, TexCoord));

    vec3 reflection = normalize(reflect(-lightDir, Normal));

    vec3 viewDir = normalize(camPos - FragPos);
    float s = pow(max(dot(viewDir, reflection), 0.0), m.shininess);
    vec3 specular = s * l.color * vec3(texture(texture_Specular1, TexCoord));

    return attenuation * (diffuse + specular) + ambient;
}

void main(void)
{
    int i;
    vec3 result = vec3(0.0);

    for (i = 0; i < MAX_LIGHTS; i++) {
      if(lights[i].active_light)
      {
        result += PhongShading(lights[i]);
      }
    }

    out_Color =  vec4(result, 1.0);
}
