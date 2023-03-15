// Cargar y procesar shaders GLSL
#pragma once

#include <filesystem>
#include <fstream>

#include "debug.h"

namespace fs = std::filesystem;

namespace tofu
{
    namespace detail
    {
        inline str leerArchivo(fs::path path) {
            // Comprobamos que el directorio existe
            if (not fs::exists(path)) {
                log::error("No existe el archivo especificado: {}", path.string());
                std::exit(-1);
            }

            // Abrimos el fichero
            std::ifstream in(path.string());
            if (not in.is_open()) {
                log::error("No se ha podido abrir el fichero: {}", path.string());
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

        inline ui32 cargarShader(str vert, str frag) {
            // Creamos los shaders de vértice y fragmento
            ui32 vid = glCreateShader(GL_VERTEX_SHADER);
            ui32 fid = glCreateShader(GL_FRAGMENT_SHADER);

            // Leemos los ficheros de shaders
            str vsrc = leerArchivo(fs::current_path() / vert);
            str fsrc = leerArchivo(fs::current_path() / frag);

            // Compilamos los shaders
            compilarShader(vid, vsrc);
            compilarShader(fid, fsrc);

            // Vinculamos los shaders en un programa
            ui32 pid = glCreateProgram();
            glAttachShader(pid, vid);
            glAttachShader(pid, fid);
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
            glDetachShader(pid, vid);
            glDetachShader(pid, fid);
            glDeleteShader(vid);
            glDeleteShader(fid);

            // Cargamos el programa por defecto
            glUseProgram(pid);

            debug::gl();
            return pid;
        }
    }

    namespace shader
    {
        // Cargar una shader en el programa
        inline void cargar(str nombre, str vao = "main") {
            Shader s {
                .pid = detail::cargarShader(nombre + ".vert", nombre + ".frag"),
                .vao = vao
            };
            gl->shaders[nombre] = s;
            glUseProgram(0);
        }

        // Usar una shader
        inline void usar(str nombre = "") {
            if (gl->shader_actual == nombre)
                return;
            gl->instancia_base = 0;
            
            gl->shader_actual = nombre;
            if (nombre.empty()) {
                glUseProgram(0);
                return;
            }

            if (not gl->shaders.count(nombre)) {
                log::error("No existe el shader especificado: {}", nombre);
                std::exit(-1);
            }
            glUseProgram(gl->shaders[nombre].pid);
            glBindVertexArray(gl->VAOs[gl->shaders[nombre].vao].vao);

            debug::gl();
        }

        // Actualizar el valor de un uniform en la shader specificada
        template <typename T>
        void uniform(str nombre, T valor) {
            str& shader = gl->shader_actual;
            if (shader.empty()) {
                log::error("No se ha especificado una shader para actualizar el uniform: '{}'", nombre);
                std::exit(-1);
            }

            // Si no hemos registrado el uniform, obtenemos su localización
            if (not gl->shaders[shader].uniforms.count(nombre))
                gl->shaders[shader].uniforms[nombre] = glGetUniformLocation(gl->shaders[shader].pid, nombre.c_str());

            // Uniforms básicos
            if constexpr (std::is_same_v<T, int>) {
                glUniform1i(gl->shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, ui32>) {
                glUniform1ui(gl->shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, float>) {
                glUniform1f(gl->shaders[shader].uniforms[nombre], valor);
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                glUniform2f(gl->shaders[shader].uniforms[nombre], valor.x, valor.y);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                glUniform3f(gl->shaders[shader].uniforms[nombre], valor.x, valor.y, valor.z);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                glUniform4f(gl->shaders[shader].uniforms[nombre], valor.x, valor.y, valor.z, valor.w);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                glUniformMatrix4fv(gl->shaders[shader].uniforms[nombre], 1, GL_FALSE, glm::value_ptr(valor));
            } 

            // Arrays de uniforms
            else if constexpr (std::is_same_v<T, std::vector<int>>) {
                glUniform1iv(gl->shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<ui32>>) {
                glUniform1uiv(gl->shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<float>>) {
                glUniform1fv(gl->shaders[shader].uniforms[nombre], valor.size(), &valor[0]);
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) {
                glUniform2fv(gl->shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec3>>) {
                glUniform3fv(gl->shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec4>>) {
                glUniform4fv(gl->shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::ivec4>>) {
                glUniform4iv(gl->shaders[shader].uniforms[nombre], valor.size(), glm::value_ptr(valor[0]));
            } else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>) {
                glUniformMatrix4fv(gl->shaders[shader].uniforms[nombre], valor.size(), GL_FALSE, glm::value_ptr(valor[0]));
            } 

            // Texture buffers
            else if constexpr (std::is_same_v<T, TexBuffer>) {
                Buffer &buf = gl->buffers[valor.b];
                Textura &tex = gl->texturas[valor.t];
                if (tex.target != GL_TEXTURE_BUFFER) {
                    log::error("El uniform '{}' no es un texture buffer", nombre);
                    std::exit(-1);
                }
                glActiveTexture(GL_TEXTURE0 + valor.t);debug::gl();
                glBindTexture(GL_TEXTURE_BUFFER, tex.textura);debug::gl();
                glTexBuffer(GL_TEXTURE_BUFFER, tex.formato, buf.buffer);debug::gl();
                glUniform1i(gl->shaders[shader].uniforms[nombre], valor.t);debug::gl();
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
