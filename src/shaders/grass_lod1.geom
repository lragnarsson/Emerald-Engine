
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

struct Sphere {
    vec3 position;
    float radius;
};

layout (std140) uniform sphere_block {
    Sphere spheres[_MAX_MODELS_];
};

layout (std140) uniform sphere_info_block {
    int num_spheres;
};


uniform sampler2D wind_map;
uniform sampler2D diffuse_map;

// for converting normals to clip space
uniform mat4 projection;
uniform float upInterp; // Interpolation between up-vector and vertex own normal vector
uniform mat4 view;
uniform vec3 wind_direction;
uniform float wind_strength;
uniform vec2 time_offset;

const float MAGNITUDE = 0.5f; // Height scale for grass
const float GRASS_SCALE = 1.f; // Uniform scale for grass

// Tall straight grass:
const float GRASS_1_X[7] = float[7](-0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, 0.000000);
const float GRASS_1_Y[7] = float[7](0.000000, 0.000000, 2.490297, 2.490297, 4.847759, 4.847759, 8.000000);
// Tall bent grass:
const float GRASS_2_X[7] = float[7](-0.435275, 0.435275, 0.037324, 0.706106, 1.814639, 2.406257, 3.000000);
const float GRASS_2_Y[7] = float[7](0.000000, 0.000000, 3.691449, 3.691449, 7.022911, 7.022911, 8.000000);
// Chubby double grass:
const float GRASS_3_X[7] = float[7](-1.200000, -0.400000, -0.200000, 0.400000, 0.400000, 1.000000, 1.300000);
const float GRASS_3_Y[7] = float[7](1.800000, 1.000000, 0.000000, 0.000000, 1.800000, 1.800000, 4.000000);

// u,v coordinates for grass locations inside triangle
const float COORDS_U[6] = float[6](0.125000, 0.125000, 0.437500, 0.125000, 0.437500, 0.750000);
const float COORDS_V[6] = float[6](0.750000, 0.437500, 0.437500, 0.125000, 0.125000, 0.125000);
const int N_GRASS_STRAWS = 6;


highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}


// TODO: precompute things like interpolation values.
void generate_grass(vec2 texCoord, vec3 fragPos, vec3 inNormal,
                    vec3 base_1, vec3 base_2, const float[7] grass_x, const float[7] grass_y)
{
    highp float noise_u = 2 * rand(texCoord) - 1;
    highp float noise_v = 2 * rand(texCoord.ts) - 1;

    vec3 up = vec3(0, 1, 0);
    vec3 tangent = 0.3 * (noise_u * base_1 + noise_v * base_2);
    fragPos = fragPos + 0.3 * (noise_u * base_1 + noise_v * base_2) - 0.3 * up;

    vec2 wind_coord = fract(0.005 * texCoord + wind_strength * time_offset);
    vec3 gradient = wind_strength * (0.7 * wind_direction + 1.5 * texture(wind_map, wind_coord).rgb);
    gradient = vec3(0);

    vec3 push;
    for (int i=0; i < 25; i++) { // num_spheres ??????????????????
        push = fragPos - spheres[i].position;
        if (length(push) < 5 * spheres[i].radius) {
            gradient = gradient + 3 * push * (1 - length(push) / (5 * spheres[i].radius));
        }
    }

    gradient = gradient - up * dot(up, gradient); // Project onto xz-plane

    vec3 grass_normal = normalize(cross(tangent, inNormal));
    if (grass_normal.z < 0) {
        grass_normal = -grass_normal;
    }

    float bend_interp;

    for (int i=0; i < 7; i++) {
        TexCoord = texCoord;
        bend_interp = pow(grass_y[i] / grass_y[6], 2);
        Normal = normalize(inNormal + 0.2 * grass_normal);

        vec3 y = MAGNITUDE * inNormal * GRASS_SCALE * grass_y[i];
        vec3 xz = GRASS_SCALE * (grass_x[i] * tangent + bend_interp * gradient);

        FragPos = fragPos + xz + y;
        gl_Position = projection * vec4(FragPos, 1);

        EmitVertex();
    }
    EndPrimitive();
}


void main()
{
    vec3 frag_pos_base_02 = normalize(gs_in[2].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_01 = normalize(gs_in[1].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_01 = gs_in[1].FragPos - gs_in[0].FragPos;
    vec3 frag_pos_02 = gs_in[2].FragPos - gs_in[0].FragPos;
    vec3 frag_pos;

    vec2 tex_coord;
    vec2 tex_coord_base_01 = gs_in[1].TexCoord - gs_in[0].TexCoord;
    vec2 tex_coord_base_02 = gs_in[2].TexCoord - gs_in[0].TexCoord;

    vec3 normal = (gs_in[0].Normal + gs_in[1].Normal + gs_in[2].Normal) / 3;

    highp float noise_u, noise_v;

    bool tall = true;
    for (int i=0; i<=N_GRASS_STRAWS; i++) {
        frag_pos = gs_in[0].FragPos + frag_pos_01 * COORDS_U[i] + frag_pos_02 * COORDS_V[i];
        tex_coord = gs_in[0].TexCoord + tex_coord_base_01 * COORDS_U[i] + tex_coord_base_02 * COORDS_V[i];

        if (tall)
            generate_grass(tex_coord, frag_pos, normal,
                           frag_pos_base_01, frag_pos_base_02,
                           GRASS_2_X, GRASS_2_Y);
        else
            generate_grass(tex_coord, frag_pos, normal,
                           frag_pos_base_01, frag_pos_base_02,
                           GRASS_3_X, GRASS_3_Y);

        tall = !tall;
    }
}
