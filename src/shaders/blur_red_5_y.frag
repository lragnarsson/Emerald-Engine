
in vec2 TexCoord;
out float frag_color;

uniform sampler2D input_tex;

uniform float[5] kernel;
uniform float magnitude;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));
    vec4 result = kernel[2] * texture(input_tex, TexCoord);

    result +=
        kernel[0] * texture(input_tex, TexCoord - vec2(0, 2*texel_size.y)) +
        kernel[1] * texture(input_tex, TexCoord - vec2(0, texel_size.y)) +
        kernel[3] * texture(input_tex, TexCoord + vec2(0, texel_size.y)) +
        kernel[4] * texture(input_tex, TexCoord + vec2(0, 2*texel_size.y));

    frag_color = result.r / magnitude;
}
