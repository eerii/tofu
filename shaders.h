// Cargar y procesar shaders GLSL
#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include "debug.h"

namespace fs = std::filesystem;

namespace tofu
{
    namespace detail
    {
        inline std::optional<str> leerArchivo(fs::path path) {
            // Comprobamos que el directorio existe
            if (not fs::exists(path))
                return std::nullopt;

            // Abrimos el fichero
            std::ifstream in(path.string());
            if (not in.is_open()) {
                log::error("No se ha podido abrir el archivo: {}", path.string());
                std::exit(-1);
            }

            // Devolvemos su contenido
            str src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            return src;
        }

        inline void compilarShader(ui32& id, str src) {
            const char* src_c = src.c_str();
            int result, log_len;

            // Compilamos
            glShaderSource(id, 1, &src_c, NULL);
            glCompileShader(id);

            // Comprobamos errores
            glGetShaderiv(id, GL_COMPILE_STATUS, &result);
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
            if (log_len > 0) {
                str error(log_len, ' ');
                glGetShaderInfoLog(id, log_len, NULL, &error[0]);
                log::warn("No se pudo compilar el shader: {}", error);
            }

            debug::gl();
        }

        inline ui32 cargarShader(str nombre, const std::vector<str>& transform_feedback_var = {}) {
            ui32 pid = glCreateProgram();
            ui32 vid, fid, gid;

            // Leemos los ficheros de shaders
            auto vsrc = leerArchivo(fs::current_path() / (nombre + ".vert"));
            auto fsrc = leerArchivo(fs::current_path() / (nombre + ".frag"));
            auto gsrc = leerArchivo(fs::current_path() / (nombre + ".geom"));

            // Creamos las shaders y las añadimos al programa
            if (vsrc) {
                vid = glCreateShader(GL_VERTEX_SHADER);
                compilarShader(vid, *vsrc);
                glAttachShader(pid, vid);
            } else {
                log::error("No se ha encontrado el shader de vértices: {}", nombre);
                std::exit(-1);
            }
            if (fsrc) {
                fid = glCreateShader(GL_FRAGMENT_SHADER);
                compilarShader(fid, *fsrc);
                glAttachShader(pid, fid);
            }
            if (gsrc) {
                gid = glCreateShader(GL_GEOMETRY_SHADER);
                compilarShader(gid, *gsrc);
                glAttachShader(pid, gid);
                if (transform_feedback_var.size() > 0) {
                    std::vector<const char*> tf_var_c;
                    std::transform(transform_feedback_var.begin(), transform_feedback_var.end(), std::back_inserter(tf_var_c), [](const str& s) { return s.c_str(); });
                    glTransformFeedbackVaryings(pid, tf_var_c.size(), tf_var_c.data(), GL_INTERLEAVED_ATTRIBS);
                }
            }

            // Vinculamos el programa
            glLinkProgram(pid);

            // Comprobamos que no haya errores
            int result, log_len;
            glGetProgramiv(pid, GL_LINK_STATUS, &result);
            glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
            if (log_len > 0) {
                str error(log_len, ' ');
                glGetProgramInfoLog(pid, log_len, NULL, &error[0]);
                log::warn("No se pudo vincular el programa: {}", error);
            }

            // Eliminamos los shaders ya que ya no los necesitamos
            if (vsrc) {
                glDetachShader(pid, vid);
                glDeleteShader(vid);
            }
            if (fsrc) {
                glDetachShader(pid, fid);
                glDeleteShader(fid);
            }
            if (gsrc) {
                glDetachShader(pid, gid);
                glDeleteShader(gid);
            }

            // Cargamos el programa por defecto
            glUseProgram(pid);

            debug::gl();
            return pid;
        }
    }

    namespace shader
    {
        // Cargar una shader en el programa
        inline void cargar(str nombre, str vao = "main", ui32 fbo = 0, OpcionesShader opt = {}, std::vector<str> transform_feedback_var = {}) {
            Shader s {
                .pid = detail::cargarShader("shaders/" + nombre, transform_feedback_var),
                .vao = vao,
                .fbo = fbo,
                .opt = opt
            };
            gl.shaders[nombre] = s;
            glUseProgram(0);
        }

        // Usar una shader
        inline void usar(str nombre = "") {
            if (gl.shader_actual == nombre)
                return;
            
            gl.shader_actual = nombre;
            if (nombre.empty()) {
                glUseProgram(0);
                return;
            }
            
            // Cambiamos la shader;
            if (not gl.shaders.count(nombre)) {
                log::error("No existe el shader especificado: {}", nombre);
                std::exit(-1);
            }
            Shader& s = gl.shaders[nombre];
            glUseProgram(s.pid);
            glBindVertexArray(gl.VAOs[s.vao].vao);
           
            // Cambiamos el framebuffer
            if (s.fbo == 0) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, gl.tam_fb.x, gl.tam_fb.y);
            } else {
                Framebuffer& fb = gl.framebuffers[s.fbo];
                glBindFramebuffer(GL_FRAMEBUFFER, s.fbo > 0 ? fb.fbo : 0);
                glViewport(0, 0, fb.tam.x, fb.tam.y);
            }

            // Parámetros extra de la shader
            (s.opt.blend) ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
            (s.opt.cull) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
            (s.opt.depth) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);

            debug::gl();
        }

        // Actualizar el valor de un uniform en la shader specificada
        template <typename T>
        void uniform(str nombre, T valor) {
            str& shader = gl.shader_actual;
            if (shader.empty()) {
                log::error("No se ha especificado una shader para actualizar el uniform: '{}'", nombre);
                std::exit(-1);
            }

            // Si no hemos registrado el uniform, obtenemos su localización
            if (not gl.shaders[shader].uniforms.count(nombre))
                gl.shaders[shader].uniforms[nombre] = glGetUniformLocation(gl.shaders[shader].pid, nombre.c_str());

            // Uniforms básicos
            if constexpr (std::is_same_v<T, int>) {
                glUniform1i(gl.shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, ui32>) {
                glUniform1ui(gl.shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, float>) {
                glUniform1f(gl.shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                glUniform2f(gl.shaders[shader].uniforms[nombre], valor.x, valor.y);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                glUniform3f(gl.shaders[shader].uniforms[nombre], valor.x, valor.y, valor.z);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                glUniform4f(gl.shaders[shader].uniforms[nombre], valor.x, valor.y, valor.z, valor.w);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                glUniformMatrix4fv(gl.shaders[shader].uniforms[nombre], 1, GL_FALSE, glm::value_ptr(valor));
            } 

            // Arrays de uniforms
            else if constexpr (std::is_same_v<T, std::vector<int>>) {
                glUniform1iv(gl.shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<ui32>>) {
                glUniform1uiv(gl.shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<float>>) {
                glUniform1fv(gl.shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) {
                glUniform2fv(gl.shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec3>>) {
                glUniform3fv(gl.shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec4>>) {
                glUniform4fv(gl.shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::ivec4>>) {
                glUniform4iv(gl.shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>) {
                glUniformMatrix4fv(gl.shaders[shader].uniforms[nombre], valor.size(), GL_FALSE, glm::value_ptr(valor[0]));
            } 

            // Texture buffers
            else if constexpr (std::is_same_v<T, TexBuffer>) {
                Buffer &buf = gl.buffers[valor.b];
                Textura &tex = gl.texturas[valor.t];
                if (tex.target != GL_TEXTURE_BUFFER) {
                    log::error("El uniform '{}' no es un texture buffer", nombre);
                    std::exit(-1);
                }
                glActiveTexture(GL_TEXTURE0 + valor.t);
                glBindTexture(GL_TEXTURE_BUFFER, tex.textura);
                glTexBuffer(GL_TEXTURE_BUFFER, tex.formato, buf.buffer);
                glUniform1i(gl.shaders[shader].uniforms[nombre], valor.t);
            } 

            // Tipo no soportado
            else {
                log::error("No se puede asignar el uniform '{}' en la shader '{}'", nombre, shader);
                std::exit(-1);
            }

            debug::gl();
        }
    }
}
