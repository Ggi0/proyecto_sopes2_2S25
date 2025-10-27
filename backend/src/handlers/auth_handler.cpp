#include "handlers/auth_handler.h"
#include "auth/pam_auth.h"
#include <iostream>

namespace Handlers {

crow::response AuthHandler::handleLogin(const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer credenciales
    if (!json_data.has("username") || !json_data.has("password")) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Missing username or password";
        return crow::response(400, response);
    }
    
    std::string username = json_data["username"].s();
    std::string password = json_data["password"].s();
    
    std::cout << " Intento de login: usuario '" << username << "'" << std::endl;
    
    // Autenticar con PAM
    if (!Auth::authenticateUser(username, password)) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid credentials";
        return crow::response(401, response);
    }
    
    // Obtener grupos del usuario
    std::vector<std::string> groups = Auth::getUserGroups(username);
    
    // Determinar nivel de acceso
    AccessLevel access_level = Auth::determineAccessLevel(groups);
    
    if (access_level == AccessLevel::NONE) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "User does not have remote access permissions";
        return crow::response(403, response);
    }
    
    // Generar token
    std::string token = Auth::generateToken(username);
    
    // Construir respuesta exitosa
    crow::json::wvalue response;
    response["success"] = true;
    response["username"] = username;
    response["access_level"] = accessLevelToString(access_level);
    response["token"] = token;
    
    // Agregar lista de grupos
    crow::json::wvalue::list groups_json;
    for (const auto& group : groups) {
        groups_json.push_back(group);
    }
    response["groups"] = std::move(groups_json);
    
    // Permisos específicos
    response["can_view"] = (access_level == AccessLevel::VIEW_ONLY || 
                           access_level == AccessLevel::FULL_CONTROL);
    response["can_control"] = (access_level == AccessLevel::FULL_CONTROL);
    
    std::cout << " Login exitoso para '" << username 
              << "' con nivel de acceso: " << accessLevelToString(access_level) << std::endl;
    
    return crow::response(200, response);
}

crow::response AuthHandler::handleLogout(const crow::request& req) {
    std::string token = extractToken(req);
    
    // En una implementación real, invalidarías el token aquí
    
    crow::json::wvalue response;
    response["success"] = true;
    response["message"] = "Logged out successfully";
    
    return crow::response(200, response);
}

bool AuthHandler::checkPermissions(const crow::request& req, AccessLevel required_level) {
    // Extraer token del header Authorization
    std::string token = extractToken(req);
    
    if (token.empty()) {
        return false;
    }
    
    // Validar token (implementación básica)
    // En producción, decodificarías el JWT y extraerías los claims
    
    // Por ahora, solo verificamos que el token exista
    // En producción real, decodificarías el JWT y verificarías el access_level
    
    return !token.empty();
}

std::string AuthHandler::extractToken(const crow::request& req) {
    auto auth_header = req.get_header_value("Authorization");
    
    if (auth_header.empty()) {
        return "";
    }
    
    // Formato esperado: "Bearer <token>"
    if (auth_header.substr(0, 7) == "Bearer ") {
        return auth_header.substr(7);
    }
    
    return "";
}

std::string AuthHandler::accessLevelToString(AccessLevel level) {
    switch (level) {
        case AccessLevel::NONE:
            return "none";
        case AccessLevel::VIEW_ONLY:
            return "view_only";
        case AccessLevel::FULL_CONTROL:
            return "full_control";
        default:
            return "unknown";
    }
}

} // namespace Handlers