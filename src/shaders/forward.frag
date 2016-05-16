


struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
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
uniform Light lights[_MAX_LIGHTS_];

const float ATT_CON = 1.0;
const float ATT_LIN = 0.001;
const float ATT_QUAD = 0.0008;


vec3 PhongShading(Light l, vec3 Normal) {
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
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

    for (i = 0; i < _MAX_LIGHTS_; i++) {
      if(lights[i].active_light)
      {
          result += PhongShading(lights[i], Normal);
      }
    }

    out_Color =  vec4(result, 1.0);
}
