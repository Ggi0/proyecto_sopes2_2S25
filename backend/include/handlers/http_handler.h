#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "crow/app.h"
#include "crow/json.h"

namespace Handlers {

/**
 * @brief Gestor de endpoints HTTP para control del mouse y teclado
 */
class HTTPHandler {
public:
    /**
     * @brief Maneja click del mouse (mueve + click)
     * 
     * Espera JSON: {"x": 100, "y": 200, "button": 1}  // 1=izquierdo, 2=derecho
     * REQUIERE: Autenticación y permisos de FULL_CONTROL
     */
    static crow::response handleMouseClick(const crow::request& req);

    /**
     * @brief Maneja presión de una tecla individual
     * 
     * Espera JSON: {"key": "a"} o {"keycode": 30}
     * REQUIERE: Autenticación y permisos de FULL_CONTROL
     */
    static crow::response handleKeyPress(const crow::request& req);

    /**
     * @brief Endpoint de salud del servidor
     * NO REQUIERE autenticación
     */
    static crow::response handleHealth();
};

} // namespace Handlers

#endif // HTTP_HANDLER_H