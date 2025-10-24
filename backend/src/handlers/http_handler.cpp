#include "handlers/http_handler.h"
#include "syscalls/mouse_tracking.h"
#include "syscalls/mouse_action.h"
#include "syscalls/keyboard_caption.h"
#include <iostream>

namespace Handlers {

crow::response HTTPHandler::handleMouseMove(const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer coordenadas
    int x = json_data["x"].i();
    int y = json_data["y"].i();
    
    // Mover el mouse
    int result = Syscalls::moveMouse(x, y);
    
    crow::json::wvalue response;
    response["success"] = (result == 0);
    response["x"] = x;
    response["y"] = y;
    
    return crow::response(result == 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleMouseClick(const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer coordenadas y tipo de botón
    int x = json_data["x"].i();
    int y = json_data["y"].i();
    std::string button_str = json_data["button"].s();
    
    // Convertir string a código de botón
    int button = (button_str == "left") ? Syscalls::LEFT_CLICK : Syscalls::RIGHT_CLICK;
    
    // Realizar click en la posición
    int result = Syscalls::clickAt(x, y, button);
    
    crow::json::wvalue response;
    response["success"] = (result == 0);
    response["x"] = x;
    response["y"] = y;
    response["button"] = button_str;
    
    return crow::response(result == 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleKeyPress(const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer keycode
    int keycode = json_data["keycode"].i();
    
    // Presionar la tecla
    int result = Syscalls::pressKey(keycode);
    
    crow::json::wvalue response;
    response["success"] = (result == 0);
    response["keycode"] = keycode;
    
    return crow::response(result == 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleTypeText(const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    
    if (!json_data) {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"] = "Invalid JSON";
        return crow::response(400, response);
    }
    
    // Extraer texto
    std::string text = json_data["text"].s();
    
    // Escribir el texto
    int chars_typed = Syscalls::typeText(text);
    
    crow::json::wvalue response;
    response["success"] = (chars_typed > 0);
    response["text"] = text;
    response["chars_typed"] = chars_typed;
    
    return crow::response(chars_typed > 0 ? 200 : 500, response);
}

crow::response HTTPHandler::handleHealth() {
    crow::json::wvalue response;
    response["status"] = "healthy";
    response["service"] = "USAC Linux Remote Desktop API";
    response["version"] = "1.0.0";
    
    return crow::response(200, response);
}

} // namespace Handlers