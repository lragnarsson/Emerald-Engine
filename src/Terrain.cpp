#include "Terrain.hpp"


using namespace glm;


std::vector<Terrain*> Terrain::loaded_terrain;

// --------------------

Terrain::Terrain()
{   this->rot_matrix = glm::mat4(1.f);
    this->world_coord = glm::vec3(0.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix;
    this->clamp_textures = false;
    
    Terrain::loaded_terrain.push_back(this);
}

// --------------------

Terrain::Terrain(std::string directory, float plane_scale, float height_scale)
{   this->rot_matrix = glm::mat4(1.f);
    this->world_coord = glm::vec3(0.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix;
    this->clamp_textures = false;
    
    load_heightmap(directory, plane_scale, height_scale);

    Terrain::loaded_terrain.push_back(this);
}

// -------------------
Terrain::~Terrain(){
    for (auto mesh : meshes){
        delete(mesh);
    }
}

// ---------------------------------------------------------------------------------------------------
// PRIVATE FUNCTIONS

int Terrain::get_pixel_index(int x, int z, SDL_Surface* image)
{
    return (image->w * z + x);
}


// -------------------
// Height is chosen as the mean of the rgb pixel values in the image

float Terrain::get_pixel_height(int x, int z, SDL_Surface* image)
{
    //Uint8 black;
    int index = get_pixel_index(x, z, image);

    Uint8 *all_pixels = (Uint8*) image->pixels; 
    Uint8 pixel = all_pixels[index]; 

    //SDL_GetRGBA(pixel, image->format, &red, &green, &blue, &alpha);
    
    //std::cout << pixel << std::endl;
    return pixel;
}

// -------------------

void Terrain::load_heightmap(std::string directory, float plane_scale, float height_scale) 
{
    std::string heightmap_file = directory + "/" + "heightmap.png";
    SDL_Surface* heightmap = IMG_Load(heightmap_file.c_str());
    Mesh* m = new Mesh();

    if (heightmap == NULL){
        Error::throw_error(Error::cant_load_image, SDL_GetError());
    }

    if (heightmap->format->BitsPerPixel != 8){
        Error::throw_error(Error::cant_load_image, "Need 8-bit per pixel images for heightmap, this image is " + std::to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }
    
    // Specify triangle to vertice numbers
    m->index_count = 3*2*((heightmap->w-1) * (heightmap->h-1));
    m->vertex_count = heightmap->w * heightmap->h;
    
    for (int z = 0; z < heightmap->h; z++){
        for (int x = 0; x < heightmap->w; x++){
            float height = get_pixel_height(x, z, heightmap);
            
            // Create vertices (points in 3D space)
            m->vertices.push_back(x*plane_scale);
            m->vertices.push_back(height*height_scale);
            m->vertices.push_back(z*plane_scale);

            // Create a normal for each vertice
            vec3 normal = get_normal(x, z, heightmap);
            m->normals.push_back(normal.x);
            m->normals.push_back(normal.y);
            m->normals.push_back(normal.z);
            
            // Texture coords
            m->tex_coords.push_back(x);
            m->tex_coords.push_back(z);

            // Need tangents beacause of transform in geometry.vert
            vec3 tangent = get_tangent(x, z, heightmap);
            m->tangents.push_back(tangent.x);
            m->tangents.push_back(tangent.y);
            m->tangents.push_back(tangent.z);
            
        }
    }
    
    // Generate vertice to triangle mapping
    for(int z = 0; z < heightmap->h-1; z++){
        for (int x = 0; x < heightmap->w-1; x++){
            // Down right of quad
            m->indices.push_back(x + z*heightmap->w);
            m->indices.push_back((x+1) + z*heightmap->w);
            m->indices.push_back((x+1) + (z+1)*heightmap->w);
            // Upper left of quad
            m->indices.push_back(x + z*heightmap->w);
            m->indices.push_back((x+1) + (z+1)*heightmap->w);
            m->indices.push_back(x + (z+1)*heightmap->w);
        }
    }
    // Same default shininess as in Model
    GLfloat shininess;
    m->shininess = 4.f; // Assimp multiplies shininess by 4 because reasons

    // Use default diffuse and specular maps
    Texture* diffuse_map;
    diffuse_map = m->load_texture("albedo.jpg", directory, clamp_textures);
    diffuse_map->type = DIFFUSE;
    diffuse_map->path = directory;
    m->diffuse_map = diffuse_map;
    
    
    Texture* specular_map;
    specular_map = m->load_texture("specular.jpg", directory, clamp_textures);
    specular_map->type = SPECULAR;
    specular_map->path = directory;
    m->specular_map = specular_map;
    
    // Keep normals as normal map
    Texture* normal_map;
    normal_map = m->load_texture("normal.jpg", directory, clamp_textures);
    normal_map->type = NORMAL;
    normal_map->path = directory;
    m->normal_map = normal_map;
    
    // Translate terrain to the middle
    this->scale = plane_scale;
    this->world_coord = glm::vec3(-heightmap->w*plane_scale/2.f, 0, -heightmap->h*plane_scale/2.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix;
    
    // Save mesh
    meshes.push_back(m);
    // Generate bounding sphere
    this->generate_bounding_sphere();
    
    m->upload_mesh_data();
}

// ------------------
// Returns mesh

const std::vector<Mesh*> Terrain::get_meshes()
{
    return meshes;
}

// -----------

const std::vector<Terrain*> Terrain::get_loaded_terrain()
{
    return Terrain::loaded_terrain;
}

// -------------------
/* Private Model functions */

vec3 Terrain::get_normal(int x, int z, SDL_Surface* image){
    // If not along edges
    if ( x < image->w-1 and x > 0 and z < image->h-1 and z > 0){
        vec3 base1 = vec3(x, get_pixel_height(x, z, image), z) - vec3(x-1, get_pixel_height(x-1, z, image), z);
        vec3 base2 = vec3(x, get_pixel_height(x, z+1, image), z+1) - vec3(x-1, get_pixel_height(x-1, z, image), z);
        vec3 normal1 = normalize(cross(base1, base2));
        if (normal1.y < 0) {
           normal1 = -normal1;
        }

        base1 = vec3(x+1, get_pixel_height(x+1, z, image), z) - vec3(x, get_pixel_height(x, z, image), z);
        base2 = vec3(x+1, get_pixel_height(x+1, z+1, image), z+1) - vec3(x, get_pixel_height(x, z, image), z);
        vec3 normal2 = normalize(cross(base1, base2));
        if (normal2.y < 0) {
           normal2 = -normal2;
        }

        base1 = vec3(x, get_pixel_height(x, z-1, image), z-1) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        base2 = vec3(x, get_pixel_height(x, z, image), z) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        vec3 normal3 = normalize(cross(base1, base2));
        if (normal3.y < 0) {
           normal3 = -normal3;
        }

        return normalize(normal1 + normal2 + normal3);
    }
    else { // Along edges just return upVector for now
        // TODO: Return normal for slopes
        return vec3(0, 1, 0);
    }
}

// ----------------------

vec3 Terrain::get_tangent(int x, int z, SDL_Surface* image){
    // If not along edges
    if ( x < image->w-1 and x > 0 and z < image->h-1 and z > 0){
        vec3 tangent = normalize(vec3(x+1, get_pixel_height(x+1, z, image), z) - vec3(x, get_pixel_height(x, z, image), z));
        
        return tangent;
    }
    else { // Along edges just return upVector for now
        // TODO: Return normal for slopes
        return vec3(1, 0, 0);
    }
}

// -----------------
// Culling

void Terrain::generate_bounding_sphere()
{
    GLfloat v = this->meshes[0]->vertices[0];
    GLfloat x_max = v, y_max = v, z_max = v, x_min = v, y_min = v, z_min = v;

    for (auto mesh : this->meshes) {
        for (int i=0; i < mesh->vertices.size() - 2; i+=3) {
            if (mesh->vertices[i] > x_max){
                x_max = mesh->vertices[i];
            }

            if (mesh->vertices[i + 1] > y_max){
                y_max = mesh->vertices[i + 1];
            }

            if (mesh->vertices[i + 2] > z_max){
                z_max = mesh->vertices[i + 2];
            }

            if (mesh->vertices[i] < x_min){
                x_min = mesh->vertices[i];
            }

            if (mesh->vertices[i + 1] < y_min){
                y_min = mesh->vertices[i + 1];
            }

            if (mesh->vertices[i + 2] < z_min){
                z_min = mesh->vertices[i + 2];
            }
        }
    }
    glm::vec3 max_corner = glm::vec3(x_max, y_max, z_max);
    glm::vec3 min_corner = glm::vec3(x_min, y_min, z_min);

    glm::vec3 r_vector = 0.5f * (max_corner - min_corner);
    this->bounding_sphere_radius = glm::length(r_vector);
    this->bounding_sphere_center = min_corner + r_vector;
}

// -----------

glm::vec3 Terrain::get_center_point_world()
{
    return glm::vec3(this->m2w_matrix * glm::vec4(this->bounding_sphere_center, 1.f));
}

glm::vec3 Terrain::get_center_point()
{
    return this->bounding_sphere_center;
}

