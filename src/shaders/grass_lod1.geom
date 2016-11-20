
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


// for converting normals to clip space
uniform mat4 projection;
uniform float upInterp; // Interpolation between up-vector and vertex own normal vector
uniform mat4 view;
const float MAGNITUDE = 0.3f; // Height scale for grass
const float GRASS_SCALE = 3.f; // Uniform scale for grass
const float OFFSET = 40.f; // Position offset in triangle

// Vertices for tall straight grass:
const float GRASS_1_X[9] = float[9](-0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, -0.151465, 0.151465, 0.000000);
const float GRASS_1_Y[9] = float[9](0.000000, 0.000000, 2.490297, 2.490297, 4.847759, 4.847759, 6.651822, 6.651822, 8.000000);


void GenerateTallGrass(vec4 clipPos, vec2 texCoord, vec3 fragPos, vec3 inNormal,
                       vec3 base_1, vec3 base_2)
{
    vec3 tangent = 0.06 * base_1 + 0.06 * base_2;

    vec3 grass_normal = normalize(cross(tangent, inNormal));
    if (grass_normal.z < 0) {
        grass_normal = -grass_normal;
    }

    vec4 clip_space_normal = projection * vec4(inNormal * MAGNITUDE, 0);
    vec4 ws_up_in_cs = projection * view * vec4(0, MAGNITUDE, 0, 0); // World space up in clip space
    vec4 interpNormal = mix(clip_space_normal, ws_up_in_cs, upInterp);

    // TODO: read gradient from a wave normal map with texture pos based on world coordinates and time.
    // Possibly with an offset based on wind direction.
    vec3 gradient = vec3(1.5, 0, 0);

    for (int i=0; i < 9; i++) {
        TexCoord = texCoord;
        Normal = grass_normal;
        float bend_interp = pow(GRASS_1_Y[i] / GRASS_1_Y[8], 2.5);

        float y = GRASS_SCALE * GRASS_1_Y[i];
        vec3 xz = (OFFSET + GRASS_SCALE * GRASS_1_X[i]) * tangent + bend_interp * gradient;
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

    // TODO: Add more grass!
    /*
    GenerateTallGrass(gl_in[0].gl_Position, gs_in[0].TexCoord, gs_in[0].FragPos, gs_in[0].Normal,
                      frag_pos_base_01, frag_pos_base_02);
    GenerateTallGrass(gl_in[1].gl_Position, gs_in[1].TexCoord, gs_in[1].FragPos, gs_in[1].Normal,
                      -frag_pos_base_01, frag_pos_base_12);
    GenerateTallGrass(gl_in[2].gl_Position, gs_in[2].TexCoord, gs_in[2].FragPos, gs_in[2].Normal,
                      -frag_pos_base_12, -frag_pos_base_02);

    // Middle of triangle
    vec4 middlePos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    vec2 middleTex = (gs_in[0].TexCoord + gs_in[1].TexCoord + gs_in[2].TexCoord) / 3;
    vec3 middleFragPos = (gs_in[0].FragPos + gs_in[1].FragPos + gs_in[2].FragPos) / 3;
    vec3 middleNormal = (gs_in[0].Normal + gs_in[1].Normal + gs_in[2].Normal) / 3;

    GenerateTallGrass(middlePos, middleTex, middleFragPos, middleNormal,
                      frag_pos_base_02, frag_pos_base_01);
    */

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
        for (int j=1; j<=i; j++) {
            par2 = par2 + 3 * step_2;
            frag_pos = mix(frag_pos_01, frag_pos_02, par2);
            clip_pos = projection * vec4(frag_pos, 1.0f);
            tex_coord = mix(tex_coord_01, tex_coord_02, par2);
            normal = normalize(mix(normal_01, normal_02, par2));

            GenerateTallGrass(clip_pos, tex_coord, frag_pos, normal,
                              frag_pos_base_01, frag_pos_base_02);
        }
    }
}
