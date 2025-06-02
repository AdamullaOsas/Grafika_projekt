// include/hand.hpp
#ifndef HAND_HPP
#define HAND_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <GL/glew.h>

/**
 * Prosta klasa Hand – sześcian skalowany na dł. i grubość.
 * Geometria zawiera pozycje (vec4), normalne (vec4) i kolory (vec4).
 */
class Hand {
public:
    /**
     * @param length    Długość wskazówki (w jednostkach świata)
     * @param thickness Grubość wskazówki (w jednostkach świata)
     */
    Hand(float length, float thickness);
    ~Hand();

    /// Rysuje wskazówkę (zakłada, że uniform M już ustawiony oraz P, V).
    void draw();

private:
    void buildGeometry();

    float len;
    float thick;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t indexCount;

    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec4> colors;
    std::vector<GLuint>    indices;
};

#endif // HAND_HPP
