#include "Skydome.hpp"


void Skydome::init()
{
    this->sphere = new Model("res/models/sphere/sphere.obj");
    this->shader = load_shaders("build/shaders/skydome.vert",
                                "build/shaders/skydome.frag");
    init_uniforms();
    reset_time();
}


void Skydome::draw(const Camera &camera)
{
    calculate_sky();
    glDepthRange(0.9999f, 1.f);

    glUseProgram(this->shader);
    mat4 view = lookAt(vec3(0.f), camera.front, camera.up);
    glUniformMatrix4fv(glGetUniformLocation(this->shader, "view"),
                       1, GL_FALSE, value_ptr(view));


    glUniform3fv(glGetUniformLocation(this->shader, "zenith_color"),
                 1, value_ptr(this->current_zenith_color));
    glUniform3fv(glGetUniformLocation(this->shader, "horizon_color"),
                 1, value_ptr(this->current_horizon_color));

    Mesh* mesh = this->sphere->get_meshes()[0];

    /* DRAW */
    glBindVertexArray(mesh->get_VAO());
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    glDepthRange(0.f, 1.f);
    glUseProgram(0);
}


void Skydome::upload_sun(const GLuint shader, const Camera &camera)
{
    calculate_sun();
    glUseProgram(shader);
    // Translate world space direction to view space:
    mat4 m2w = translate(mat4(1.f), camera.get_pos());
    vec3 view_space_dir = vec3(camera.get_view_matrix() * m2w *
                               vec4(this->sun_direction, 1.f));
    glUniform3fv(glGetUniformLocation(shader, "sun_direction"),
                 1, value_ptr(view_space_dir));
    glUniform3fv(glGetUniformLocation(shader, "sun_color"),
                 1, value_ptr(this->sun_color));
    glUseProgram(0);
}


void Skydome::init_uniforms()
{
    // Upload constant uniforms:
    mat4 projection_matrix;
    projection_matrix = perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    glUseProgram(this->shader);
    glUniformMatrix4fv(glGetUniformLocation(this->shader, "projection"),
                       1, GL_FALSE, value_ptr(projection_matrix));
    glUniformMatrix4fv(glGetUniformLocation(this->shader, "model"),
                       1, GL_FALSE, value_ptr(this->sphere->m2w_matrix));
    glUseProgram(0);
}


void Skydome::propagate_time(float elapsed_time)
{
    //std::cout << elapsed_time << "  " << time_of_day << std::endl;
    time_of_day += elapsed_time * time_scale;
    if (time_of_day >= 24.f)
        time_of_day -= 24.f;
}


void Skydome::reset_time()
{
    time_of_day = 12.f;
    time_scale = 0.001f;
}


void Skydome::calculate_sky()
{
    // TODO: Interpolate based on time of day:
    this->current_horizon_color = horizon_noon;
    this->current_zenith_color = zenith_noon;
}


void Skydome::calculate_sun()
{
    // Assuming declination = 0
    float latitude = 50.f * M_PI / 180.f; // radians
    float solar_hour_angle = (time_of_day - 12.f) * 15 * M_PI / 180;
    int sha_sign = (solar_hour_angle > 0) - (solar_hour_angle < 0);
    float altitude = asin(cos(latitude) *
                          cos(solar_hour_angle));
    float azimuth = sha_sign * acos(sin(altitude) * sin(latitude) /
                                    (cos(altitude) * cos(latitude)));

    //std::cout << time_of_day << " " << solar_hour_angle << "  " << altitude << "  " << azimuth << std::endl;

    this->sun_direction = normalize(vec3(sin(azimuth), sin(altitude), -cos(azimuth)));
    std::cout << time_of_day << ":   "<< sun_direction.x << ",  " << sun_direction.y << ",  " << sun_direction.z << std::endl;

    if (altitude <= 0)
        this->sun_color = vec3(0.f);
    else
        this->sun_color = vec3(.5f, .35f, 0.2f);
}
