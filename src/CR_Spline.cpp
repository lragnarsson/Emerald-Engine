#include "CR_Spline.hpp"


const glm::mat4 CR_Spline::CR_matrix = glm::transpose(glm::mat4(-0.5f,  1.5f, -1.5f,  0.5f,
                                                                1.0f,  -2.5f,  2.0f, -0.5f,
                                                                -0.5f,  0.0f,  0.5f,  0.0f,
                                                                0.0f,   1.0f,  0.0f,  0.0f));

glm::vec3 CR_Spline::calc_pos_on_spline(float u, glm::vec3  p1, glm::vec3 p2,
                                        glm::vec3 p3, glm::vec3 p4)
{
    glm::vec4 u_vec = glm::vec4(std::pow(u,3), std::pow(u,2), u, 1);
    glm::mat3x4 points = glm::transpose(glm::mat4x3(p1,p2,p3,p4));
    return u_vec * CR_matrix * points;
}

glm::vec3 CR_Spline::calc_pos_on_spline(float u, glm::mat3x4 points)
{
    glm::vec4 u_vec = glm::vec4(std::pow(u,3), std::pow(u,2), u, 1);
    return u_vec * CR_matrix * points;
}
