#include "Model.hpp"


/* --- MESH --- */
/* Public Mesh functions */
void Mesh::draw(GLuint shader_program) {
    GLuint diffuse_num = 1;
    GLuint specular_num = 1;
    glUseProgram(shader_program);
    
    for(GLuint i = 0; i < this->textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        if(this->textures[i]->type == DIFFUSE) {
            glUniform1i(glGetUniformLocation(shader_program, ("texture_Diffuse" + std::to_string(diffuse_num++)).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
        }
        else if(this->textures[i]->type == SPECULAR) {
            glUniform1i(glGetUniformLocation(shader_program, ("texture_Specular" + std::to_string(specular_num++)).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
        }
    }

    glUniform1f(glGetUniformLocation(shader_program, "shininess"), 40.0f);

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for (GLuint i = 0; i < this->textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}

void Mesh::upload_mesh_data(GLuint shader_program) {
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    glGenBuffers(1, &this->EBO);
    glGenBuffers(3, this->VBO);

    GLuint* indices = &this->indices[0];
    GLfloat* vertices = &this->vertices[0];
    GLfloat* normals = &this->normals[0];
    GLfloat* tex_coords = &this->tex_coords[0];

    /* Element array buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    /* Vertex coordinates */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(glGetAttribLocation(shader_program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(shader_program, "in_Position"));

    /* Normal vectors */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(glGetAttribLocation(shader_program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(shader_program, "in_Normal"));

    /* Texture coordinates */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, 2 * vertex_count * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(glGetAttribLocation(shader_program, "in_TexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(shader_program, "in_TexCoord"));

    /* Unbind VAO */
    glBindVertexArray(0);
}



/* --- MODEL ---*/
std::vector<Texture*> Model::loaded_textures;

Model::Model(const std::string path, const GLuint shader_program,
             const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix) {

        this->rot_matrix = rot_matrix;
        this->m2w_matrix = m2w_matrix;
        shader_programs.push_back(shader_program);
        load(path);
}

Model::Model(std::string path, const GLuint shader_program,
             const glm::mat4 rot_matrix, const glm::mat4 m2w_matrix, std::vector<Light*> lightsources) {
        this->rot_matrix = rot_matrix;
        this->m2w_matrix = m2w_matrix;
        this->attached_lightsources = lightsources;
        shader_programs.push_back(shader_program);
        load(path);
}

/* Public Model functions */
void Model::draw(GLuint shader_program) {
    glUseProgram(shader_program);
    /* Upload model to world matrix and model rotation for normal calculation */
    GLuint m2w = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(m2w, 1, GL_FALSE, glm::value_ptr(this->m2w_matrix));
    GLuint rot = glGetUniformLocation(shader_program, "modelRot");
    glUniformMatrix4fv(rot, 1, GL_FALSE, glm::value_ptr(this->rot_matrix));

    if (this->attached_lightsources.size() > 0) {
        GLuint color = glGetUniformLocation(shader_program, "color");
        glUniform3fv(color, 1, glm::value_ptr(this->attached_lightsources[0]->get_color()));
    }
        
    for (auto mesh : this->meshes) {
        mesh.draw(shader_program);
    }
    glUseProgram(0);
}

void Model::load(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of('/'));
    this->unfold_assimp_node(scene->mRootNode, scene);
}

void Model::attach_light(Light* light) {
    this->attached_lightsources.push_back(light);
}

void Model::move_to(glm::vec3 world_coord) {
    this->m2w_matrix = glm::translate(glm::mat4(1.f), world_coord) * this->rot_matrix;
    /* Upload new uniform */
    for (auto program : this->shader_programs) {
        glUseProgram(program);
        GLuint m2w = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(m2w, 1, GL_FALSE, glm::value_ptr(this->m2w_matrix));
    }
    glUseProgram(0);
}

void Model::move(glm::vec3 relative) {
    /* TODO! fix world_coord. Constructor should take world_coord and use to generate
       m2w_matrix. */
}

/* Private Model functions */
void Model::unfold_assimp_node(aiNode* node, const aiScene* scene) {
    for(GLuint i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->load_mesh(mesh, scene));
    }

    for(GLuint i = 0; i < node->mNumChildren; i++) {
        this->unfold_assimp_node(node->mChildren[i], scene);
    }
}

Mesh Model::load_mesh(aiMesh* mesh, const aiScene* scene) {
    Mesh m;

    m.index_count = 3 * mesh->mNumFaces;
    m.vertex_count = mesh->mNumVertices;

    for(GLuint i = 0; i < mesh->mNumVertices; i++) {
        m.vertices.push_back(mesh->mVertices[i].x);
        m.vertices.push_back(mesh->mVertices[i].y);
        m.vertices.push_back(mesh->mVertices[i].z);

        m.normals.push_back(mesh->mNormals[i].x);
        m.normals.push_back(mesh->mNormals[i].y);
        m.normals.push_back(mesh->mNormals[i].z);

        if(mesh->mTextureCoords[0]) {
            m.tex_coords.push_back(mesh->mTextureCoords[0][i].x);
            m.tex_coords.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            m.tex_coords.push_back(0.0f);
            m.tex_coords.push_back(0.0f);
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
            texture = load_texture(filepath.C_Str(), this->directory);
            texture->type = DIFFUSE;
            texture->path = filepath;
            m.textures.push_back(texture);
        }

        for(GLuint i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++) {
            aiString filepath;
            material->GetTexture(aiTextureType_SPECULAR, i, &filepath);
            Texture* texture;
            texture = load_texture(filepath.C_Str(), this->directory);
            texture->type = SPECULAR;
            texture->path = filepath;
            m.textures.push_back(texture);
        }
    }

    /* Upload for all relevant shader programs */
    if (!shader_programs.size())
        std::cerr << "No shader programs attached to model." << std::endl;
    for (auto program : shader_programs) {
        glUseProgram(program);
        m.upload_mesh_data(program);
    }
    glUseProgram(0);
    return m;
}


Texture* Model::load_texture(const char* filename, std::string basepath)
{
    std::string filepath = basepath + "/" + std::string(filename);

    for (uint i = 0; i < Model::loaded_textures.size(); i++) {
        if (!strcmp(Model::loaded_textures[i]->path.C_Str(), filename)) {
            return Model::loaded_textures[i];
        }
    }

    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (surface == NULL) {
      std::cerr << "Can not load image!" << SDL_GetError() << std::endl;
    }

    /* Upload texture */
    Texture* texture = new Texture();
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    Model::loaded_textures.push_back(texture);

    return texture;
}
