
in vec2 TexCoord;
out vec3 frag_color;

uniform sampler2D input_tex;

// [1 4 6 4 1]' kernel
// magnitude 16
void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result =
        1 * texture(input_tex, TexCoord - vec2(0, 2*texel_size.x)) +
        4 * texture(input_tex, TexCoord - vec2(0, texel_size.x)) +
        6 * texture(input_tex, TexCoord) +
        4 * texture(input_tex, TexCoord + vec2(0, texel_size.x)) +
        1 * texture(input_tex, TexCoord + vec2(0, 2*texel_size.x));

    frag_color = result.rgb / 16.0;
}
