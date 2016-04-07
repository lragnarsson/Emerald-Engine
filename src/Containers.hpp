#ifndef CONTAINERS_H
#define CONTAINERS_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <assimp/Importer.hpp>

enum texture_type {
    DIFFUSE,
    SPECULAR,
    NORMAL
};

struct Texture {
    GLuint id;
    texture_type type;
    aiString path;
};

struct Camera {
    glm::vec3 position, front, up, right;
    float speed, rot_speed;
};

#endif
