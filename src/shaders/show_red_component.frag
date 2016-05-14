

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D input_tex;


void main()
{
    OutColor = vec4(vec3(texture(input_tex, TexCoord).r), 1.0f);
}
