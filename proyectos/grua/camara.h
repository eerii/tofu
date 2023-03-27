// Implementación de una cámara libre en 3D
#pragma once

#include "grua.h"

// ---

namespace cam
{
    inline v3 pos = {0.f, -20.f, 50.f};
    inline v3 vel = {0.f, 0.f, 0.f};

    inline float phi = 0.f;
    inline float theta = 0.5f * M_PI - 0.2f;
    inline v2 vel_raton = {0.f, 0.f};
    inline v3 up = {0.f, 1.f, 0.f};
    inline v3 front, right;

    const float aceleracion = 2.f;
    const float deceleracion = 0.9f;
    const float vel_max = 20.f;
    const float vel_min = 0.05f;

    const float acc_raton = 0.6f;
    const float dec_raton = 0.7f;
    const float raton_max = 4.f;

    const float DT = 1.f / 60.f;
}

// ---

inline float sign(float x) {
    return x < 0.f ? -1.f : 1.f;
}

inline float vlen(v3 v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
}

inline v3 vnormalize(v3 v) {
    return v * (1.f / vlen(v));
}

inline v3 vcross(v3 a, v3 b) {
    return v3(a.y() * b.z() - a.z() * b.y(),
              a.z() * b.x() - a.x() * b.z(),
              a.x() * b.y() - a.y() * b.x());
}

inline void camara() {
    using namespace cam;

    // Movimiento delante-detras
    if (controles.cam_delante)
        vel += front * aceleracion;
    if (controles.cam_detras)
        vel += front * -aceleracion;
    if (not controles.cam_delante and not controles.cam_detras)
        vel.x() *= deceleracion;

    // Movimiento izquierda-derecha
    if (controles.cam_der)
        vel += right * aceleracion;
    if (controles.cam_izq)
        vel += right * -aceleracion;
    if (not controles.cam_der and not controles.cam_izq)
        vel.z() *= deceleracion;

    // Movimiento arriba-abajo
    if (controles.cam_abajo)
        vel += up * aceleracion;
    if (controles.cam_arriba)
        vel += up * -aceleracion;
    if (not controles.cam_abajo and not controles.cam_arriba)
        vel.y() *= deceleracion;

    // Velocidad máxima y mínima
    if (vlen(vel) > vel_max)
        vel = vnormalize(vel) * vel_max;
    if (vlen(vel) < vel_min)
        vel = v3();

    // Giro
    if (gl.raton_conectado) {
        // Yaw - Ratón X
        vel_raton.x() += controles.raton_offx * acc_raton;
        // Pitch - Ratón Y
        vel_raton.y() += controles.raton_offy * acc_raton;

        // Deceleración
        vel_raton *= dec_raton;

        // Ratón máximo y mínimo
        if (abs(vel_raton.x()) > raton_max)
            vel_raton.x() = raton_max * sign(vel_raton.x());
        if (abs(vel_raton.y()) > raton_max)
            vel_raton.y() = raton_max * sign(vel_raton.y());
        if (abs(vel_raton.x()) < 0.1f)
            vel_raton.x() = 0.f;
        if (abs(vel_raton.y()) < 0.1f)
            vel_raton.y() = 0.f;
        
        // Añadir giros
        phi += vel_raton.x() * dt;
        phi = std::fmod(phi, 2.f * M_PI);

        theta += vel_raton.y() * dt;
        theta = std::clamp(theta, 0.001f, (float)M_PI);
    }

    // Cámara libre
    pos += vel * DT;
    front = v3(cos(phi - 0.5f * M_PI) * sin(theta), cos(theta), sin(phi - 0.5f * M_PI) * sin(theta));
    right = vcross(front, up);
}
