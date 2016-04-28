#version 330


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
uniform float shininess;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.001;
const float ATT_QUAD = 0.0008;

const int MAX_LIGHTS = 20;
uniform Light lights[MAX_LIGHTS];


vec3 PhongShading(Light l) {
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
    vec3 lightDir = normalize(l.position - FragPos);

    float d = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = d * l.color * vec3(texture(texture_Diffuse1, TexCoord));

    vec3 reflection = normalize(reflect(-lightDir, Normal));

    vec3 viewDir = normalize(camPos - FragPos);
    float s = pow(max(dot(viewDir, reflection), 0.0), shininess);
    vec3 specular = s * l.color * vec3(texture(texture_Specular1, TexCoord));

    return attenuation * (diffuse + specular);
}

void main(void)
{
    int i;
    vec3 result = 0.1 * vec3(texture(texture_Diffuse1, TexCoord));

    for (i = 0; i < MAX_LIGHTS; i++) {
      if(lights[i].active_light)
      {
        result += PhongShading(lights[i]);
      }
    }
    out_Color =  vec4(result, 1.0);
}
