// Inicialización y gestión de OpenGL
#pragma once

#include <functional>

#include "debug.h"
#include "window.h"
#include "gui.h"
#include "shaders.h"

namespace tofu
{
    // Inicializar OpenGL
    inline void initGL(ui16 w, ui16 h, str nombre) {
        // Creamos la estructura de datos de OpenGL
        gl = std::make_unique<GL>();
        gl->win = crearContexto(w, h, nombre);

        // Cargar OpenGL con GLAD
        if (not gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log::error("No se han podido cargar los símbolos de OpenGL con GLAD");
            std::exit(-1);
        }

        // Cargar ImGui
        gui::init();

        // Colores para clear
        glClearDepth(1.0f);
        glClearColor(0.05f, 0.0f, 0.2f, 1.0f);

        // Profundidad
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // Transparencia
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Multisamplin
        glEnable(GL_MULTISAMPLE);

        // Matriz de perspectiva
        ajustarPerspectiva(w, h);

        debug::gl();
    }

    // Bucle de la aplicación
    using update_fun_t = std::function<void()>;
    inline bool update(update_fun_t render = []{}, update_fun_t gui_render = []{}) {
        // Metricas de debug
        #ifdef DEBUG
        debug::prev_time = debug::curr_time;
        debug::curr_time = debug::time();
        debug::frame_time = debug::curr_time - debug::prev_time;
        debug::num_draw = 0;
        debug::num_instancias = 0;
        debug::num_triangulos = 0;
        debug::num_vertices = 0;
        #endif

        // Limpiar la pantalla antes de seguir
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl->instancia_base = 0;

        // Llamar a los comandos de renderizados especificados
        TIME(render(), debug::render_usuario_time);
        TIME(gui::render(gui_render), debug::render_gui_time);

        // Cambiar los buffers y presentar a pantalla
        TIME(glfwSwapBuffers(gl->win), debug::present_time);

        // Limpiar el estado de las teclas y volver a llamar a los eventos
        for (auto& [c, t] : gl->io.teclas) {
            t.liberada = false;
            t.presionada = false;
        }
        gl->io.mouse.xoff = 0; gl->io.mouse.yoff = 0;
        glfwPollEvents();

        debug::gl();
        return not glfwWindowShouldClose(gl->win);
    }

    // Dibujar objeto por instancias
    inline void dibujar(ui32 n, str geom) {
        // Métricas de debug
        #ifdef DEBUG
        debug::num_instancias += n;
        debug::num_vertices += gl->geometrias[geom].vcount * n;
        debug::num_triangulos += gl->geometrias[geom].icount * n / 3;
        #endif

        if (not gl->geometrias.count(geom)) {
            log::error("No se ha encontrado la geometría con nombre: ", geom);
            return;
        }

        // Actualizamos la instancia base (todas las shaders tienen que tener un uniform baseins)
        shader::uniform("baseins", gl->instancia_base);

        // Sin índices
        if (gl->geometrias[geom].icount == 0) {
            #ifdef DEBUG
            if (debug::usar_instancias) {
                glDrawArraysInstanced(
                    gl->geometrias[geom].tipo_dibujo,
                    gl->geometrias[geom].voff / gl->v_offset,
                    gl->geometrias[geom].vcount / gl->v_offset,
                    n);
                debug::num_draw++;
            } else {
                for (ui32 i = 1; i <= n; i++) {
                    glDrawArrays(
                        gl->geometrias[geom].tipo_dibujo,
                        gl->geometrias[geom].voff / gl->v_offset,
                        gl->geometrias[geom].vcount / gl->v_offset);
                    shader::uniform<int>("baseins", gl->instancia_base + i);
                    debug::num_draw++;
                }
            }
            #else
            glDrawArraysInstanced(
                gl->geometrias[geom].tipo_dibujo,
                gl->geometrias[geom].voff / gl->v_offset,
                gl->geometrias[geom].vcount / gl->v_offset,
                n);
            #endif
        }
        // Con índices
        else {
            #ifdef DEBUG
            if (debug::usar_instancias) {
                glDrawElementsInstancedBaseVertex(
                    gl->geometrias[geom].tipo_dibujo,
                    gl->geometrias[geom].icount,
                    GL_UNSIGNED_INT,
                    (void*)(gl->geometrias[geom].ioff * sizeof(ui32)),
                    n, 
                    gl->geometrias[geom].voff / gl->v_offset);
                debug::num_draw++;
            } else {
                debug::num_draw--;
                for (ui32 i = 1; i <= n; i++) {
                    glDrawElementsBaseVertex(
                        gl->geometrias[geom].tipo_dibujo,
                        gl->geometrias[geom].icount,
                        GL_UNSIGNED_INT,
                        (void*)(gl->geometrias[geom].ioff * sizeof(ui32)),
                        gl->geometrias[geom].voff / gl->v_offset);
                    shader::uniform<int>("baseins", gl->instancia_base + i);
                    debug::num_draw++;
                }
            }
            #else
            glDrawElementsInstancedBaseVertex(
                gl->geometrias[geom].tipo_dibujo,
                gl->geometrias[geom].icount,
                GL_UNSIGNED_INT,
                (void*)(gl->geometrias[geom].ioff * sizeof(ui32)),
                n, 
                gl->geometrias[geom].voff / gl->v_offset);
            #endif
        }

        gl->instancia_base += n;
        debug::gl();
    }

    // Limpieza
    inline void terminarGL() {
        gui::terminar();

        glDeleteVertexArrays(1, &gl->VAO);

        for (auto& [i, b] : gl->buffers)
            glDeleteBuffers(1, &b.buffer);

        for (auto& [i, t] : gl->texturas)
            glDeleteTextures(1, &t.textura);

        for (auto& [n, s] : gl->shaders)
            glDeleteProgram(s.pid);

        debug::gl();
        glfwTerminate();
    }
}
