#ifndef TERRAIN_H
#define TERRAIN_H

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
#include <algorithm>

#include "Error.hpp"
#include "Mesh.hpp"


class Terrain {
public:
    Terrain() { };
    Terrain(std::string heightmap_file);
    ~Terrain() { };

    const std::vector<Mesh*> get_meshes();
    void load_heightmap(std::string heightmap_file);

    glm::vec3 get_center_point_world();
    glm::vec3 get_center_point();
private:
    GLuint VAO, EBO;
    GLuint VBO[4]; // Vertices, normals, texCoords, tangents
    std::vector<Mesh*> meshes;
    
    int get_pixel_height(int x, int y, SDL_Surface* image);
    int get_pixel_index(int x, int y, SDL_Surface* image);
    Mesh* load_mesh(aiMesh* ai_mesh, const aiScene* scene);
};



#endif
