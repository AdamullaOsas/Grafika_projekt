// src/main_file.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "gear.hpp"          // popraw ścieżkę, jeśli nagłówki są gdzie indziej
#include "hand.hpp"          // popraw ścieżkę, jeśli nagłówki są gdzie indziej
#include "shaderprogram.h"   // popraw ścieżkę, jeśli nagłówki są gdzie indziej

// ————————————————————————————————————————————————————————————————————————————————
// ▶ Uwaga: dopasuj wg swojej struktury katalogów:
//    jeśli Twoje shadery są w „pliki zasobów/” lub „shaders/”.
// ————————————————————————————————————————————————————————————————————————————————
static const char* VERTEX_SHADER_PATH = "v_simplest.glsl";   // <- dopasuj ścieżkę
static const char* FRAGMENT_SHADER_PATH = "f_simplest.glsl";   // <- dopasuj ścieżkę

// ————————————————————————————————————————————————————————————————————————————————
// Globalne zmienne aplikacji
// ————————————————————————————————————————————————————————————————————————————————
int windowWidth = 1280;
int windowHeight = 720;
GLFWwindow* window = nullptr;

ShaderProgram* spLambert = nullptr;

// Obiekty zegara
Gear* gearA = nullptr;
Gear* gearB = nullptr;
Hand* secondHand = nullptr;

// Uniform locations
GLuint locP, locV, locM, locLP;

bool paused = false;

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
        // Resetujemy czas
        glfwSetTime(0.0);
    }
}

// ————————————————————————————————————————————————————————————————————————————————
// Inicjalizacja OpenGL, tworzenie okna, ładowanie shaderów, obiektów
// ————————————————————————————————————————————————————————————————————————————————
void initOpenGLProgram() {
    // Inicjalizacja GLFW
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

    // Inicjalizacja GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Błąd: nie udało się zainicjalizować GLEW\n";
        glfwTerminate();
        std::exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // ——————————————————————————————————————————————————————————————————————
    // Ładowanie programu shaderowego (v_simplest + f_simplest)
    // ——————————————————————————————————————————————————————————————————————
    spLambert = new ShaderProgram(
        VERTEX_SHADER_PATH,
        nullptr,
        FRAGMENT_SHADER_PATH
    );
    spLambert->use();

    // Pobranie lokacji uniformów P, V, M, lp
    locP = spLambert->u("P");
    locV = spLambert->u("V");
    locM = spLambert->u("M");
    locLP = spLambert->u("lp");

    // Ustawienie pozycji źródła światła (lp) – raz na cały czas
    // Przykład: światło z przodu i z góry = (1, 1, 1, 1)
    glUniform4f(locLP, 1.0f, 1.0f, 1.0f, 1.0f);

    // Tworzenie obiektów zegara
    gearA = new Gear(1.0f, 0.8f, 60, 1.0f);  // 60 zębów, 1 rpm
    gearB = new Gear(0.2f, 0.15f, 12, -5.0f); // 12 zębów, -5 rpm (synchronizacja)
    secondHand = new Hand(1.2f, 0.02f);            // długość 1.2, grubość 0.02

    std::cout << "[Init] GearA=" << gearA << " GearB=" << gearB
        << " Hand=" << secondHand << "\n";
}

// ————————————————————————————————————————————————————————————————————————————————
// Sprzątanie zasobów
// ————————————————————————————————————————————————————————————————————————————————
void cleanup() {
    delete gearA;
    delete gearB;
    delete secondHand;
    delete spLambert;
    glfwTerminate();
}

// ————————————————————————————————————————————————————————————————————————————————
// Rysowanie całej sceny
// ————————————————————————————————————————————————————————————————————————————————
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    spLambert->use();

    // 1) Ustawiamy macierze P i V
    glm::mat4 Pm = glm::perspective(glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 100.0f);
    glm::mat4 Vm = glm::lookAt(glm::vec3(0.0f, 1.5f, -5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(locP, 1, GL_FALSE, &Pm[0][0]);
    glUniformMatrix4fv(locV, 1, GL_FALSE, &Vm[0][0]);

    // Oblicz pełny czas (w sekundach) od startu
    float t = static_cast<float>(glfwGetTime());

    // Obliczamy kąt dla gearA: rpm (obr./min) -> deg/s = rpm*360/60
    float angleA_deg = fmod(t * (gearA->getRPM() * 360.0f / 60.0f), 360.0f);
    // Macierz modelu dla gearA (obrót w osi Z)
    glm::mat4 M_A = glm::rotate(glm::mat4(1.0f),
        glm::radians(angleA_deg),
        glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_A[0][0]);
    gearA->draw();

    // Synchronizacja gearB: rpm_B = -rpm_A * teethA/teethB
    float rpmB = -gearA->getRPM() * (float)gearA->getTeethCount() / (float)gearB->getTeethCount();
    float angleB_deg = fmod(t * (rpmB * 360.0f / 60.0f), 360.0f);
    // Macierz Modelu dla gearB: najpierw translacja (outerR_A + outerR_B), potem obrót
    float offsetX = gearA->getOuterRadius() + gearB->getOuterRadius();
    glm::mat4 M_B = glm::translate(glm::mat4(1.0f),
        glm::vec3(offsetX, 0.0f, 0.0f));
    M_B = glm::rotate(M_B,
        glm::radians(angleB_deg),
        glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_B[0][0]);
    gearB->draw();

    // Wskaźnik sekundowy: 6°/s
    float angleSec = fmod(t * 6.0f, 360.0f);
    glm::mat4 M_hand = glm::rotate(glm::mat4(1.0f),
        glm::radians(angleSec),
        glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(locM, 1, GL_FALSE, &M_hand[0][0]);
    secondHand->draw();
}

// ————————————————————————————————————————————————————————————————————————————————
// Główna pętla programu
// ————————————————————————————————————————————————————————————————————————————————
int main() {
    initOpenGLProgram();

    // Reset zegara na start
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
