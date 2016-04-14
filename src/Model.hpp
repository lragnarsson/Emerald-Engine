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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <iostream>
#include <vector>
#include "Containers.hpp"
#include "Light.hpp"

class Mesh {
public:
    GLuint index_count, vertex_count;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices, normals, tex_coords;
    std::vector<Texture*> textures;

    Mesh() { };
    ~Mesh() { };

    void draw(GLuint shader_program);
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
    
    Model() { };
    Model(std::string path, const GLuint shader_program,
          const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix);
    Model(std::string path, const GLuint shader_program,
          const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix, std::vector<Light*> lightsources);
    ~Model() { };

    void draw(GLuint shader_program);
    void load(std::string path);
    void attach_light(Light* light);
    void move_to(glm::vec3 world_coord);
    void move(glm::vec3 relative);
    
private:
    static std::vector<Texture*> loaded_textures;
    std::vector<Light*> attached_lightsources;
    
    std::vector<Mesh> meshes;
    std::string directory;

    glm::vec3 world_coord;
    void unfold_assimp_node(aiNode* node, const aiScene* scene);
    Mesh load_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_texture(const char* filename, std::string basepath);
};

#endif
