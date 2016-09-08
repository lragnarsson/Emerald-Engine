
in vec2 TexCoord;
out float frag_color;

uniform sampler2D input_tex;

// [1 4 6 4 1] kernel
// magnitude 16
void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result =
        1 * texture(input_tex, TexCoord - vec2(2*texel_size.x, 0)) +
        4 * texture(input_tex, TexCoord - vec2(texel_size.x, 0)) +
        6 * texture(input_tex, TexCoord) +
        4 * texture(input_tex, TexCoord + vec2(texel_size.x, 0)) +
        1 * texture(input_tex, TexCoord + vec2(2*texel_size.x, 0));

    frag_color = result.r / 16.0;
}
