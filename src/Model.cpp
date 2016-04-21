#include "Model.hpp"

/* --- MESH --- */
/* Public Mesh functions */
void Mesh::draw_forward(GLuint shader_program)
{
  GLuint diffuse_num = 1;
  GLuint specular_num = 1;
  glUseProgram(shader_program);

  for(GLuint i = 0; i < this->textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    if(this->textures[i]->type == DIFFUSE) {
      glUniform1i(glGetUniformLocation(shader_program, ("texture_Diffuse" + std::to_string(diffuse_num++)).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
    }
    else if(this->textures[i]->type == SPECULAR) {
      glUniform1i(glGetUniformLocation(shader_program, ("texture_Specular" + std::to_string(specular_num++)).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
    }
  }

  glUniform1f(glGetUniformLocation(shader_program, "m.shininess"), this->shininess);
  glUniform3fv(glGetUniformLocation(shader_program, "m.ambient"), 1, glm::value_ptr(this->ambient_color));
  glUniform3fv(glGetUniformLocation(shader_program, "m.diffuse"), 1, glm::value_ptr(this->diffuse_color));
  glUniform3fv(glGetUniformLocation(shader_program, "m.specular"), 1, glm::value_ptr(this->specular_color));

  glBindVertexArray(this->VAO);
  glDrawElements(GL_TRIANGLES, this->index_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  for (GLuint i = 0; i < this->textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glUseProgram(0);
}


void Mesh::upload_mesh_data()
{
  glGenVertexArrays(1, &this->VAO);
  glBindVertexArray(this->VAO);
  glGenBuffers(1, &this->EBO);
  glGenBuffers(3, this->VBO);

  GLuint* indices = &this->indices[0];
  GLfloat* vertices = &this->vertices[0];
  GLfloat* normals = &this->normals[0];
  GLfloat* tex_coords = &this->tex_coords[0];

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

  /* Unbind VAO */
  glBindVertexArray(0);
}



/* --- MODEL ---*/
std::vector<Model*> Model::loaded_models, Model::loaded_flat_models;
std::vector<Texture*> Model::loaded_textures;

Model::Model(const std::string path, const glm::mat4 rot_matrix, const glm::vec3 world_coord, bool flat)
{
  this->rot_matrix = rot_matrix;
  this->m2w_matrix = glm::translate(glm::mat4(1.0f), world_coord) * rot_matrix;
  this->world_coord = world_coord;

  load(path);
  generate_bounding_sphere();
  if (!flat) {
    Model::loaded_models.push_back(this);
  }
  else {
    Model::loaded_flat_models.push_back(this);
  }
}


/* Public Model functions */
void Model::draw_forward(GLuint shader_program)
{
  glUseProgram(shader_program);
  /* Upload model to world matrix and model rotation for normal calculation */
  GLuint m2w_location = glGetUniformLocation(shader_program, "model");
  glUniformMatrix4fv(m2w_location, 1, GL_FALSE, glm::value_ptr(this->m2w_matrix));
  GLuint rot_location = glGetUniformLocation(shader_program, "modelRot");
  glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(this->rot_matrix));

  if (this->attached_lightsources.size() > 0) {
    GLuint color = glGetUniformLocation(shader_program, "color");
    glUniform3fv(color, 1, glm::value_ptr(this->attached_lightsources[0].light->get_color()));
  }

  for (auto mesh : this->meshes) {
    mesh.draw_forward(shader_program);
  }
  glUseProgram(0);
}


void Model::draw_deferred(GLuint shader_program) {
  return;
}


glm::vec3 Model::get_center_point()
{
  //return this->bounding_sphere_center;
  return glm::vec3(this->m2w_matrix * glm::vec4(this->bounding_sphere_center, 1.f));
}


void Model::attach_light(Light* light, glm::vec3 relative_pos) {
  light_container new_light = {light, relative_pos};

  glm::vec3 light_pos = glm::vec3(m2w_matrix * glm::vec4(relative_pos, 1.f));
  light->move_to(light_pos);
  light->upload();

  this->attached_lightsources.push_back(new_light);
}


/* Move model and all attached lights to world_coord and upload
the changed values to GPU.
Important: the lights does not currently keep their relative
position to the model */
void Model::move_to(glm::vec3 world_coord) {
  this->m2w_matrix = glm::translate(glm::mat4(1.f), world_coord) * this->rot_matrix;
  this->world_coord = world_coord;
  /* Upload new uniform */
  glUseProgram(Light::shader_program);
  GLuint m2w_loc = glGetUniformLocation(Light::shader_program, "model");
  glUniformMatrix4fv(m2w_loc, 1, GL_FALSE, glm::value_ptr(this->m2w_matrix));
  for (auto light_container : this->attached_lightsources) {
    glm::vec3 new_pos = glm::vec3(m2w_matrix * glm::vec4(light_container.relative_pos, 1.f));
    light_container.light->move_to(new_pos);
    light_container.light->upload_pos();
  }
  glUseProgram(0);
}

void Model::move(glm::vec3 relative) {
  move_to(this->world_coord + relative);
}

void Model::rotate(glm::vec3 axis, float angle) {
  rot_matrix = glm::rotate(rot_matrix, angle, axis);
  m2w_matrix = glm::translate(glm::mat4(1.0f), world_coord) * rot_matrix;

  for (auto light_container : this->attached_lightsources) {
    glm::vec3 new_pos = glm::vec3(m2w_matrix * glm::vec4(light_container.relative_pos, 1.f));
    light_container.light->move_to(new_pos);
    light_container.light->upload_pos();
  }
}


void Model::load(std::string path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

  if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));
  unfold_assimp_node(scene->mRootNode, scene);
}

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

Mesh Model::load_mesh(aiMesh* ai_mesh, const aiScene* scene) {
  Mesh m;

  m.index_count = 3 * ai_mesh->mNumFaces;
  m.vertex_count = ai_mesh->mNumVertices;

  for(GLuint i = 0; i < ai_mesh->mNumVertices; i++) {
    m.vertices.push_back(ai_mesh->mVertices[i].x);
    m.vertices.push_back(ai_mesh->mVertices[i].y);
    m.vertices.push_back(ai_mesh->mVertices[i].z);

    m.normals.push_back(ai_mesh->mNormals[i].x);
    m.normals.push_back(ai_mesh->mNormals[i].y);
    m.normals.push_back(ai_mesh->mNormals[i].z);

    if(ai_mesh->HasTextureCoords(0)) {
      m.tex_coords.push_back(ai_mesh->mTextureCoords[0][i].x);
      m.tex_coords.push_back(ai_mesh->mTextureCoords[0][i].y);
    } else {
      m.tex_coords.push_back(0.0f);
      m.tex_coords.push_back(0.0f);
    }
  }

  for(GLuint i = 0; i < ai_mesh->mNumFaces; i++) {
    aiFace face = ai_mesh->mFaces[i];
    for(GLuint j = 0; j < face.mNumIndices; j++) {
      m.indices.push_back(face.mIndices[j]);
    }
  }

  if(ai_mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];

    GLfloat shininess;
    material->Get(AI_MATKEY_SHININESS, shininess);
    m.shininess = shininess / 4.f; // Assimp multiplies shininess by 4 because reasons

    aiColor3D ambient, diffuse, specular;
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

    m.ambient_color = glm::vec3(ambient.r, ambient.g, ambient.b);
    m.diffuse_color = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    m.specular_color = glm::vec3(specular.r, specular.g, specular.b);

    for(GLuint i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
      aiString filepath;
      material->GetTexture(aiTextureType_DIFFUSE, i, &filepath);
      Texture* texture;
      texture = load_texture(filepath.C_Str(), this->directory);
      texture->type = DIFFUSE;
      texture->path = filepath;
      m.textures.push_back(texture);
    }

    for(GLuint i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++) {
      aiString filepath;
      material->GetTexture(aiTextureType_SPECULAR, i, &filepath);
      Texture* texture;
      texture = load_texture(filepath.C_Str(), this->directory);
      texture->type = SPECULAR;
      texture->path = filepath;
      m.textures.push_back(texture);
    }
  }

  m.upload_mesh_data();
  return m;
}


Texture* Model::load_texture(const char* filename, std::string basepath)
{
  glUseProgram(Light::shader_program);
  std::string filepath = basepath + "/" + std::string(filename);

  for (uint i = 0; i < Model::loaded_textures.size(); i++) {
    if (!strcmp(Model::loaded_textures[i]->path.C_Str(), filename)) {
      return Model::loaded_textures[i];
    }
  }

  SDL_Surface* surface = IMG_Load(filepath.c_str());
  if (surface == NULL) {
    std::cerr << "Can not load image!" << SDL_GetError() << std::endl;
  }

  /* Upload texture */
  Texture* texture = new Texture();
  glGenTextures(1, &texture->id);
  glBindTexture(GL_TEXTURE_2D, texture->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

  Model::loaded_textures.push_back(texture);

  glUseProgram(0);
  return texture;
}

std::vector<Light *> Model::get_lights()
{
  std::vector<Light *> lights;
  for (auto container : attached_lightsources) {
    lights.push_back(container.light);
  }
  return lights;
}

void Model::generate_bounding_sphere()
{
  GLfloat v = this->meshes[0].vertices[0];
  GLfloat x_max = v, y_max = v, z_max = v, x_min = v, y_min = v, z_min = v;

  for (auto mesh : this->meshes) {
    for (int i=0; i < mesh.vertices.size() - 2; i++) {
      if (mesh.vertices[i] > x_max)
      x_max = mesh.vertices[i];
      if (mesh.vertices[i + 1] > y_max)
      y_max = mesh.vertices[i + 1];
      if (mesh.vertices[i + 2] > z_max)
      z_max = mesh.vertices[i + 2];
      if (mesh.vertices[i] < x_min)
      x_min = mesh.vertices[i];
      if (mesh.vertices[i + 1] < y_min)
      y_min = mesh.vertices[i + 1];
      if (mesh.vertices[i + 2] < z_min)
      z_min = mesh.vertices[i + 2];
    }
  }
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
