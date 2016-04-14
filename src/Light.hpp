#ifndef LIGHT_H
#define LIGHT_H

#ifdef __linux__
  #include <GL/glew.h>
#endif

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <string>
#include <vector>

class Light
{
public:
    static void upload_all(const GLuint shader_program);

    Light(const glm::vec3 world_coord, const glm::vec3 ambient_color,
          const glm::vec3 diffuse_color, const glm::vec3 specular_color);

    void upload(const GLuint shader_program);
    glm::vec3 get_color();
    
private:
    unsigned int id;
    glm::vec3 position, ambient_color, diffuse_color, specular_color;

    static std::vector<Light*> lights;
};



#endif
