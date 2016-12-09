
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 Normal;   // view space normal
} gs_in[];

// for converting normals to clip space
uniform mat4 projection;
uniform mat4 view;

const float MAGNITUDE = 2.0f; // Length of generated lines


void GenerateLine(vec4 clipPos, vec3 inNormal)
{
    gl_Position = clipPos;
    EmitVertex();
    vec4 clip_space_normal = projection * vec4(inNormal* MAGNITUDE, 0.0);
    gl_Position = clipPos + clip_space_normal;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(gl_in[0].gl_Position, gs_in[0].Normal); // First vertex normal
    GenerateLine(gl_in[1].gl_Position, gs_in[1].Normal); // Second vertex normal
    GenerateLine(gl_in[2].gl_Position, gs_in[2].Normal); // Third vertex normal

}
