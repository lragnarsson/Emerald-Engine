
in vec2 TexCoord;
out vec3 frag_color;

uniform sampler2D input_tex;

uniform float[5] kernel;
uniform float magnitude;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result =
        kernel[0] * texture(input_tex, TexCoord - vec2(2*texel_size.x, 0)) +
        kernel[1] * texture(input_tex, TexCoord - vec2(texel_size.x, 0)) +
        kernel[2] * texture(input_tex, TexCoord) +
        kernel[3] * texture(input_tex, TexCoord + vec2(texel_size.x, 0)) +
        kernel[4] * texture(input_tex, TexCoord + vec2(2*texel_size.x, 0));

    frag_color = result.rgb / magnitude;
}
