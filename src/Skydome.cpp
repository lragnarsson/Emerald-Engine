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

const float Skydome::altitude_margin = -0.12f;


// Calculate the light projection used for shadow map
const float SUN_LEFT = -_FAR_/2.f;
const float SUN_RIGHT = _FAR_/2.f;
const float SUN_BOTTOM = -_FAR_/10.f;
const float SUN_TOP = _FAR_/10.f;
const float SUN_NEAR = _NEAR_;
const float SUN_FAR = _FAR_/1.3f;
const mat4 Skydome::light_projection = ortho(SUN_LEFT, SUN_RIGHT,
                                             SUN_BOTTOM, SUN_TOP,
                                             SUN_NEAR, SUN_FAR);


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
    glCullFace(GL_FRONT);
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
    glCullFace(GL_BACK);
}


void Skydome::upload_sun(const GLuint shader, const Camera &camera)
{
    calculate_sun();
    glUseProgram(shader);
    // Translate world space direction to view space:
    GLuint sun_up = altitude > altitude_margin;
    mat4 m2w = translate(mat4(1.f), camera.get_pos());
    vec3 view_space_dir = vec3(camera.get_view_matrix() * m2w *
                               vec4(this->sun_direction, 1.f));
    glUniform3fv(glGetUniformLocation(shader, "sun_direction"),
                 1, value_ptr(view_space_dir));
    glUniform3fv(glGetUniformLocation(shader, "sun_color"),
                 1, value_ptr(normalize(this->sun_color)));
    glUniform1i(glGetUniformLocation(shader, "sun_up"), sun_up);
    glUseProgram(0);
}


void Skydome::propagate_time(const float elapsed_time)
{
    time_of_day += elapsed_time * time_scale;
    if (time_of_day >= 24.f)
        time_of_day -= 24.f;
    else if (time_of_day < 0.f)
        time_of_day += 24.f;

}


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
    float latitude = 40.f * M_PI / 180.f; // lat 40 deg in rad
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

// ---------------
// Shadow mapping
void Skydome::update_light_space(const Camera &camera)
{
    vec3 camera_pos = camera.get_pos();
    vec3 camera_front = camera.front;

    vec3 look_at = camera_pos + (_FAR_ / 500.f) * camera_front;// + (_FAR_ / 1000.f) * camera_front;
    vec3 sun_pos = camera_pos + (0.4f * _FAR_) * sun_direction;

    // Adjust lookAt/look_at for terrain height
    Terrain* my_terrain = nullptr;
    for (auto terrain : Terrain::get_loaded_terrain() ){
        if ( terrain->point_in_terrain(look_at.x, look_at.z) ){
            float height = terrain->get_height(look_at.x, look_at.z);
            if ( height + 15.f > look_at.y ){
                look_at.y = height + 15.0;
            }
        }
    }

    this->light_view_matrix = glm::lookAt(sun_pos, // position
                                          look_at, // look at mid frustum
                                          cross(sun_direction, camera_front)); // up

    // This matrix transforms from world space to light view space
    this->light_space_matrix = Skydome::light_projection * this->light_view_matrix;
    update_sun_frustum(sun_pos, -sun_direction, camera_front);
}

// -------------

mat4 Skydome::get_light_space_matrix(){
    return this->light_space_matrix;
}


void Skydome::update_sun_frustum(const vec3 sun_pos, const vec3 sun_front, const vec3 sun_right)
{
    // Enlarge frustum due to bad frustum overlap at high camera pitch angles:
    const float F_HEIGHT = (SUN_TOP - SUN_BOTTOM) / 1.5f;
    const float F_WIDTH = (SUN_RIGHT - SUN_LEFT) / 1.5f;

    vec3 far_center = SUN_FAR * sun_front;
    vec3 real_up = normalize(cross(sun_right, sun_front));
    vec3 top_left = far_center + F_HEIGHT * real_up - F_WIDTH * sun_right;
    vec3 top_right = far_center + F_HEIGHT * real_up + F_WIDTH * sun_right;
    vec3 bottom_left = far_center - F_HEIGHT * real_up - F_WIDTH * sun_right;
    vec3 bottom_right = far_center - F_HEIGHT * real_up + F_WIDTH * sun_right;

    // Normals are defined as pointing inward
    frustum_normals[0] = normalize(cross(bottom_left, top_left));     // Left normal
    frustum_normals[1] = normalize(cross(top_right, bottom_right));   // Right normal
    frustum_normals[2] = normalize(cross(bottom_right, bottom_left)); // Bottom normal
    frustum_normals[3] = normalize(cross(top_left, top_right));       // Top normal
    frustum_normals[4] = normalize(-far_center);                           // Far normal

    for (int i=0; i < 4; i++) {
        frustum_offsets[i] = -dot(frustum_normals[i], sun_pos);
    }
    frustum_offsets[4] = -dot(sun_pos + far_center, frustum_normals[4]);;
}


bool Skydome::sphere_in_sun_frustum(vec3 center, float radius)
{
    for (int i=0; i < 5; i++) {
        if (dot(center, frustum_normals[i]) + radius + frustum_offsets[i] <= 0.f ) {
            return false;
        }
    }
    return true;
}
