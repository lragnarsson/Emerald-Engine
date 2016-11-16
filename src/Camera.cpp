#include "Camera.hpp"


Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, glm::vec3 right, float speed, float rot_speed)
{
    this->position = position;
    this->front = front;
    this->up = up;
    this->right = right;
    this->speed = speed;
    this->rot_speed = rot_speed;
    this->has_move_anim_path = false;
    this->has_look_anim_path = false;
    this->free_cam = true;
    this->free_look = true;
    this->height_lock = false;
}


void Camera::set_pos(glm::vec3 new_pos)
{
    if (!this->free_cam) {
        std::string extra_info = std::string("Tried to set camera position when it was not in free mode.\n");
        Error::throw_error(Error::camera_free_mode, extra_info);
    }
    this->position = new_pos;
}

void Camera::set_height(float height)
{
    if (!this->free_cam) {
        std::string extra_info = std::string("Tried to set camera position when it was not in free mode.\n");
        Error::throw_error(Error::camera_free_mode, extra_info);
    }
    this->position.y = height;
}

void Camera::attach_move_animation_path(int animation_id, float start_parameter)
{
    this->move_anim_path_id = animation_id;
    this->spline_move_parameter = start_parameter;
    this->has_move_anim_path = true;
}


void Camera::attach_look_animation_path(int animation_id, float start_parameter)
{
    this->look_anim_path_id = animation_id;
    this->spline_look_parameter = start_parameter;
    this->has_look_anim_path = true;
}


void Camera::move_along_path(float elapsed_time)
{
    if (this->free_cam) {
        std::string extra_info = std::string("Tried to move camera along path when it was in free mode.\n");
        Error::throw_error(Error::camera_free_mode, extra_info);
    }
    glm::vec3 new_pos;
    // get_pos updates the spline parameter for next iteration
    if (has_move_anim_path) {
        Animation_Path* path = Animation_Path::get_animation_path_with_id(this->move_anim_path_id);
        new_pos = path->get_pos(this->spline_move_parameter,
                                elapsed_time);
    } else {
        Error::throw_error(Error::model_has_no_path);
    }
    this->position = new_pos;
}


void Camera::move_look_point_along_path(float elapsed_time)
{
    if (this->free_look) {
        std::string extra_info = std::string("Tried to move camera look point along path when it was in free look.\n");
        Error::throw_error(Error::camera_free_mode, extra_info);
    }
    glm::vec3 new_pos;
    float diff = 2.0f;
    // get_pos updates the spline parameter for next iteration
    if (has_look_anim_path) {
        Animation_Path* path = Animation_Path::get_animation_path_with_id(this->look_anim_path_id);
        // If look point and position are on the same spline and too close, move look point
        // forward along spline
        if (this->look_anim_path_id == this->move_anim_path_id &&
            std::abs(this->spline_move_parameter - this->spline_look_parameter) < diff) {
            this->spline_look_parameter += diff;
        }
        new_pos = path->get_pos(this->spline_look_parameter, elapsed_time);
    } else {
        Error::throw_error(Error::model_has_no_path);
    }
    this->look_pos = new_pos;
    this->front = glm::normalize(this->look_pos - this->position);
    this->right = glm::normalize(glm::cross(this->front, this->up));
}


void Camera::toggle_free_move()
{
    if (!this->has_move_anim_path) {
        Error::throw_error(Error::camera_has_no_path);
    }
    this->free_cam = !this->free_cam;
}

void Camera::toggle_free_look()
{
    if (!this->has_look_anim_path) {
        Error::throw_error(Error::camera_has_no_path);
    }
    this->free_look = !this->free_look;
}

void Camera::toggle_height_lock()
{
    this->height_lock = !this->height_lock;
}

bool Camera::get_height_lock()
{
    return this->height_lock;
}


void Camera::cycle_move_anim_path(int& parameter)
{
    parameter = spline_move_parameter;
    int number_of_anim_paths = Animation_Path::get_number_of_animation_paths();
    int end_id = number_of_anim_paths - 1;
    if (this->move_anim_path_id == end_id) {
        this->move_anim_path_id = 0;
    } else {
        this->move_anim_path_id++;
        this->spline_move_parameter = 1.0f;
    }
}


void Camera::cycle_look_anim_path(int& parameter)
{
    parameter = spline_look_parameter;
    int number_of_anim_paths = Animation_Path::get_number_of_animation_paths();
    int end_id = number_of_anim_paths - 1;
    if (this->look_anim_path_id == end_id) {
        this->look_anim_path_id = 0;
    } else {
        this->look_anim_path_id++;
        this->spline_look_parameter = 10.0f;
    }
}


void Camera::update_culling_frustum()
{
    glm::vec3 far_center = FAR * front;
    glm::vec3 real_up = glm::normalize(glm::cross(right, front));
    glm::vec3 top_left = far_center + FAR_H * real_up - FAR_W * right;
    glm::vec3 top_right = far_center + FAR_H * real_up + FAR_W * right;
    glm::vec3 bottom_left = far_center - FAR_H * real_up - FAR_W * right;
    glm::vec3 bottom_right = far_center - FAR_H * real_up + FAR_W * right;

    // Normals are defined as pointing inward
    frustum_normals[0] = glm::normalize(glm::cross(bottom_left, top_left));     // Left normal
    frustum_normals[1] = glm::normalize(glm::cross(top_right, bottom_right));   // Right normal
    frustum_normals[2] = glm::normalize(glm::cross(bottom_right, bottom_left)); // Bottom normal
    frustum_normals[3] = glm::normalize(glm::cross(top_left, top_right));       // Top normal
    frustum_normals[4] = glm::normalize(-far_center);                           // Far normal

    for (int i=0; i < 4; i++) {
        frustum_offsets[i] = -glm::dot(frustum_normals[i], position);
    }
    frustum_offsets[4] = -glm::dot(position + far_center, frustum_normals[4]);;
}


bool Camera::sphere_in_frustum(glm::vec3 center, float radius)
{
    glm::vec3 closest_point;
    for (int i=0; i < 5; i++) {
        if (glm::dot(center, frustum_normals[i]) + radius + frustum_offsets[i] <= 0.f ) {
            return false;
        }
    }
    return true;
}
