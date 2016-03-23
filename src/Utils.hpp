#pragma once
#define GLEW_STATIC
#define GL3_PROTOTYPES 1
#define GLM_FORCE_RADIANS

#ifndef __APPLE__
    #include <GL/glew.h>
#endif

#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#endif

#include <SDL2/SDL.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


/* Loads, compiles and activates vertex and fragment shaders */
GLuint loadShaders(const GLchar* vertexFilePath, const GLchar* fragmentFilePath);

enum TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL
};

struct Texture {
    GLuint id;
    TextureType type;
    aiString path;
};
