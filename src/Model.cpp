#include "Model.hpp"

/* --- MODEL ---*/
std::vector<Model*> Model::loaded_models, Model::loaded_flat_models;

Model::Model(const std::string path)
{
    this->rot_matrix = glm::mat4(1.f);
    this->scale = 1.f;
    this->scale_matrix = glm::mat4(1.f);
    this->world_coord = glm::vec3(0.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix * scale_matrix;
    this->clamp_textures = true;
    this->num_lights = 0;

    load(path);
    generate_bounding_sphere();
}


Model::Model(const std::string path, const glm::mat4 rot_matrix, const glm::vec3 world_coord, float scale, bool flat)
{
    this->rot_matrix = rot_matrix;
    this->scale = scale;
    this->scale_matrix = glm::scale(glm::mat4(1.f), glm::vec3(scale));
    this->world_coord = world_coord;;
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix * scale_matrix;
    this->num_lights = 0;

    load(path);
    generate_bounding_sphere();
    if (!flat) {
        Model::loaded_models.push_back(this);
    }
    else {
        Model::loaded_flat_models.push_back(this);
    }

    has_animation = false;
}


/* Public Model functions */
glm::vec3 Model::get_center_point_world()
{
    return glm::vec3(this->m2w_matrix * glm::vec4(this->bounding_sphere_center, 1.f));
}

void Model::attach_animation_path(int animation_id, float start_parameter)
{
    this->anim_path = Animation_Path::get_animation_path_with_id(animation_id);
    this->spline_parameter = start_parameter;
    this->has_animation = true;
}

void Model::move_along_path(float elapsed_time)
{
    glm::vec3 new_pos;
    // get_pos updates the spline parameter for next iteration
    if (has_animation) {
        new_pos = this->anim_path->get_pos(this->spline_parameter,
                                                     elapsed_time);
    } else {
        Error::throw_error(Error::model_has_no_path);
    }
    this->move_to(new_pos);
}

glm::vec3 Model::get_center_point()
{
    return this->bounding_sphere_center;
}


glm::vec3 Model::get_light_color()
{
    if (num_lights > 0)
        return attached_lights[0].light->color;
    return glm::vec3(1.f);
}

bool Model::get_light_active()
{
    if (num_lights > 0)
        return attached_lights[0].light->active;
    return true;
}

void Model::attach_light(Light *light, glm::vec3 relative_pos) {
    light_container new_light = {light, relative_pos};

    glm::vec3 light_pos = glm::vec3(m2w_matrix * glm::vec4(relative_pos, 1.f));
    light->position = light_pos;
    this->num_lights++;
    this->attached_lights.push_back(new_light);
}


/* Move model and all attached lights to world_coord.
Important: the lights does not currently keep their relative
position to the model */
void Model::move_to(glm::vec3 world_coord) {
    this->world_coord = world_coord;

    move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    m2w_matrix =  move_matrix * rot_matrix * scale_matrix;

    for (auto container : this->attached_lights) {
        glm::vec3 new_pos = glm::vec3(m2w_matrix * glm::vec4(container.relative_pos, 1.f));
        container.light->position = new_pos;
    }
}


void Model::move(glm::vec3 relative) {
    move_to(this->world_coord + relative);
}


void Model::rotate(glm::vec3 axis, float angle) {
    rot_matrix = glm::rotate(rot_matrix, angle, axis);
    m2w_matrix = move_matrix * rot_matrix * scale_matrix;

    for (auto container : this->attached_lights) {
        glm::vec3 new_pos = glm::vec3(m2w_matrix * glm::vec4(container.relative_pos, 1.f));
        container.light->position = new_pos;
    }
}


void Model::load(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs |
                                             aiProcess_GenNormals |
                                             aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Error::throw_error(Error::cant_load_model, importer.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    unfold_assimp_node(scene->mRootNode, scene);
}



// --------------------------------------------------------------------------
/* Private Model functions */
void Model::unfold_assimp_node(aiNode* node, const aiScene* scene) {
    for(GLuint i = 0; i < node->mNumMeshes; i++) {
        aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(load_mesh(ai_mesh, scene));
    }

    for(GLuint i = 0; i < node->mNumChildren; i++) {
        unfold_assimp_node(node->mChildren[i], scene);
    }
}


Mesh* Model::load_mesh(aiMesh* ai_mesh, const aiScene* scene) {
    Mesh* m = new Mesh();

    m->index_count = 3 * ai_mesh->mNumFaces;
    m->vertex_count = ai_mesh->mNumVertices;

    for(GLuint i = 0; i < ai_mesh->mNumVertices; i++) {
        m->vertices.push_back(ai_mesh->mVertices[i].x);
        m->vertices.push_back(ai_mesh->mVertices[i].y);
        m->vertices.push_back(ai_mesh->mVertices[i].z);

        m->normals.push_back(ai_mesh->mNormals[i].x);
        m->normals.push_back(ai_mesh->mNormals[i].y);
        m->normals.push_back(ai_mesh->mNormals[i].z);

        if(ai_mesh->HasTextureCoords(0)) {
            m->tex_coords.push_back(ai_mesh->mTextureCoords[0][i].x);
            m->tex_coords.push_back(ai_mesh->mTextureCoords[0][i].y);
        } else {
            m->tex_coords.push_back(0.0f);
            m->tex_coords.push_back(0.0f);
        }

        if(ai_mesh->HasTangentsAndBitangents()) {
            m->tangents.push_back(ai_mesh->mTangents[i].x);
            m->tangents.push_back(ai_mesh->mTangents[i].y);
            m->tangents.push_back(ai_mesh->mTangents[i].z);
        } else {
            m->tangents.push_back(0.f);
            m->tangents.push_back(0.f);
            m->tangents.push_back(0.f);
        }
    }

    for(GLuint i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for(GLuint j = 0; j < face.mNumIndices; j++) {
            m->indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];

    GLfloat shininess;
    material->Get(AI_MATKEY_SHININESS, shininess);
    shininess = shininess ? shininess : 86.f;
    m->shininess = shininess / 3.f; // Assimp multiplies shininess by 4 because reasons

    if(material->GetTextureCount(aiTextureType_DIFFUSE)) {
        aiString filepath;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);
        Texture* texture;
        texture = m->load_texture(std::string(filepath.C_Str()), this->directory, clamp_textures);
        texture->type = DIFFUSE;
        texture->path = filepath;
        m->diffuse_map = texture;
    } else {
        Texture* texture;
        texture = m->load_texture(DEFAULT_DIFFUSE, DEFAULT_PATH, clamp_textures);
        texture->type = DIFFUSE;
        texture->path = DEFAULT_PATH;
        m->diffuse_map = texture;
    }

    if(material->GetTextureCount(aiTextureType_SPECULAR)) {
        aiString filepath;
        material->GetTexture(aiTextureType_SPECULAR, 0, &filepath);
        Texture* texture;
        texture = m->load_texture(std::string(filepath.C_Str()), this->directory, clamp_textures);
        texture->type = SPECULAR;
        texture->path = filepath;
        m->specular_map = texture;
    } else { // Use diffuse map as a specular map when specular is missing
        m->specular_map = m->diffuse_map;
    }

    if(material->GetTextureCount(aiTextureType_HEIGHT)) {
        aiString filepath;
        material->GetTexture(aiTextureType_HEIGHT, 0, &filepath);
        Texture* texture;
        texture = m->load_texture(std::string(filepath.C_Str()), this->directory, clamp_textures);
        texture->type = NORMAL;
        texture->path = filepath;
        m->normal_map = texture;
    } else { // Default normal map keeps the geometry defined normals
        Texture* texture;
        texture = m->load_texture(DEFAULT_NORMAL, DEFAULT_PATH, clamp_textures);
        texture->type = NORMAL;
        texture->path = DEFAULT_PATH;
        m->normal_map = texture;
    }

    m->upload_mesh_data();

    return m;
}

// -----------------
// Culling

void Model::generate_bounding_sphere()
{
    GLfloat v = this->meshes[0]->vertices[0];
    GLfloat x_max = v, y_max = v, z_max = v, x_min = v, y_min = v, z_min = v;

    for (auto mesh : this->meshes) {
        GLfloat v_local = mesh->vertices[0];
        GLfloat x_local_max = v_local, y_local_max = v_local, z_local_max = v_local, x_local_min = v_local, y_local_min = v_local, z_local_min = v_local;
        for (int i=0; i < mesh->vertices.size() - 2; i+=3) {
            // Maxes
            if (mesh->vertices[i] > x_max){
                x_max = mesh->vertices[i];
            }
            if(mesh->vertices[i] > x_local_max){
                x_local_max = mesh->vertices[i];
            }

            if (mesh->vertices[i + 1] > y_max){
                y_max = mesh->vertices[i + 1];
            }
            if(mesh->vertices[i + 1] > y_local_max){
                y_local_max = mesh->vertices[i + 1];
            }

            if (mesh->vertices[i + 2] > z_max){
                z_max = mesh->vertices[i + 2];
            }
            if(mesh->vertices[i + 2] > z_local_max){
                z_local_max = mesh->vertices[i + 2];
            }
            
            // Mins
            if (mesh->vertices[i] < x_min){
                x_min = mesh->vertices[i];
            }
            if (mesh->vertices[i] < x_local_min){
                x_local_min = mesh->vertices[i];
            }

            if (mesh->vertices[i + 1] < y_min){
                y_min = mesh->vertices[i + 1];
            }
            if (mesh->vertices[i + 1] < y_local_min){
                y_local_min = mesh->vertices[i + 1];
            }

            if (mesh->vertices[i + 2] < z_min){
                z_min = mesh->vertices[i + 2];
            }
            if (mesh->vertices[i + 2] < z_local_min){
                z_local_min = mesh->vertices[i + 2];
            }
        }
        // Make sure meshes has bounding spheres
        glm::vec3 max_corner = glm::vec3(x_local_max, y_local_max, z_local_max);
        glm::vec3 min_corner = glm::vec3(x_local_min, y_local_min, z_local_min);

        glm::vec3 r_vector = 0.5f * (max_corner - min_corner);
        mesh->bounding_sphere_radius = glm::length(r_vector);
        mesh->bounding_sphere_center = min_corner + r_vector;
    }
    // Make sure entire model has bounding sphere
    glm::vec3 max_corner = glm::vec3(x_max, y_max, z_max);
    glm::vec3 min_corner = glm::vec3(x_min, y_min, z_min);

    glm::vec3 r_vector = 0.5f * (max_corner - min_corner);
    this->bounding_sphere_radius = glm::length(r_vector);
    this->bounding_sphere_center = min_corner + r_vector;

}


// -----------

const std::vector<Model*> Model::get_loaded_models()
{
    return Model::loaded_models;
}

// -----------

const std::vector<Model*> Model::get_loaded_flat_models()
{
    return Model::loaded_flat_models;
}

const std::vector<Mesh*> Model::get_meshes()
{
    return meshes;
}


// ------------
//

unsigned Model::cull_me(Camera* camera){
    unsigned drawn_meshes = 0;
    bool draw_me = camera->sphere_in_frustum(this->get_center_point_world(), \
            this->bounding_sphere_radius * this->scale);
    this->draw_me = draw_me;

    // If draw me - see if we can cull meshes
    if (draw_me){
        for (auto mesh : this->get_meshes()) {
            bool draw_me = camera->sphere_in_frustum(mesh->get_center_point_world(this->m2w_matrix), \
                    mesh->bounding_sphere_radius * this->scale);
            mesh->draw_me = draw_me;
            if (draw_me)
                drawn_meshes++; 
        }
    }

    return drawn_meshes;
}
