


struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
};

out vec4 out_Color;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 CamPos;
in vec3 LightPos[_MAX_LIGHTS_];

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;

uniform float shininess;

const float ATT_CON = 1.0;
const float ATT_LIN = 0.001;
const float ATT_QUAD = 0.0008;

uniform Light lights[_MAX_LIGHTS_];

vec3 Normal;

vec3 PhongShading(Light l, int i) {
    float distance = length(LightPos[i] - FragPos);
    float attenuation = 1.0 / (ATT_CON + ATT_LIN * distance + ATT_QUAD * distance * distance);
    vec3 lightDir = normalize(LightPos[i] - FragPos);

    float d = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = d * l.color * vec3(texture(diffuse_map, TexCoord));

    vec3 reflection = normalize(reflect(-lightDir, Normal));

    vec3 viewDir = normalize(CamPos - FragPos);
    float s = pow(max(dot(viewDir, reflection), 0.0), shininess);
    vec3 specular = s * l.color * vec3(texture(specular_map, TexCoord));

    return attenuation * (diffuse + specular);
}

void main(void)
{
    int i;
    vec3 result = 0.1 * vec3(texture(diffuse_map, TexCoord));
    Normal = texture(normal_map, TexCoord).rgb;
    Normal = normalize(Normal * 2.0 - 1.0);

    for (i = 0; i < _MAX_LIGHTS_; i++) {
      if(lights[i].active_light)
      {
          result += PhongShading(lights[i], i);
      }
    }
    //out_Color = vec4(Normal, 1.0);
    out_Color =  vec4(result, 1.0);
}
