// Inicialización y gestión de OpenGL
#pragma once

#include <functional>
#include <numeric>

#include "debug.h"
#include "window.h"
#include "gui.h"
#include "shaders.h"

namespace tofu
{
    // Inicializar OpenGL
    inline void initGL(ui16 w, ui16 h, str nombre) {
        // Creamos la estructura de datos de OpenGL
        gl.win = crearContexto(w, h, nombre);

        // Cargar OpenGL con GLAD
        NOWEB(if (not gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log::error("No se han podido cargar los símbolos de OpenGL con GLAD");
            std::exit(-1);
        })

        // Funciones extra de ventana
        windowSizeCallback(gl.win, w, h);

        // Cargar ImGui
        gui::init();

        // Configuración
        // Nota: Estos parámetros se activan dependiendo de la shader utilizada
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Multisampling
        glEnable(GL_MULTISAMPLE);

        // Obtener atributos
        // Tamaño máximo de textura
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&gl.max_tex_size);

        t = debug::time();        
        debug::gl();
    }

    // Bucle de la aplicación
    inline bool update(update_fun_t render = []{}, update_fun_t gui_render = []{}) {
        dt = debug::time() - t;
        t += dt;

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
        for (auto& [k, f] : gl.framebuffers) {
            glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);
            glClearColor(f.clear.r, f.clear.g, f.clear.b, f.clear.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearDepth(1.0f);
        glClearColor(0.05f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Resetear la instancia base
        gl.instancia_base = 0;

        // Llamar a los comandos de renderizados especificados
        TIME(render(), debug::render_usuario_time);
        TIME(gui::render(gui_render), debug::render_gui_time);

        // Cambiar los buffers y presentar a pantalla
        TIME(glfwSwapBuffers(gl.win), debug::present_time);

        // Limpiar el estado de las teclas y volver a llamar a los eventos
        for (auto& [c, t] : gl.io.teclas) {
            t.liberada = false;
            t.presionada = false;
        }
        gl.io.mouse.xoff = 0; gl.io.mouse.yoff = 0;
        glfwPollEvents();

        debug::gl();
        return not glfwWindowShouldClose(gl.win);
    }

    // Dibujar objeto por instancias
    inline void dibujar(ui32 n, str geom, str vao = "main") {
        // Métricas de debug
        #ifdef DEBUG
        debug::num_instancias += n;
        debug::num_vertices += gl.geometrias[geom].vcount * n;
        debug::num_triangulos += gl.geometrias[geom].icount * n / 3;
        #endif

        if (not gl.geometrias.count(geom)) {
            log::error("No se ha encontrado la geometría con nombre: ", geom);
            return;
        }

        // Actualizamos la instancia base (todas las shaders tienen que tener un uniform baseins)
        shader::uniform("baseins", gl.instancia_base);
        ui32 attr_offset = std::accumulate(gl.VAOs[vao].atributos.begin(), gl.VAOs[vao].atributos.end(), 0); 

        // Sin índices
        if (gl.geometrias[geom].icount == 0) {
            #ifdef DEBUG
            if (debug::usar_instancias) {
                glDrawArraysInstanced(
                    gl.geometrias[geom].tipo_dibujo,
                    gl.geometrias[geom].voff / attr_offset,
                    gl.geometrias[geom].vcount / attr_offset,
                    n);
                debug::num_draw++;
            } else {
                for (ui32 i = 1; i <= n; i++) {
                    glDrawArrays(
                        gl.geometrias[geom].tipo_dibujo,
                        gl.geometrias[geom].voff / attr_offset,
                        gl.geometrias[geom].vcount / attr_offset);
                    shader::uniform<int>("baseins", gl.instancia_base + i);
                    debug::num_draw++;
                }
            }
            #else
            glDrawArraysInstanced(
                gl.geometrias[geom].tipo_dibujo,
                gl.geometrias[geom].voff / attr_offset,
                gl.geometrias[geom].vcount / attr_offset,
                n);
            #endif
        }
        // Con índices
        else {
            #ifdef DEBUG
            if (debug::usar_instancias) {
                glDrawElementsInstancedBaseVertex(
                    gl.geometrias[geom].tipo_dibujo,
                    gl.geometrias[geom].icount,
                    GL_UNSIGNED_INT,
                    (void*)(gl.geometrias[geom].ioff * sizeof(ui32)),
                    n, 
                    gl.geometrias[geom].voff / attr_offset);
                debug::num_draw++;
            } else {
                debug::num_draw--;
                for (ui32 i = 1; i <= n; i++) {
                    glDrawElementsBaseVertex(
                        gl.geometrias[geom].tipo_dibujo,
                        gl.geometrias[geom].icount,
                        GL_UNSIGNED_INT,
                        (void*)(gl.geometrias[geom].ioff * sizeof(ui32)),
                        gl.geometrias[geom].voff / attr_offset);
                    shader::uniform<int>("baseins", gl.instancia_base + i);
                    debug::num_draw++;
                }
            }
            #else
            glDrawElementsInstancedBaseVertex(
                gl.geometrias[geom].tipo_dibujo,
                gl.geometrias[geom].icount,
                GL_UNSIGNED_INT,
                (void*)(gl.geometrias[geom].ioff * sizeof(ui32)),
                n, 
                gl.geometrias[geom].voff / attr_offset);
            #endif
        }

        gl.instancia_base += n;
        debug::gl();
    }

    // Limpieza
    inline void terminarGL() {
        gui::terminar();

        for (auto& [n, v] : gl.VAOs) {
            glDeleteVertexArrays(1, &v.vao);
            glDeleteBuffers(1, &v.vbo);
            glDeleteBuffers(1, &v.ebo);
        }

        for (auto& [i, b] : gl.buffers)
            glDeleteBuffers(1, &b.buffer);

        for (auto& [i, t] : gl.texturas)
            glDeleteTextures(1, &t.textura);

        for (auto& [i, f] : gl.framebuffers)
            glDeleteFramebuffers(1, &f.fbo);

        for (auto& [n, s] : gl.shaders)
            glDeleteProgram(s.pid);

        debug::gl();
        glfwTerminate();
    }
}
