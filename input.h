// Manejar la entrada
#pragma once

#include "debug.h"

namespace tofu::input
{
    inline void keymaps() {
        // Ver si el usuario quiere cerrar la ventana
        if (gl.io.teclas[GLFW_KEY_LEFT_SHIFT].mantenida and gl.io.teclas[GLFW_KEY_Q].presionada)
            glfwSetWindowShouldClose(gl.win, true);

        // Cambiar el modo del ratón
        if (gl.io.teclas[GLFW_KEY_ESCAPE].presionada) {
            glfwSetInputMode(gl.win, GLFW_CURSOR, gl.raton_conectado ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            gl.raton_conectado = not gl.raton_conectado;
        }
    }

    inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            gl.io.teclas[key].presionada = true;
            gl.io.teclas[key].mantenida = true;
        }
        if (action == GLFW_RELEASE) {
            gl.io.teclas[key].liberada = true;
            gl.io.teclas[key].mantenida = false;
        }
        keymaps();
    }

    inline void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        gl.io.mouse.xoff = xpos - gl.io.mouse.x;
        gl.io.mouse.yoff = gl.io.mouse.y - ypos;
        gl.io.mouse.x = xpos;
        gl.io.mouse.y = ypos;
    }
}
