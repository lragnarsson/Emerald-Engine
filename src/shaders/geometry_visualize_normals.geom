
layout (triangles) in;
layout (line_strip, max_vertices = 40) out;

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
uniform mat4 view;
uniform float upInterp; // Interpolation between up-vector and vertex own normal vector
uniform int n_lines;

const float MAGNITUDE = 2.0f; // Length of generated lines


void GenerateLine(vec4 clipPos, vec2 texCoord, vec3 fragPos, vec3 inNormal)
{
    gl_Position = clipPos;
    TexCoord = texCoord;
    FragPos = fragPos;
    Normal = inNormal;
    EmitVertex();
    vec4 clip_space_normal = projection * vec4(inNormal* MAGNITUDE, 0.0);
    vec4 clip_space_up = projection * view * vec4(0.0, MAGNITUDE, 0.0, 0.0);

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
    GenerateLine(gl_in[0].gl_Position, gs_in[0].TexCoord, gs_in[0].FragPos, 2 * gs_in[0].Normal); // First vertex normal
    
    vec3 frag_pos, frag_pos_01, frag_pos_02, normal, normal_01, normal_02;
    vec2 tex_coord, tex_coord_01, tex_coord_02;
    vec4 clip_pos;
    float par2 = 0.0f;
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
        
            GenerateLine(clip_pos, tex_coord, frag_pos, normal);
        }
    }
}
