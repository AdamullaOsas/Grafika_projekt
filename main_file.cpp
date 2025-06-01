// main_file.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// -----------------------------------------------------------------------------
// ▶ Uwaga: w razie innej struktury folderów, popraw poniższe ścieżki:
//    - „gear.hpp” oraz „hand.hpp” powinny być w katalogu z nagłówkami.
//    - „shaderprogram.h” w katalogu z plikami nagłówkowymi (lub tam, gdzie go trzymasz).
// -----------------------------------------------------------------------------
#include "gear.hpp"            // <- np. #include "pliki naglowkowe/gear.hpp"
#include "hand.hpp"            // <- np. #include "pliki naglowkowe/hand.hpp"
#include "shaderprogram.h"     // <- np. #include "pliki naglowkowe/shaderprogram.h"

// ————————————————————————————————————————————————————————————————————————————————
// ▶ Uwaga: ścieżki do plików *.glsl musisz dopasować według swojego układu katalogów.
//    Poniżej zakładam, że shader’y leżą w „pliki zasobów/”.
//    Jeśli są w „shaders/” lub innym katalogu, zmień ścieżki poniżej.
// ————————————————————————————————————————————————————————————————————————————————
static const char* VERTEX_SHADER_PATH = "pliki zasobow/v_lambert.glsl";   // <- dopasuj ścieżkę
static const char* FRAGMENT_SHADER_PATH = "pliki zasobow/f_lambert.glsl";   // <- dopasuj ścieżkę

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

// Macierze kamery/projekcji
glm::mat4 P, V;

// Sterowanie czasem
bool paused = false;
float lastTime = 0.0f;

// ————————————————————————————————————————————————————————————————————————————————
// Funkcja wywoływana przy zmianie rozmiaru okna
// ————————————————————————————————————————————————————————————————————————————————
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
    P = glm::perspective(glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 100.0f);
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
        // Resetujemy obiekty: usuwamy i tworzymy na nowo
        delete gearA; delete gearB; delete secondHand;
        gearA = new Gear(1.0f, 0.5f, 60, 1.0f);
        gearB = new Gear(0.2f, 0.1f, 12, -5.0f);
        secondHand = new Hand(1.0f, 0.02f); // <- zmieniono: tylko 2 argumenty
        lastTime = static_cast<float>(glfwGetTime());
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

    // Ustawienia ogólne
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // ——————————————————————————————————————————————————————————————————————
    // Ładowanie programu shaderowego (prosty Lambert)
    // ——————————————————————————————————————————————————————————————————————
    spLambert = new ShaderProgram(
        VERTEX_SHADER_PATH,    // <- upewnij się, że ścieżka jest poprawna
        nullptr,               // brak geometry shadera
        FRAGMENT_SHADER_PATH   // <- upewnij się, że ścieżka jest poprawna
    );
    spLambert->use();

    // Pobranie lokacji uniformów P i V
    GLuint locP = spLambert->u("P");
    GLuint locV = spLambert->u("V");

    // Ustawienie początkowych macierzy projekcji i widoku
    P = glm::perspective(glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 100.0f);
    V = glm::lookAt(glm::vec3(0.0f, 3.0f, 5.0f),  // pozycja kamery
        glm::vec3(0.0f, 0.0f, 0.0f),  // punkt, w który patrzymy
        glm::vec3(0.0f, 1.0f, 0.0f));// wektor "up"
    glUniformMatrix4fv(locP, 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(locV, 1, GL_FALSE, &V[0][0]);

    // ——————————————————————————————————————————————————————————————————————
    // Tworzenie obiektów zegara (koła i wskazówki)
    // ——————————————————————————————————————————————————————————————————————
    gearA = new Gear(1.0f, 0.5f, 60, 1.0f);
    gearB = new Gear(0.2f, 0.1f, 12, -5.0f);
    secondHand = new Hand(1.0f, 0.02f); // <- tylko długość i grubość

    lastTime = static_cast<float>(glfwGetTime());
}

// ————————————————————————————————————————————————————————————————————————————————
// Czyszczenie zasobów przed zamknięciem
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
    GLuint locP = spLambert->u("P");
    GLuint locV = spLambert->u("V");
    GLuint locM = spLambert->u("M");

    // Odświeżamy macierze P i V (na wypadek resize’u)
    glUniformMatrix4fv(locP, 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(locV, 1, GL_FALSE, &V[0][0]);

    // ———————————————————————————————————————————————————————————————————
    // Rysujemy koło A (w punkcie (0,0,0))
    // ———————————————————————————————————————————————————————————————————
    gearA->draw();

    // ———————————————————————————————————————————————————————————————————
    // Rysujemy koło B przesunięte o (outerR_A + outerR_B) w prawo:
    // Jeśli outerRadius gearA=1.0f, gearB=0.2f, to wektor = (1.0f + 0.2f, 0, 0).
    // Jeśli używasz innych wartości, zmień poniższy wektor translacji.
    // ———————————————————————————————————————————————————————————————————
    glm::mat4 translateB = glm::translate(glm::mat4(1.0f),
        glm::vec3(1.0f + 0.2f, 0.0f, 0.0f)); // <- dostosuj, jeśli promienie się różnią
    glUniformMatrix4fv(locM, 1, GL_FALSE, &translateB[0][0]);
    gearB->draw();

    // ———————————————————————————————————————————————————————————————————
    // Rysowanie wskazówki sekundowej: obrót zależny od czasu rzeczywistego
    // ———————————————————————————————————————————————————————————————————
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (!paused) {
        // Kąty kół aktualizują się wewnątrz metody draw(), więc nie wołamy żadnej update()
    }

    // Obliczamy macierz obrotu dla sekundnika (6°/s)
    float angleSec = fmod(currentTime * 6.0f, 360.0f);
    glm::mat4 rotateHand = glm::rotate(glm::mat4(1.0f),
        glm::radians(angleSec),
        glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(locM, 1, GL_FALSE, &rotateHand[0][0]);

    secondHand->draw(); // <- usunięty argument kąta, bo Hand::draw() nie przyjmuje parametrów
}

// ————————————————————————————————————————————————————————————————————————————————
// Główna pętla programu
// ————————————————————————————————————————————————————————————————————————————————
int main() {
    initOpenGLProgram();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        drawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}
