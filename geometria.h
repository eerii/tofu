// Generador de geometría
#pragma once

#include "debug.h"

namespace tofu
{
    namespace detail
    {
        // Generador de puntos de fibonacci para la esfera
        inline std::array<float, 3> fib(ui32 i, ui32 n) {
            float k = i + .5f;

            float phi = std::acos(1.f - 2.f * k / n);
            float theta = M_PI * k * (1.f + std::sqrt(5.f));

            return {std::cos(theta) * std::sin(phi),
                    std::sin(theta) * std::sin(phi),
                    std::cos(phi)};
        }

        // Slerp
        inline std::array<float, 3> slerp(std::array<float, 3> v1, std::array<float, 3> v2, float t) {
            float dot = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
            float theta = std::acos(dot);
            float sinTheta = std::sin(theta);

            float s1 = std::sin((1.f - t) * theta) / sinTheta;
            float s2 = std::sin(t * theta) / sinTheta;

            return {s1 * v1[0] + s2 * v2[0],
                    s1 * v1[1] + s2 * v2[1],
                    s1 * v1[2] + s2 * v2[2]};
        }

        // Puntos de un plano
        const std::vector<float> plane_vert = {
             1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f,
            -1.f,-1.f, 0.f,
             1.f,-1.f, 0.f
        };
        const std::vector<ui32> plane_ind = {
            0, 2, 1,
            0, 3, 2
        };

        // Puntos de un cubo
        const std::vector<float> cube_vert = {
             1.f, 1.f, 1.f,
             1.f, 1.f,-1.f,
            -1.f, 1.f,-1.f,
            -1.f, 1.f, 1.f,
             1.f,-1.f, 1.f,
             1.f,-1.f,-1.f,
            -1.f,-1.f,-1.f,
            -1.f,-1.f, 1.f
        };
        const std::vector<ui32> cube_ind = {
            0, 2, 1, 0, 3, 2, // Arriba
            4, 5, 6, 4, 6, 7, // Abajo
            0, 1, 5, 0, 5, 4, // Frente
            3, 6, 2, 3, 7, 6, // Atrás
            0, 7, 3, 0, 4, 7, // Izquierda
            1, 2, 6, 1, 6, 5  // Derecha
        };

        // Puntos de un octaedro
        const std::vector<float> oct_vert = {
             0.f, 1.f, 0.f, // Arriba
             1.f, 0.f, 0.f,
             0.f, 0.f, 1.f,
            -1.f, 0.f, 0.f,
             0.f, 0.f,-1.f,
             0.f,-1.f, 0.f  // Abajo
        };
    }

    namespace geometria
    {
        // Plano
        inline auto plano() {
            return std::make_pair(detail::plane_vert, detail::plane_ind);
        }

        // Cubo
        inline auto cubo() {
            return std::make_pair(detail::cube_vert, detail::cube_ind);
        }

        // Esfera a partir de octaedro
        // Creamos las subdivisiones dividiendo cada arista n veces, no como se hace tradicionalmente, para tener más control sobre la división
        inline auto esferaOct(ui32 n) {
            if (n < 1) {
                log::error("El nivel de subdivisión debe ser mayor o igual a 1");
                std::exit(1);
            }

            using V = std::array<float, 3>;
            using I = std::array<ui32, 3>;
            auto& ov = detail::oct_vert;
            std::vector<float> vertices;
            std::vector<ui32> indices;

            // Primer punto
            V v0 = {ov[0], ov[1], ov[2]};
            vertices.insert(vertices.end(), v0.begin(), v0.end());
            //log::info("v0[{}] {} {} {}", vertices.size() / 3 - 1, v0[0], v0[1], v0[2]);

            // Iteramos por cada subdivisión del eje
            for (ui32 i = 1; i < 2*n; i++) {
                // Ahora iteramos por cada punto intermedio (1-4)
                for (ui32 j = 1; j <= 4; j++) {
                    // Eje 0 - j
                    V v1 = {ov[j * 3], ov[j * 3 + 1], ov[j * 3 + 2]};
                    // Eje 0 - (j-1)
                    ui32 k = (j == 1) ? 4 : j - 1;
                    V v2 = {ov[k * 3], ov[k * 3 + 1], ov[k * 3 + 2]};
                    // Punto intermedio de este eje
                    auto va = detail::slerp(v0, v1, (float)i / n);

                    // Punto intermedio del eje anterior
                    auto vb = detail::slerp(v0, v2, (float)i / n);

                    // Añadimos las subdivisiones entre los dos ejes
                    int ii = (i < n) ? i : 2*n - i;
                    for (ui32 l = 1; l < ii; l++) {
                        auto vc = detail::slerp(va, vb, (float)(ii-l) / ii);
                        vertices.insert(vertices.end(), vc.begin(), vc.end());
                        // log::info("vs[{}] {} {} {}", vertices.size() / 3 - 1, vc[0], vc[1], vc[2]);
                    }

                    // Y finalmente la subdivisión de este eje
                    vertices.insert(vertices.end(), va.begin(), va.end());
                    // log::info("v{}[{}] {} {} {}", j, vertices.size() / 3 - 1, va[0], va[1], va[2]);
                }
            }

            // Ultimo punto
            V vn = {ov[15], ov[16], ov[17]};
            vertices.insert(vertices.end(), vn.begin(), vn.end());
            // log::info("vn[{}] {} {} {}", vertices.size() / 3 - 1, vn[0], vn[1], vn[2]);

            // Triangulación
            // Recorremos horizontalmente los niveles
            ui32 vert_acc = 1;
            ui32 vert_anterior = 0;
            I ind = {0, 0, 0}, ind_ccw = {0, 0, 0};
            for (ui32 i = 1; i < n+1; i++) {
                ui32 tri = (2 * (i-1) + 1) * 4;
                ui32 vert = i * 4;
                ui32 primer_vertice = vert_acc + i-1;

                // Iniciamos los primeros índices
                ind[0] = primer_vertice;
                ind[1] = vert_anterior;
                ind[2] = ind[0] + 1;

                // Recorremos cada triángulo que hay que crear
                for (ui32 j = 0; j < tri; j++) {
                    //log::info("{} - {} {} {}", j, ind[0], ind[1], ind[2]);

                    // Corregimos e insertamos los índices
                    ind_ccw = ind;
                    if (ind[0] > ind[1])
                        std::swap(ind_ccw[0], ind_ccw[1]);
                    indices.insert(indices.end(), ind_ccw.begin(), ind_ccw.end());

                    // Actualizamos los índices
                    if ((j+1) % (tri/4) == 0) { // Caso borde
                        ind[0] = ind[2];
                    } else {
                        ind[0] = ind[1];
                        ind[1] = ind[2];
                    }
                    ind[2] = ind[0] + 1;

                    // Repetición circular
                    if (ind[2] == vert_acc) ind[2] = vert_anterior - (i-2);
                    if (ind[2] == vert_acc + i*4) ind[2] = vert_acc;
                }

                vert_acc += vert;
                vert_anterior = primer_vertice;
            }

            // Recorremos los niveles inferiores
            vert_acc = 0;
            for (ui32 i = 1; i < 2*n; i++)
                vert_acc += ((i < n) ? i : 2*n - i) * 4;
            vert_anterior = vert_acc + 1;

            for (ui32 i = 1; i < n+1; i++) {
                ui32 tri = (2 * (i-1) + 1) * 4;
                ui32 vert = i * 4;
                ui32 primer_vertice = vert_acc;

                // Iniciamos los primeros índices
                ind[0] = primer_vertice;
                ind[1] = vert_anterior;
                ind[2] = ind[0] - 1;

                // Recorremos cada triángulo que hay que crear
                for (ui32 j = 0; j < tri; j++) {
                    //log::info("{} - {} {} {}", j, ind[0], ind[1], ind[2]);

                    // Corregimos e insertamos los índices
                    ind_ccw = ind;
                    if (ind[0] < ind[1])
                        std::swap(ind_ccw[0], ind_ccw[1]);
                    indices.insert(indices.end(), ind_ccw.begin(), ind_ccw.end());

                    // Actualizamos los índices
                    if ((j+1) % (tri/4) == 0) { // Caso borde
                        ind[0] = ind[2];
                    } else {
                        ind[0] = ind[1];
                        ind[1] = ind[2];
                    }
                    ind[2] = ind[0] - 1;

                    // Repetición circular
                    if (ind[2] == vert_acc) ind[2] = vert_anterior;
                    if (ind[2] == vert_acc - i*4) ind[2] = vert_acc;
                }
               
                vert_acc -= vert;
                vert_anterior = primer_vertice;
            }

            return std::make_pair(vertices, indices);
        }
    }
}
