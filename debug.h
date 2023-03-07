// Utilidades de depuración
#pragma once

#include <iostream>
#include <sstream>

#include "tipos.h"

namespace tofu {
    // Formatear strings

    namespace detail
    {
        template <typename T>
        inline str to_string(const T& a) {
            std::ostringstream ss;
            ss << a;
            return ss.str();
        }

        template <size_t N>
        inline void format_args(std::array<str, N>& fmt) {}

        template <size_t N, typename T, typename ... A>
        inline void format_args(std::array<str, N>& fmt, T&& arg, A&& ... other) {
            fmt[N - 1 - sizeof...(A)] = to_string(arg);
            format_args(fmt, std::forward<A>(other)...);
        }
    }

    template <typename ... A>
    str format(const str& fmt, A&& ... args) {
        // Formatear los argumentos
        std::array<str, sizeof...(A)> args_formateados;
        detail::format_args(args_formateados, std::forward<A>(args)...);

        // Variables necesarias
        std::ostringstream ss;
        bool en_formato = false;
        std::array<bool, sizeof...(A)> args_usados;
        std::fill(args_usados.begin(), args_usados.end(), false);

        // Recorremos el string de formato
        for (size_t i = 0; i < fmt.size(); i++) {
            // Abrimos un formato
            if (fmt[i] == '{') {
                en_formato = true;
                continue;
            }

            if (en_formato) {
                bool contiene_numero = false;
                for (size_t j = i; j < fmt.size(); j++) {
                    // Si se indica un número, intentamos añadir ese argumento
                    if (fmt[j] >= '0' && fmt[j] <= '9') {
                        size_t arg = fmt[j] - '0';
                        if (arg >= args_formateados.size())
                            throw std::runtime_error("El string de formato no está bien construído, hay un argumento que no existe");
                        if (args_usados[arg])
                            throw std::runtime_error("El string de formato no está bien construído, hay un argumento que se usa más de una vez");
                        args_usados[arg] = true;
                        ss << args_formateados[arg];
                        contiene_numero = true;
                    }

                    // Cerramos un formato y lo completamos si no se ha especificado un número
                    if (fmt[j] == '}') {
                        if (not contiene_numero) {
                            bool hay_argumento = false;
                            for (size_t k = 0; k < args_formateados.size(); k++) {
                                if (not args_usados[k]) {
                                    args_usados[k] = true;
                                    ss << args_formateados[k];
                                    hay_argumento = true;
                                    break;
                                }
                            }
                            if (not hay_argumento)
                                throw std::runtime_error("El string de formato no está bien construído, hay corchetes de más");
                        }
                        i = j;
                        en_formato = false;
                        break;
                    }
                }
            } else {
                // Si no estamos añadiendo una variable, ponemos la cadena original
                ss << fmt[i];
            }
        }
        // Comprobamos que no haya habido errores
        if (en_formato)
            throw std::runtime_error("El string de formato no está bien construído, hay un corchete sin cerrar");
        for (size_t i = 0; i < args_formateados.size(); i++)
            if (not args_usados[i])
                throw std::runtime_error("El string de formato no está bien construído, hay un argumento sin usar");
    
        return ss.str();
    }

    // ---

    namespace color
    {
        inline std::ostream& bold_on(std::ostream& os) { return os << "\e[1m"; }
        inline std::ostream& bold_off(std::ostream& os) { return os << "\e[0m"; }
        inline std::ostream& red(std::ostream& os) { return os << "\e[31m"; }
        inline std::ostream& green(std::ostream& os) { return os << "\e[32m"; }
        inline std::ostream& yellow(std::ostream& os) { return os << "\e[33m"; }
        inline std::ostream& blue(std::ostream& os) { return os << "\e[34m"; }
        inline std::ostream& reset(std::ostream& os) { return os << "\033[0m"; }
    }

    // ---

    namespace log
    {
        template <typename ... T>
        inline void info(const str& fmt, T&& ... args) { std::clog << color::bold_on << color::blue << "[INF]: " << color::reset << color::bold_off << format(fmt, std::forward<T>(args)...) << std::endl; }

        template <typename ... T>
        inline void warn(const str& fmt, T&& ... args) { std::clog << color::bold_on << color::yellow << "[WAR]: " << color::reset << color::bold_off << format(fmt, std::forward<T>(args)...) << std::endl; }

        template <typename ... T>
        inline void error(const str& fmt, T&& ... args) { std::cerr << color::bold_on << color::red << "[ERR]: " << color::reset << color::bold_off << format(fmt, std::forward<T>(args)...) << std::endl; }
    }
}
