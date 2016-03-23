#include "Mesh.hpp"

struct ModelData {
    std::vector<Texture> textures;
    std::vector<Model> models;
};

ModelData loadedModelData;

class Model {
public:
    Model() { };

    Model(std::string path) {
        Load(path);
    };

    ~Model() { };

    void Draw(GLuint shaderProgram);

    void Load(std::string path);

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void unfoldAssimpNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    Texture* LoadTexture(const char* filename, std::string basepath);
};
