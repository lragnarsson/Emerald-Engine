

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D input_tex;


void main()
{
    OutColor = vec4(texture(input_tex, TexCoord).rgb, 1.0f);
}
