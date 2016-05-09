

layout (location = 0) out vec4 g_position_depth;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_specular;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;


float linearize_depth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * _NEAR_ * _FAR_) / (_FAR_ + _NEAR_ - z * (_FAR_ - _NEAR_));	
}

void main()
{
    g_position_depth.rgba = vec4(FragPos, linearize_depth(gl_FragCoord.z));
    g_normal = normalize(Normal);
    g_albedo_specular.rgb = texture(texture_diffuse1, TexCoord).rgb;
    g_albedo_specular.a = texture(texture_specular1, TexCoord).r;
}
