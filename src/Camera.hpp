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


const GLuint SCREEN_WIDTH = 800;
const GLuint SCREEN_HEIGHT = 640;

const GLfloat Y_FOV = 45.f;
const GLfloat ASPECT_RATIO = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;

const GLfloat NEAR = 0.1f;
const GLfloat FAR = 100.f;

const GLfloat FAR_H = glm::tan(Y_FOV / 2) * FAR;
const GLfloat FAR_W = FAR_H * ASPECT_RATIO;


class Camera
{
public:
    glm::vec3 position, front, up, right;
    float speed, rot_speed;

    Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, glm::vec3 right, float speed, float rot_speed);

    void update_culling_frustum();
    bool sphere_in_frustum(glm::vec3 center, float radius);

private:
    glm::vec3 frustum_normals[5];
    GLfloat frustum_offsets[5];
};

#endif
