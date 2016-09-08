
in vec2 TexCoord;
out float frag_color;

uniform sampler2D input_tex;

uniform float[3] kernel;
uniform float magnitude;


void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result =
        kernel[0] * texture(input_tex, TexCoord - vec2(texel_size.x, 0)) +
        kernel[1] * texture(input_tex, TexCoord) +
        kernel[2] * texture(input_tex, TexCoord + vec2(texel_size.x, 0));

    frag_color = result.r / magnitude;
}
