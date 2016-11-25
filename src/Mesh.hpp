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

#include "Error.hpp"

using namespace std;

const string DEFAULT_PATH = "res/models/default";
const string DEFAULT_DIFFUSE = "default_diffuse.jpg";
const string DEFAULT_NORMAL = "default_normal.jpg";


enum texture_type {
    DIFFUSE,
    SPECULAR,
    NORMAL
};

struct Texture {
    GLuint id;
    texture_type type;
    string full_path;
};


class Mesh {
public:
    GLuint index_count, vertex_count;
    GLfloat shininess = 80;
    vector<GLuint> indices;
    vector<GLfloat> vertices, normals, tex_coords, tangents;
    Texture *diffuse_map, *specular_map, *normal_map;
    bool draw_me = true;
    float bounding_sphere_radius = -1.f;
    glm::vec3 bounding_sphere_center;

    Mesh() { };
    ~Mesh() { };
    void clear_mem();

    void upload_mesh_data();
    GLuint get_VAO();
    void set_texture(const string full_path, const bool clamp,
                     const texture_type tex_type);
    static Texture *load_texture(const string full_path, const bool clamp,
                                 const texture_type tex_type);
    void generate_bounding_sphere();
    glm::vec3 get_center_point_world(glm::mat4 m2w_matrix);
    glm::vec3 get_center_point();

private:
    static vector<Texture*> loaded_textures;
    GLuint VAO, EBO;
    GLuint VBO[4]; // Vertices, normals, texCoords, tangents
};



#endif
