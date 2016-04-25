

out vec4 out_Color;

uniform vec3 color;

void main(void)
{
    out_Color = vec4(color, 1.0);
}
