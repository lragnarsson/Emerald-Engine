


in vec3 sky_pos;

out vec4 out_color;

uniform vec3 zenith_color;
uniform vec3 horizon_color;
uniform vec3 sun_color;
uniform vec3 sun_direction;


void main(void)
{
    vec3 norm_sky_pos = normalize(sky_pos);
    // Blend sky colors based on height over horizon:
    float height = pow(abs(norm_sky_pos.y), 0.5); // Over horizon
    out_color = vec4(mix(horizon_color, zenith_color, height), 1);

    // Add a sun:
    if (norm_sky_pos.y > 0) { // if over horizon
        vec3 compensated = normalize(sun_direction - 9.5 * norm_sky_pos / 10.0);
        float sun = 1 * pow(max(dot(sky_pos, compensated), 0.0), 8);
        out_color += vec4(sun * normalize(sun_color), 1);
    }
}
