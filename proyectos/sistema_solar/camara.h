// Implementación de una cámara libre en 3D
#pragma once

#include "tofu.h"
using namespace tofu;

#include "planetas.h"

// ---

namespace cam
{

    inline glm::vec3 pos = glm::vec3(0.f, -5.f, 20.f);
    inline glm::vec3 vel = glm::vec3(0.f);

    inline float phi = 0.f;
    inline float theta = 0.5f * M_PI - 0.2f;
    inline glm::vec2 vel_raton = glm::vec3(0.f);
    inline glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    inline glm::vec3 front, right;

    enum ModoCamara { CAMARA_LIBRE, CAMARA_PLANETA, CAMARA_TELESCOPIO };
    inline ModoCamara modo = CAMARA_LIBRE;
    inline glm::mat4 view_fix = glm::mat4(1.f);

    inline str planeta = "Tierra";
    inline str mirar_a = "Sol";

    const float aceleracion = 2.f;
    const float deceleracion = 0.9f;
    const float vel_max = 20.f;
    const float vel_min = 0.05f;

    const float acc_raton = 0.6f;
    const float dec_raton = 0.7f;
    const float raton_max = 4.f;
}

// ---

inline void camara() {
    using namespace cam;

    // Movimiento delante-detras
    if (gl.io.teclas[GLFW_KEY_W].mantenida)
        vel += front * aceleracion;
    if (gl.io.teclas[GLFW_KEY_S].mantenida)
        vel += front * -aceleracion;
    if (not gl.io.teclas[GLFW_KEY_W].mantenida and not gl.io.teclas[GLFW_KEY_S].mantenida)
        vel.x *= deceleracion;

    // Movimiento izquierda-derecha
    if (gl.io.teclas[GLFW_KEY_D].mantenida)
        vel += right * aceleracion;
    if (gl.io.teclas[GLFW_KEY_A].mantenida)
        vel += right * -aceleracion;
    if (not gl.io.teclas[GLFW_KEY_A].mantenida and not gl.io.teclas[GLFW_KEY_D].mantenida)
        vel.z *= deceleracion;

    // Movimiento arriba-abajo
    if (gl.io.teclas[GLFW_KEY_Q].mantenida)
        vel += up * aceleracion;
    if (gl.io.teclas[GLFW_KEY_SPACE].mantenida)
        vel += up * -aceleracion;
    if (not gl.io.teclas[GLFW_KEY_Q].mantenida and not gl.io.teclas[GLFW_KEY_SPACE].mantenida)
        vel.y *= deceleracion;

    // Velocidad máxima y mínima
    if (glm::length(vel) > vel_max)
        vel = glm::normalize(vel) * vel_max;
    if (glm::length(vel) < vel_min)
        vel = glm::vec3(0.f);

    // Giro
    if (gl.raton_conectado) {
        // Yaw - Ratón X
        vel_raton.x += gl.io.mouse.xoff * acc_raton;
        // Pitch - Ratón Y
        vel_raton.y += gl.io.mouse.yoff * acc_raton;

        // Deceleración
        if (modo == CAMARA_PLANETA and gl.io.mouse.xoff == 0 and gl.io.mouse.yoff == 0)
            vel_raton *= 0.95f;
        else
            vel_raton *= dec_raton;

        // Ratón máximo y mínimo
        if (abs(vel_raton.x) > raton_max)
            vel_raton.x = raton_max * glm::sign(vel_raton.x);
        if (abs(vel_raton.y) > raton_max)
            vel_raton.y = raton_max * glm::sign(vel_raton.y);
        if (abs(vel_raton.x) < 0.1f)
            vel_raton.x = 0.f;
        if (abs(vel_raton.y) < 0.1f)
            vel_raton.y = 0.f;
        
        // Añadir giros
        phi += vel_raton.x * std::min(velocidad, 1.f) * (float)dt;
        phi = std::fmod(phi, 2.f * M_PI);

        theta += vel_raton.y * std::min(velocidad, 1.f) * (float)dt;
        theta = std::clamp(theta, 0.001f, (float)M_PI);
    }

    // Cámara libre
    static bool cull_status = culling, libre_previo = true;
    if (modo == CAMARA_LIBRE) {
        if (not libre_previo) {
            culling = cull_status;
            libre_previo = true;
        }

        pos += vel * velocidad * (float)dt;

        front = glm::vec3(cos(phi - 0.5f * M_PI) * sin(theta), cos(theta), sin(phi - 0.5f * M_PI) * sin(theta));
        right = glm::normalize(glm::cross(front, up));

        gl.view = glm::lookAt(pos, pos + front, up);
        return;
    }

    // Guardar el estado de culling y desactivarlo (para que se visualizen correctamente los planetas en este modo)
    if (libre_previo) {
        cull_status = culling;
        libre_previo = false;
    }
    desactivarCulling();

    // Obtener matriz de planeta de la gpu
    glm::mat4 planeta_mat;
    ui32 i = std::distance(planetas.begin(), planetas.find(planeta));

    glBindBuffer(GL_TEXTURE_BUFFER, gl.buffers[buf_modelos.b].buffer);
    glGetBufferSubData(GL_TEXTURE_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &planeta_mat);
    debug::gl();

    glm::vec3 planeta_pos = planeta_mat[3];
    float planeta_radio = planeta_mat[1][1];

    // Cámara planeta
    // Rota alrededor del planeta con theta/phi
    if (modo == CAMARA_PLANETA) {
        front = glm::vec3(cos(phi - 0.5f * M_PI) * sin(theta), cos(theta), sin(phi - 0.5f * M_PI) * sin(theta));
        gl.view = glm::lookAt(planeta_pos - front * (planeta_radio * 2.f + 3.f), planeta_pos, up);
        return;
    }

    // Matriz del segundo planeta
    glm::mat4 mirar_a_mat;
    ui32 j = std::distance(planetas.begin(), planetas.find(mirar_a));

    glGetBufferSubData(GL_TEXTURE_BUFFER, j * sizeof(glm::mat4), sizeof(glm::mat4), &mirar_a_mat);
    debug::gl();

    glm::vec3 mirar_a_pos = mirar_a_mat[3];

    // Cámara telescopio
    if (modo == CAMARA_TELESCOPIO) {
        gl.view = glm::lookAt(planeta_pos, mirar_a_pos, up);
        return;
    }
}
