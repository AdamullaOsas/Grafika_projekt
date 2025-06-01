#ifndef HAND_HPP
#define HAND_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>

class Hand {
public:
    Hand(float length, float thickness);
    void draw();
    void setAngle(float deg) { angle = deg; }
private:
    float length, thickness;
    float angle{};  // stopnie

    static GLuint vao;
    static void   buildGeom();
};

#endif