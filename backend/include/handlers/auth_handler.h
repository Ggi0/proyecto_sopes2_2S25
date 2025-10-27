#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

#include "crow/app.h"
#include "crow/json.h"
#include "../types.h"

namespace Handlers {

class AuthHandler {
public:
    /**
     * @brief Maneja solicitud de login
     * 
     * Espera JSON: {"username": "admin", "password": "123"}
     * Retorna: {
     *   "success": true,
     *   "username": "admin",
     *   "groups": ["remote_control", "sudo"],
     *   "access_level": "full_control",
     *   "token": "abc123..."
     * }
     */
    static crow::response handleLogin(const crow::request& req);
    
    /**
     * @brief Maneja solicitud de logout
     */
    static crow::response handleLogout(const crow::request& req);
    
    /**
     * @brief Verifica si un usuario tiene permisos para una acción
     * 
     * @param req Request HTTP (debe contener header Authorization)
     * @param required_level Nivel de acceso requerido
     * @return true si tiene permisos, false en caso contrario
     */
    static bool checkPermissions(const crow::request& req, AccessLevel required_level);
    
    /**
     * @brief Extrae el token del header Authorization
     */
    static std::string extractToken(const crow::request& req);
    
    /**
     * @brief Convierte AccessLevel a string
     */
    static std::string accessLevelToString(AccessLevel level);
};

} // namespace Handlers

#endif // AUTH_HANDLER_H