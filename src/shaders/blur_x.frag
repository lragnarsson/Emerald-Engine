
in vec2 TexCoord;
out float frag_color;

uniform sampler2D input_tex;

// [1 2 1] kernel
// magnitude 4
void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result = texture(input_tex, TexCoord - vec2(texel_size.x, 0)) +
        2 * texture(input_tex, TexCoord) +
        texture(input_tex, TexCoord + vec2(texel_size.x, 0));

    frag_color = result.r / 4.0;
}
