// Gestor de ventanas (GLFW)
#pragma once

#include "debug.h"
#include "input.h"

namespace tofu
{
    // Crea una ventana con GLFW e inicializa un contexto de OpenGL 3.3 core
    inline GLFWwindow* crearContexto(ui16 w, ui16 h, str nombre) {
        // Iniciar GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

        // Crear la ventana							
        GLFWwindow* win = glfwCreateWindow(w, h, nombre.c_str(), NULL, NULL);
        if (not win) {
            log::error("No se ha podido crear la ventana con GLFW");
            glfwTerminate();
            std::exit(-1);
        }
        glfwMakeContextCurrent(win);

        // Input callback
        glfwSetKeyCallback(win, input::keyCallback);
        glfwSetCursorPosCallback(win, input::mouseCallback);

        // Ratón desactivado para tener movimiento ilimitado en la cámara
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        return win;
    }
   
    // Procesa los eventos de GLFW
    // Debe ser llamado en cada iteración del bucle principal
    inline void procesarEventos(GLFWwindow* window) {
        if (gl->io.teclas[GLFW_KEY_ESCAPE].presionada)
            glfwSetWindowShouldClose(window, true); 
    }
}
