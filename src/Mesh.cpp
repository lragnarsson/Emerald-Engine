#include "Mesh.hpp"

/* --- MESH --- */
/* Public Mesh functions */
std::vector<Texture*> Mesh::loaded_textures;

void Mesh::upload_mesh_data()
{
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    glGenBuffers(1, &this->EBO);
    glGenBuffers(4, this->VBO);

    GLuint* indices = &this->indices[0];
    GLfloat* vertices = &this->vertices[0];
    GLfloat* normals = &this->normals[0];
    GLfloat* tex_coords = &this->tex_coords[0];
    GLfloat* tangents = &this->tangents[0];

    /* Element array buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    /* Vertex coordinates */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    /* Normal vectors */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    /* Texture coordinates */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, 2 * vertex_count * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    /* Tangent vectors */
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), tangents, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    /* Unbind VAO */
    glBindVertexArray(0);
}

// --------------------

GLuint Mesh::get_VAO()
{
    return VAO;
}

// --------------------

Texture* Mesh::load_texture(const std::string filename, const std::string basepath, bool clamp)
{
    std::string filepath = basepath + "/" + filename;
    for (uint i = 0; i < Mesh::loaded_textures.size(); i++) {
        if (!filename.compare(std::string(Mesh::loaded_textures[i]->path.C_Str()))) {
            return Mesh::loaded_textures[i];
        }
    }

    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (surface == NULL) {
        Error::throw_error(Error::cant_load_image, SDL_GetError());
    }

    /* Upload texture */
    Texture* texture = new Texture();
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    if (clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (surface->format->BytesPerPixel == 4) {
        GLenum color_format = surface->format->Rmask == 255 ? GL_RGBA : GL_BGRA;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, color_format, GL_UNSIGNED_BYTE, surface->pixels);
    } else {
        GLenum color_format = surface->format->Rmask == 255 ? GL_RGB : GL_BGR;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, color_format, GL_UNSIGNED_BYTE, surface->pixels);
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    Mesh::loaded_textures.push_back(texture);

    return texture;
}
