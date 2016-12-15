#include "Terrain.hpp"


using namespace glm;


std::vector<Terrain*> Terrain::loaded_terrain;
Texture* Terrain::wind_map;

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

Terrain::Terrain(std::string directory, float plane_scale, float height_scale, unsigned chunk_size)
{   this->rot_matrix = glm::mat4(1.f);
    this->scale = plane_scale;
    this->height_scale = height_scale;
    this->scale_matrix = glm::scale(glm::mat4(1.f), glm::vec3(plane_scale, height_scale, plane_scale));
    this->clamp_textures = false;
    
    load_heightmap(directory, plane_scale, height_scale, chunk_size);
    load_wind(directory + "/wind.jpg");

    Terrain::loaded_terrain.push_back(this);

    for (auto mesh : this->meshes){
        mesh->clear_mem();
    }

    // Debug
    printf("Terrain loaded. Number of meshes: %lu\n", meshes.size());
}

// -------------------
Terrain::~Terrain(){
    for (auto mesh : meshes){
        delete(mesh);
    }
}

// -------------------

float Terrain::get_height(float x_world, float z_world){

    // Translate coordinates from world to model coordinates
    float x_model = (x_world + this->scale * (float)this->total_x / 2.f);
    float z_model = (z_world + this->scale * (float)this->total_z / 2.f);
    // Translate coordinates from world to index values for the heightmap
    float x = (x_world + this->scale * (float)this->total_x / 2.f) / this->scale;
    float z = (z_world + this->scale * (float)this->total_z / 2.f) / this->scale;

    int int_x = (int)x;
    int int_z = (int)z;
    float deltax = x - (float)int_x;
    float deltaz = z - (float)int_z;

    // Needed for calculating normal
    vec3 normal;
    std::vector<vec3> vertices;

    if ( deltax + deltaz < 1 ) { // Decide wether we are in upper or lower part of quad
        vec3 p0(int_x * this->scale,
                get_pixel_height(int_x, int_z, this->heightmap)*this->height_scale,
                int_z * this->scale);
        vec3 p1(int_x * this->scale,
                get_pixel_height(int_x, int_z+1, this->heightmap)*this->height_scale,
                (int_z+1) * this->scale);
        vec3 p2((int_x+1) * this->scale,
                get_pixel_height(int_x+1, int_z, this->heightmap)*this->height_scale,
                int_z * this->scale);

        vertices.push_back(p0);
        vertices.push_back(p1);
        vertices.push_back(p2);

        normal = cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);
    }
    else {
        vec3 p0((int_x+1) * this->scale,
                get_pixel_height(int_x+1, int_z+1, this->heightmap)*this->height_scale,
                (int_z+1) * this->scale);
        vec3 p1((int_x+1) * this->scale,
                get_pixel_height(int_x+1, int_z, this->heightmap)*this->height_scale,
                int_z * this->scale);
        vec3 p2(int_x * this->scale,
                get_pixel_height(int_x, int_z+1, this->heightmap)*this->height_scale,
                (int_z+1) * this->scale);

        vertices.push_back(p0);
        vertices.push_back(p1);
        vertices.push_back(p2);

        normal = cross(vertices[1]-vertices[0], vertices[2] - vertices[0]);
    }

    // Plane equation
    float D = dot(vertices[0], normal);
    return 5.f + (D - normal.x * x_model - normal.z * z_model) / normal.y;
}

// ------------------
bool Terrain::point_in_terrain(float x_world, float z_world){

    float x = x_world + this->scale*this->total_x/2.f;
    float z = z_world + this->scale*this->total_z/2.f;

    if ( x > this->total_x*this->scale or z > this->total_z*this->scale or x < 0 or z < 0){
        return false;
    }

    return true;
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

    return pixel;
}

// -------------------

void Terrain::load_heightmap(std::string directory, float plane_scale, float height_scale, unsigned chunk_size)
{
    std::string heightmap_file = directory + "/" + "heightmap.png";
    SDL_Surface* heightmap = IMG_Load(heightmap_file.c_str());

    if (heightmap == NULL){
        Error::throw_error(Error::cant_load_image, SDL_GetError());
    }

    if (heightmap->format->BitsPerPixel != 8){
        Error::throw_error(Error::cant_load_image, "Need 8-bit per pixel images for heightmap, this image is " + std::to_string(heightmap->format->BitsPerPixel) + "-bit!");
    }

    // We need to save some size data in order to access correct indexes
    this->chunk_size = chunk_size;
    this->total_x = heightmap->w;
    this->total_z = heightmap->h;

    for (int z_total = 0; z_total < heightmap->h; z_total += chunk_size){
        for (int x_total = 0; x_total < heightmap->w; x_total += chunk_size){

            Mesh* m = new Mesh();
            // Specify triangle to vertex numbers
            m->index_count = 3*2*((chunk_size-1+1) * (chunk_size-1+1));
            m->vertex_count = (chunk_size+1) * (chunk_size+1);

            // Overlap so we don't get visible divisors between chunks
            for (int z = z_total; z < z_total+chunk_size+1; z++){
                for (int x = x_total; x < x_total+chunk_size+1; x++){

                    float height = get_pixel_height(x, z, heightmap);

                    // Create vertices (points in 3D space)
                    m->vertices.push_back(x);
                    m->vertices.push_back(height);
                    m->vertices.push_back(z);

                    // Create a normal for each vertice
                    vec3 normal = get_normal(x, z, heightmap, plane_scale, height_scale);
                    m->normals.push_back(normal.x);
                    m->normals.push_back(normal.y);
                    m->normals.push_back(normal.z);

                    // Texture coords
                    m->tex_coords.push_back(x);
                    m->tex_coords.push_back(z);

                    // Need tangents beacause of transform in geometry.vert
                    vec3 tangent = get_tangent(x, z, heightmap, plane_scale, height_scale);
                    m->tangents.push_back(tangent.x);
                    m->tangents.push_back(tangent.y);
                    m->tangents.push_back(tangent.z);

                }
            }

            // Generate vertex to triangle mapping
            for(int z = 0; z < chunk_size-1+1; z++){
                for (int x = 0; x < chunk_size-1+1; x++){
                    // Down right of quad
                    m->indices.push_back(x + z*(chunk_size+1));
                    m->indices.push_back((x+1) + (z+1)*(chunk_size+1));
                    m->indices.push_back((x+1) + z*(chunk_size+1));
                    // Upper left of quad
                    m->indices.push_back(x + z*(chunk_size+1));
                    m->indices.push_back(x + (z+1)*(chunk_size+1));
                    m->indices.push_back((x+1) + (z+1)*(chunk_size+1));
                }
            }
            m->shininess = 1.f;

            // Use default diffuse and specular maps
            m->set_texture(directory + "/albedo.png",
                           clamp_textures, DIFFUSE);

            m->set_texture(directory + "/specular.png",
                           clamp_textures, SPECULAR);

            // Keep normals as normal map
            m->set_texture(directory + "/normal.png",
                           clamp_textures, NORMAL);

            // Upload to GPU and save mesh
            m->upload_mesh_data();
            meshes.push_back(m);
        }
    }


    // Translate terrain to the middle
    this->world_coord = glm::vec3(-heightmap->w*plane_scale/2.f, 0.f, -heightmap->h*plane_scale/2.f);
    this->move_matrix = glm::translate(glm::mat4(1.f), world_coord);
    this->m2w_matrix = move_matrix  * rot_matrix * scale_matrix;
    // Generate bounding spheres
    this->generate_bounding_sphere();
    // Save image
    this->heightmap = heightmap;
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

/* ps = plane_scale
   hs = height_scale */
vec3 Terrain::get_normal(int x, int z, SDL_Surface* image,
                         float ps, float hs){
    vec3 p1(x*ps,     get_pixel_height(x, z, image)*hs,   z*ps);
    vec3 p2((x-1)*ps, get_pixel_height(x-1, z, image)*hs, z*ps);
    vec3 p3(x*ps,     get_pixel_height(x, z-1, image)*hs, (z-1)*ps);
    vec3 p4((x+1)*ps, get_pixel_height(x+1, z, image)*hs, z*ps);
    vec3 p5(x*ps,    get_pixel_height(x, z+1, image)*hs, (z+1)*ps);

    // If not along edges
    if ( x < image->w-1 and x > 0 and z < image->h-1 and z > 0){
        vec3 base1 = p1-p2;
        vec3 base2 = p1-p3;

        vec3 normal1 = normalize(cross(base1, base2));
        if (normal1.y < 0) {
            normal1 = -normal1;
        }

        base1 = p1-p4;
        base2 = p1-p3;
        vec3 normal2 = normalize(cross(base1, base2));
        if (normal2.y < 0) {
            normal2 = -normal2;
        }

        base1 = p1-p5;
        base2 = p1-p4;
        vec3 normal3 = normalize(cross(base1, base2));
        if (normal3.y < 0) {
            normal3 = -normal3;
        }


        base1 = p1-p2;
        base2 = p1-p5;
        vec3 normal4 = normalize(cross(base1, base2));
        if (normal4.y < 0) {
            normal4 = -normal4;
        }

        return normalize(normal1 + normal2 + normal3 + normal4);
    }
    else { // Along edges just return upVector for now
        // TODO: Return normal for slopes
        return vec3(0, 1, 0);
    }
}

// ----------------------

/* ps = plane_scale
   hs = height_scale */
vec3 Terrain::get_tangent(int x, int z, SDL_Surface* image, float ps, float hs){
    // If not along edges
    if ( x < image->w-1 and x > 0 and z < image->h-1 and z > 0){
        vec3 tangent = normalize(vec3((x+1)*ps, get_pixel_height(x+1, z, image)*hs, z*ps) -
                                 vec3(x*ps, get_pixel_height(x, z, image)*hs, z*ps));

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
    GLfloat x = this->meshes[0]->vertices[0];
    GLfloat y = this->meshes[0]->vertices[1];
    GLfloat z = this->meshes[0]->vertices[2];
    GLfloat x_max = x, y_max = y, z_max = z, x_min = x, y_min = y, z_min = z;

    for (auto mesh : this->meshes) {
        GLfloat x_local = mesh->vertices[0];
        GLfloat y_local = mesh->vertices[1];
        GLfloat z_local = mesh->vertices[2];
        GLfloat x_local_max = x_local, y_local_max = y_local, z_local_max = z_local, x_local_min = x_local, y_local_min = y_local, z_local_min = z_local;
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

        glm::vec3 r_vector_model = 0.5f * (max_corner - min_corner);
        glm::vec3 r_vector_world = glm::vec3(this->scale_matrix * glm::vec4(r_vector_model, 1.0f));
        mesh->bounding_sphere_radius = glm::length(r_vector_world);
        mesh->bounding_sphere_center = min_corner + r_vector_model;
    }
    // Make sure entire model has bounding sphere
    glm::vec3 max_corner = glm::vec3(x_max, y_max, z_max);
    glm::vec3 min_corner = glm::vec3(x_min, y_min, z_min);

    glm::vec3 r_vector_model = 0.5f * (max_corner - min_corner);
    glm::vec3 r_vector_world = glm::vec3(this->scale_matrix * glm::vec4(r_vector_model, 1.0f));
    this->bounding_sphere_radius = glm::length(r_vector_world);
    this->bounding_sphere_center = min_corner + r_vector_model;
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

// -------------
//

unsigned Terrain::cull_me(Camera* camera){
    unsigned drawn_meshes = 0;
    bool draw_me = camera->sphere_in_frustum(this->get_center_point_world(), this->bounding_sphere_radius);

    this->draw_me = draw_me;
    // If draw me - see if we can cull meshes
    if (draw_me){
        for (auto mesh : this->get_meshes()) {
            bool draw_me = camera->sphere_in_frustum(mesh->get_center_point_world(this->m2w_matrix), \
                                                     mesh->bounding_sphere_radius);
            mesh->draw_me = draw_me;
            if (draw_me)
                drawn_meshes++;
        }
    }

    return drawn_meshes;
}


//------------------

void Terrain::load_wind(std::string full_path) {

    Terrain::wind_map = Mesh::load_texture(full_path, false, NORMAL);
}
