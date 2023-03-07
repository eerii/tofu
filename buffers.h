// Crear VAOs, VBOs y otros buffers
#pragma once

#include <numeric>

#include "debug.h"

namespace tofu
{
    // Configurar el VAO y sus atributos
    inline void configurarVAO(ui32 VAO, const std::vector<ui32> atributos) {
        glBindVertexArray(VAO);
        ui32 tam_total = std::accumulate(atributos.begin(), atributos.end(), 0);
        ui32 offset = 0;
        for (ui32 i = 0; i < atributos.size(); i++) {
            glVertexAttribPointer(i, atributos[i], GL_FLOAT, GL_FALSE, tam_total * sizeof(float), (void*)(offset * sizeof(float)));
            glEnableVertexAttribArray(i);
            offset += atributos[i];
        }
    }

    // Cargar los datos de los vértices en la GPU
    void cargarVertices(std::pair<std::vector<float>, std::vector<ui32>> vertices, std::vector<ui32> atributos) {
        glBindVertexArray(gl->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, gl->VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.first.size() * sizeof(float), vertices.first.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.second.size() * sizeof(ui32), vertices.second.data(), GL_STATIC_DRAW);

        configurarVAO(gl->VAO, atributos);
    }
}
