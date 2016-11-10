#include "Terrain.hpp"


using namespace glm;

Terrain::Terrain(std::string heightmap_file)
{
    load_heightmap(heightmap_file);
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
    SDL_Surface* heightmap = nullptr;
    heightmap = IMG_Load(heightmap_file.c_str());
    Mesh* m = new Mesh();

    if (heightmap == nullptr){
        Error::throw_error(Error::cant_load_image, heightmap_file);
    }

    if (heightmap->format->BitsPerPixel != 32){
        Error::throw_error(Error::cant_load_image, "Need 32-bit per pixel images for heightmap, this image is " + std::to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }

    for (int z = 0; z < heightmap->h; z++){
        for (int x = 0; x < heightmap->w; x++){
            // Create vertices (points in 3D space)
            m->vertices.push_back(x);
            m->vertices.push_back(get_pixel_height(x, z, heightmap));
            m->vertices.push_back(z);

            // Create a normal for each vertice unless at edges of terrain.
            // Then normal is pointing straight upward
            vec3 normal = get_normal(x, z, heightmap);
            m->normals.push_back(normal.x);
            m->normals.push_back(normal.y);
            m->normals.push_back(normal.z);
        }
    }
}


// -------------------
/* Private Model functions */

vec3 Terrain::get_normal(int x, int z, SDL_Surface* image){
    // If not along edges
    if ( x < image->w-1 and x > 0 and z < image->h-1 and z > 0){
        vec3 base1 = vec3(x, get_pixel_height(x, z, image), z) - vec3(x-1, get_pixel_height(x-1, z, image), z);
        vec3 base2 = vec3(x, get_pixel_height(x, z+1, image), z+1) - vec3(x-1, get_pixel_height(x-1, z, image), z);
        vec3 normal1 = cross(base1, base2);

        base1 = vec3(x+1, get_pixel_height(x+1, z, image), z) - vec3(x, get_pixel_height(x, z, image), z);
        base2 = vec3(x+1, get_pixel_height(x+1, z+1, image), z+1) - vec3(x, get_pixel_height(x, z, image), z);
        vec3 normal2 = cross(base1, base2);
        
        base1 = vec3(x, get_pixel_height(x, z-1, image), z-1) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        base2 = vec3(x, get_pixel_height(x, z, image), z) - vec3(x-1, get_pixel_height(x-1, z-1, image), z-1);
        vec3 normal3 = cross(base1, base2);

        return normalize(normal1 + normal2 + normal3);
    }
    else { // Along edges just return upVector for now
        // TODO: Return normal for slopes
        return vec3(0, 1, 0);
    }
}



