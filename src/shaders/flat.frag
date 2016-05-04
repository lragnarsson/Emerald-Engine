

out vec4 out_Color;

uniform vec3 color;


const float NEAR = 0.1f;
const float FAR = 100.0f;


float linearize_depth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
}

void main(void)
{
    out_Color = vec4(vec3(linearize_depth(gl_FragCoord.z)/FAR), 1.0);
}
