


struct Light {
    vec3 position;
    vec3 color;
    bool active_light;
};


layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoord;
layout (location = 3) in vec3 in_Tangent;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 CamPos;
out vec3 LightPos[_MAX_LIGHTS_];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPos;
uniform Light lights[_MAX_LIGHTS_];


void main(void)
{
    vec3 fragPos = vec3(model * vec4(in_Position, 1.0));
    gl_Position = projection * view * vec4(fragPos, 1.0);
    TexCoord = in_TexCoord;

    // Transform camera position, Fragment position and all light positions to tangent space.
    mat3 normal_matrix = transpose(inverse(mat3(model)));

    vec3 T = normalize(normal_matrix * in_Tangent);
    vec3 N = normalize(normal_matrix * in_Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);

    mat3 TBN = transpose(mat3(T, B, N));

    CamPos = TBN * camPos;
    FragPos = TBN * fragPos;

    int i;
    for (i = 0; i < _MAX_LIGHTS_; i++) {
      if(lights[i].active_light)
      {
          LightPos[i] = TBN * lights[i].position;
      }
    }
}
