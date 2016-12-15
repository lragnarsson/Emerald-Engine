#include "Mesh.hpp"

/* --- MESH --- */
/* Public Mesh functions */
vector<Texture*> Mesh::loaded_textures;

void Mesh::clear_mem(){
    vector<GLuint>().swap(this->indices);
    vector<GLfloat>().swap(this->vertices);
    vector<GLfloat>().swap(this->normals);
    vector<GLfloat>().swap(this->tex_coords);
    vector<GLfloat>().swap(this->tangents);

    //delete(this->diffuse_map);
    //delete(this->specular_map);
    //delete(this->normal_map);
}

// -------------------------

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

void Mesh::set_texture(const string full_path, const bool clamp,
                       const texture_type tex_type)
{
    Texture *texture;
    bool already_loaded = false;
    // Check if texture has been loaded already:
    for (int i = 0; i < Mesh::loaded_textures.size(); i++) {
        if (!full_path.compare(Mesh::loaded_textures[i]->full_path)) {
            texture = Mesh::loaded_textures[i];
            already_loaded = true;
        }
    }

    if (!already_loaded) {
        texture = Mesh::load_texture(full_path, clamp, tex_type);
    }

    switch (tex_type) {
    case DIFFUSE:
        this->diffuse_map = texture;
        break;
    case SPECULAR:
        this->specular_map = texture;
        break;
    case NORMAL:
        this->normal_map = texture;
        break;
    }
}

// -----------------

Texture *Mesh::load_texture(const string full_path, const bool clamp,
                            const texture_type tex_type)
{
    SDL_Surface* surface = IMG_Load(full_path.c_str());
    if (surface == NULL) {
        Error::throw_error(Error::cant_load_image, SDL_GetError());
    }

    /* Upload texture */
    Texture* texture = new Texture();
    texture->type = tex_type;
    texture->full_path = full_path;
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Enable max anisotropic filtering:
    float aniso = 0.0f;
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

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
    delete surface;

    Mesh::loaded_textures.push_back(texture);

    return texture;
}

// ------------------
// Culling

glm::vec3 Mesh::get_center_point_world(glm::mat4 m2w_matrix)
{
    return glm::vec3(m2w_matrix * glm::vec4(this->bounding_sphere_center, 1.f));
}

glm::vec3 Mesh::get_center_point()
{
    return this->bounding_sphere_center;
}
