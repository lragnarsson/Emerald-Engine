out float FragColor;
in vec2 TexCoord;

uniform sampler2D g_position_depth;
uniform sampler2D g_normal;
uniform sampler2D tex_noise;

uniform vec3 samples[64]; // Kernel samples from surrounding screen space geometry
uniform mat4 projection;

uniform float kernel_radius;

const float SCREEN_WIDTH = 800.0;
const float SCREEN_HEIGHT = 640.0;

// scaling for the noise texture coords to ensure that the noise texture tiles across the screen.
const vec2 noise_scale = vec2(SCREEN_WIDTH / 4.0, SCREEN_HEIGHT / 4.0);

int kernel_size = 64;

void main()
{
    vec3 frag_pos = texture(g_position_depth, TexCoord).xyz;
    vec3 normal = texture(g_normal, TexCoord).rgb;
    vec3 random_vec = texture(tex_noise, TexCoord * noise_scale).xyz;

    // Create tangent-to-viewspace matrix to transform tangent-space samples to view-space
    // creates an ON-basis using Gram-Schmidt
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < kernel_size; ++i)
        {
            // get sample position
            vec3 sample = TBN * samples[i]; // From tangent to view-space
            sample = frag_pos + sample * kernel_radius; 

            vec4 offset = vec4(sample, 1.0);
            offset = projection * offset; // from view to clip-space
            offset.xyz /= offset.w; // perspective divide
            offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
            float sample_depth = -texture(g_position_depth, offset.xy).w; 
            float range_check = smoothstep(0.0, 1.0, kernel_radius / abs(frag_pos.z - sample_depth));
            occlusion += (sample_depth >= sample.z ? 1.0 : 0.0) * range_check;    
        }  
    occlusion = 1.0 - occlusion / kernel_radius; // To make sure that the occlusion is [0 1]

    FragColor = occlusion;
}
