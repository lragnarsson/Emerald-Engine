

out vec4 out_Color;

uniform vec3 color;


const float NEAR = 0.1f;
const float FAR = 100.0f;


void main(void)
{
    out_Color = vec4(color, 1.0);
}
