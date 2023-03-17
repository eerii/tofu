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
        ui32 crear(ui32 tipo, std::vector<T> datos = {}, ui32 modo = GL_STATIC_DRAW) {
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

            ui32 id = gl->buffers.size();
            gl->buffers[id] = buf;
            return id;
        }

        // Redimensionar un buffer usando copy buffers
        inline void redimensionar(ui32 buffer, ui32 tam_nuevo) {
            Buffer& buf = gl->buffers[buffer];
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
        void cargar(ui32 buffer, std::vector<T> datos, ui32 pos = 0) {
            Buffer& buf = gl->buffers[buffer];
            ui32 tam = datos.size();
            
            // Redimensionar el buffer si es necesario
            if (pos + tam > buf.tam)
                redimensionar(buffer, pos + tam);

            // Cargamos los datos
            glBindBuffer(buf.tipo, buf.buffer);
            glBufferSubData(buf.tipo, pos * buf.bytes, tam * buf.bytes, datos.data());

            debug::gl();
        }

        // Inicializamos los buffers principales de OpenGL
        inline void iniciarVAO(std::vector<ui32> attr, str n = "main", ui32 vert_alloc = 0, ui32 ind_alloc = 0) {
            // VAO
            glGenVertexArrays(1, &gl->VAOs[n].vao);

            // Creamos los buffers de vértices e índices
            gl->VAOs[n].vbo = crear(GL_ARRAY_BUFFER, std::vector<float>(vert_alloc), GL_STATIC_DRAW);
            gl->VAOs[n].ebo = crear(GL_ELEMENT_ARRAY_BUFFER, std::vector<ui32>(ind_alloc), GL_STATIC_DRAW);

            // Guardamos los atributos del VAO
            gl->VAOs[n].atributos = attr;

            debug::gl();
        }

        // Configurar el VAO y sus atributos
        inline void configurarVAO(str n) {
            if (gl->VAOs.find(n) == gl->VAOs.end()) {
                log::error("No existe el VAO " + n);
                std::exit(-1);
            }
            VAO& v = gl->VAOs[n];
            if (v.atributos.size() == 0) {
                log::error("No se han especificado los atributos del VAO");
                std::exit(-1);
            }

            glBindVertexArray(v.vao);
            ui32 tam_total = std::accumulate(v.atributos.begin(), v.atributos.end(), 0);
            ui32 attr_offset = 0;
            for (ui32 i = 0; i < v.atributos.size(); i++) {
                glVertexAttribPointer(i, v.atributos[i], GL_FLOAT, GL_FALSE, tam_total * sizeof(float), (void*)(attr_offset * sizeof(float)));
                glEnableVertexAttribArray(i);
                attr_offset += v.atributos[i];
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
        inline void cargarVert(str nombre, std::vector<float> vertices, str vao = "main", ui32 tipo_dibujo = GL_TRIANGLES) {
            // Activar el VAO
            glBindVertexArray(gl->VAOs[vao].vao);

            // Obtener últimas posiciones utilizadas
            Geometria pos = ultimaPosVert();
            pos.voff += pos.vcount;
            pos.vcount = vertices.size();
            pos.icount = 0;
            pos.tipo_dibujo = tipo_dibujo;

            // Añadir vértices
            cargar(gl->VAOs[vao].vbo, vertices, pos.voff);

            // Guardar la posición de la geometría
            gl->geometrias[nombre] = pos;

            debug::gl();
            configurarVAO(vao);
        }

        // Cargar los datos de los vértices en la GPU (con índices)
        inline void cargarVert(str nombre, std::pair<std::vector<float>, std::vector<ui32>> vertices, str vao = "main", ui32 tipo_dibujo = GL_TRIANGLES) {
            // Activar el VAO
            glBindVertexArray(gl->VAOs[vao].vao);

            // Obtener últimas posiciones utilizadas
            Geometria pos = ultimaPosVert();
            pos.voff += pos.vcount;
            pos.vcount = vertices.first.size();
            pos.ioff += pos.icount;
            pos.icount = vertices.second.size();
            pos.tipo_dibujo = tipo_dibujo;

            // Añadir vértices e índices
            cargar(gl->VAOs[vao].vbo, vertices.first, pos.voff);
            cargar(gl->VAOs[vao].ebo, vertices.second, pos.ioff);

            // Guardar la posición de la geometría
            gl->geometrias[nombre] = pos;

            debug::gl();
            configurarVAO(vao);
        }
    }

    namespace textura
    {
        // Crea una textura
        inline ui32 crear(ui32 target, ui32 formato, ui32 tipo, ui32 i_offset = 0, glm::ivec2 tam = glm::ivec2(0, 0)) {
            ui32 indice = i_offset;
            for (auto& [i, t] : gl->texturas)
                if (i == indice)
                    indice++;

            Textura tex {
                .target = target,
                .formato = formato,
                .tipo = tipo,
                .tam = tam,
            };

            glGenTextures(1, &tex.textura);
            debug::gl();

            gl->texturas[indice] = tex;
            return indice;
        }

        inline ui32 fi_a_formato(ui32 fint) {
            ui32 formato;

            if (fint == GL_R32F or fint == GL_R32I or fint == GL_R32UI)
                formato = GL_RED;
            else if (fint == GL_RG32F or fint == GL_RG32I or fint == GL_RG32UI)
                formato = GL_RG;
            else if (fint == GL_RGB32F or fint == GL_RGB32I or fint == GL_RGB32UI)
                formato = GL_RGB;
            else if (fint == GL_RGBA32F or fint == GL_RGBA32I or fint == GL_RGBA32UI)
                formato = GL_RGBA;
            else {
                log::error("Formato de textura no soportado");
                std::exit(-1);
            }

            return formato;
        }

        inline ui32 fi_a_tipo(ui32 fint) {
            ui32 tipo;

            if (fint == GL_R32F or fint == GL_RG32F or fint == GL_RGB32F or fint == GL_RGBA32F)
                tipo = GL_FLOAT;
            else if (fint == GL_R32I or fint == GL_RG32I or fint == GL_RGB32I or fint == GL_RGBA32I)
                tipo = GL_INT;
            else if (fint == GL_R32UI or fint == GL_RG32UI or fint == GL_RGB32UI or fint == GL_RGBA32UI)
                tipo = GL_UNSIGNED_INT;
            else {
                log::error("Formato de textura no soportado");
                std::exit(-1);
            }

            return tipo;
        }
    }

    namespace texbuffer
    {
        inline const ui32 texbuffer_offset = 8;

        // Crea un texture buffer
        template <typename T>
        TexBuffer crear(std::vector<T> datos = {}) {
            ui32 formato;
            if constexpr (std::is_same_v<T, float>)
                formato = GL_R32F;
            else if constexpr (std::is_same_v<T, glm::vec2>)
                formato = GL_RG32F;
            else if constexpr (std::is_same_v<T, glm::vec3>)
                formato = GL_RGB32F;
            else if constexpr (std::is_same_v<T, glm::vec4>)
                formato = GL_RGBA32F;
            else if constexpr (std::is_same_v<T, ui32>)
                formato = GL_R32UI;
            else if constexpr (std::is_same_v<T, glm::uvec2>)
                formato = GL_RG32UI;
            else if constexpr (std::is_same_v<T, glm::uvec3>)
                formato = GL_RGB32UI;
            else if constexpr (std::is_same_v<T, glm::uvec4>)
                formato = GL_RGBA32UI;
            else if constexpr (std::is_same_v<T, int>)
                formato = GL_R32I;
            else if constexpr (std::is_same_v<T, glm::ivec2>)
                formato = GL_RG32I;
            else if constexpr (std::is_same_v<T, glm::ivec3>)
                formato = GL_RGB32I;
            else if constexpr (std::is_same_v<T, glm::ivec4>)
                formato = GL_RGBA32I;
            else if constexpr (std::is_same_v<T, glm::mat4>)
                formato = GL_RGBA32F;
            else {
                log::error("Formato de textura buffer no soportado");
                std::exit(-1);
            }

            ui32 buffer = buffer::crear(GL_TEXTURE_BUFFER, datos, GL_STATIC_DRAW);
            ui32 textura = textura::crear(GL_TEXTURE_BUFFER, formato, 0, texbuffer_offset);
            return {buffer, textura};
        }
    }

    namespace framebuffer
    {
        inline ui32 crear(glm::ivec2 tam, glm::vec4 clear) {
            Framebuffer fb {
                .tam = glm::ivec3(tam, 1),
                .clear = clear,
            };

            glGenFramebuffers(1, &fb.fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

            // Calculamos la dimensión
            // - Su tamaño y determina si es 1D o 2D
            // - Si su tamaño en x es mayor que el tamaño máximo de una textura, creamos un array
            ui32 dimension = tam.y < 2 ? GL_TEXTURE_1D : GL_TEXTURE_2D;
            if (tam.x > gl->max_tex_size) {
                dimension = tam.y < 2 ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_2D_ARRAY;
                fb.tam.z = tam.x / gl->max_tex_size;
                fb.tam.x = gl->max_tex_size;
            }

            // Creamos la textura
            fb.tex = textura::crear(dimension, GL_RGBA32F, 0);
            Textura& tex = gl->texturas[fb.tex];
            glBindTexture(dimension, tex.textura);

            if (dimension == GL_TEXTURE_1D) { // 1D
                glTexImage1D(GL_TEXTURE_1D, 0, tex.formato, fb.tam.x, 0, textura::fi_a_formato(tex.formato), textura::fi_a_tipo(tex.formato), NULL);
                glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tex.textura, 0);
            } else if (dimension == GL_TEXTURE_2D) { // 2D
                glTexImage2D(GL_TEXTURE_2D, 0, tex.formato, fb.tam.x, fb.tam.y, 0, textura::fi_a_formato(tex.formato), textura::fi_a_tipo(tex.formato), NULL);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.textura, 0);
            } else if (dimension == GL_TEXTURE_1D_ARRAY) { // 1D pero hecho array
                glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, tex.formato, fb.tam.x, fb.tam.z, 0, textura::fi_a_formato(tex.formato), textura::fi_a_tipo(tex.formato), NULL);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.textura, 0);
                log::warn("Todavía no está implementado en las shaders usar un array de texturas para el framebuffer");
            } else {
                log::error("Dimension de framebuffer no soportada");
                std::exit(-1);
            }

            // Comprobamos que el framebuffer se haya creado bien
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                log::error("Error al crear framebuffer");
                debug::gl();
                std::exit(-1);
            }

            debug::gl();

            ui32 id = gl->buffers.size() + 1;
            gl->framebuffers[id] = fb;
            return id;
        }
    }
}
