#ifndef MODEL_H
#define MODEL_H

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
    glm::vec3 ambient_color = glm::vec3(0.8f), diffuse_color = glm::vec3(0.8f), specular_color = glm::vec3(0.8f);
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices, normals, tex_coords;
    std::vector<Texture*> textures;

    Mesh() { };
    ~Mesh() { };

    void draw_forward(GLuint shader_program);
    /* Upload vertices, normals etc to the GPU */
    void upload_mesh_data(GLuint shader_program);

private:
    GLuint VAO, EBO;
    GLuint VBO[3]; // Vertices, normals, texCoords
};


class Model {
public:
    std::vector<GLuint> shader_programs;
    glm::mat4 m2w_matrix, rot_matrix;
    float bounding_sphere_radius = -1.f;
    bool draw_me = true;

    Model() { };
    Model(std::string path, const GLuint shader_program,
          const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix);
    Model(std::string path, const GLuint shader_program,
          const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix,
          const float bounding_sphere_radius);

    ~Model() { };

    void draw_forward(GLuint shader_program);
    void draw_deferred(GLuint shader_program);
    void load(std::string path);
    glm::vec3 get_center_point();

private:
    static std::vector<Texture*> loaded_textures;

    std::vector<Mesh> meshes;
    std::string directory;
    glm::vec3 bounding_sphere_center;

    void unfold_assimp_node(aiNode* node, const aiScene* scene);
    Mesh load_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_texture(const char* filename, std::string basepath);
    void generate_bounding_sphere();
};

#endif
