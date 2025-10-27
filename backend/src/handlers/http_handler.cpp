#include "handlers/http_handler.h"
#include "handlers/auth_handler.h"
#include "syscalls/mouse_action.h"
#include "syscalls/keyboard_caption.h"
#include <iostream>

namespace Handlers {

crow::response HTTPHandler::handleMouseClick(const crow::request& req) {
    // Verificar autenticación y permisos
    if (!AuthHandler::checkPermissions(req, AccessLevel::FULL_CONTROL)) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Unauthorized: Full control access required";
        return crow::response(403, response);
    }
    
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer coordenadas y botón
    int x = json_data["x"].i();
    int y = json_data["y"].i();
    int button = json_data["button"].i();  // 1 o 2
    
    // Validar botón
    if (button != 1 && button != 2) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid button value (must be 1 or 2)";
        return crow::response(400, response);
    }
    
    // Realizar click en la posición (mueve + click)
    int result = Syscalls::clickAt(x, y, button);
    
    crow::json::wvalue response;
    response["success"] = (result == 0);
    response["x"] = x;
    response["y"] = y;
    response["button"] = button;
    
    return crow::response(result == 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleKeyPress(const crow::request& req) {
    // Verificar autenticación y permisos
    if (!AuthHandler::checkPermissions(req, AccessLevel::FULL_CONTROL)) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Unauthorized: Full control access required";
        return crow::response(403, response);
    }
    
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    int keycode = -1;
    
    // Aceptar tanto "key" (carácter) como "keycode" (número)
    if (json_data.has("keycode")) {
        keycode = json_data["keycode"].i();
    } else if (json_data.has("key")) {
        std::string key_str = json_data["key"].s();
        if (key_str.length() == 1) {
            keycode = Syscalls::charToKeycode(key_str[0]);
        }
    }
    
    if (keycode == -1) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid key or keycode";
        return crow::response(400, response);
    }
    
    // Presionar la tecla
    int result = Syscalls::pressKey(keycode);
    
    crow::json::wvalue response;
    response["success"] = (result == 0);
    response["keycode"] = keycode;
    
    return crow::response(result == 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleHealth() {
    crow::json::wvalue response;
    response["status"] = "healthy";
    response["service"] = "USAC Linux Remote Desktop API";
    response["version"] = "1.0.0";
    
    return crow::response(200, response);
}

} // namespace Handlers