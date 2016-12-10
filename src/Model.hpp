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
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Light.hpp"
#include "Error.hpp"
#include "Animation_Path.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"


typedef struct {
    vec3 position;  // 12 +
    float radius;   // 4 = 16
} gpu_sphere;

class Model {
public:
    static int models_drawn;
    static std::vector<GLuint> shader_programs; // For UBO binding on init.
    glm::mat4 m2w_matrix, move_matrix, rot_matrix, scale_matrix;
    float bounding_sphere_radius = -1.f, scale = 1.f;
    bool draw_me = true, clamp_textures = false;
    int num_lights;

    Model() { };
    Model(const std::string path);
    Model(const std::string path, const glm::mat4 rot_matrix,
          const glm::vec3 world_coord, float scale, bool flat);

    ~Model() { };

    void load(std::string path);
    static void init_ubos();
    static void upload_spheres(Camera *camera);
    static const std::vector<Model*> get_loaded_models();
    static const std::vector<Model*> get_loaded_flat_models();
    const std::vector<Mesh*> get_meshes();

    void attach_light(Light *light, glm::vec3 relative_pos);
    void move_to(glm::vec3 world_coord);
    void move(glm::vec3 relative);
    void rotate(glm::vec3 axis, float angle);
    glm::vec3 get_center_point_world();
    glm::vec3 get_center_point();
    glm::vec3 get_light_color();
    bool get_light_active();
    void attach_animation_path(int animation_id, float start_parameter);
    bool has_animation_path() {return has_animation;}
    void move_along_path(float elapsed_time);
    unsigned cull_me(Camera* camera);

private:
    struct light_container {
        Light *light;
        glm::vec3 relative_pos;
    };

    static const int sphere_size = 16;
    static const int info_size = 4;
    static GLuint ubos[2];
    static std::vector<Model*> loaded_models, loaded_flat_models;
    static gpu_sphere gpu_spheres[_MAX_MODELS_]; // Sorted, pushed to GPU
    std::vector<light_container> attached_lights;
    std::vector<Mesh*> meshes;
    std::string directory;
    glm::vec3 world_coord;
    glm::vec3 bounding_sphere_center;
    float spline_parameter;
    bool has_animation;
    Animation_Path* anim_path;

    void unfold_assimp_node(aiNode* node, const aiScene* scene);
    Mesh* load_mesh(aiMesh* mesh, const aiScene* scene);
    void generate_bounding_sphere();
};

#endif
