#ifndef MODEL_H
#define MODEL_H

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

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Model.hpp"
#include "Error.hpp"


class Skybox
{
public:


private:
    GLuint cubemap;
}

#endif
