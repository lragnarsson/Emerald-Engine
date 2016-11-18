#include "Skydome.hpp"


using namespace glm;

const vec3 Skydome::sun_dawn = {.8f, .3f, 0.2f};
const vec3 Skydome::sun_noon = {.5f, .35f, 0.2f};
const vec3 Skydome::sun_dusk = {0.9f, 0.3f, 0.1f};
const vec3 Skydome::sun_midnight = {0.f, 0.f, 0.f};

const vec3 Skydome::zenith_dawn = {0.1f, 0.1f, 0.65f};
const vec3 Skydome::horizon_dawn = {0.5f, 0.15f, 0.4f};

const vec3 Skydome::zenith_noon = {0.1f, 0.4f, 1.f};
const vec3 Skydome::horizon_noon = {0.34f, 0.54f, 0.88f};

const vec3 Skydome::zenith_dusk = {0.5f, 0.4f, 0.3f};
const vec3 Skydome::horizon_dusk = {0.9f, 0.4f, 0.1f};

const vec3 Skydome::zenith_midnight = {0.f, 0.0f, 0.01f};
const vec3 Skydome::horizon_midnight = {0.f, 0.01f, 0.05f};

const float Skydome::altitude_margin = -0.055f;


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
                 1, value_ptr(this->zenith_color));
    glUniform3fv(glGetUniformLocation(this->shader, "horizon_color"),
                 1, value_ptr(this->horizon_color));
    glUniform3fv(glGetUniformLocation(this->shader, "sun_direction"),
                 1, value_ptr(this->sun_direction));
    glUniform3fv(glGetUniformLocation(this->shader, "sun_color"),
                 1, value_ptr(this->sun_color));
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


void Skydome::propagate_time(float elapsed_time)
{
    time_of_day += elapsed_time * time_scale;
    if (time_of_day >= 24.f)
        time_of_day -= 24.f;
    else if (time_of_day < 0.f)
        time_of_day += 24.f;
}

// --------------

vec3 Skydome::get_sun_direction(){
    return this->sun_direction;
}

// --------------



void Skydome::reset_time()
{
    time_of_day = 12.f;
    time_scale = 0.001f;
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


void Skydome::calculate_sky()
{
    // TODO: Interpolate based on time of day:
    // Between dawn and noon:
    if (altitude >= altitude_margin && azimuth <= 0) {
        this->horizon_color = horizon_dawn * interp + horizon_noon * (1 - interp);
        this->zenith_color = zenith_dawn* interp + zenith_noon * (1 - interp);
    }
    // Between noon and dusk:
    else if (altitude >= altitude_margin && azimuth > 0) {
        this->horizon_color = horizon_dusk * interp + horizon_noon * (1 - interp);
        this->zenith_color = zenith_dusk* interp + zenith_noon * (1 - interp);
    }
    // Between dusk and midnight:
    else if (altitude < altitude_margin && azimuth >= 0) {
        this->horizon_color = horizon_dusk * interp_night + horizon_midnight * (1 - interp_night);
        this->zenith_color = zenith_dusk * interp_night + zenith_midnight * (1 - interp_night);
    }
    // Between midnight and dawn:
    else if (altitude < altitude_margin && azimuth < 0) {
        this->horizon_color = horizon_dawn * interp_night + horizon_midnight * (1 - interp_night);
        this->zenith_color = zenith_dawn * interp_night + zenith_midnight * (1 - interp_night);
    }
}


void Skydome::calculate_sun()
{
    // Assuming declination = 0
    float latitude = 50.f * M_PI / 180.f; // lat 50 deg in rad
    float solar_hour_angle = (time_of_day - 12.f) * 15 * M_PI / 180.f;
    int sha_sign = (solar_hour_angle > 0) - (solar_hour_angle < 0);
    this->altitude = asin(cos(latitude) *
                          cos(solar_hour_angle));

    this->azimuth = sha_sign * acos(sin(altitude) * sin(latitude) /
                                    (cos(altitude) * cos(latitude)));

    this->sun_direction = normalize(vec3(sin(azimuth), sin(altitude), -cos(azimuth)));

    // Linear interpolation between two adjacent colors:
    float max_altitude = asin(cos(latitude)) - altitude_margin;
    float min_altitude = asin(cos(latitude)) + altitude_margin;
    this->interp = (max_altitude - altitude + altitude_margin) / max_altitude;
    // Take night interpolation to power of 4 to speed up arrival of darkness:
    this->interp_night = pow((min_altitude + altitude - altitude_margin) / min_altitude, 4);

    // Between dawn and noon:
    if (altitude >= altitude_margin && azimuth <= 0) {
        this->sun_color = sun_dawn * interp + sun_noon * (1 - interp);
    }
    // Between noon and dusk:
    else if (altitude >= altitude_margin && azimuth > 0) {
        this->sun_color = sun_dusk * interp + sun_noon * (1 - interp);
    }
    // Between dusk and midnight:
    else if (altitude < altitude_margin && azimuth >= 0) {
        this->sun_color = sun_midnight;
    }
    // Between midnight and dawn:
    else if (altitude < altitude_margin && azimuth < 0) {
        this->sun_color = sun_midnight;
    }

}

// -----------------
