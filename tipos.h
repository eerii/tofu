// Declaración de tipos
#pragma once

#include <stdint.h>
#include <string>
#include <cstdlib>
#include <ctime>

#include <array>
#include <vector>
#include <map>
#include <unordered_map>

#include <functional>
#include <numeric>
#include <memory>

#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #define WEB(x) x
    #define NOWEB(x)
#else
    #include <glad.h>
    #define WEB(x)
    #define NOWEB(x) x
#endif
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace tofu 
{
    // Alias de tipos
    using ui64 = std::uint64_t;
    using ui32 = std::uint32_t;
    using ui16 = std::uint16_t;
    using ui8  = std::uint8_t;

    using str = std::string;
    using str_view = std::string_view;

    using update_fun_t = std::function<void()>;

    // Estructura de un shader
    struct OpcionesShader {
        bool blend = true;
        bool depth = true;
        bool cull = true;
    };
    struct Shader {
        ui32 pid;
        str vao;
        ui32 fbo;
        std::unordered_map<str, ui32> uniforms;
        OpcionesShader opt;
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
    struct Textura {
        ui32 textura;
        ui32 target;
        ui32 formato;
        ui32 tipo;
        glm::ivec2 tam;
    };
    struct TexBuffer {
        ui32 b;
        ui32 t;
    };

    // Framebuffers
    struct Framebuffer {
        ui32 fbo;
        std::vector<ui32> attachments;
        std::vector<ui32> attachment_description;
        glm::ivec3 tam;
        glm::vec4 clear;
    };

    // Posición relativa en el vector de vértices/indices
    struct Geometria {
        ui32 voff, vcount;
        ui32 ioff, icount;
        ui32 tipo_dibujo;
    };

    // VAO
    struct VAO {
        ui32 vao, vbo, ebo;
        std::vector<ui32> atributos;
    };

    // Estructura de datos de OpenGL
    inline struct GL {
        GLFWwindow* win;
        glm::ivec2 tam_win, tam_fb;
        ui32 max_tex_size;

        Input io;
        bool raton_conectado = true;

        std::unordered_map<str, VAO> VAOs;
        int instancia_base = 0;

        str shader_actual = "";
        std::unordered_map<str, Shader> shaders;
        std::unordered_map<str, Geometria> geometrias;

        std::unordered_map<ui32, Buffer> buffers;
        std::map<ui32, Textura> texturas;
        std::unordered_map<ui32, Framebuffer> framebuffers;

        glm::mat4 view;
        glm::mat4 proj;
    } gl;

    inline double t, dt;
}
