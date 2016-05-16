#ifndef ANIMATION_PATH_H
#define ANIMATION_PATH_H



#include <glm/glm.hpp>
#include <vector>


#include "CR_Spline.hpp"

class Animation_Path {
public:
    glm::vec3 get_pos(float &spline_time, float elapsed_time);

    Animation_Path(std::vector<glm::vec3> points, float period);
    ~Animation_Path() { };
    static int get_number_of_animation_paths() {return animation_paths.size();}
    static Animation_Path* get_animation_path_with_id(unsigned id);
private:
    float period_time;
    float time_per_section;
    std::vector<glm::vec3> control_points;

    void update_time(float &spline_time, float elapsed_time);
    glm::mat3x4 get_spline_points(float t, float &u);
    unsigned int get_vector_index_circular(unsigned int start, unsigned int offset);

    static std::vector<Animation_Path*> animation_paths;
};

#endif
