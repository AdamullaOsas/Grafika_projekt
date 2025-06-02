// src/main_file.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

#include "gear.hpp"
#include "hand.hpp"
#include "shaderprogram.h"

// Ścieżki do shaderów:
static const char* VERTEX_SHADER_PATH = "v_simplest.glsl";
static const char* FRAGMENT_SHADER_PATH = "f_simplest.glsl";

// Globalne zmienne aplikacji
int windowWidth = 800;    // lekko poszerzone
int windowHeight = 1280;
GLFWwindow* window = nullptr;

ShaderProgram* spLambert = nullptr;

// Obiekty zegara
Gear* gearA = nullptr;
Gear* gearB = nullptr;
Hand* secondHand = nullptr;
Hand* minuteHand = nullptr;
Hand* hourHand = nullptr;
Hand* markerHand = nullptr; // znaczniki godzin

// Lokalizacje uniformów w shaderze
GLuint locP, locV, locM, locLP;

bool paused = false;
float elapsedTime = 0.0f;
float prevTime = 0.0f;

// ————————————————————————————————————————————————————————————————————————————————
// Funkcja wywoływana przy zmianie rozmiaru okna
// ————————————————————————————————————————————————————————————————————————————————
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}

// ————————————————————————————————————————————————————————————————————————————————
// Przetwarzanie klawiszy (ESC zamyka, SPACJA pauza, R resetuje)
// ————————————————————————————————————————————————————————————————————————————————
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        paused = !paused;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        elapsedTime = 0.0f;
        prevTime = static_cast<float>(glfwGetTime());
    }
}

// ————————————————————————————————————————————————————————————————————————————————
// Inicjalizacja OpenGL, tworzenie okna, ładowanie shaderów, obiektów
// ————————————————————————————————————————————————————————————————————————————————
void initOpenGLProgram() {
    if (!glfwInit()) {
        std::cerr << "Błąd: nie udało się zainicjalizować GLFW\n";
        std::exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "Zegar mechaniczny", nullptr, nullptr);
    if (!window) {
        std::cerr << "Błąd: nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        std::exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Błąd: nie udało się zainicjalizować GLEW\n";
        glfwTerminate();
        std::exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // Ładowanie shaderów
    spLambert = new ShaderProgram(
        VERTEX_SHADER_PATH,
        nullptr,
        FRAGMENT_SHADER_PATH
    );
    spLambert->use();

    locP = spLambert->u("P");
    locV = spLambert->u("V");
    locM = spLambert->u("M");
    locLP = spLambert->u("lp");

    glUniform4f(locLP, 1.0f, 1.0f, 1.0f, 1.0f);

    // Duża zębatka – outerR=1.3, innerR=1.0
    gearA = new Gear(1.3f, 1.0f, 60, 1.0f);
    // Mała zębatka: outerR=0.2, innerR=0.15
    gearB = new Gear(0.2f, 0.15f, 12, -5.0f);
    // Wskazówki
    secondHand = new Hand(0.9f, 0.015f);
    minuteHand = new Hand(0.7f, 0.015f);
    hourHand = new Hand(0.5f, 0.015f);
    markerHand = new Hand(0.1f, 0.02f);

    prevTime = static_cast<float>(glfwGetTime());
    std::cout << "[Init] GearA=" << gearA << " GearB=" << gearB
        << " 2nd=" << secondHand << " min=" << minuteHand
        << " hr=" << hourHand << " marker=" << markerHand << "\n";
}

// ————————————————————————————————————————————————————————————————————————————————
// Sprzątanie zasobów
// ————————————————————————————————————————————————————————————————————————————————
void cleanup() {
    delete gearA;
    delete gearB;
    delete secondHand;
    delete minuteHand;
    delete hourHand;
    delete markerHand;
    delete spLambert;
    glfwTerminate();
}

// ————————————————————————————————————————————————————————————————————————————————
// Rysowanie całej sceny
// ————————————————————————————————————————————————————————————————————————————————
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    spLambert->use();

    // Projekcja i widok – kamera w frontowym widoku na środek
    glm::mat4 Pm = glm::perspective(
        glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 100.0f
    );
    glm::mat4 Vm = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),   // kamera na osi Z
        glm::vec3(0.0f, 0.0f, 0.0f),   // patrzy na środek
        glm::vec3(0.0f, 1.0f, 0.0f)    // "up" = Y
    );
    glUniformMatrix4fv(locP, 1, GL_FALSE, &Pm[0][0]);
    glUniformMatrix4fv(locV, 1, GL_FALSE, &Vm[0][0]);

    // Obsługa pauzy i elapsedTime
    float currentTime = static_cast<float>(glfwGetTime());
    float dt = currentTime - prevTime;
    prevTime = currentTime;
    if (!paused) {
        elapsedTime += dt;
    }
    float t = elapsedTime;

    // 1) Duża zębatka
    float angleA_deg = fmod(t * (gearA->getRPM() * 360.0f / 60.0f), 360.0f);
    glm::mat4 M_A = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(angleA_deg),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_A[0][0]);
    gearA->draw();

    // 2) Mała zębatka – synchronizacja i pozycja pod kątem 225°
    float rpmB = -gearA->getRPM() * (float)gearA->getTeethCount() / (float)gearB->getTeethCount();
    float angleB_deg = fmod(t * (rpmB * 360.0f / 60.0f), 360.0f);

    float offsetR = gearA->getOuterRadius() + gearB->getOuterRadius();
    float cos225 = std::cos(glm::radians(225.0f));
    float sin225 = std::sin(glm::radians(225.0f));
    glm::vec3 posB = glm::vec3(offsetR * cos225, offsetR * sin225, 0.0f);

    glm::mat4 M_B = glm::translate(glm::mat4(1.0f), posB);
    M_B = glm::rotate(
        M_B,
        glm::radians(angleB_deg),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_B[0][0]);
    gearB->draw();

    // 3) Sekundnik: 6°/s, +90° start
    float angleSec = fmod(t * 6.0f + 90.0f, 360.0f);
    glm::mat4 M_sec = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(angleSec),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_sec[0][0]);
    secondHand->draw();

    // 4) Minutnik: 0.1°/s, +90° start
    float angleMin = fmod(t * 0.1f + 90.0f, 360.0f);
    glm::mat4 M_min = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(angleMin),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_min[0][0]);
    minuteHand->draw();

    // 5) Godzinnik: 0.0083333°/s, +90° start
    float angleHour = fmod(t * 0.0083333f + 90.0f, 360.0f);
    glm::mat4 M_hour = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(angleHour),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_hour[0][0]);
    hourHand->draw();

    // 6) Znaczniki godzin (12 prostokątów)
    float outerRA = gearA->getOuterRadius();
    for (int i = 0; i < 12; ++i) {
        float angDeg = float(i) * 30.0f;
        glm::mat4 M_mk = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(angDeg),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        float r = outerRA + 0.05f;
        M_mk = glm::translate(M_mk, glm::vec3(r, 0.0f, 0.0f));
        M_mk = glm::rotate(
            M_mk,
            glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(locM, 1, GL_FALSE, &M_mk[0][0]);
        markerHand->draw();
    }
}

// ————————————————————————————————————————————————————————————————————————————————
// Główna pętla programu
// ————————————————————————————————————————————————————————————————————————————————
int main() {
    initOpenGLProgram();

    glfwSetTime(0.0);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        drawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}
