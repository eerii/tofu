// Proyecto: Grua (OpenGL 3.3)
// José Pazos Pérez

#include "tofu.h"
using namespace tofu;

#include "camara.h"

#include <map>

// Ajustes
constexpr ui32 WIDTH = 800;
constexpr ui32 HEIGHT = 800;
const std::vector<ui32> atributos = { 3 /* Pos */ };

// ---

// Posiciones relativas de los objetos entre si
enum PosicionamientoRelativo {
    POS_CENTRO,
    POS_ENCIMA,
    POS_DEBAJO,
    POS_IZQUIERDA,
    POS_DERECHA,
    POS_DELANTE,
    POS_DETRAS,
};

// Objetos a dibujar
struct Objeto {
    int id, padre;

    PosicionamientoRelativo rel;
    glm::vec3 pos_rel;
    glm::mat4 trans = glm::mat4(1.0f);

    glm::vec3 escala;
    glm::mat4 esc = glm::mat4(1.0f);

    float angulo = 0.f;
    glm::mat4 rot = glm::mat4(1.0f);

    glm::vec3 color;
};
std::vector<Objeto> objetos;
int base, torre, brazo, cable;

// Buffers de instancias
TexBuffer buf_modelo;
TexBuffer buf_color;

// ---

int nuevoObjeto(glm::vec3 pos, glm::vec3 escala, glm::vec3 color, int padre = -1, PosicionamientoRelativo rel = POS_CENTRO) {
    int id = objetos.size();

    objetos.push_back({
        .id = id,
        .padre = padre,
        .rel = rel,
        .pos_rel = pos,
        .escala = escala,
        .color = color
    });

    return id;
}

void eliminarObjeto(ui32 id) {
    objetos.erase(objetos.begin() + id);
    for (ui32 i = id; i < objetos.size(); i++)
        objetos[i].id--;
}

glm::vec3 posRelativa(ui32 objeto) {
    Objeto& obj = objetos[objeto];
    Objeto* padre = obj.padre < 0 ? nullptr : &objetos[obj.padre];
   
    glm::vec3 pos = obj.pos_rel;
    switch (obj.rel) {
        case POS_CENTRO:
            break;
        case POS_ENCIMA:
            pos.y -= padre->escala.y + obj.escala.y;
            break;
        case POS_DEBAJO:
            pos.y += padre->escala.y + obj.escala.y;
            break;
        case POS_IZQUIERDA:
            pos.x -= padre->escala.x + obj.escala.x;
            break;
        case POS_DERECHA:
            pos.x += padre->escala.x + obj.escala.x;
            break;
        case POS_DELANTE:
            pos.z -= padre->escala.z + obj.escala.z;
            break;
        case POS_DETRAS:
            pos.z += padre->escala.z + obj.escala.z;
            break;
    }

    return pos;
}

glm::mat4 modeloObjeto(ui32 id) {
    glm::mat4 modelo = glm::mat4(1.f);

    Objeto& obj = objetos[id];
    Objeto* padre = obj.padre < 0 ? nullptr : &objetos[obj.padre];

    std::vector<glm::mat4> modelos;
    while (padre != nullptr) {
        modelos.push_back(padre->trans * padre->rot);
        padre = padre->padre < 0 ? nullptr : &objetos[padre->padre];
    }
    for (auto it = modelos.rbegin(); it != modelos.rend(); it++)
        modelo *= *it;

    obj.trans = glm::translate(glm::mat4(1.f), posRelativa(id));
    obj.rot = glm::rotate(glm::mat4(1.f), obj.angulo, glm::vec3(modelo[1].x, modelo[1].y, modelo[1].z));
    obj.esc = glm::scale(glm::mat4(1.f), obj.escala);
    modelo *= obj.trans * obj.rot * obj.esc;

    return modelo;
}

void actualizarModelosObjetos() {
    std::vector<glm::mat4> modelos;
    for (ui32 id = 0; id < objetos.size(); id++) {
        if (modelos.size() == id)
            modelos.push_back(modeloObjeto(id));
        else if (modelos.size() > id)
            modelos[id] = modeloObjeto(id);
        else {
            log::error("No se ha podido actualizar el modelo del objeto {}", id);
            std::exit(1);
        }
    }

    buffer::cargar(buf_modelo.b, modelos, 0);
    
    shader::usar("grua");
    shader::uniform("modelos", buf_modelo);
}

void actualizarColoresObjetos() {
    std::vector<glm::vec4> colores;
    std::transform(objetos.begin(), objetos.end(), std::back_inserter(colores), [](Objeto &p) { return glm::vec4(p.color, 0.f); });

    buffer::cargar(buf_color.b, colores, 0);

    shader::usar("grua");
    shader::uniform("colores", buf_color);
}

// ---

void controlarGrua() {
    // Mover base
    if (gl.io.teclas[GLFW_KEY_UP].mantenida)
        objetos[base].pos_rel.z -= 0.1f;
    if (gl.io.teclas[GLFW_KEY_DOWN].mantenida)
        objetos[base].pos_rel.z += 0.1f;
    if (gl.io.teclas[GLFW_KEY_LEFT].mantenida)
        objetos[base].pos_rel.x -= 0.1f;
    if (gl.io.teclas[GLFW_KEY_RIGHT].mantenida)
        objetos[base].pos_rel.x += 0.1f;

    // Mover torre
    if (gl.io.teclas[GLFW_KEY_H].mantenida)
        objetos[torre].angulo += 0.1f;
    if (gl.io.teclas[GLFW_KEY_J].mantenida)
        objetos[torre].angulo -= 0.1f;
    if (gl.io.teclas[GLFW_KEY_U].mantenida)
        objetos[torre].escala.y = std::clamp(objetos[torre].escala.y + 0.05f, 3.f, 10.f);
    if (gl.io.teclas[GLFW_KEY_Y].mantenida)
        objetos[torre].escala.y = std::clamp(objetos[torre].escala.y - 0.05f, 3.f, 10.f);

    // Mover brazo
    if (gl.io.teclas[GLFW_KEY_K].mantenida and objetos[brazo].escala.x < 12.f) {
        objetos[brazo].escala.x += 0.1f;
        objetos[brazo].pos_rel.x -= 0.1f;
        objetos[cable].pos_rel.x -= 0.1f;
    }
    if (gl.io.teclas[GLFW_KEY_L].mantenida and objetos[brazo].escala.x > 6.f) {
        objetos[brazo].escala.x -= 0.1f;
        objetos[brazo].pos_rel.x += 0.1f;
        objetos[cable].pos_rel.x += 0.1f;
    }

    // Mover cable
    if (gl.io.teclas[GLFW_KEY_I].mantenida)
        objetos[cable].escala.y -= 0.1f;
    if (gl.io.teclas[GLFW_KEY_O].mantenida)
        objetos[cable].escala.y += 0.1f;
    if (objetos[cable].escala.y < 2.f)
        objetos[cable].escala.y = 2.f;
    if (objetos[cable].escala.y - objetos[torre].escala.y > 0.f)
        objetos[cable].escala.y = objetos[torre].escala.y;

    actualizarModelosObjetos();
}

// ---

void render() {
    // Variables
    camara();
    controlarGrua();

    // Shader y uniforms
    shader::usar("grua");
    shader::uniform("viewproj", gl.proj * gl.view);
    
    // Dibujar objetos por instancias
    dibujar(objetos.size(), "cubo");
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
    nuevoObjeto( // Suelo
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(60.f, 1.f, 60.f),
        glm::vec3(0.3f, 0.2f, 0.5f)
    );
    base = nuevoObjeto( // Base
        glm::vec3(0.f, -2.f, 0.f),
        glm::vec3(3.f, 0.5f, 4.f),
        glm::vec3(1.f, 1.f, 0.5f)
    );
    nuevoObjeto( // Pie izquierdo de la base
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(1.f, 1.f, 4.5f),
        glm::vec3(0.7f, 1.f, 0.9f),
        base, POS_IZQUIERDA
    );
    nuevoObjeto( // Pie derecho de la base
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(1.f, 1.f, 4.5f),
        glm::vec3(0.7f, 1.f, 0.9f),
        base, POS_DERECHA
    );
    torre = nuevoObjeto( // Torre central
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(1.f, 8.f, 1.f),
        glm::vec3(0.5f, 0.5f, 1.f),
        base, POS_ENCIMA
    );
    brazo = nuevoObjeto( // Brazo de la grua
        glm::vec3(-4.f, 0.f, 0.f),
        glm::vec3(8.f, 1.f, 1.f),
        glm::vec3(1.f, 0.5f, 0.5f),
        torre, POS_ENCIMA
    );
    cable = nuevoObjeto( // Cable de la grua
        glm::vec3(-7.f, 0.f, 0.f),
        glm::vec3(0.1f, 6.f, 0.1f),
        glm::vec3(1.f, 0.8f, 1.f),
        brazo, POS_DEBAJO
    );
    nuevoObjeto( // Gancho de la grua
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.f, 0.9f, 0.5f),
        cable, POS_DEBAJO
    );
    
    actualizarModelosObjetos();
    actualizarColoresObjetos();

	// Actualización cada frame
	while ( update(render) ) {};

    terminarGL();
	return 0;
}
