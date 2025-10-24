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
     * @brief Maneja solicitud de movimiento del mouse
     * 
     * Espera JSON: {"x": 100, "y": 200}
     */
    static crow::response handleMouseMove(const crow::request& req);

    /**
     * @brief Maneja solicitud de click del mouse
     * 
     * Espera JSON: {"x": 100, "y": 200, "button": "left" o "right"}
     */
    static crow::response handleMouseClick(const crow::request& req);

    /**
     * @brief Maneja presión de una tecla individual
     * 
     * Espera JSON: {"keycode": 30}
     */
    static crow::response handleKeyPress(const crow::request& req);

    /**
     * @brief Maneja escritura de texto completo
     * 
     * Espera JSON: {"text": "hola mundo"}
     */
    static crow::response handleTypeText(const crow::request& req);

    /**
     * @brief Endpoint de salud del servidor
     */
    static crow::response handleHealth();
};

} // namespace Handlers

#endif // HTTP_HANDLER_H