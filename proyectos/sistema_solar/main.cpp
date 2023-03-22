// Proyecto: Sistema Solar
// José Pazos Pérez

// TODO:
// - Documentar funciones tofu
// - Cambiar a usar instanced arrays!!! https://learnopengl.com/Advanced-OpenGL/Instancing
//      No lo puedo usar para dibujar planetas por límites y no tener offsets, pero si se pueden usar para calcular los modelos
// - Poner en itch.io con emscripten
// - Mapas de terreno
// - Ruído perlin, múltiples niveles
// - Cráteres
// - Triplanar mapping para texturas y normales
// - Atmósferas
// - Outlines a los planetas (manifold garden siggraph, sdf)
// - N body dynamics

// Librería de OpenGL 3.3 (core)
// Define las funciones básicas que se pueden utilizar en mútiples proyectos
// Utiliza un planteamiento de renderizado por instancias con el objetivo de minimizar las llamadas a la GPU
// No se puede contruír un motor totalmente indirecto en el que la GPU haga todo el trabajo ya que estas capacidades se incluyen en OpenGL 4
// De todas maneras, aprovecha muchas herramientas disponibles para una mejor disponibilidad de los datos a dibujar
// Incluye GLFW, GLAD y GLM
#define DEBUG
#include "tofu.h"
using namespace tofu;

// Datos de planetas
#include "planetas.h"

// Función auxiliar con una cámara 3D
// Se mueve con <WASD> y se usa el ratón para girar
// Con <Espacio> se va hacia arriba y con <X> hacia abajo
#include "camara.h"

// Función auxiliar que carga los paneles de la GUI
#include "solar_gui.h"

// ---

// ···········
// · AJUSTES ·
// ···········

// Tamaño de la ventana
constexpr ui32 WIDTH = 800;
constexpr ui32 HEIGHT = 800;

// Lista de atributos del VAO
// Cada elemento de la lista es el número de bytes de un atributo
// Se pasa a la función iniciarVAO
const std::vector<ui32> atributos = { 3 /* Pos */ };

// La shader para calcular los modelos de los planetas no necesita ningún atributo
const std::vector<ui32> atributos_planeta = {};

// La shader para mostrar una textura 2D necesita un atributo de posición 2D
const std::vector<ui32> atributos_textura = { 2 /* Pos */ };

// Vista y proyección
// Matriz que combina gl.view y gl.proj para transformar los modelos
glm::mat4 viewproj;

// ---

// ·······················
// · UTILIDADES PLANETAS ·
// ·······················

void iniciarDatosPlanetas() {
    // Estructura preparada para la gpu:
    // 0 - radio
    // 1 - distancia
    // 2 - indice del padre
    std::vector<glm::vec3> planetas_gpu;

    // Iteramos por los planetas que tenemos
    ui32 i = 0;
    for (const auto &[n, p] : planetas) {
        // Buscamos la órbita padre
        float padre = -1.f;
        if (p.orbita != "") {
            auto it = planetas.find(p.orbita);
            if (it == planetas.end()) {
                log::error("Planeta {} tiene como padre a {} que no existe", n, p.orbita);
                std::exit(-1);
            }
            padre = (float)std::distance(planetas.begin(), it);
        }
        // Añadimos el planeta a la lista
        planetas_gpu.push_back({ p.radio, p.distancia, padre });
        i++;
    }

    // Colores de los planetas
    std::vector<glm::vec4> color;
    std::transform(planetas.begin(), planetas.end(), std::back_inserter(color), [](auto &pl) {
        return glm::vec4(pl.second.color, 0.f);
    });

    // Órbitas de los planetas
    std::vector<glm::mat4> orbitas;
    for (const auto &[n, p] : planetas) {
        if (p.orbita != "")
            continue;
        orbitas.push_back(glm::scale(glm::mat4(1.f), glm::vec3(p.distancia)));
    }

    // Creamos los asteroides y los anillos de saturno
    for (ui32 i = 0; i < num_asteroides - 100; i++) {
        float radio = (std::rand() % 100 / 100.f) * 0.1f + 0.1f;
        float distancia = (std::rand() % 100 / 100.f) * 4.f + 29.f;
        planetas_gpu.push_back({ radio, distancia, -1.f });
        color.push_back(glm::vec4(cos(i), sin(i), 1.f, 0.f));
    }
    auto it = planetas.find("Saturno");
    float padre = (float)std::distance(planetas.begin(), it);
    for (ui32 i = 0; i < 100; i++) {
        float radio = (std::rand() % 100 / 100.f) * 0.04f + 0.06f;
        float distancia = (std::rand() % 100 / 100.f) * 0.5f + 2.f;
        planetas_gpu.push_back({ radio, distancia, padre });
        color.push_back(glm::vec4(1.f, 0.9f, (sin(i) + 1.f) * 0.7f, 0.f));
    }

    // Creamos las estrellas
    std::vector<glm::mat4> estrellas;
    for (ui32 i = 0; i < num_estrellas; i++) {
        float theta = (std::rand() % 1000 / 1000.f) * 2.f * M_PI;
        float phi = (std::rand() % 1000 / 1000.f) * M_PI;
        float distancia = (std::rand() % 1000) + 4000.f;

        glm::vec3 pos = glm::vec3(
            distancia * sin(phi) * cos(theta),
            distancia * sin(phi) * sin(theta),
            distancia * cos(phi)
        );

        glm::mat4 m = glm::mat4(1.f);
        m = glm::translate(m, pos);

        estrellas.push_back(m);
    }

    // Cargar buffers a la GPU
    buffer::cargar(buf_planetas.b, planetas_gpu);
    buffer::cargar(buf_color.b, color, 0);
    buffer::redimensionar(buf_modelos.b, 2*num_planetas + num_asteroides + num_estrellas);
    buffer::cargar(buf_modelos.b, orbitas, num_planetas + num_asteroides);
    buffer::cargar(buf_estrellas.b, estrellas, 0);

    // Shaders
    shader::usar("calc_modelos");
    shader::uniform("bplanetas", buf_planetas);

    shader::usar("calc_estrellas");
    shader::uniform("bestrellas", buf_estrellas);

    shader::usar("planetas");
    shader::uniform("bmodelos", buf_modelos);
    shader::uniform("bcolor", buf_color);

    shader::usar("orbitas");
    shader::uniform("borbitas", buf_modelos);

    shader::usar("estrellas");
    shader::uniform("bestrellas", buf_modelos);
}

// ---

// ···················
// · BUCLE PRINCIPAL ·
// ···················

// Renderizar
// Esta función se llama cada frame dentro de tofu::update()
// Es la manera que tenemos de indicar fuera de la librería qué objetos queremos dibujar y actualizar los uniforms que cambian cada frame
void render() {
    // Planetas
    shader::usar("planetas");
    shader::uniform("viewproj", viewproj);
    gl.instancia_base = 0;
    DIBUJAR_SI(planetas, cull_planetas, esfera20) // Planetas
    gl.instancia_base = num_planetas;
    DIBUJAR_SI(asteroides, cull_asteroides, esfera5) // Asteroides (modelo con menos resolucion)

    // Orbitas
    shader::usar("orbitas");
    shader::uniform("viewproj", viewproj);
    gl.instancia_base = num_planetas + num_asteroides;
    DIBUJAR_SI(orbitas, num_planetas, circulo)

    // Estrellas
    shader::usar("estrellas");
    shader::uniform("time", tiempo);
    shader::uniform("viewproj", viewproj);
    DIBUJAR_SI(estrellas, cull_estrellas, cubo)

    // ---

    // Recalculamos los parámetros variables con el tiempo
    // Lo hacemos después de dibujar porque parece producir resultados más estables (va con un frame de retraso que debería de ser imperceptible)
    tiempo += velocidad * dt;

    // Calculamos los modelos de los planetas y asteroides usando una vertex shader (no tenemos acceso a compute)
    // Utilizamos "transform feedback" para guardar los resultados directamente en buf_modelos
    shader::usar("calc_modelos");
    shader::uniform("time", tiempo);
    shader::uniform("viewproj", viewproj);
    shader::uniform("culling", (int)culling);
    cull_planetas = transformFeedback(0, num_planetas, gl.buffers[buf_modelos.b]);
    cull_asteroides = transformFeedback(num_planetas, num_asteroides, gl.buffers[buf_modelos.b]);

    // Calculamos también las estrellas visibles (frustrum culling)
    if (culling) {
        shader::usar("calc_estrellas");
        shader::uniform("viewproj", viewproj);
        shader::uniform("culling", 1);
        cull_estrellas = transformFeedback(2*num_planetas + num_asteroides, num_estrellas, gl.buffers[buf_modelos.b]);
    }

    // Calculamos la matriz de la cámara
    camara();
    viewproj = gl.proj * gl.view;
}

// ---

// ············
// · PROGRAMA ·
// ············

int main() {
    // Iniciamos GLFW y OpenGL
    log::info("Iniciando OpenGL");
    initGL(WIDTH, HEIGHT, "Sistema Solar");

    // Creamos los buffers principales que almacenan la información por instancia
    log::info("Iniciando VAOs");
    buffer::iniciarVAO(atributos);
    buffer::iniciarVAO(atributos_planeta, "vao_calc_modelos");
    log::info("Creado Buffers");
    buf_planetas = texbuffer::crear<glm::vec3>();
    buf_modelos = texbuffer::crear<glm::mat4>();
    buf_color = texbuffer::crear<glm::vec4>();
    buf_estrellas = texbuffer::crear<glm::mat4>();

    // Cargamos las shader a utilizar
    log::info("Cargando Shaders");
    shader::cargar("planetas");
    shader::cargar("orbitas");
    shader::cargar("estrellas");
    shader::cargar("calc_modelos", "vao_calc_modelos", 0, {}, { "out_modelo" });
    shader::cargar("calc_estrellas", "vao_calc_modelos", 0, {}, { "out_modelo" });

    // Cargamos en memoria las figuras a dibujar
    // Se añaden automáticamente al VBO/EBO y guardamos la información de indexado
    // Utilizamos un mismos buffer para guardar todos los vértices y pasamos offsets al dibujar
    // Esto evita que tengamos que desvincular y vincular varios VBOs en cada frame, operación bastante costosa
    // Además, si tuvieramos acceso, es la manera más recomendada de hacer renderizado indirecto
    log::info("Cargando Geometría");
    buffer::cargarVert("esfera20", geometria::esferaOct(20));
    buffer::cargarVert("esfera5", geometria::esferaOct(5));
    buffer::cargarVert("cubo", geometria::cubo());
    buffer::cargarVert("circulo", geometria::circulo(100), "main", GL_LINE_STRIP); 

    // Generador de números aleatorios
    std::srand(std::time(nullptr));

    // Crear planetas
    // Construímos los planetas y asteroides, unos a partir de las constantes que indicamos y otros por variables aleatorias
    // Luego llamamos a iniciarDatosPlanetas para cargar estos datos en la GPU
    log::info("Cargando Datos de Planetas");
    iniciarDatosPlanetas();

    // Creamos queries
    glGenQueries(1, &tf_query);
    debug::gl();

	// Llamamos al bucle principal de la aplicación
    // Devuelve false cuando se cierra la ventana
	while ( update(render, solar_gui) ) {};

    // Antes de salir hacemos limpieza de los objetos utilizados
    terminarGL();
	return 0;
}
