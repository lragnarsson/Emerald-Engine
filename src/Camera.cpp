#include "Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, glm::vec3 right, float speed, float rot_speed)
{
    this->position = position;
    this->front = front;
    this->up = up;
    this->right = right;
    this->speed = speed;
    this->rot_speed = rot_speed;
}


void Camera::update_culling_frustum()
{
    glm::vec3 far_center = FAR * front;
    glm::vec3 top_left = far_center + FAR_H * up - FAR_W * right;
    glm::vec3 top_right = far_center + FAR_H * up + FAR_W * right;
    glm::vec3 bottom_left = far_center - FAR_H * up - FAR_W * right;
    glm::vec3 bottom_right = far_center - FAR_H * up + FAR_W * right;

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
