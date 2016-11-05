


in vec3 sky_pos;

out vec4 out_color;

uniform vec3 zenith_color;
uniform vec3 horizon_color;


void main(void)
{
    float height = pow(abs(sky_pos.y) / 1.5, 0.5); // Over horizon
    out_color = vec4(mix(horizon_color, zenith_color, height), 1);
}
