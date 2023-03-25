// Estructura con los datos de la grúa
// Compatible con las versiones en 1.2 y 3.3
#pragma once

#include <array>
#include <vector>
#include <cstdint>

using v3 = std::array<float, 3>;
using ui32 = std::uint32_t;

// Posiciones relativas de los objetos entre si
enum PosicionamientoRelativo {
    POS_CENTRO,
    POS_ENCIMA,
    POS_DEBAJO,
    POS_IZQUIERDA,
    POS_DERECHA,
    POS_DELANTE,
    POS_DETRAS,
};

// Piezas de la grúa
struct PiezaGrua {
    int id, padre;
    PosicionamientoRelativo rel;
    v3 pos_rel;
    v3 escala;
    v3 color;
    float angulo;

    PiezaGrua(v3 p_pos_rel, v3 p_escala, v3 p_color, int p_padre = -1, PosicionamientoRelativo p_rel = POS_CENTRO, float p_angulo = 0.f)
        : pos_rel(p_pos_rel), escala(p_escala), color(p_color), padre(p_padre), rel(p_rel), angulo(p_angulo) {
        static int count = 0;
        id = count++;
    }
};

enum PiezasImportantes {
    PIEZA_BASE = 1,
    PIEZA_TORRE = 4,
    PIEZA_BRAZO = 5,
    PIEZA_CABLE = 6
};

// Lista de piezas de la grua
// Tienen que estar ordenadas por su dependencia de padres
inline std::vector<PiezaGrua> piezas_grua = {
    { {0.f, 0.f, 0.f}, {60.f, 1.f, 60.f}, {0.3f, 0.2f, 0.5f} }, // Suelo
    { {0.f, -2.f, 0.f}, {3.f, 0.5f, 4.f}, {1.f, 1.f, 0.5f} }, // Base
    { {0.f, 0.f, 0.f}, {1.f, 1.f, 4.5f}, {0.7f, 1.f, 0.9f}, PIEZA_BASE, POS_IZQUIERDA }, // Pie izquierdo de la base
    { {0.f, 0.f, 0.f}, {1.f, 1.f, 4.5f}, {0.7f, 1.f, 0.9f}, PIEZA_BASE, POS_DERECHA }, // Pie derecho de la base
    { {0.f, 0.f, 0.f}, {1.f, 8.f, 1.f}, {0.5f, 0.5f, 1.f}, PIEZA_BASE, POS_ENCIMA }, // Torre central
    { {-4.f, 0.f, 0.f}, {8.f, 1.f, 1.f}, {1.f, 0.5f, 0.5f}, PIEZA_TORRE, POS_ENCIMA }, // Brazo
    { {-7.f, 0.f, 0.f}, {0.1f, 6.f, 0.1f}, {1.f, 0.8f, 1.f}, PIEZA_BRAZO, POS_DEBAJO }, // Cable
    { {0.f, 0.f, 0.f}, {0.5f, 0.5f, 0.5f}, {1.f, 0.9f, 0.5f}, PIEZA_CABLE, POS_DEBAJO }, // Gancho
};

// Obtiene la posición relativa de una pieza
inline v3 posRelativa(ui32 pieza) {
    PiezaGrua& p = piezas_grua[pieza];
    PiezaGrua* padre = p.padre < 0 ? nullptr : &piezas_grua[p.padre];

    v3 pos = p.pos_rel;
    switch (p.rel) {
        case POS_CENTRO:
            break;
        case POS_ENCIMA:
            pos[1] -= padre->escala[1] + p.escala[1];
            break;
        case POS_DEBAJO:
            pos[1] += padre->escala[1] + p.escala[1];
            break;
        case POS_IZQUIERDA:
            pos[0] -= padre->escala[0] + p.escala[0];
            break;
        case POS_DERECHA:
            pos[0] += padre->escala[0] + p.escala[0];
            break;
        case POS_DELANTE:
            pos[2] -= padre->escala[2] + p.escala[2];
            break;
        case POS_DETRAS:
            pos[2] += padre->escala[2] + p.escala[2];
            break;
    }

    return pos;
}

inline struct Controles {
    bool delante, detras, girar_der, girar_izq;
    bool torre_der, torre_izq, torre_arriba, torre_abajo;
    bool brazo_extender, brazo_contraer;
    bool cable_soltar, cable_recoger;
} controles;

// Controlar la grua
void controlarGrua() {
    // Mover base
    if (controles.delante)
        piezas_grua[PIEZA_BASE].pos_rel[2] -= 0.1f;
    if (controles.detras)
        piezas_grua[PIEZA_BASE].pos_rel[2] += 0.1f;

    // Mover torre
    if (controles.torre_der)
        piezas_grua[PIEZA_TORRE].angulo += 0.1f;
    if (controles.torre_izq)
        piezas_grua[PIEZA_TORRE].angulo -= 0.1f;
    if (controles.torre_arriba)
        piezas_grua[PIEZA_TORRE].escala[1] = std::clamp(piezas_grua[PIEZA_TORRE].escala[1] + 0.05f, 3.f, 10.f);
    if (controles.torre_abajo)
        piezas_grua[PIEZA_TORRE].escala[1] = std::clamp(piezas_grua[PIEZA_TORRE].escala[1] - 0.05f, 3.f, 10.f);

    // Mover brazo
    if (controles.brazo_extender and piezas_grua[PIEZA_BRAZO].escala[0] < 12.f) {
        piezas_grua[PIEZA_BRAZO].escala[0] += 0.1f;
        piezas_grua[PIEZA_BRAZO].pos_rel[0] -= 0.1f;
        piezas_grua[PIEZA_CABLE].pos_rel[0] -= 0.1f;
    }
    if (controles.brazo_contraer and piezas_grua[PIEZA_BRAZO].escala[0] > 6.f) {
        piezas_grua[PIEZA_BRAZO].escala[0] -= 0.1f;
        piezas_grua[PIEZA_BRAZO].pos_rel[0] += 0.1f;
        piezas_grua[PIEZA_CABLE].pos_rel[0] += 0.1f;
    }

    // Mover cable
    if (controles.cable_recoger)
        piezas_grua[PIEZA_CABLE].escala[1] -= 0.1f;
    if (controles.cable_soltar)
        piezas_grua[PIEZA_CABLE].escala[1] += 0.1f;
    if (piezas_grua[PIEZA_CABLE].escala[1] < 2.f)
        piezas_grua[PIEZA_CABLE].escala[1] = 2.f;
    if (piezas_grua[PIEZA_CABLE].escala[1] - piezas_grua[PIEZA_TORRE].escala[1] > 0.f)
        piezas_grua[PIEZA_CABLE].escala[1] = piezas_grua[PIEZA_TORRE].escala[1];
}

