#include "Terrain.hpp"


using namespace std;

Terrain::Terrain(string heightmap_file)
{
    load_heightmap(heightmap_file);
}

// -------------------

int Terrain::get_pixel_index(int x, int z, SDL_Surface* image)
{
    return image->w * z + x;
}


// -------------------
// Height is the mean of the rgb pixel values in the image

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

void Terrain::load_heightmap(string heightmap_file) 
{
    SDL_Surface* heightmap = nullptr;
    heightmap = IMG_Load(heightmap_file.c_str());
    Mesh* m = new Mesh();

    if (heightmap == nullptr){
        Error::throw_error(Error::cant_load_image, heightmap_file);
    }

    if (heightmap->format->BitsPerPixel != 32){
        Error::throw_error(Error::cant_load_image, "Need 32-bit per pixel images for heightmap, this image is " + to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }
    
    for (int z = 0; z < heightmap->h; z++){
        for (int x = 0; x < heightmap->w; x++){
            // Create vertices (points in 3D space)
            m->vertices.push_back(x);
            m->vertices.push_back(get_pixel_height(x, z, heightmap));
            m->vertices.push_back(z);

            // Create a normal for each vertice unless at edges of terrain.
            // Then normal is pointing straight upward
            if ( x < heightmap->w-1 and x > 0 and z < heightmap->h-1 and z > 0){
                
            }
        }
    }
}


// -------------------

/* Private Model functions */


Mesh* Terrain::load_mesh(aiMesh* ai_mesh, const aiScene* scene) 
{

}

