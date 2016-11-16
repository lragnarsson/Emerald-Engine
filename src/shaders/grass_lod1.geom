
layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

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
const float MAGNITUDE = 1.8f; // Length of generated lines


void GenerateTallGrass(vec4 clipPos, vec2 texCoord, vec3 fragPos, vec3 inNormal,
                       vec3 base_1, vec3 base_2)
{
    vec3 diff = 0.1 * base_1 + 0.1 * base_2;

    vec3 grass_normal = vec3(view * vec4(0, 1, 0, 0)); //normalize(cross(diff, inNormal));

    gl_Position = clipPos + projection * vec4(diff, 0);
    FragPos = fragPos + diff;
    TexCoord = texCoord;
    Normal = grass_normal;
    EmitVertex();

    TexCoord = texCoord;
    Normal = grass_normal;
    gl_Position = clipPos + projection * vec4(3 * diff, 0);
    FragPos = fragPos + 3 * diff;
    EmitVertex();

    vec4 clip_space_normal = projection * vec4(inNormal * MAGNITUDE, 0);
    vec4 clip_space_up = projection * view * vec4(0, MAGNITUDE, 0, 0);

    vec4 interpNormal = mix(clip_space_normal, clip_space_up, 1);
    gl_Position = clipPos + interpNormal + projection * vec4(2 * diff, 0);
    TexCoord = texCoord; // should make the line have the color of the originating vertex
    FragPos = fragPos + inNormal * MAGNITUDE + 2 * diff;
    Normal = grass_normal;
    EmitVertex();
    EndPrimitive();
}


void main()
{
    vec3 frag_pos_base_1 = normalize(gs_in[2].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_2 = normalize(gs_in[1].FragPos - gs_in[0].FragPos);
    vec3 frag_pos_base_3 = normalize(gs_in[2].FragPos - gs_in[1].FragPos);

    GenerateTallGrass(gl_in[0].gl_Position, gs_in[0].TexCoord, gs_in[0].FragPos, gs_in[0].Normal,
                      frag_pos_base_2, frag_pos_base_1);
    GenerateTallGrass(gl_in[1].gl_Position, gs_in[1].TexCoord, gs_in[1].FragPos, gs_in[1].Normal,
                      -frag_pos_base_2, frag_pos_base_3);
    GenerateTallGrass(gl_in[2].gl_Position, gs_in[2].TexCoord, gs_in[2].FragPos, gs_in[2].Normal,
                      -frag_pos_base_3, -frag_pos_base_1);

    // Middle of triangle
    vec4 middlePos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    vec2 middleTex = (gs_in[0].TexCoord + gs_in[1].TexCoord + gs_in[2].TexCoord) / 3;
    vec3 middleFragPos = (gs_in[0].FragPos + gs_in[1].FragPos + gs_in[2].FragPos) / 3;
    vec3 middleNormal = (gs_in[0].Normal + gs_in[1].Normal + gs_in[2].Normal) / 3;

    GenerateTallGrass(middlePos, middleTex, middleFragPos, middleNormal,
                      frag_pos_base_1, frag_pos_base_2);
}
