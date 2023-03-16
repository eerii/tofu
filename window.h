// Gestor de ventanas (GLFW)
#pragma once

#include "debug.h"
#include "input.h"

namespace tofu
{
    void windowSizeCallback(GLFWwindow* win, int w, int h);

    // Crea una ventana con GLFW e inicializa un contexto de OpenGL 3.3 core
    inline GLFWwindow* crearContexto(ui16 w, ui16 h, str nombre) {
        // Iniciar GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_SAMPLES, 4);

        // Crear la ventana							
        GLFWwindow* win = glfwCreateWindow(w, h, nombre.c_str(), NULL, NULL);
        if (not win) {
            log::error("No se ha podido crear la ventana con GLFW");
            glfwTerminate();
            std::exit(-1);
        }
        glfwMakeContextCurrent(win);

        // Callbacks
        glfwSetKeyCallback(win, input::keyCallback);
        glfwSetCursorPosCallback(win, input::mouseCallback);
        glfwSetWindowSizeCallback(win, windowSizeCallback);

        // Ratón desactivado para tener movimiento ilimitado en la cámara
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // VSync
        glfwSwapInterval(1);

        return win;
    }

    // Ajustar perspectiva
    inline void ajustarPerspectiva(int w, int h) {
        gl->proj = glm::perspective(glm::radians(45.f), (float)w / (float)h, 0.1f, 10000.f);
        gl->proj[1][1] *= -1;
    }

    // Callback de cambio de tamaño de ventana
    inline void windowSizeCallback(GLFWwindow* win, int w, int h) {
        if (gl->tam_win.x == w and gl->tam_win.y == h)
            return;

        // Ajuste de la matriz de perspectiva
        ajustarPerspectiva(w, h);

        // Guardamos el tamaño de la ventana
        // El tamaño del framebuffer principal (usado para glViewport) puede ser diferente al tamaño de la ventana
        // Por ejemplo, al trabajar con pantallas HiDPI. Por eso utilzamos una función específica de glfw para guardarlo por separado
        gl->tam_win = {w, h};
        glfwGetFramebufferSize(win, &gl->tam_fb.x, &gl->tam_fb.y);
    }
}
