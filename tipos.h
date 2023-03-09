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

#define TEX_BUFFER_OFF 8

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

    // Buffer de datos
    struct Buffer {
        ui32 buffer;
        ui32 tipo;
        ui32 modo;
        ui32 tam;
        ui32 bytes;
    };
    struct Texture {
        ui32 texture;
    };
    struct TexBuffer {
        Buffer b;
        Texture t;
    };

    // Posición relativa en el vector de vértices/indices
    struct Geometria {
        ui32 voff, vcount;
        ui32 ioff, icount;
        ui32 tipo_dibujo;
    };

    // Estructura de datos de OpenGL
    struct GL {
        GLFWwindow* win;
        Input io;
        bool raton_conectado = true;

        ui32 VAO;
        Buffer VBO, EBO;
        std::vector<ui32> atributos;
        ui32 v_offset = 0;
        int instancia_base = 0;

        std::unordered_map<str, Shader> shaders;
        str shader_actual = "";
        std::unordered_map<str, Geometria> geometrias;

        glm::mat4 view;
        glm::mat4 proj;
    };

    // Referencia a los datos de OpenGL
    inline std::unique_ptr<GL> gl;
}
