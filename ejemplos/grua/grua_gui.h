// Funciones GUI para el sistema solar
#pragma once

#include "tofu.h"
using namespace tofu;

#include "camara.h"

#ifndef DISABLE_GUI
    #define DIBUJAR_SI(nombre, num, shader) \
        if (sgui.dibujar[#nombre]) { dibujar(num, #shader); } \
        else { gl.instancia_base += num; }
#else
    #define DIBUJAR_SI(nombre, num, shader) dibujar(num, #shader);
#endif

// ---

#ifndef DISABLE_GUI
inline struct GruaGui {
    bool ventana_camara = false;
} sgui;
#endif

// ---

inline void grua_gui() {
#ifndef DISABLE_GUI
    // Menú superior
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("Grua")) {
            ImGui::Text("by José Pazos Pérez");
            
            if (ImGui::MenuItem("Salir", "Shift + Q"))
                glfwSetWindowShouldClose(gl.win, true);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Venanas")) {
            ImGui::MenuItem("Camara", NULL, &sgui.ventana_camara);
            ImGui::MenuItem("Rendimiento", NULL, &imgui->ventana_rendimiento);
            ImGui::MenuItem("Demo", NULL, &imgui->ventana_demo);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Ventana Cámara
    // Muestra las propiedades de la cámara y permite cambiar entre modos (libre, planeta y telescopio)
    if (sgui.ventana_camara) {
        ImGui::Begin("Camara", &sgui.ventana_camara);
            // Posición
            ImGui::DragFloat3("Pos", cam::pos.v.data(), 0.1f);

            // Modo de cámara
            ImGui::RadioButton("Libre", (int*)&cam::modo, cam::CAMARA_LIBRE);
            ImGui::RadioButton("3º Persona", (int*)&cam::modo, cam::CAMARA_TERCERA);
            ImGui::RadioButton("1º Persona", (int*)&cam::modo, cam::CAMARA_PRIMERA);
        ImGui::End();
    }
#endif
}
