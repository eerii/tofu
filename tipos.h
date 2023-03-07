// Declaración de tipos
#pragma once

#include <stdint.h>
#include <string>

#include <array>
#include <vector>
#include <unordered_map>

#include <glad.h>
#include <glfw3.h>

namespace tofu 
{
    using ui64 = std::uint64_t;
    using ui32 = std::uint32_t;
    using ui16 = std::uint16_t;
    using ui8  = std::uint8_t;

    using str = std::string;
    using str_view = std::string_view;

    struct GL {
        GLFWwindow* win;
        ui32 VAO;
        ui32 VBO;
        ui32 EBO;
        ui32 pid;
    };
}
