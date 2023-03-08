// Inicialización y gestión de OpenGL
#pragma once

#include <functional>

#include "debug.h"
#include "window.h"

namespace tofu
{
    // Inicializar OpenGL
    inline void initGL(ui16 w, ui16 h, str nombre) {
        // Creamos la estructura de datos de OpenGL
        gl = std::make_unique<GL>();
        gl->win = crearContexto(w, h, "Sistema Solar");

        // Cargar OpenGL con GLAD
        if (not gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log::error("No se han podido cargar los símbolos de OpenGL con GLAD");
            std::exit(-1);
        }

        // Colores para clear
        glClearDepth(1.0f);
        glClearColor(0.05f, 0.0f, 0.2f, 1.0f);

        // Profundidad
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // Inicializar los buffers y activarlos
        glGenVertexArrays(1, &gl->VAO);
        glGenBuffers(1, &gl->VBO);
        glGenBuffers(1, &gl->EBO);

        // Matriz de perspectiva
        gl->proj = glm::perspective(glm::radians(45.f), (float)w / (float)h, 0.1f, 100.f);
        gl->proj[1][1] *= -1;
    }

    // Bucle de la aplicación
    using update_fun_t = std::function<void()>;
    inline bool update(update_fun_t render = []{}) {
        // Procesar la entrada
        procesarEventos(gl->win); 

        // Limpiar la pantalla antes de seguir
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Llamar a los comandos de renderizados especificados
        render();

        // Cambiar los buffers y presentar a pantalla
        glfwSwapBuffers(gl->win);
        for (auto& [c, t] : gl->io.teclas) {
            t.liberada = false;
            t.presionada = false;
        }
        gl->io.mouse.xoff = 0; gl->io.mouse.yoff = 0;
        glfwPollEvents();

        return not glfwWindowShouldClose(gl->win);
    }

    // Limpieza
    inline void terminarGL() {
        glDeleteVertexArrays(1, &gl->VAO);
        glDeleteBuffers(1, &gl->VBO);
        glDeleteBuffers(1, &gl->EBO);
        for (auto& [n, s] : gl->shaders)
            glDeleteProgram(s.pid);
        glfwTerminate();
    }
}
