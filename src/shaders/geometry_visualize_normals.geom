
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

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

void GenerateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
    TexCoord = gs_in[index].TexCoord;
    FragPos = gs_in[index].FragPos;
    Normal = gs_in[index].Normal;
    EmitVertex();
    vec4 clip_space_normal = projection * vec4(gs_in[index].Normal * MAGNITUDE, 0.0);
    vec4 clip_space_up = projection * vec4(0.0, MAGNITUDE, 0.0, 0.0);

    vec4 outNormal = (1 - upInterp) * clip_space_normal + upInterp * clip_space_up;
    gl_Position = gl_in[index].gl_Position + outNormal;
    TexCoord = gs_in[index].TexCoord; // should make the line have the color of the originating vertex
    FragPos = gs_in[index].FragPos + gs_in[index].Normal * MAGNITUDE;
    Normal = gs_in[index].Normal;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // First vertex normal
    GenerateLine(1); // Second vertex normal
    GenerateLine(2); // Third vertex normal
} 
