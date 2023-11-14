// Datos de planetas
#pragma once

#include "tofu.h"
using namespace tofu;

// ---

struct Material {
    str albedo;
};

inline std::map<str, Material> materiales = {
    { "PlanetaGas", { "texturas/planeta_gas.png" } },
    { "PlanetaRocoso", { "texturas/planeta_rocoso.png" } },
    { "Tierra", { "texturas/tierra.png" } },
    { "Luna", { "texturas/luna.png" } },
    { "Asteroide", { "texturas/asteroide.png" } },
    { "Sol", { "texturas/sol.png" } },
};

// Planetas
// Estructura con todos los datos necesarios para dibujar un planeta
// Notamos que no incluye las matrices de modelo, y es porque estas se calculan y almacenan directamente en buffers de la GPU
// Al renderizar por instancias, es mucho más eficiente cargar un buffer con todos los valores necesarios en la shader e indexar con gl_InstanceID
struct Planeta {
    float radio;
    float distancia;
    glm::vec3 color;
    str orbita;
    float excentricidad;
    str mat;
};

inline std::map<str, Planeta> planetas = {
    { "Sol",        {  3.f,  0.f, {1.0f, 0.9f, 0.8f},         "", 0.0f , "Sol" } },
    { "Mercurio",   { 0.5f, 10.f, {0.5f, 0.5f, 0.5f},         "", 0.3f , "PlanetaRocoso" } },
    { "Venus",      { 0.8f, 13.f, {1.0f, 0.4f, 0.0f},         "", 0.3f , "PlanetaRocoso" } },
    { "Tierra",     { 1.0f, 17.f, {0.2f, 0.4f, 1.0f},         "", 0.3f , "Tierra" } },
    { "Luna",       { 0.2f,  2.f, {0.6f, 0.6f, 0.6f},   "Tierra", 0.0f , "Luna" } },
    { "Marte",      { 0.7f, 20.f, {1.0f, 0.0f, 0.4f},         "", 0.3f , "PlanetaRocoso" } },
    { "Deimos",     {0.15f,  2.f, {0.2f, 0.3f, 0.4f},    "Marte", 0.0f , "Asteroide" } },
    { "Fobos",      { 0.1f, 2.5f, {0.4f, 0.3f, 0.2f},    "Marte", 0.0f , "Asteroide" } },
    { "Júpiter",    { 1.5f, 40.f, {1.0f, 0.6f, 0.3f},         "", 0.3f , "PlanetaGas" } },
    { "Io",         {0.15f, 2.5f, {1.0f, 0.8f, 0.7f},  "Júpiter", 0.0f , "Asteroide" } },
    { "Europa",     {0.12f,  3.f, {1.0f, 0.5f, 0.7f},  "Júpiter", 0.0f , "Asteroide" } },
    { "Ganímedes",  { 0.3f,  4.f, {0.8f, 0.8f, 0.9f},  "Júpiter", 0.0f , "Asteroide" } },
    { "Calisto",    { 0.2f, 4.5f, {0.8f, 0.7f, 1.0f},  "Júpiter", 0.0f , "Asteroide" } },
    { "Saturno",    { 1.3f, 45.f, {1.0f, 0.8f, 0.5f},         "", 0.3f , "PlanetaGas" } },
    { "Urano",      { 1.1f, 55.f, {0.4f, 0.9f, 1.0f},         "", 0.3f , "PlanetaGas" } },
    { "Neptuno",    { 1.0f, 60.f, {0.3f, 0.5f, 1.0f},         "", 0.3f , "PlanetaGas" } },
    { "Plutón",     { 0.3f, 70.f, {0.3f, 0.3f, 0.3f},         "", 0.3f , "PlanetaRocoso" } },
};
const ui32 num_planetas = planetas.size();
const ui32 num_asteroides = 1000;
const ui32 num_estrellas = 10000;

// Buffers de instancias
// Representación de los buffers de la GPU que almacenan los datos de las instancias
// Como estamos utilizando OpenGL 3.3 no tenemos acceso a Storage Buffers, la herramienta actual para trabajar con buffers grandes en shaders
// Utilizar Uniform Buffers no es una buena alternativa, ya que suelen estar muy limitados por la implementación
// En las pruebas que realizamos, utilizando uniforms se alcanza el límite con tan poco como 1000 instancias
// Sin embargo, podemos usar Texture Buffers (core desde 3.1) para almacenar datos (https://www.khronos.org/opengl/wiki/Buffer_Texture)
// Son similares a utilizar una textura 1D, pero con un límite generalmente mucho mayor
// Aunque se llamen "texture" no tienen por qué representar una imágen, en este caso los usamos para almacenar matrices o vectores, indicando el formato apropiado
// Al utilizar estos el número de instancias puede ser mucho mayor, de más de 100k, y el límite aparece en la capacidad de la GPU para renderizarlas, no en la cantidad que podemos pasar
// Una nota importante es que en la shader utilizaremos texelFetch para acceder a los componentes de los mismos
// También juntaremos varias variables en un mismo TexBuffer cuando sea apropiado para no ocupar demasiados índices de textura y que los datos estén bien comprimidos (vec4)
inline TexBuffer buf_planetas;
inline TexBuffer buf_modelos;
inline TexBuffer buf_color;
inline TexBuffer buf_estrellas;

// Tiempo de simulación (distinto a debug::time() ya que puede cambiar la velocidad)
inline float tiempo = 0.0;
inline float velocidad = 1.0;

// Habilitar / deshabilitar culling
inline bool culling = true;

// Query para contar las instancias renderizadas con transform feedback
inline ui32 tf_query, cull_planetas, cull_asteroides, cull_estrellas;

// ---

// Calcular modelos
inline int transformFeedback(int base, int num, Buffer &b) {
    // Transform feedback
    glEnable(GL_RASTERIZER_DISCARD);
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, b.buffer, base * b.bytes, num * b.bytes);
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tf_query);
    glBeginTransformFeedback(GL_POINTS);

    // "Dibujar" los modelos en el buffer
    shader::uniform("baseins", base);
    glDrawArrays(GL_POINTS, 0, num);
   
    // Acabar transform feedback
    glEndTransformFeedback();
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glDisable(GL_RASTERIZER_DISCARD);

    // Obtener el número de modelos que se han dibujado
    ui32 num_modelos;
    glGetQueryObjectuiv(tf_query, GL_QUERY_RESULT, &num_modelos);

    debug::gl();
    return num_modelos;
}

// Desactivar culling (activa todas las estrellas)
inline void desactivarCulling() {
    culling = false;

    shader::usar("calc_estrellas");
    shader::uniform("culling", 0);
    cull_estrellas = transformFeedback(2*num_planetas + num_asteroides, num_estrellas, gl.buffers[buf_modelos.b]);
}
