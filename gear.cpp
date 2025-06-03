// src/gear.cpp

#include "gear.hpp"
#include <glm/gtc/constants.hpp>  // glm::pi<float>()
#include <iostream>

Gear::Gear(float outerRadius, float innerRadius, int teethCount_, float rpm_)
    : outerR(outerRadius), innerR(innerRadius), teethCount(teethCount_), rpm(rpm_),
    vao(0), vbo(0), ebo(0), indexCount(0)
{
    buildGeometry();

    // Tworzymy VAO/VBO/EBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Przygotowujemy bufor interleaved: [pos (4 floats)] [normal (4 floats)] [color (4 floats)]
    std::vector<float> interleaved;
    interleaved.reserve(vertices.size() * (4 + 4 + 4));

    for (size_t i = 0; i < vertices.size(); ++i) {
        // Pozycja
        interleaved.push_back(vertices[i].x);
        interleaved.push_back(vertices[i].y);
        interleaved.push_back(vertices[i].z);
        interleaved.push_back(vertices[i].w);
        // Normalna
        interleaved.push_back(normals[i].x);
        interleaved.push_back(normals[i].y);
        interleaved.push_back(normals[i].z);
        interleaved.push_back(normals[i].w);
        // Kolor
        interleaved.push_back(colors[i].x);
        interleaved.push_back(colors[i].y);
        interleaved.push_back(colors[i].z);
        interleaved.push_back(colors[i].w);
    }

    // VBO – interleaved
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        interleaved.size() * sizeof(float),
        interleaved.data(),
        GL_STATIC_DRAW);

    GLsizei stride = (4 + 4 + 4) * sizeof(float);
    // Pozycja: location = 0
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // Normal: location = 2
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Kolor: location = 1
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)((4 + 4) * sizeof(float)));
    glEnableVertexAttribArray(1);

    // EBO – indeksy
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW);

    indexCount = indices.size();

    glBindVertexArray(0);

    // Debug
    std::cout << "[Gear] VAO=" << vao << " VBO=" << vbo
        << " EBO=" << ebo << " indices=" << indexCount << "\n";
}

Gear::~Gear()
{
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Gear::buildGeometry()
{
    vertices.clear();
    normals.clear();
    colors.clear();
    indices.clear();

    // Parametry torusa
    const int radialSegs = 32;
    const float tubeR = (outerR - innerR) * 0.5f;
    const float midR = innerR + tubeR;

    // Generujemy wierzchołki torusa (upraszczając – normalna = (x, y, z) znormalizowane)
    for (int i = 0; i <= radialSegs; ++i) {
        float theta = 2.0f * glm::pi<float>() * float(i) / float(radialSegs);
        float cosT = cos(theta), sinT = sin(theta);
        for (int j = 0; j <= radialSegs; ++j) {
            float phi = 2.0f * glm::pi<float>() * float(j) / float(radialSegs);
            float cosP = cos(phi), sinP = sin(phi);
            float x = (midR + tubeR * cosP) * cosT;
            float y = (midR + tubeR * cosP) * sinT;
            float z = tubeR * sinP;
            vertices.emplace_back(glm::vec4(x, y, z, 1.0f));

            // Normalna: po prostu (x, y, z) znormalizowane
            glm::vec3 n3 = glm::normalize(glm::vec3(x, y, z));
            normals.emplace_back(glm::vec4(n3, 0.0f));

            // Kolor stali
            colors.emplace_back(glm::vec4(0.7f, 0.7f, 0.75f, 1.0f));
        }
    }

    // Indeksy torusa (triangle strip)
    for (int i = 0; i < radialSegs; ++i) {
        for (int j = 0; j < radialSegs; ++j) {
            int first = i * (radialSegs + 1) + j;
            int second = first + radialSegs + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Prosta symulacja zębów – pojedyncze kostki na obwodzie
    for (int t = 0; t < teethCount; ++t) {
        float ang = 2.0f * glm::pi<float>() * float(t) / float(teethCount);
        float toothDepth = tubeR * 0.5f;
        float baseR1 = outerR + 0.0f * toothDepth;
        float baseR2 = outerR + 1.0f * toothDepth;
        // 4 wierzchołki kwadratu na obwodzie:
        glm::vec3 pts[4] = {
            { baseR1 * cos(ang), baseR1 * sin(ang),  0.0f },
            { baseR2 * cos(ang), baseR2 * sin(ang),  0.0f },
            { baseR2 * cos(ang), baseR2 * sin(ang),  tubeR * 0.2f },
            { baseR1 * cos(ang), baseR1 * sin(ang),  tubeR * 0.2f }
        };
        GLuint startIdx = vertices.size();
        for (int k = 0; k < 4; ++k) {
            vertices.emplace_back(glm::vec4(pts[k], 1.0f));
            // Normalna przybliżona jako (cos(ang), sin(ang), 0)
            glm::vec3 n3 = glm::normalize(glm::vec3(cos(ang), sin(ang), 0.0f));
            normals.emplace_back(glm::vec4(n3, 0.0f));
            // Kolor zęba jaśniejszy
            colors.emplace_back(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        }
        // Indeksy: 2 trójkąty na kwadrat
        indices.push_back(startIdx + 0);
        indices.push_back(startIdx + 1);
        indices.push_back(startIdx + 2);
        indices.push_back(startIdx + 2);
        indices.push_back(startIdx + 3);
        indices.push_back(startIdx + 0);
    }

    std::cout << "[Gear::buildGeometry] vertices=" << vertices.size()
        << " normals=" << normals.size()
        << " colors=" << colors.size()
        << " indices=" << indices.size() << "\n";
}

void Gear::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(indexCount),
        GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
