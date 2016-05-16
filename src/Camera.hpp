#ifndef CAMERA_H
#define CAMERA_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
#include <iostream>

#include "Error.hpp"
#include "Animation_Path.hpp"

const GLfloat Y_FOV = 45.f;
const GLfloat ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;

const GLfloat NEAR = _NEAR_;
const GLfloat FAR = _FAR_;

const GLfloat FAR_H = glm::tan(Y_FOV / 2) * FAR;
const GLfloat FAR_W = FAR_H * ASPECT_RATIO;


class Camera
{
public:
    glm::vec3 front, up, right;
    float speed, rot_speed;

    Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, glm::vec3 right, float speed, float rot_speed);
    glm::vec3 get_pos() const {return position;}
    void set_pos(glm::vec3 new_pos);
    bool can_move_free() const {return free_cam;}
    bool can_look_free() const {return free_look;}
    
    bool has_move_path() const {return has_move_anim_path;}
    bool has_look_path() const {return has_look_anim_path;}
    void attach_move_animation_path(int animation_id, float start_parameter);
    void move_along_path(float elapsed_time);
    void toggle_free_move();
    void toggle_free_look();

    void update_culling_frustum();
    bool sphere_in_frustum(glm::vec3 center, float radius);

private:
    glm::vec3 position, look_pos;
    float spline_parameter;
    bool has_move_anim_path;
    bool has_look_anim_path;
    Animation_Path* move_anim_path;
    Animation_Path* look_anim_path;
    bool free_cam;
    bool free_look;
    
    glm::vec3 frustum_normals[5];
    GLfloat frustum_offsets[5];
};

#endif
