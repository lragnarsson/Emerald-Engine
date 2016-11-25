
layout (triangles) in;
layout (triangle_strip, max_vertices = 54) out;

in VS_OUT {
    vec2 TexCoord;
    vec3 FragPos;  // view space position
    vec3 Normal;   // view space normal
} gs_in[];


out vec2 TexCoord;
out vec3 FragPos;  // view space position
out vec3 Normal;   // view space normal

uniform sampler2D wind_map;
uniform sampler2D diffuse_map;

// for converting normals to clip space
uniform mat4 projection;
uniform float upInterp; // Interpolation between up-vector and vertex own normal vector
uniform mat4 view;
uniform vec3 wind_direction;
uniform float wind_strength;
uniform vec2 time_offset;

const float MAGNITUDE = 0.1f; // Height scale for grass
const float GRASS_SCALE = 2.f; // Uniform scale for grass
const float OFFSET = 40.f; // Position offset in triangle

// Vertices for tall straight grass:
const float GRASS_1_X[9] = float[9](-0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, -0.151465, 0.151465, 0.000000);
const float GRASS_1_Y[9] = float[9](0.000000, 0.000000, 2.490297, 2.490297, 4.847759, 4.847759, 6.651822, 6.651822, 8.000000);
const float GRASS_2_X[7] = float[7](-0.435275, 0.435275, 0.037324, 0.706106, 1.814639, 2.406257, 3.000000);
const float GRASS_2_Y[7] = float[7](0.000000, 0.000000, 3.691449, 3.691449, 7.022911, 7.022911, 8.000000);
const float GRASS_3_X[7] = float[7](-1.200000, -0.300000, -0.600000, 0.600000, 0.600000, 1.200000, 1.800000);
const float GRASS_3_Y[7] = float[7](3.250000, 2.000000, 0.000000, 0.000000, 3.250000, 3.250000, 5.000000);

void generate_grass(vec4 clipPos, vec2 texCoord, vec3 fragPos, vec3 inNormal,
                       vec3 base_1, vec3 base_2, const float[7] grass_x, const float[7] grass_y)
{
    vec3 tangent = 0.06 * base_1 + 0.06 * base_2;
    vec3 grass_normal = normalize(cross(tangent, inNormal));
    if (grass_normal.z < 0) {
        grass_normal = -grass_normal;
    }

    vec4 clip_space_normal = projection * vec4(inNormal * MAGNITUDE, 0);
    vec4 ws_up_in_cs = projection * view * vec4(0, MAGNITUDE, 0, 0); // World space up in clip space
    vec4 interpNormal = mix(clip_space_normal, ws_up_in_cs, upInterp);

    vec2 wind_coord = fract(texCoord + wind_strength * time_offset);
    vec3 gradient = vec3(view * vec4(wind_strength * (wind_direction +
                                                      texture(wind_map, wind_coord).rgb), 0));
    gradient = gradient - dot(inNormal, gradient); // Project onto xz-plane
    gradient = vec3(view * vec4(gradient, 0));

    for (int i=0; i < 7; i++) {
        TexCoord = texCoord;
        Normal = grass_normal;
        float bend_interp = pow(grass_y[i] / grass_y[6], 2.7);

        float y = GRASS_SCALE * grass_y[i];
        vec3 xz = (GRASS_SCALE * grass_x[i]) * tangent + bend_interp * gradient;
        gl_Position = clipPos + projection * vec4(xz, 0) + interpNormal * y;
        FragPos = fragPos + xz + inNormal * y;

        EmitVertex();
    }
    EndPrimitive();
}


void main()
{
    vec3 frag_pos_base_02 = normalize(gs_in[2].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_01 = normalize(gs_in[1].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_12 = normalize(gs_in[2].FragPos - gs_in[1].FragPos);

    vec3 frag_pos, frag_pos_01, frag_pos_02, normal, normal_01, normal_02;
    vec2 tex_coord, tex_coord_01, tex_coord_02;
    vec4 clip_pos;
    float par2 = 0.0f;
    const int n_lines = 3;
    float step_len = 1/float(2 * (n_lines + 1));
    float par = -2 *step_len;
    /* This loop creates lines in a pattern like this:
       The pipes are the lines created. No lines are created on the triangle edges.
                      V0
                     / | \
                    /|   |\
                   /|  |  |\
                 V1---------V2
    */
    for (int i=1; i<=n_lines; i++) {
        par = par + 3 *step_len;

        frag_pos_01 = mix(gs_in[0].FragPos, gs_in[1].FragPos, par);
        tex_coord_01 = mix(gs_in[0].TexCoord, gs_in[1].TexCoord, par);
        normal_01 = normalize(mix(gs_in[0].Normal, gs_in[2].Normal, par));
        frag_pos_02 = mix(gs_in[0].FragPos, gs_in[2].FragPos, par);
        tex_coord_02 = mix(gs_in[0].TexCoord, gs_in[2].TexCoord, par);
        normal_02 = normalize(mix(gs_in[0].Normal, gs_in[2].Normal, par));

        float step_2 = 1/float(2 * (i + 1));
        par2 = -2 *step_2;
        bool tall = true;
        for (int j=1; j<=i; j++) {
            par2 = par2 + 3 * step_2;
            frag_pos = mix(frag_pos_01, frag_pos_02, par2);
            clip_pos = projection * vec4(frag_pos, 1.0f);
            tex_coord = mix(tex_coord_01, tex_coord_02, par2);
            normal = normalize(mix(normal_01, normal_02, par2));

            if (tall)
                generate_grass(clip_pos, tex_coord, frag_pos, normal,
                               frag_pos_base_01, frag_pos_base_02,
                               GRASS_2_X, GRASS_2_Y);
            else
                generate_grass(clip_pos, tex_coord, frag_pos, normal,
                               frag_pos_base_01, frag_pos_base_02,
                               GRASS_3_X, GRASS_3_Y);
            tall = !tall;
        }
    }
}
