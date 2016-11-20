
layout (triangles) in;
layout (triangle_strip, max_vertices = 28) out;

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
const float MAGNITUDE = 1.0f; // Length of generated lines
const float OFFSET = 5.f;
const float TARGET = 15.f;
const float grass_1_x[9] = float[9](-0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, -0.151465, 0.151465, 0.000000);
const float grass_1_y[9] = float[9](0.000000, 0.000000, 7.836071, 7.836071, 12.914148, 12.914148, 16.372519, 16.372519, 20.000000);


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

    float tangent_step = TARGET / 10.0;
    for (int i=0; i < 3; i++) {
        TexCoord = texCoord;
        Normal = grass_normal;
        gl_Position = clipPos + interpNormal * i +
            projection * vec4((OFFSET + tangent_step * pow(i, 2)) * tangent, 0);
        FragPos = fragPos + inNormal * MAGNITUDE * i + (OFFSET + tangent_step * pow(i, 2)) * tangent;
        EmitVertex();

        TexCoord = texCoord;
        Normal = grass_normal;
        gl_Position = clipPos + interpNormal * i +
            projection * vec4((OFFSET + tangent_step * (pow(i, 2) + 1)) * tangent, 0);
        FragPos = fragPos + inNormal * MAGNITUDE * i +
            (OFFSET + tangent_step * (pow(i, 2) + 1)) * tangent;
        EmitVertex();
    }

    gl_Position = clipPos + 3 * interpNormal + projection * vec4((OFFSET + TARGET) * tangent, 0);
    TexCoord = texCoord; // should make the line have the color of the originating vertex
    FragPos = fragPos + 3 * inNormal * MAGNITUDE + (OFFSET + TARGET) * tangent;
    Normal = grass_normal;
    EmitVertex();

    EndPrimitive();
}


void main()
{
    vec3 frag_pos_base_02 = normalize(gs_in[2].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_01 = normalize(gs_in[1].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_12 = normalize(gs_in[2].FragPos - gs_in[1].FragPos);

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

}
