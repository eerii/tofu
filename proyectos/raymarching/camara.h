// Implementación de una cámara libre en 3D
#pragma once

#include "tofu.h"
using namespace tofu;

// ---

namespace cam
{
    inline glm::vec3 pos = glm::vec3(0., 0., -3.5);
    inline glm::vec3 vel = glm::vec3(0.f);

    inline float phi = 0.f;
    inline float theta = 0.5f * M_PI - 0.2f;
    inline glm::mat4 rot;
    inline glm::vec2 vel_raton = glm::vec3(0.f);
    inline glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    inline glm::vec3 front, right;

    const float aceleracion = 0.5f;
    const float deceleracion = 0.9f;
    const float vel_max = 2.f;
    const float vel_min = 0.05f;

    const float acc_raton = 0.6f;
    const float dec_raton = 0.7f;
    const float raton_max = 1.f;
}

// ---

inline void camara() {
    using namespace cam;

    // Movimiento delante-detras
    if (gl.io.teclas[GLFW_KEY_W].mantenida)
        vel += front * -aceleracion;
    if (gl.io.teclas[GLFW_KEY_S].mantenida)
        vel += front * aceleracion;
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
        vel_raton.x -= gl.io.mouse.xoff * acc_raton;
        // Pitch - Ratón Y
        vel_raton.y -= gl.io.mouse.yoff * acc_raton;

        // Deceleración
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
        phi += vel_raton.x * (float)dt;
        phi = std::fmod(phi, 2.f * M_PI);

        theta += vel_raton.y * (float)dt;
        theta = std::clamp(theta, 0.001f, (float)M_PI);

        rot = glm::rotate(glm::mat4(1.f), (float)phi, glm::vec3(0.f, 1.f, 0.f));
        rot = glm::rotate(rot, (float)theta - 0.5f * (float)M_PI, right);
    }

    // Cámara libre
    pos += vel * (float)dt;

    front = glm::vec3(cos(phi - 0.5f * M_PI) * sin(theta), cos(theta), sin(phi - 0.5f * M_PI) * sin(theta));
    right = glm::normalize(glm::cross(front, up));

    //gl.view = glm::lookAt(pos, pos + front, up);
}
