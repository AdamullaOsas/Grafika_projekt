// pliki zasobow/f_simplest.glsl
#version 330 core

out vec4 pixelColor;  // wyjściowy kolor fragmentu

in vec4 iC;           // kolor z wierzchołka
in vec4 l;            // wektor do światła
in vec4 n;            // normalna
in vec4 v;            // wektor do obserwatora

void main(void) {
    vec4 ml = normalize(l);
    vec4 mn = normalize(n);
    vec4 mv = normalize(v);
    vec4 mr = reflect(-ml, mn); // wektor odbity

    float nl = clamp(dot(mn, ml), 0.0, 1.0);                 // Lambert
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 25);        // Phong specular

    // Diffuse (iC.rgb * nl) + specular (rv)
    pixelColor = vec4(nl * iC.rgb, iC.a) + vec4(rv, rv, rv, 0.0);
}
