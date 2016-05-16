



layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoord;
layout (location = 3) in vec3 in_Tangent;

out vec2 TexCoord;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void)
{
    FragPos = vec3(model * vec4(in_Position, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoord = in_TexCoord;

    // Create TBN Matrix to convert normals to world space in fragment shader
    vec3 T = normalize(vec3(model * vec4(in_Tangent,   0.0)));
    vec3 N = normalize(vec3(model * vec4(in_Normal,    0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);

    TBN = mat3(T, B, N);
}
