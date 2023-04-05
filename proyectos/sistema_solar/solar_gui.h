// Funciones GUI para el sistema solar
#pragma once

#include "tofu.h"
using namespace tofu;

#include "planetas.h"
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
inline struct SolarGui {
    bool ventana_dibujar = false;
    bool ventana_camara = false;

    std::unordered_map<str, bool> dibujar = {
        { "planetas", true },
        { "asteroides", true },
        { "orbitas", true },
        { "estrellas", true }
    };

    bool activar_luz = true;
    bool activar_bordes = true;
    bool activar_toon = true;
} sgui;
#endif

// ---

inline void solar_gui() {
#ifndef DISABLE_GUI
    // Menú superior
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("Sistema Solar")) {
            ImGui::Text("by José Pazos Pérez");
            
            if (ImGui::MenuItem("Salir", "Shift + Q"))
                glfwSetWindowShouldClose(gl.win, true);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Venanas")) {
            ImGui::MenuItem("Dibujar", NULL, &sgui.ventana_dibujar);
            ImGui::MenuItem("Camara", NULL, &sgui.ventana_camara);
            ImGui::MenuItem("Rendimiento", NULL, &imgui->ventana_rendimiento);
            ImGui::MenuItem("Demo", NULL, &imgui->ventana_demo);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Ventana Dibujo
    // Permite habilitar y deshabilitar los objetos a dibujar
    if (sgui.ventana_dibujar) {
        ImGui::Begin("Dibujar", &sgui.ventana_dibujar);

        // Opciones sobre qué objetos dibujar
        ImGui::Text("dibujar:");
        for (auto& [k, v] : sgui.dibujar)
            ImGui::Checkbox(k.c_str(), &v);

        ImGui::Text("opciones:");

        // Velocidad de simulación
        ImGui::SliderFloat("velocidad", &velocidad, 0.0, 10.0, "%.2f", ImGuiSliderFlags_Logarithmic);

        // Culling, desactivar si no está en modo camara libre
        ImGui::BeginDisabled(cam::modo != cam::CAMARA_LIBRE);
        if (ImGui::Checkbox("culling", &culling) and not culling)
            desactivarCulling();
        ImGui::EndDisabled();

        // Iluminación y bordes
        if (ImGui::Checkbox("iluminación", &sgui.activar_luz)) {
            shader::usar("planetas");
            shader::uniform("activar_luz", sgui.activar_luz ? 1.f : 0.f);
        }
        if (ImGui::Checkbox("bordes", &sgui.activar_bordes)) {
            shader::usar("deferred");
            shader::uniform("activar_bordes", sgui.activar_bordes ? 1.f : 0.f);
        }
        if (ImGui::Checkbox("toon shading", &sgui.activar_toon)) {
            shader::usar("deferred");
            shader::uniform("activar_toon", sgui.activar_toon ? 1.f : 0.f);
        }

        
        ImGui::End();
    }

    // Ventana Cámara
    // Muestra las propiedades de la cámara y permite cambiar entre modos (libre, planeta y telescopio)
    if (sgui.ventana_camara) {
        ImGui::Begin("Camara", &sgui.ventana_camara);

        // Posición
        ImGui::DragFloat3("Pos", glm::value_ptr(cam::pos), 0.1f);

        // Rotación
        imguiGizmo::setGizmoFeelingRot(0.6f);
        imguiGizmo::setDollyScale(25.f);
        imguiGizmo::setPanScale(25.f);
        glm::quat q = glm::quat(glm::vec3(cam::theta - 0.5f * M_PI, cam::phi, 0.f));
        if (ImGui::gizmo3D("##gizmo", cam::pos, q, 100, imguiGizmo::sphereAtOrigin)) {
            cam::theta = glm::pitch(q) + 0.5f * M_PI;
            cam::phi = glm::yaw(q);
        }

        // Modo de cámara
        ImGui::RadioButton("Libre", (int*)&cam::modo, cam::CAMARA_LIBRE);
        ImGui::RadioButton("Planeta", (int*)&cam::modo, cam::CAMARA_PLANETA);
        ImGui::RadioButton("Telescopio", (int*)&cam::modo, cam::CAMARA_TELESCOPIO);

        // Seleccion de planetas
        if (cam::modo == cam::CAMARA_PLANETA or cam::modo == cam::CAMARA_TELESCOPIO) {
            const char* preview_value = cam::planeta.c_str();
            if (ImGui::BeginCombo("Planeta##select", preview_value, 0)) {
                for (auto [k, v] : planetas) {
                    const bool is_selected = cam::planeta == k;
                    if (ImGui::Selectable(k.c_str(), is_selected))
                        cam::planeta = k;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
        if (cam::modo == cam::CAMARA_TELESCOPIO) {
            const char* preview_value = cam::mirar_a.c_str();
            if (ImGui::BeginCombo("Mirar a##select", preview_value, 0)) {
                for (auto [k, v] : planetas) {
                    const bool is_selected = cam::mirar_a == k;
                    if (ImGui::Selectable(k.c_str(), is_selected))
                        cam::mirar_a = k;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        } 
    
        ImGui::End();
    }
#endif
}
