// Crear VAOs, VBOs y otros buffers
#pragma once

#include <numeric>

#include "debug.h"

namespace tofu
{
    namespace buffer
    {
        // Crea un buffer y lo rellena con los datos indicados
        template <typename T>
        Buffer crear(ui32 tipo, std::vector<T> datos = {}, ui32 modo = GL_STATIC_DRAW) {
            ui32 tam = datos.size();
            Buffer buf {
                .tipo = tipo,
                .modo = modo,
                .tam = tam,
                .bytes = sizeof(T)
            };


            glGenBuffers(1, &buf.buffer);
            if (datos.size() > 0) {
                glBindBuffer(buf.tipo, buf.buffer);
                glBufferData(buf.tipo, buf.tam * buf.bytes, datos.data(), buf.modo);
            }

            debug::gl();
            return buf;
        }

        // Redimensionar un buffer usando copy buffers
        inline void redimensionar(Buffer& buf, ui32 tam_nuevo) {
            glBindBuffer(GL_COPY_READ_BUFFER, buf.buffer);

            // Crear nuevo buffer
            ui32 nuevo;
            glGenBuffers(1, &nuevo);
            glBindBuffer(GL_COPY_WRITE_BUFFER, nuevo);
            glBufferData(GL_COPY_WRITE_BUFFER, tam_nuevo * buf.bytes, nullptr, GL_DYNAMIC_DRAW);

            // Copiar datos al principio
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, buf.tam * buf.bytes);

            // Eliminar el buffer antiguo
            glDeleteBuffers(1, &buf.buffer);

            // Guardamos la nueva referencia
            buf.buffer = nuevo;
            buf.tam = tam_nuevo;

            debug::gl();
        }

        // Cargar datos en un buffer
        template <typename T>
        void cargar(Buffer& buf, std::vector<T> datos, ui32 pos = 0) {
            ui32 tam = datos.size();
            
            // Redimensionar el buffer si es necesario
            if (pos + tam > buf.tam)
                redimensionar(buf, pos + tam);

            // Cargamos los datos
            glBindBuffer(buf.tipo, buf.buffer);
            glBufferSubData(buf.tipo, pos * buf.bytes, tam * buf.bytes, datos.data());

            debug::gl();
        }

        // Inicializamos los buffers principales de OpenGL
        inline void iniciarVAO(std::vector<ui32> attr, ui32 vert_alloc = 0, ui32 ind_alloc = 0) {
            // VAO
            glGenVertexArrays(1, &gl->VAO);

            // Creamos los buffers de vértices e índices
            gl->VBO = crear(GL_ARRAY_BUFFER, std::vector<float>(vert_alloc), GL_STATIC_DRAW);
            gl->EBO = crear(GL_ELEMENT_ARRAY_BUFFER, std::vector<ui32>(ind_alloc), GL_STATIC_DRAW);

            // Guardamos los atributos del VAO
            gl->atributos = attr;

            debug::gl();
        }

        // Configurar el VAO y sus atributos
        inline void configurarVAO(ui32 VAO) {
            if (gl->atributos.size() == 0) {
                log::error("No se han especificado los atributos del VAO");
                std::exit(-1);
            }

            glBindVertexArray(VAO);
            ui32 tam_total = std::accumulate(gl->atributos.begin(), gl->atributos.end(), 0);
            gl->v_offset = 0;
            for (ui32 i = 0; i < gl->atributos.size(); i++) {
                glVertexAttribPointer(i, gl->atributos[i], GL_FLOAT, GL_FALSE, tam_total * sizeof(float), (void*)(gl->v_offset * sizeof(float)));
                glEnableVertexAttribArray(i);
                gl->v_offset += gl->atributos[i];
            }

            debug::gl();
        }

        // Obtener la posición libre de los vértices e índices
        inline Geometria ultimaPosVert() {
            Geometria pos = {0, 0, 0, 0, 0};
            ui32 max_i = 0;
            for (auto& [n, p] : gl->geometrias) {
                if (p.voff + p.vcount > max_i) {
                    max_i = p.voff + p.vcount;
                    pos = p;
                }
            }
            return pos;
        }

        // Cargar los datos de los vértices en la GPU (sin índices)
        inline void cargarVert(str nombre, std::vector<float> vertices, ui32 tipo_dibujo = GL_TRIANGLES) {
            // Activar el VAO
            glBindVertexArray(gl->VAO);

            // Obtener últimas posiciones utilizadas
            Geometria pos = ultimaPosVert();
            pos.voff += pos.vcount;
            pos.vcount = vertices.size();
            pos.icount = 0;
            pos.tipo_dibujo = tipo_dibujo;

            // Añadir vértices
            cargar(gl->VBO, vertices, pos.voff);

            // Guardar la posición de la geometría
            gl->geometrias[nombre] = pos;

            debug::gl();
            configurarVAO(gl->VAO);
        }

        // Cargar los datos de los vértices en la GPU (con índices)
        inline void cargarVert(str nombre, std::pair<std::vector<float>, std::vector<ui32>> vertices, ui32 tipo_dibujo = GL_TRIANGLES) {
            // Activar el VAO
            glBindVertexArray(gl->VAO);

            // Obtener últimas posiciones utilizadas
            Geometria pos = ultimaPosVert();
            pos.voff += pos.vcount;
            pos.vcount = vertices.first.size();
            pos.ioff += pos.icount;
            pos.icount = vertices.second.size();
            pos.tipo_dibujo = tipo_dibujo;

            // Añadir vértices e índices
            cargar(gl->VBO, vertices.first, pos.voff);
            cargar(gl->EBO, vertices.second, pos.ioff);

            // Guardar la posición de la geometría
            gl->geometrias[nombre] = pos;

            debug::gl();
            configurarVAO(gl->VAO);
        }
    }
}
