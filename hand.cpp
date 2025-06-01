#include "hand.hpp"
#include <vector>

GLuint Hand::vao = 0;

void Hand::buildGeom() {
    if (vao) return;
    glm::vec3 verts[] = {
        {-0.5f, 0.f, 0.f}, {0.5f, 0.f, 0.f}, // prosta linia – placeholder
    };
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);
}

Hand::Hand(float len, float thick) : length(len), thickness(thick), angle(0.f) {
    buildGeom();
}

void Hand::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2);
}