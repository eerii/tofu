// Implementación de una cámara libre en 3D
#pragma once

#include "debug.h"

// ---

glm::vec3 cam_pos = glm::vec3(0.f, 0.f, 4.f);
glm::vec3 cam_vel = glm::vec3(0.f);

float yaw = 0.f;
float pitch = 0.5f * M_PI;
float roll;
glm::vec3 cam_up = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 cam_front, cam_right;

const float aceleracion = 0.01f;
const float deceleracion = 0.9f;
const float vel_max = 0.1f;
const float vel_min = 0.005f;

const float acc_raton = 0.005f;
const float raton_max = 0.08f;

// ---

namespace tofu
{
    inline void camera() {
        // Movimiento delante-detras
        if (gl->io.teclas[GLFW_KEY_W].mantenida)
            cam_vel += cam_front * aceleracion;
        if (gl->io.teclas[GLFW_KEY_S].mantenida)
            cam_vel += cam_front * -aceleracion;
        if (not gl->io.teclas[GLFW_KEY_W].mantenida and not gl->io.teclas[GLFW_KEY_S].mantenida)
            cam_vel.x *= deceleracion;

        // Movimiento izquierda-derecha
        if (gl->io.teclas[GLFW_KEY_D].mantenida)
            cam_vel += cam_right * aceleracion;
        if (gl->io.teclas[GLFW_KEY_A].mantenida)
            cam_vel += cam_right * -aceleracion;
        if (not gl->io.teclas[GLFW_KEY_A].mantenida and not gl->io.teclas[GLFW_KEY_D].mantenida)
            cam_vel.z *= deceleracion;

        // Movimiento arriba-abajo
        if (gl->io.teclas[GLFW_KEY_Q].mantenida)
            cam_vel += cam_up * aceleracion;
        if (gl->io.teclas[GLFW_KEY_E].mantenida)
            cam_vel += cam_up * -aceleracion;
        if (not gl->io.teclas[GLFW_KEY_E].mantenida and not gl->io.teclas[GLFW_KEY_Q].mantenida)
            cam_vel.y *= deceleracion;

        // Velocidad máxima
        if (glm::length(cam_vel) > vel_max)
            cam_vel = glm::normalize(cam_vel) * vel_max;
        if (glm::length(cam_vel) < vel_min)
            cam_vel = glm::vec3(0.f);

        // Aplicar velocidad
        cam_pos += cam_vel;

        // Yaw - Ratón X
        float yaw_d = gl->io.mouse.xoff * acc_raton;
        yaw += std::clamp(yaw_d, -raton_max, raton_max);
        yaw = std::fmod(yaw, 2.f * M_PI);

        // Pitch - Ratón Y
        float pitch_d = gl->io.mouse.yoff * acc_raton;
        pitch += std::clamp(pitch_d, -raton_max, raton_max);
        pitch = std::clamp(pitch, 0.f, (float)M_PI);

        // Calcular los nuevos vectores de cámara
        cam_front = glm::vec3(cos(yaw - 0.5f * M_PI) * sin(pitch), cos(pitch), sin(yaw - 0.5f * M_PI) * sin(pitch));
        cam_right = glm::normalize(glm::cross(cam_front, cam_up));

        // Matriz de cámara
        gl->view = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
    }
}
