#ifndef MESH_H 
#define MESH_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Light.hpp"
#include "Error.hpp"
#include "Animation_Path.hpp"

const std::string DEFAULT_PATH = "res/models/default";
const std::string DEFAULT_DIFFUSE = "default_diffuse.jpg";
const std::string DEFAULT_NORMAL = "default_normal.jpg";


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


class Mesh {
public:
    GLuint index_count, vertex_count;
    GLfloat shininess = 80;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices, normals, tex_coords, tangents;
    Texture *diffuse_map, *specular_map, *normal_map;

    Mesh() { };
    ~Mesh() { };

    /* Upload vertices, normals etc to the GPU */
    void upload_mesh_data();
    GLuint get_VAO();

private:
    GLuint VAO, EBO;
    GLuint VBO[4]; // Vertices, normals, texCoords, tangents
};



#endif
