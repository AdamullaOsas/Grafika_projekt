#include "gear.hpp"
#include <glm/gtc/constants.hpp>

Gear::Gear(float outerRadius_, float innerRadius_, int teethCount_, float rpm_)
    : outerRadius(outerRadius_), innerRadius(innerRadius_), teethCount(teethCount_), rpm(rpm_), angleDeg(0.0f), vao(0), vbo(0), nbo(0), ebo(0) {
    generateGeometry();
    setupBuffers();
}

Gear::~Gear() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (nbo) glDeleteBuffers(1, &nbo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Gear::generateGeometry() {
    // Generowanie uproszczonej geometrii: walec z zębami jako proste wycięcia
    const int segments = teethCount * 2;
    float depth = (outerRadius - innerRadius) * 0.5f;
    float midRadius = (outerRadius + innerRadius) * 0.5f;

    // Wierzchołki walca
    for (int i = 0; i <= segments; ++i) {
        float theta = glm::two_pi<float>() * float(i) / float(segments);
        float cosT = cos(theta);
        float sinT = sin(theta);
        // Górny pierścień
        vertices.emplace_back(midRadius * cosT, midRadius * sinT, depth);
        normals.emplace_back(cosT, sinT, 0.0f);
        // Dolny pierścień
        vertices.emplace_back(midRadius * cosT, midRadius * sinT, -depth);
        normals.emplace_back(cosT, sinT, 0.0f);
    }
    // Indeksy dla bocznej powierzchni walca
    for (int i = 0; i < segments; ++i) {
        int idx = i * 2;
        // Trójkąty
        indices.push_back(idx);
        indices.push_back(idx + 2);
        indices.push_back(idx + 1);

        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
        indices.push_back(idx + 3);
    }

    // TODO: można dodać geometrię zębów korzystając z kosztnego podejścia (omijamy dla prostoty)
}

void Gear::setupBuffers() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &nbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Wierzchołki
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Normale
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Indeksy
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Gear::update(float deltaTime) {
    // deltaTime w sekundach, rpm => obroty na minutę => stopnie na sekundę
    float degPerSec = rpm * 360.0f / 60.0f;
    angleDeg += degPerSec * deltaTime;
    if (angleDeg >= 360.0f) angleDeg -= 360.0f;
}

void Gear::draw() {
    // Obliczenie macierzy modelu
    glm::mat4 M = glm::rotate(glm::mat4(1.0f), glm::radians(angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    GLuint locM = glGetUniformLocation(0, "M"); // Zakładamy, że shader jest już użyty i ma uniform M
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M[0][0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}