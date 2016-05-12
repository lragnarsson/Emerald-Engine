#include "Animation_Path.hpp"
#include "Error.hpp"

Animation_Path::Animation_Path(std::vector<glm::vec3> points, float period_time)
{
    control_points = points;
    period_time = period_time;
    time_per_section = period_time / points.size();
    t = 0.0f;
}


glm::vec3 Animation_Path::get_pos(float elapsed_time)
{
    update_time(elapsed_time);
    float u;
    glm::mat3x4 points;

    points = get_spline_points(t, u);
    return CR_Spline::calc_pos_on_spline(u, points);
}

void Animation_Path::update_time(float elapsed_time)
{
    if (elapsed_time < 0.0f) {
        Error::throw_error(Error::animation_path_time_error);
    } else {
        t += elapsed_time;
    }
    /*while (t > period_time) {
        t = t - period_time;
        }*/
}


glm::mat3x4 Animation_Path::get_spline_points(float t, float &u)
{
    glm::vec3 p1,p2,p3,p4;
    
    if (t < 0.0f) {
        t = 0.0f;
    }
    unsigned int start = uint(t / time_per_section);
    p1 = control_points[start];
    p2 = control_points[get_vector_index_circular(start,1)];
    p3 = control_points[get_vector_index_circular(start,2)];
    p4 = control_points[get_vector_index_circular(start,3)];

    // Parameter between 0 and 1 used for interpolation on spline
    u = fmod(t,time_per_section) / time_per_section;
    return glm::mat3x4(p1.x,p2.x,p3.x,p4.x,
                       p1.y,p2.y,p3.y,p4.y,
                       p1.z,p2.z,p3.z,p4.z);
}
    
unsigned int Animation_Path::get_vector_index_circular(unsigned int start,
                                                       unsigned int offset)
{
    unsigned long size = control_points.size();
    if (start > size) {
        Error::throw_error(Error::animation_path_vector_error);
    }

    unsigned int i = start + offset;
    while (i > size) {
        i -= size;
    }
    return i;
}
