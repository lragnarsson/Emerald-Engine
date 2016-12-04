


layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoord;
layout (location = 3) in vec3 in_Tangent;

out VS_OUT {
    out vec2 TexCoord;
    out vec3 FragPos;   // view space position
    out vec3 Normal;    // view space normal
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    vs_out.FragPos = vec3(view * model * vec4(in_Position, 1.0));
    gl_Position = projection * vec4(vs_out.FragPos, 1.0);
    vs_out.TexCoord = in_TexCoord;

    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.Normal = normalize(normalMatrix * in_Normal);
}
