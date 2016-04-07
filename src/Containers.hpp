#ifndef CONTAINERS_H
#define CONTAINERS_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>


struct Light {
    glm::vec3 position;
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;

    Light(glm::vec3 world_coord, glm::vec3 ambient_color,
          glm::vec3 diffuse_color, glm::vec3 specular_color);
};

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
