#ifndef ANIMATION_PATH_H
#define ANIMATION_PATH_H



#include <glm/glm.hpp>
#include <vector>


#include "CR_Spline.hpp"

class Animation_Path {
public:
    glm::vec3 get_pos(float elapsed_time);
    
    Animation_Path(std::vector<glm::vec3> points, float period_time);
    ~Animation_Path() { };
private:
    float period_time;
    float time_per_section;
    float t;
    std::vector<glm::vec3> control_points;

    void update_time(float elapsed_time);
    glm::mat3x4 get_spline_points(float t, float &u);
    unsigned int get_vector_index_circular(unsigned int start, unsigned int offset);
};

#endif
