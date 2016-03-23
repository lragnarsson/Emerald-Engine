#include "Utils.hpp"


class Mesh {
public:
    GLuint indexCount, vertexCount;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices, normals, texCoords;
    std::vector<Texture*> textures;

    Mesh() { };

    ~Mesh() { };

    void Draw(GLuint shaderProgram):

    /* Upload all mesh data to the GPU */
    void UploadMeshData();

private:
    GLuint VAO, EBO;
    GLuint VBO[3]; // Vertices, normals, texCoords
};
