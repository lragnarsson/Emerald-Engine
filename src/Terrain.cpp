#include "Terrain.hpp"


using namespace std;

Terrain::Terrain(string heightmap_file)
{
    load_heightmap(heightmap_file);
}

// -------------------

int Terrain::get_pixel_index(int x, int y, SDL_Surface* image)
{
    return image->w * y + x;
}


// -------------------
// Height is the mean of the rgb pixel values in the image

int Terrain::get_pixel_height(int x, int y, SDL_Surface* image)
{
    Uint8 red,green,blue,alpha;
    int index = get_pixel_index(x, y, image);

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

    if (heightmap == nullptr){
        Error::throw_error(Error::cant_load_image, heightmap_file);
    }

    if (heightmap->format->BitsPerPixel != 32){
        Error::throw_error(Error::cant_load_image, "Need 32-bit per pixel images for heightmap, this image is " + to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }
    
    for (int y = 0; y < heightmap->h; y++){
        for (int x = 0; x < heightmap->w; x++){
            NULL;
        }
    }
}


// -------------------

/* Private Model functions */


Mesh* Terrain::load_mesh(aiMesh* ai_mesh, const aiScene* scene) 
{

}

