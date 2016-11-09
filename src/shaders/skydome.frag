


in vec3 sky_pos;

out vec4 out_color;

uniform vec3 zenith_color;
uniform vec3 horizon_color;
uniform vec3 sun_color;
uniform vec3 sun_direction;


void main(void)
{
    // Normalize to get position on unit sphere:
    vec3 unit_pos = normalize(sky_pos);
    // Blend sky colors based on height over horizon:
    // Raise to power lower than 1 to make horizon color more local
    float height = pow(2 * abs(unit_pos.y), 0.6);
    out_color = vec4(mix(horizon_color, zenith_color, height), 1);

    // Add a sun:
    if (unit_pos.y > 0) { // If over horizon
        // Align vectors more to make sun smaller:
        vec3 compensated = normalize(sun_direction - 9 * unit_pos / 10.0);
        float sun = 100 * pow(max(dot(unit_pos, compensated), 0.0), 50);
        out_color += vec4(sun * sun_color, 1);
    }
}
