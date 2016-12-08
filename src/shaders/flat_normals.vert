

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 Normal;   // view space normal
} vs_out;

void main(void)
{
    vs_out.Normal = vec3(view * model * vec4(in_Normal, 0.0));
    gl_Position = projection * view * model *  vec4(in_Position, 1.0);
}
