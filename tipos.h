// Declaración de tipos
#pragma once

#include <stdint.h>
#include <string>

#include <array>
#include <vector>
#include <unordered_map>

#include <glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace tofu 
{
    // Alias de tipos
    using ui64 = std::uint64_t;
    using ui32 = std::uint32_t;
    using ui16 = std::uint16_t;
    using ui8  = std::uint8_t;

    using str = std::string;
    using str_view = std::string_view;

    // Estructura de un shader
    struct Shader {
        ui32 pid;
        std::unordered_map<str, ui32> uniforms;
    };

    // Estructura de datos de entrada
    struct Key {
        bool presionada;
        bool liberada;
        bool mantenida;
    };
    struct Mouse {
        double x, y;
        double xoff, yoff;
    };
    struct Input {
        std::unordered_map<int, Key> teclas;
        Mouse mouse;
    };

    // Estructura de datos de OpenGL
    struct GL {
        GLFWwindow* win;
        Input io;

        ui32 VAO;
        ui32 VBO;
        ui32 EBO;

        std::unordered_map<str, Shader> shaders;

        glm::mat4 view;
        glm::mat4 proj;
    };

    // Referencia a los datos de OpenGL
    inline std::unique_ptr<GL> gl;
}
