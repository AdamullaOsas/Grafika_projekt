// src/hand.cpp
#include "hand.hpp"
#include <glm/gtc/type_ptr.hpp>
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

    // 8 wierzchołków sześcianu (płaszczyzna XY, grubość w Z = 0.2)
    float halfThick = thick * 0.5f;
    glm::vec3 base[8] = {
        {0.0f,       -halfThick, 0.0f},
        {len,        -halfThick, 0.0f},
        {len,         halfThick, 0.0f},
        {0.0f,        halfThick, 0.0f},
        {0.0f,       -halfThick, 0.2f},
        {len,        -halfThick, 0.2f},
        {len,         halfThick, 0.2f},
        {0.0f,        halfThick, 0.2f}
    };

    for (int i = 0; i < 8; ++i) {
        vertices.emplace_back(glm::vec4(base[i], 1.0f));
        // Przybliżona normalna dla wszystkich wierzchołków: (0,0,1)
        normals.emplace_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        // Kolor czerwony
        colors.emplace_back(glm::vec4(0.8f, 0.0f, 0.0f, 1.0f));
    }

    // Indeksy dla 12 trójkątów (6 ścian sześcianu)
    indices = {
        0,1,2,   2,3,0,   // front face (z=0)
        4,5,6,   6,7,4,   // back face (z=0.2)
        0,1,5,   5,4,0,   // bottom
        2,3,7,   7,6,2,   // top
        1,2,6,   6,5,1,   // right
        3,0,4,   4,7,3    // left
    };

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
