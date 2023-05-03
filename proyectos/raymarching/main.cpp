// Proyecto: Raymarching
// José Pazos Pérez

// Opciones
#define DEBUG
//#define USE_MULTISAMPLING
//#define USE_RETINA_FB

// Librería de OpenGL 3.3 (core)
// Define las funciones básicas que se pueden utilizar en mútiples proyectos
// Utiliza un planteamiento de renderizado por instancias con el objetivo de minimizar las llamadas a la GPU
// No se puede contruír un motor totalmente indirecto en el que la GPU haga todo el trabajo ya que estas capacidades se incluyen en OpenGL 4
// De todas maneras, aprovecha muchas herramientas disponibles para una mejor disponibilidad de los datos a dibujar
// Incluye GLFW, GLAD y GLM
#include "tofu.h"
using namespace tofu;

// ---

// ···········
// · AJUSTES ·
// ···········

// Tamaño de la ventana
constexpr ui32 WIDTH = 640;
constexpr ui32 HEIGHT = 640;

// Lista de atributos del VAO
// Cada elemento de la lista es el número de bytes de un atributo
// Se pasa a la función iniciarVAO
const std::vector<ui32> atributos = {};

// Vista y proyección
// Matriz que combina gl.view y gl.proj para transformar los modelos
glm::mat4 viewproj;

// ---

// ···················
// · BUCLE PRINCIPAL ·
// ···················

// Renderizar
// Esta función se llama cada frame dentro de tofu::update()
// Es la manera que tenemos de indicar fuera de la librería qué objetos queremos dibujar y actualizar los uniforms que cambian cada frame
void render() {
    shader::usar("raymarch");
    
    shader::uniform("time", (float)glfwGetTime());

    glDrawArrays(GL_TRIANGLES, 0, 3);
    debug::gl();
}

// ---

// ············
// · PROGRAMA ·
// ············

int main(int arcg, char** argv) {
    // Cambiamos el directorio actual por el del ejecutable
    // Esto es necesario para que las rutas de los archivos sean correctas
    fs::path path = fs::weakly_canonical(fs::path(argv[0])).parent_path();
    fs::current_path(path);

    // Iniciamos GLFW y OpenGL
    initGL(WIDTH, HEIGHT, "Raymarching");

    // Creamos los buffers principales que almacenan la información por instancia
    buffer::iniciarVAO(atributos);

    // Cargamos las shader a utilizar
    shader::cargar("raymarch");

    // Especificamos los attachments a usar en deferred

    // Generador de números aleatorios
    std::srand(std::time(nullptr));

	// Llamamos al bucle principal de la aplicación
    // Devuelve false cuando se cierra la ventana
    while ( update(render) ) {};

    // Antes de salir hacemos limpieza de los objetos utilizados
    terminarGL();
	return 0;
}
