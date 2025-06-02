// src/hand.cpp
#include "hand.hpp"
#include <iostream>

Hand::Hand(float length, float thickness)
    : len(length), thick(thickness),
    vao(0), vbo(0), ebo(0), indexCount(0)
{
    buildGeometry();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Interleaved: [pos (4)] [normal (4)] [color (4)]
    std::vector<float> interleaved;
    interleaved.reserve(vertices.size() * (4 + 4 + 4));

    for (size_t i = 0; i < vertices.size(); ++i) {
        interleaved.push_back(vertices[i].x);
        interleaved.push_back(vertices[i].y);
        interleaved.push_back(vertices[i].z);
        interleaved.push_back(vertices[i].w);

        interleaved.push_back(normals[i].x);
        interleaved.push_back(normals[i].y);
        interleaved.push_back(normals[i].z);
        interleaved.push_back(normals[i].w);

        interleaved.push_back(colors[i].x);
        interleaved.push_back(colors[i].y);
        interleaved.push_back(colors[i].z);
        interleaved.push_back(colors[i].w);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        interleaved.size() * sizeof(float),
        interleaved.data(),
        GL_STATIC_DRAW);

    // Atrybuty według layout(location) w shaderze:
    // location=0 → vertex
    // location=1 → color
    // location=2 → normal
    GLsizei stride = (4 + 4 + 4) * sizeof(float);
    // vertex (0)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // normal (2)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // color (1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)((4 + 4) * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW);

    indexCount = indices.size();
    glBindVertexArray(0);

    std::cout << "[Hand] VAO=" << vao << " VBO=" << vbo
        << " EBO=" << ebo << " indices=" << indexCount << "\n";
}

Hand::~Hand()
{
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Hand::buildGeometry()
{
    vertices.clear();
    normals.clear();
    colors.clear();
    indices.clear();

    // Cztery wierzchołki płaskiego prostokąta w płaszczyźnie XY:
    // (−thick/2, 0, 0), ( +thick/2, 0, 0 ), ( +thick/2, len, 0 ), ( −thick/2, len, 0 )
    float halfTh = thick * 0.5f;
    vertices.emplace_back(glm::vec4(-halfTh, 0.0f, 0.0f, 1.0f));
    vertices.emplace_back(glm::vec4(halfTh, 0.0f, 0.0f, 1.0f));
    vertices.emplace_back(glm::vec4(halfTh, len, 0.0f, 1.0f));
    vertices.emplace_back(glm::vec4(-halfTh, len, 0.0f, 1.0f));

    // Wszystkie normalne skierowane w +Z (bo płaszczyzna XY)
    for (int i = 0; i < 4; ++i) {
        normals.emplace_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        // Przybliżony kolor wskazówki (ciemny szary)
        colors.emplace_back(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    // Indeksy – dwa trójkąty składające się na prostokąt
    indices = { 0, 1, 2,   2, 3, 0 };

    std::cout << "[Hand::buildGeometry] vertices=" << vertices.size()
        << " normals=" << normals.size()
        << " colors=" << colors.size()
        << " indices=" << indices.size() << "\n";
}

void Hand::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(indexCount),
        GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
