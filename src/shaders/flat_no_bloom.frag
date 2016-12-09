

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 color;


void main(void)
{
    const vec3 eye_colors = vec3(0.2126, 0.7152, 0.0722);
    OutColor = vec4(color, 1.0);
    BrightColor = vec4(0.0);
}
