// Debug GUI
// Utiliza ImGui para mostrar una interfaz de desarrollo donde configurar parámetros e imprimir datos
#pragma once

// Permite deshabilitar por completo la integración con ImGui
#ifndef DISABLE_GUI

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "debug.h"

namespace tofu
{
    // Estructura para almacenar la información necesaria de ImGui
    // (Separada de la estructura principal para permitir deshabilitarlo fácilmente)
    struct GuiData {
        GuiData(ImGuiIO& _io) : io(_io) {}

        ImGuiIO& io;
        bool mostrar_demo = false;
    };
    inline std::unique_ptr<GuiData> imgui;

    namespace gui 
    {
        inline void init() {
            // Creamos el contexto y los datos
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            imgui = std::make_unique<GuiData>(
                ImGui::GetIO()
            );

            // Configaración
            ImGui::StyleColorsDark(); // Colores oscuros
            imgui->io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Permite sacar las ventanas fuera de la aplicación

            // Inicializamos los backends necesarios (GLFW y OpenGL 3.3)
            ImGui_ImplGlfw_InitForOpenGL(gl->win, true);
            ImGui_ImplOpenGL3_Init("#version 330 core");
        }

        inline void render(std::function<void()> custom) {
            // Nuevo frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Demo
            if (imgui->mostrar_demo)
                ImGui::ShowDemoWindow(&imgui->mostrar_demo);

            // Crear funciones de usuario
            custom();

            // Render
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Múltiples viewports
            if (imgui->io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* curr_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(curr_context);
            }
        }

        inline void terminar() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }
}

#else

namespace tofu::gui
{
    inline void init() {};
    inline void render(std::function<void()> custom) {};
    inline void terminar() {};
}

#endif
