
layout (triangles) in;
layout (line_strip, max_vertices = 8) out;

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

const float MAGNITUDE = 0.4f; // Length of generated lines


void GenerateLine(vec4 clipPos, vec2 texCoord, vec3 fragPos, vec3 inNormal)
{
    gl_Position = clipPos;
    TexCoord = texCoord;
    FragPos = fragPos;
    Normal = inNormal;
    EmitVertex();
    vec4 clip_space_normal = projection * vec4(inNormal* MAGNITUDE, 0.0);
    vec4 clip_space_up = projection * vec4(0.0, MAGNITUDE, 0.0, 0.0);

    vec4 interpNormal = mix(clip_space_normal, clip_space_up, upInterp);
    gl_Position = clipPos + interpNormal;
    TexCoord = texCoord; // should make the line have the color of the originating vertex
    FragPos = fragPos + inNormal * MAGNITUDE;
    Normal = inNormal;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(gl_in[0].gl_Position, gs_in[0].TexCoord, gs_in[0].FragPos, gs_in[0].Normal); // First vertex normal
    GenerateLine(gl_in[1].gl_Position, gs_in[1].TexCoord, gs_in[1].FragPos, gs_in[1].Normal); // Second vertex normal
    GenerateLine(gl_in[2].gl_Position, gs_in[2].TexCoord, gs_in[2].FragPos, gs_in[2].Normal); // Third vertex normal

    // Middle of triangle
    vec4 middlePos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    vec2 middleTex = (gs_in[0].TexCoord + gs_in[1].TexCoord + gs_in[2].TexCoord) / 3;
    vec3 middleFragPos = (gs_in[0].FragPos + gs_in[1].FragPos + gs_in[2].FragPos) / 3;
    vec3 middleNormal = (gs_in[0].Normal + gs_in[1].Normal + gs_in[2].Normal) / 3;

    GenerateLine(middlePos, middleTex, middleFragPos, middleNormal);
}
