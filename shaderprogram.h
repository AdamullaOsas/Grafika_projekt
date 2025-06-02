// include/shaderprogram.h
#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <GL/glew.h>

class ShaderProgram {
private:
    GLuint shaderProgram;
    GLuint vertexShader;
    GLuint geometryShader;
    GLuint fragmentShader;
    char* readFile(const char* fileName);
    GLuint loadShader(GLenum shaderType, const char* fileName);

public:
    ShaderProgram(const char* vertexShaderFile, const char* geometryShaderFile, const char* fragmentShaderFile);
    ~ShaderProgram();
    void use();
    GLuint u(const char* variableName);
    GLuint a(const char* variableName);
};

#endif // SHADERPROGRAM_H
