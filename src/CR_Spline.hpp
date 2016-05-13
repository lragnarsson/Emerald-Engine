#ifndef CR_SPLINE_H
#define CR_SPLINE_H


#include <glm/glm.hpp>

class CR_Spline {
public:
    static glm::vec3 calc_pos_on_spline(float u, glm::vec3 p1, glm::vec3 p2,
                                        glm::vec3 p3, glm::vec3 p4);
    static glm::vec3 calc_pos_on_spline(float u, glm::mat3x4 points);
private:
    static const glm::mat4 CR_matrix;
};

#endif
