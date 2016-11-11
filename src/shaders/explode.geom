layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 TexCoord;
    vec3 FragPos;
    mat3 TBN_viewSpace;
} gs_in[];

out vec2 TexCoords_geom; 
out vec3 FragPos_geom;
out mat3 TBN_viewSpace_geom;

//uniform float time;

vec4 explode_clip(vec4 position, vec3 normal)
{
    float time = 0.0f;
    float magnitude = 2.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude; 
    return position + vec4(direction, 0.0f);
}

vec3 explode_view(vec3 position, vec3 normal)
{
    float time = 0.0f;
    float magnitude = 2.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude; 
    return position + direction;
}
    

vec3 GetNormal(vec3 v1, vec3 v2, vec3 v3)
{
    vec3 a = v1 - v2;
    vec3 b = v2 - v3;
    return normalize(cross(a, b));
}

void main() {
    vec3 clip_normal = GetNormal(vec3(gl_in[0].gl_Position),
                                 vec3(gl_in[1].gl_Position),
                                 vec3(gl_in[2].gl_Position));
    vec3 view_normal = GetNormal(gs_in[0].FragPos,
                                 gs_in[1].FragPos,
                                 gs_in[2].FragPos);

    gl_Position = explode_clip(gl_in[0].gl_Position, clip_normal);
    TexCoords_geom = gs_in[0].TexCoord;
    FragPos_geom = explode_view(gs_in[0].FragPos, view_normal);
    TBN_viewSpace_geom = gs_in[0].TBN_viewSpace;
    EmitVertex();
    gl_Position = explode_clip(gl_in[1].gl_Position, clip_normal);
    TexCoords_geom = gs_in[1].TexCoord;
    FragPos_geom = explode_view(gs_in[1].FragPos, view_normal);
    TBN_viewSpace_geom = gs_in[1].TBN_viewSpace;
    EmitVertex();
    gl_Position = explode_clip(gl_in[2].gl_Position, clip_normal);
    TexCoords_geom = gs_in[2].TexCoord;
    FragPos_geom = explode_view(gs_in[2].FragPos, view_normal);
    TBN_viewSpace_geom = gs_in[2].TBN_viewSpace;
    EmitVertex();
    EndPrimitive();
}  
