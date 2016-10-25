

in vec2 TexCoord;
out vec3 frag_color;

uniform sampler2D input_tex;

uniform float[11] kernel;
uniform float magnitude;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));
    vec3 result = kernel[5] * texture(input_tex, TexCoord).rgb;

    result +=
        kernel[0] * texture(input_tex, TexCoord - vec2(0, 5*texel_size.y)).rgb +
        kernel[1] * texture(input_tex, TexCoord - vec2(0, 4*texel_size.y)).rgb +
        kernel[2] * texture(input_tex, TexCoord - vec2(0, 3*texel_size.y)).rgb +
        kernel[3] * texture(input_tex, TexCoord - vec2(0, 2*texel_size.y)).rgb +
        kernel[4] * texture(input_tex, TexCoord - vec2(0, texel_size.y)).rgb +
        kernel[6] * texture(input_tex, TexCoord + vec2(0, texel_size.y)).rgb +
        kernel[7] * texture(input_tex, TexCoord + vec2(0, 2*texel_size.y)).rgb +
        kernel[8] * texture(input_tex, TexCoord + vec2(0, 3*texel_size.y)).rgb +
        kernel[9] * texture(input_tex, TexCoord + vec2(0, 4*texel_size.y)).rgb +
        kernel[10] * texture(input_tex, TexCoord + vec2(0, 5*texel_size.y)).rgb;

    frag_color = result / magnitude;
}
