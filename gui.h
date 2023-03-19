// Debug GUI
// Utiliza ImGui para mostrar una interfaz de desarrollo donde configurar parámetros e imprimir datos
#pragma once

#ifndef DEBUG
#define DISABLE_GUI
#endif

// Permite deshabilitar por completo la integración con ImGui
#ifndef DISABLE_GUI

#define VGIZMO_USES_GLM
#define IMGUIZMO_IMGUI_FOLDER 
#include "imGuIZMOquat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "debug.h"

#include <numeric>

namespace tofu
{
    // Estructura para almacenar la información necesaria de ImGui
    // (Separada de la estructura principal para permitir deshabilitarlo fácilmente)
    struct GuiData {
        GuiData(ImGuiIO& _io) : io(_io) {
            std::fill(frame_hist.begin(), frame_hist.end(), 0.0);
            std::fill(render_hist.begin(), render_hist.end(), 0.0);
            std::fill(gui_hist.begin(), gui_hist.end(), 0.0);
            std::fill(present_hist.begin(), present_hist.end(), 0.0);
        }

        ImGuiIO& io;
        bool ventana_demo = false;
        bool ventana_rendimiento = false;
        std::array<double, 100> frame_hist, render_hist, gui_hist, present_hist;
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
            ImGui_ImplGlfw_InitForOpenGL(gl.win, true);
            ImGui_ImplOpenGL3_Init("#version 330 core");
        }

        inline void render(std::function<void()> custom) {
            // Nuevo frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Demo
            if (imgui->ventana_demo)
                ImGui::ShowDemoWindow(&imgui->ventana_demo);

            // Panel de rendimiento
            if (imgui->ventana_rendimiento) {
                ImGui::Begin("Rendimiento", &imgui->ventana_rendimiento);
                #ifdef DEBUG

                // ---
                // Tiempo

                auto tiempos = [](auto& hist, double time) {
                    std::rotate(hist.begin(), hist.begin() + 1, hist.end());
                    hist.back() = time;
                    double max = *std::max_element(hist.begin(), hist.end());
                    double min = *std::min_element(hist.begin(), hist.end());
                    double avg = std::accumulate(hist.begin(), hist.end(), 0.0) / hist.size();
                    return std::make_tuple(max * 1000.0, min * 1000.0, avg * 1000.0);
                };

                auto [max_frame, min_frame, avg_frame] = tiempos(imgui->frame_hist, debug::frame_time);
                auto [max_render, min_render, avg_render] = tiempos(imgui->render_hist, debug::render_usuario_time);
                auto [max_gui, min_gui, avg_gui] = tiempos(imgui->gui_hist, debug::render_gui_time);
                auto [max_present, min_present, avg_present] = tiempos(imgui->present_hist, debug::present_time);

                ImGui::Text("tiempo (ms)");

                ImGui::Text("       frame  rendr  imgui  prsnt");
                ImGui::Text("avg:   %5.2f  %5.2f  %5.2f  %5.2f", avg_frame, avg_render, avg_gui, avg_present);
                ImGui::Text("min:   %5.2f  %5.2f  %5.2f  %5.2f", min_frame, min_render, min_gui, min_present);
                ImGui::Text("max:   %5.2f  %5.2f  %5.2f  %5.2f", max_frame, max_render, max_gui, max_present);
                ImGui::Text("fps:   %5d", (int)std::floor(imgui->io.Framerate));

                // ---
                // Contadores
                ImGui::Separator();
                ImGui::Text("contadores");

                // Instancias
                ImGui::Text("draw calls: %21d", debug::num_draw);
                ImGui::Text("objetos:    %21d", debug::num_instancias);
                ImGui::Text("triangulos: %21d", debug::num_triangulos);
                ImGui::Text("vertices:   %21d", debug::num_vertices);
        
                // ---
                // Ajustes
                ImGui::Separator();
                ImGui::Text("ajustes");
                
                // VSync
                static bool vsync = true;
                if (ImGui::Checkbox("vsync", &vsync))
                    glfwSwapInterval(vsync);

                // Usar instancias
                // NOTA: Esta opción realmente no muestra el rendimiento de no utilizar instancias,
                //       ya que seguimos usando los buffers que evitan modificar todos los uniforms frame a frame,
                //       por lo que seguiría teniendo que tener mejor rendimiento que el método clásico.
                //       De todas formas, al usar muchos objetos se puede apreciar claramente la subida de tiempo.
                //       Al no tener contadores de la GPU tampoco podemos medir lo que tardan en ejecutarse las shaders, algo que sería útil.
                ImGui::Checkbox("usar instancias", &debug::usar_instancias);

                #else

                ImGui::Text("activa el modo debug para ver este panel");

                #endif
                ImGui::End();
            }

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
