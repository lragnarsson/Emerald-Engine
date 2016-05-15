

in vec2 TexCoord;

out vec4 out_Color;

uniform sampler2D tex_unit;


void main(void)
{
    out_Color = texture(tex_unit, TexCoord);
}
