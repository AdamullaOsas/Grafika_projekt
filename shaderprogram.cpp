// src/shaderprogram.cpp
#define _CRT_SECURE_NO_WARNINGS  // aby uniknąć ostrzeżeń przy fopen

#include "shaderprogram.h"
#include <vector>
#include <iostream>

// Procedura wczytuje plik do tablicy znaków.
char* ShaderProgram::readFile(const char* fileName) {
    FILE* plik;
    long filesize;
    char* result;

    // Użycie fopen_s zamiast fopen, aby uniknąć ostrzeżenia
#ifdef _WIN32
    errno_t err = fopen_s(&plik, fileName, "rb");
    if (err != 0 || plik == nullptr) {
        return nullptr;
    }
#else
    plik = fopen(fileName, "rb");
    if (plik == nullptr) {
        return nullptr;
    }
#endif

    fseek(plik, 0, SEEK_END);
    filesize = ftell(plik);
    fseek(plik, 0, SEEK_SET);

    result = new char[filesize + 1];
    size_t readsize = fread(result, 1, filesize, plik);
    result[filesize] = '\0';
    fclose(plik);

    return result;
}

// Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
GLuint ShaderProgram::loadShader(GLenum shaderType, const char* fileName) {
    // Generujemy uchwyt na shader
    GLuint shader = glCreateShader(shaderType);

    // Wczytaj plik ze źródłem shadera do tablicy znaków
    const GLchar* shaderSource = readFile(fileName);
    if (!shaderSource) {
        std::cerr << "[ShaderProgram] Nie mogę wczytać pliku: " << fileName << "\n";
        return 0;
    }

    // Powiąż źródło z uchwytem shadera
    glShaderSource(shader, 1, &shaderSource, nullptr);
    // Skompiluj źródło
    glCompileShader(shader);
    delete[] shaderSource;

    // Sprawdź status kompilacji
    GLint compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "[ShaderProgram] Błąd kompilacji shadera (" << fileName << "):\n"
            << log.data() << "\n";
    }
    return shader;
}

ShaderProgram::ShaderProgram(const char* vertexShaderFile, const char* geometryShaderFile, const char* fragmentShaderFile) {
    // Wczytaj vertex shader
    std::cout << "[ShaderProgram] Loading vertex shader: " << vertexShaderFile << "\n";
    vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderFile);

    // Wczytaj geometry shader, jeśli podano
    if (geometryShaderFile != nullptr) {
        std::cout << "[ShaderProgram] Loading geometry shader: " << geometryShaderFile << "\n";
        geometryShader = loadShader(GL_GEOMETRY_SHADER, geometryShaderFile);
    }
    else {
        geometryShader = 0;
    }

    // Wczytaj fragment shader
    std::cout << "[ShaderProgram] Loading fragment shader: " << fragmentShaderFile << "\n";
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderFile);

    // Wygeneruj uchwyt programu cieniującego
    shaderProgram = glCreateProgram();

    // Podłącz shadery i zlinkuj
    if (vertexShader) glAttachShader(shaderProgram, vertexShader);
    if (fragmentShader) glAttachShader(shaderProgram, fragmentShader);
    if (geometryShader) glAttachShader(shaderProgram, geometryShader);

    // Optional: jawne bindowanie lokacji
    glBindAttribLocation(shaderProgram, 0, "vertex");
    glBindAttribLocation(shaderProgram, 1, "color");
    glBindAttribLocation(shaderProgram, 2, "normal");

    glLinkProgram(shaderProgram);

    // Sprawdź status linkowania
    GLint linkStatus = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        GLint logLen = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(shaderProgram, logLen, nullptr, log.data());
        std::cerr << "[ShaderProgram] Błąd linkowania programu:\n"
            << log.data() << "\n";
    }
    else {
        std::cout << "[ShaderProgram] Shader program created: " << shaderProgram << "\n";
    }
}

ShaderProgram::~ShaderProgram() {
    if (vertexShader) { glDetachShader(shaderProgram, vertexShader);   glDeleteShader(vertexShader); }
    if (geometryShader) { glDetachShader(shaderProgram, geometryShader); glDeleteShader(geometryShader); }
    if (fragmentShader) { glDetachShader(shaderProgram, fragmentShader); glDeleteShader(fragmentShader); }
    if (shaderProgram)  glDeleteProgram(shaderProgram);
}

void ShaderProgram::use() {
    glUseProgram(shaderProgram);
}

GLuint ShaderProgram::u(const char* variableName) {
    return glGetUniformLocation(shaderProgram, variableName);
}

GLuint ShaderProgram::a(const char* variableName) {
    return glGetAttribLocation(shaderProgram, variableName);
}
