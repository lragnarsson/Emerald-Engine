#version 330

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

out vec4 out_Color;
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 ViewPos;

uniform sampler2D texture_Diffuse1;
uniform sampler2D texture_Specular1;

uniform vec3 camPos;

uniform Material m;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.008;
const float ATT_QUAD = 0.005;

const int MAX_LIGHTS = 20;
uniform Light lights[MAX_LIGHTS];
uniform int nLights;


vec3 PhongShading(Light l) {
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
    vec3 lightDir = normalize(l.position - FragPos);

    vec3 ambient =  l.ambientColor * m.ambient * vec3(texture(texture_Diffuse1, TexCoord));

    float d = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = d * l.diffuseColor * m.diffuse * vec3(texture(texture_Diffuse1, TexCoord));

    vec3 reflection = normalize(reflect(-lightDir, Normal));
    vec3 viewDir = normalize(ViewPos - FragPos);
    float s = pow(max(dot(viewDir, reflection), 0.0), m.shininess);
    vec3 specular = s * l.specularColor * m.specular * vec3(texture(texture_Specular1, TexCoord));

    return attenuation * (ambient + diffuse + specular);
}

void main(void)
{
    int i;
    vec3 result = vec3(0.0);

    for (i = 0; i < nLights; i++) {
        result += PhongShading(lights[i]);
    }

    out_Color =  vec4(result, 1.0);
}
