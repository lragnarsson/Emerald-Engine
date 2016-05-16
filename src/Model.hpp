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

const std::string DEFAULT_PATH = "res/models/default";
const std::string DEFAULT_DIFFUSE = "default_diffuse.jpg";
const std::string DEFAULT_NORMAL = "default_normal.jpg";

class Model;

enum texture_type {
    DIFFUSE,
    SPECULAR,
    NORMAL
};

struct Texture {
    GLuint id;
    texture_type type;
    aiString path;
};

typedef struct {
    GLuint diffuse_map_id;
    GLuint specular_map_id;
    GLuint normal_map_id;
    GLuint mesh_VAO;
    GLfloat shininess;
    GLuint index_count;
} mesh_struct;

typedef struct {
    bool draw_me;
    float bounding_sphere_radius;
    float scale;
    glm::vec3 bounding_sphere_center;
    bool has_animation;
    glm::mat4 m2w_matrix;
    std::vector<mesh_struct> mesh_data;
    Model* me;
} render_struct;


class Mesh {
public:
    GLuint index_count, vertex_count;
    GLfloat shininess = 80;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices, normals, tex_coords, tangents;
    Texture *diffuse_map, *specular_map, *normal_map;

    Mesh() { };
    ~Mesh() { };

    /* Upload vertices, normals etc to the GPU */
    void upload_mesh_data();
    GLuint get_VAO();

private:
    GLuint VAO, EBO;
    GLuint VBO[4]; // Vertices, normals, texCoords, tangents
};


class Model {
public:
    glm::mat4 move_matrix, rot_matrix, scale_matrix;
    float bounding_sphere_radius = -1.f, scale = 1.f;

    Model() { };
    Model(const std::string path, bool flat);
    Model(const std::string path, const glm::mat4 rot_matrix, const glm::vec3 world_coord, float scale, bool flat);

    ~Model() { };

    void load(std::string path);
    static const std::vector<Model*> get_loaded_models();
    static const std::vector<Model*> get_loaded_flat_models();
    static const std::vector<render_struct>* get_render_data();
    static const std::vector<render_struct>* get_render_data_flat();
    const std::vector<Mesh*> get_meshes();

    void attach_light(Light* light, glm::vec3 relative_pos);
    void move_to(glm::vec3 world_coord);
    void move(glm::vec3 relative);
    void rotate(glm::vec3 axis, float angle);
    std::vector<Light*> get_lights();
    glm::vec3 get_center_point_world();
    glm::vec3 get_center_point();
    void attach_animation_path(int animation_id, float start_parameter);
    bool has_animation_path() {return render_data.at(id).has_animation;}
    void move_along_path(float elapsed_time);

private:
    struct light_container {
        Light* light;
        glm::vec3 relative_pos;
    };

    unsigned id;

    static std::vector<Model*> loaded_models, loaded_flat_models;
    static std::vector<Texture*> loaded_textures;
    static std::vector<render_struct> render_data, render_data_flat;
    std::vector<render_struct>* render_data_pointer;

    std::vector<light_container> attached_lightsources;
    std::vector<Mesh*> meshes;
    std::string directory;
    glm::vec3 world_coord;
    float spline_parameter;
    Animation_Path* anim_path;

    void unfold_assimp_node(aiNode* node, const aiScene* scene);
    Mesh* load_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_texture(const std::string filename, const std::string basepath);
    void generate_bounding_sphere();
    void create_render_data(float scale, glm::mat4 m2w_matrix, bool flat);
};

#endif
