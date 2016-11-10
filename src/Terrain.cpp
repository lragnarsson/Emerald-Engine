#include "Terrain.hpp"


using namespace glm;


std::vector<Terrain*> Terrain::loaded_terrain;


Terrain::Terrain(std::string heightmap_file)
{   this->rot_matrix = glm::mat4(1.f);
    this->scale = 1.f;
    this->scale_matrix = glm::mat4(1.f);
    this->world_coord = glm::vec3(0.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix * scale_matrix;
    this->clamp_textures = true;
    
    load_heightmap(heightmap_file);

    Terrain::loaded_terrain.push_back(this);
}

// -------------------

int Terrain::get_pixel_index(int x, int z, SDL_Surface* image)
{
    return image->w * z + x;
}


// -------------------
// Height is chosen as the mean of the rgb pixel values in the image

float Terrain::get_pixel_height(int x, int z, SDL_Surface* image)
{
    Uint8 red,green,blue,alpha;
    int index = get_pixel_index(x, z, image);

    Uint32 *all_pixels = (Uint32*) image->pixels; 
    Uint32 pixel = all_pixels[index]; 

    SDL_GetRGBA(pixel, image->format, &red, &green, &blue, &alpha);
    
    return (red+green+blue)/3.f;
}

// -------------------

void Terrain::load_heightmap(std::string heightmap_file) 
{
    SDL_Surface* heightmap = IMG_Load(heightmap_file.c_str());
    Mesh* m = new Mesh();
    std::string directory = heightmap_file.substr(0, heightmap_file.find_last_of('/'));

    if (heightmap == NULL){
        Error::throw_error(Error::cant_load_image, heightmap_file);
    }

    if (heightmap->format->BitsPerPixel != 32){
        Error::throw_error(Error::cant_load_image, "Need 32-bit per pixel images for heightmap, this image is " + std::to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }

    m->index_count = 3*(heightmap->w * heightmap->h);
    m->vertex_count = heightmap->w * heightmap->h;
    
    for (int z = 0; z < heightmap->h; z++){
        for (int x = 0; x < heightmap->w; x++){
            float height = get_pixel_height(x, z, heightmap);
            // Create vertices (points in 3D space)
            m->vertices.push_back(x);
            m->vertices.push_back(height);
            m->vertices.push_back(z);

            // Create a normal for each vertice
            vec3 normal = get_normal(x, z, heightmap);
            m->normals.push_back(normal.x);
            m->normals.push_back(normal.y);
            m->normals.push_back(normal.z);
            
            // Texture coords
            m->tex_coords.push_back(x);
            m->tex_coords.push_back(z);

            // No bitangents
            m->tangents.push_back(0.f);
            m->tangents.push_back(0.f);
            m->tangents.push_back(0.f);
        }
    }

    // Same default shininess as in Model
    GLfloat shininess;
    m->shininess = 86.f / 3.f; // Assimp multiplies shininess by 4 because reasons

    // Use default diffuse and specular maps
    Texture* diffuse_map;
    diffuse_map = m->load_texture(DEFAULT_DIFFUSE, DEFAULT_PATH, clamp_textures);
    diffuse_map->type = DIFFUSE;
    diffuse_map->path = DEFAULT_PATH;
    m->diffuse_map = diffuse_map;
    m->specular_map = m->diffuse_map;
    
    // Keep normals as normal map
    Texture* normal_map;
    normal_map = m->load_texture(DEFAULT_NORMAL, DEFAULT_PATH, clamp_textures);
    normal_map->type = NORMAL;
    normal_map->path = DEFAULT_PATH;
    m->normal_map = normal_map;
    
    // Load texture
    m->load_texture("texture.jpg", directory, clamp_textures);

    meshes.push_back(m);
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
        vec3 normal1 = cross(base1, base2);
        normal1.y = (normal1.y > 0) ? normal1.y : -normal1.y;

        base1 = vec3(x+1, get_pixel_height(x+1, z, image), z) - vec3(x, get_pixel_height(x, z, image), z);
        base2 = vec3(x+1, get_pixel_height(x+1, z+1, image), z+1) - vec3(x, get_pixel_height(x, z, image), z);
        vec3 normal2 = cross(base1, base2);
        normal2.y = (normal2.y > 0) ? normal2.y : -normal2.y;

        base1 = vec3(x, get_pixel_height(x, z-1, image), z-1) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        base2 = vec3(x, get_pixel_height(x, z, image), z) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        vec3 normal3 = cross(base1, base2);
        normal3.y = (normal3.y > 0) ? normal3.y : -normal3.y;

        return normalize(normal1 + normal2 + normal3);
    }
    else { // Along edges just return upVector for now
        // TODO: Return normal for slopes
        return vec3(0, 1, 0);
    }
}



