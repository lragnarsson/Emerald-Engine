

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D input_tex1;
uniform sampler2D input_tex2;

uniform float alpha;
uniform float beta;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor =
        alpha * texture(input_tex1, TexCoord).rgb +
        beta * texture(input_tex2, TexCoord).rgb;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));
    OutColor  = vec4(result, 1.0f);
}
