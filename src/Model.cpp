#include "Model.hpp"


class Model {
public:
    void Draw(GLuint shaderProgram) {
        for (auto mesh : this->meshes) {
            mesh.Draw(shaderProgram);
        }
    }

    void Load(std::string path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        this->directory = path.substr(0, path.find_last_of('/'));
        this->unfoldAssimpNode(scene->mRootNode, scene);
    }


private:
    void unfoldAssimpNode(aiNode* node, const aiScene* scene) {
        for(GLuint i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->processMesh(mesh, scene));
        }

        for(GLuint i = 0; i < node->mNumChildren; i++) {
            this->unfoldAssimpNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        Mesh m;

        m.indexCount = 3 * mesh->mNumFaces;
        m.vertexCount = mesh->mNumVertices;

        for(GLuint i = 0; i < mesh->mNumVertices; i++) {
            m.vertices.push_back(mesh->mVertices[i].x);
            m.vertices.push_back(mesh->mVertices[i].y);
            m.vertices.push_back(mesh->mVertices[i].z);

            m.normals.push_back(mesh->mNormals[i].x);
            m.normals.push_back(mesh->mNormals[i].y);
            m.normals.push_back(mesh->mNormals[i].z);

            if(mesh->mTextureCoords[0]) {
                m.texCoords.push_back(mesh->mTextureCoords[0][i].x);
                m.texCoords.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                m.texCoords.push_back(0.0f);
                m.texCoords.push_back(0.0f);
            }
        }

        for(GLuint i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for(GLuint j = 0; j < face.mNumIndices; j++) {
                m.indices.push_back(face.mIndices[j]);
            }
        }

        if(mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            for(GLuint i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
                aiString filepath;
                material->GetTexture(aiTextureType_DIFFUSE, i, &filepath);
                Texture* texture;
                texture = LoadTexture(filepath.C_Str(), this->directory);
                texture->type = DIFFUSE;
                texture->path = filepath;
                m.textures.push_back(texture);
            }

            for(GLuint i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++) {
                aiString filepath;
                material->GetTexture(aiTextureType_SPECULAR, i, &filepath);
                Texture* texture;
                texture = LoadTexture(filepath.C_Str(), this->directory);
                texture->type = SPECULAR;
                texture->path = filepath;
                m.textures.push_back(texture);
            }
        }
        m.UploadMeshData();
        return m;
    }


    Texture* LoadTexture(const char* filename, std::string basepath) {
        std::string filepath = basepath + "/" + std::string(filename);
        std::cout << filename << "\n";

        for (uint i = 0; i < loadedModelData.textures.size(); i++) {
            if (!strcmp(loadedModelData.textures[i]->path.C_Str(), filename)) {
                return loadedModelData.textures[i];
            }
        }

        int width, height;
        unsigned char* image = SOIL_load_image(filepath.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

        /* Upload texture */
        Texture* texture = new Texture();
        glGenTextures(1, &texture->id);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);

        loadedModelData.textures.push_back(texture);

        return texture;
    }
};
