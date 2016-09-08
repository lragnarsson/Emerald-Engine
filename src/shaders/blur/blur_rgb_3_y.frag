
in vec2 TexCoord;
out vec3 frag_color;

uniform sampler2D input_tex;

uniform float[3] kernel;
uniform float magnitude;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(input_tex, 0));

    vec4 result =
        kernel[0] * texture(input_tex, TexCoord - vec2(0, texel_size.x)) +
        kernel[1] * texture(input_tex, TexCoord) +
        kernel[2] * texture(input_tex, TexCoord + vec2(0, texel_size.x));

    frag_color = result.rgb / magnitude;
}
