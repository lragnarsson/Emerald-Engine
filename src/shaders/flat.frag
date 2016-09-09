

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 color;


void main(void)
{
    OutColor = 10.0 * vec4(color, 1.0);

    /* Filter out parts above 1 in brightness */
    // Human eye is more sensitive to red and green than blue. learnopengl.com
    float brightness = dot(OutColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(OutColor.rgb, 1.0);
}
