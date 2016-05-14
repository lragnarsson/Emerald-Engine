
in vec2 TexCoord;
out float frag_color;

uniform sampler2D ssao_input;
const int blur_size = 4;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(ssao_input, 0));
    float result = 0.0;

    for (int x = -2; x < blur_size/2; x++) {
        for (int y = -blur_size/2; y < blur_size/2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            result += texture(ssao_input, TexCoord + offset).r;
        }
    }
    frag_color = result / float(blur_size * blur_size);
}
