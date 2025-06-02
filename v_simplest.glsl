// pliki zasobow/v_simplest.glsl
#version 330 core

layout(location = 0) in vec4 vertex;   // pozycja wierzchołka (x,y,z,1)
layout(location = 1) in vec4 color;    // kolor wierzchołka (r,g,b,a)
layout(location = 2) in vec4 normal;   // normalna wierzchołka (nx,ny,nz,0)

uniform mat4 P;    // macierz projekcji
uniform mat4 V;    // macierz widoku
uniform mat4 M;    // macierz modelu
uniform vec4 lp;   // pozycja źródła światła w przestrzeni świata

out vec4 iC;       // kolor przekazany do fragment shadera
out vec4 l;        // wektor do światła (w przestrzeni oka)
out vec4 n;        // normalna (w przestrzeni oka)
out vec4 v;        // wektor do obserwatora (w przestrzeni oka)

void main(void) {
    // Wektor do światła: (lp - M*vertex) przekształcone macierzą V
    l = normalize(V * (lp - M * vertex));
    // Normalna przekształcona podobnie
    n = normalize(V * M * normal);
    // Wektor do obserwatora (kamery w (0,0,0) w przestrzeni oka)
    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex);
    iC = color;
    gl_Position = P * V * M * vertex;
}
