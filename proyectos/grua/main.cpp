// Proyecto: Grua (OpenGL 3.3)
// José Pazos Pérez

#include "tofu.h"
using namespace tofu;

#include "grua.h"
#include "camara.h"

#include <map>

// Ajustes
constexpr ui32 WIDTH = 800;
constexpr ui32 HEIGHT = 800;
const std::vector<ui32> atributos = { 3 /* Pos */ };

// ---

struct Modelo {
    glm::mat4 trans;
    glm::mat4 rot;
    glm::mat4 model;
};
inline std::vector<Modelo> modelos_grua;

// Buffers de instancias
TexBuffer buf_modelo;
TexBuffer buf_color;

// ---

glm::vec3 vecFromArray(v3 v) {
    return glm::vec3(v[0], v[1], v[2]);
}

glm::mat4 modeloObjeto(ui32 id) {
    glm::mat4 modelo = glm::mat4(1.f);

    PiezaGrua& obj = piezas_grua[id];
    Modelo& m_obj = modelos_grua[id];
    PiezaGrua* padre = obj.padre < 0 ? nullptr : &piezas_grua[obj.padre];
    Modelo* m_padre = obj.padre < 0 ? nullptr : &modelos_grua[obj.padre];

    std::vector<glm::mat4> modelos;
    while (padre != nullptr) {
        modelos.push_back(m_padre->trans * m_padre->rot);
        m_padre = padre->padre < 0 ? nullptr : &modelos_grua[padre->padre];
        padre = padre->padre < 0 ? nullptr : &piezas_grua[padre->padre];
    }
    for (auto it = modelos.rbegin(); it != modelos.rend(); it++)
        modelo *= *it;

    m_obj.trans = glm::translate(glm::mat4(1.f), vecFromArray(posRelativa(id)));
    m_obj.rot = glm::rotate(glm::mat4(1.f), obj.angulo, glm::vec3(modelo[1].x, modelo[1].y, modelo[1].z));
    glm::mat4 esc = glm::scale(glm::mat4(1.f), vecFromArray(obj.escala));
    modelo *= m_obj.trans * m_obj.rot * esc;

    return modelo;
}

void actualizarModelosObjetos() {
    std::vector<glm::mat4> modelos;
    for (ui32 id = 0; id < piezas_grua.size(); id++) {
        modelos.push_back(modeloObjeto(id));
    }

    buffer::cargar(buf_modelo.b, modelos, 0);
    
    shader::usar("grua");
    shader::uniform("modelos", buf_modelo);
}

void actualizarColoresObjetos() {
    std::vector<glm::vec4> colores;
    std::transform(piezas_grua.begin(), piezas_grua.end(), std::back_inserter(colores), [](PiezaGrua &p) { return glm::vec4(vecFromArray(p.color), 0.f); });

    buffer::cargar(buf_color.b, colores, 0);

    shader::usar("grua");
    shader::uniform("colores", buf_color);
}

// ---

void inputGrua() {
    // Mover base
    controles.delante = gl.io.teclas[GLFW_KEY_UP].mantenida;
    controles.detras = gl.io.teclas[GLFW_KEY_DOWN].mantenida;
    controles.girar_der = gl.io.teclas[GLFW_KEY_LEFT].mantenida;
    controles.girar_izq = gl.io.teclas[GLFW_KEY_RIGHT].mantenida;

    // Mover torre
    controles.torre_der = gl.io.teclas[GLFW_KEY_H].mantenida;
    controles.torre_izq = gl.io.teclas[GLFW_KEY_J].mantenida;
    controles.torre_arriba = gl.io.teclas[GLFW_KEY_U].mantenida;
    controles.torre_abajo = gl.io.teclas[GLFW_KEY_Y].mantenida;

    // Mover brazo
    controles.brazo_extender = gl.io.teclas[GLFW_KEY_K].mantenida;
    controles.brazo_contraer = gl.io.teclas[GLFW_KEY_L].mantenida;

    // Mover cable
    controles.cable_recoger = gl.io.teclas[GLFW_KEY_I].mantenida;
    controles.cable_soltar = gl.io.teclas[GLFW_KEY_O].mantenida;
}

// ---

void render() {
    // Variables
    camara();
    inputGrua();
    controlarGrua();
    actualizarModelosObjetos();

    // Shader y uniforms
    shader::usar("grua");
    shader::uniform("viewproj", gl.proj * gl.view);
    
    // Dibujar objetos por instancias
    dibujar(piezas_grua.size(), "cubo");
}

// ---

int main() {
    // Iniciamos GLFW y OpenGL
    initGL(WIDTH, HEIGHT, "Grua - OpenGL 3.3");

    // Cargamos la shader a utilizar
    shader::cargar("grua");

    // Creamos los buffers principales
    buffer::iniciarVAO(atributos);
    buf_modelo = texbuffer::crear<glm::mat4>();
    buf_color = texbuffer::crear<glm::vec4>();

    // Cargar en memoria las figuras a dibujar
    buffer::cargarVert("cubo", geometria::cubo());

    // Generador de números aleatorios
    std::srand(std::time(nullptr));

    // Crear objetos
    modelos_grua.resize(piezas_grua.size());
    actualizarModelosObjetos();
    actualizarColoresObjetos();

	// Actualización cada frame
	while ( update(render) ) {};

    terminarGL();
	return 0;
}
