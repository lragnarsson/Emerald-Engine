#include "Skybox.hpp"


void Skybox::load_cubemap(const std::string basepath,
                          const std::vector<std::string> &filename)
{
    Texture* texture = new Texture();
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture->id);

    filepath = basepath + "/" + filenames[i];
    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (surface == NULL) {
        Error::throw_error(Error::cant_load_image, SDL_GetError());
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, surface->w,
                 surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}
