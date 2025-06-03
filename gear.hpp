// include/gear.hpp

#ifndef GEAR_HPP
#define GEAR_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <GL/glew.h>

/**
 * Prosta klasa Gear generująca low-poly koło zębate.
 * Geometria zawiera pozycje (vec4), normalne (vec4) i kolory (vec4).
 * Rysowanie odbywa się przez wywołanie draw(), pod warunkiem że przedtem
 * w głównym kodzie ustawiono uniformy P, V, M, lp w shaderze.
 */
class Gear {
public:
    /**
     * @param outerRadius Promień zewnętrzny (do czubka zębów)
     * @param innerRadius Promień wewnętrzny (przy podstawie zębów)
     * @param teethCount  Liczba zębów
     * @param rpm         Obroty na minutę (tylko przechowujemy do synchronizacji)
     */
    Gear(float outerRadius, float innerRadius, int teethCount, float rpm);
    ~Gear();

    /// Rysuje koło zębate (zakłada, że uniform M jest już ustawiony).
    void draw();

    /// Dostęp do liczby zębów (w synchronizacji koła B względem A).
    int getTeethCount() const { return teethCount; }

    /// Dostęp do promienia zewnętrznego (używany przy translacji koła B).
    float getOuterRadius() const { return outerR; }

    /// Dostęp do promienia wewnętrznego (używany przy znacznikach godzin).
    float getInnerRadius() const { return innerR; }

    /// Zwrot wartości rpm (tylko gdy chcemy odczytać prędkość).
    float getRPM() const { return rpm; }

private:
    void buildGeometry();

    float outerR;
    float innerR;
    int   teethCount;
    float rpm;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t indexCount;

    // Bufory geometrii
    std::vector<glm::vec4> vertices; // (x, y, z, 1)
    std::vector<glm::vec4> normals;  // (nx, ny, nz, 0)
    std::vector <glm::vec4> colors;   // (r, g, b, a)
    std::vector<GLuint>    indices;
};

#endif // GEAR_HPP
