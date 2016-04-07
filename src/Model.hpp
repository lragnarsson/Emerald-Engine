#ifndef MODEL
#define MODEL

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Utils.hpp"


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
    static std::vector<Texture*> loaded_textures;
    std::vector<GLuint> shader_programs;
    glm::mat4 m2w_matrix, rot_matrix;

    Model() { };
    Model(std::string path, const GLuint shader_program,
          const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix);
    ~Model() { };

    void draw(GLuint shader_program);
    void load(std::string path);

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void unfold_assimp_node(aiNode* node, const aiScene* scene);
    Mesh load_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_texture(const char* filename, std::string basepath);
};

#endif
