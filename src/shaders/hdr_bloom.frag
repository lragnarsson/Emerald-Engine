

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D input_tex1;
uniform sampler2D input_tex2;

uniform float exposure;
uniform float scene_brightness; // TODO: Calculate average of brightness of screen.


void main()
{
    const float gamma = 1.8;

    vec3 hdrColor = texture(input_tex1, TexCoord).rgb + texture(input_tex2, TexCoord).rgb;

    // Tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma correction:
    result = pow(result, vec3(1.0 / gamma));

    OutColor  = vec4(result, 1.0f);
}
