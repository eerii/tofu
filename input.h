// Manejar la entrada
#pragma once

#include "debug.h"

namespace tofu
{
    namespace input
    {
        inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS) {
                gl->io.teclas[key].presionada = true;
                gl->io.teclas[key].mantenida = true;
            }
            if (action == GLFW_RELEASE) {
                gl->io.teclas[key].liberada = true;
                gl->io.teclas[key].mantenida = false;
            }
        }

        inline void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
            gl->io.mouse.xoff = xpos - gl->io.mouse.x;
            gl->io.mouse.yoff = gl->io.mouse.y - ypos;
            gl->io.mouse.x = xpos;
            gl->io.mouse.y = ypos;
        }
    
    }
}
