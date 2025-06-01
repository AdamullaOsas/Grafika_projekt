#ifndef GEAR_HPP
#define GEAR_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <GL/glew.h>

/**
 * Proste niskopoziomowe koło zębate – torus + zęby z kostek.
 */
class Gear {
public:
    Gear(float outerRadius, float innerRadius, int teethCount, float rpm);
    ~Gear();
    void draw();          // Rysuje koło z aktualnym obrotem
    void update(float deltaTime); // Aktualizuje kąt obrotu

private:
    void generateGeometry();
    void setupBuffers();

    float outerRadius;
    float innerRadius;
    int teethCount;
    float rpm;
    float angleDeg;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;
    GLuint vao;
    GLuint vbo;
    GLuint nbo; // normal buffer
    GLuint ebo;
};

#endif // GEAR_HPP