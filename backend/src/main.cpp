#include "crow/app.h"
#include "crow/websocket.h"
#include "crow/middlewares/cors.h"
#include "handlers/websocket_handler.h"
#include "handlers/http_handler.h"
#include "handlers/auth_handler.h"
#include "types.h"
#include <iostream>
#include <csignal>
#include <memory>

// Handler global para WebSocket
std::unique_ptr<Handlers::WebSocketHandler> ws_handler;

// Manejador de señales para cierre limpio
void signalHandler(int signal) {
    std::cout << "\n Señal recibida: " << signal << std::endl;
    if (ws_handler) {
        ws_handler->stop();
    }
    exit(0);
}

int main() {
    // Configurar manejador de señales
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   USAC Linux Remote Desktop - Backend API     ║" << std::endl;
    std::cout << "║   Puerto: " << Config::WEBSOCKET_PORT << "                                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
    
    // Crear aplicación Crow con CORS
    crow::App<crow::CORSHandler> app;
    
    // Configurar CORS para permitir solicitudes desde cualquier origen
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("Content-Type", "Authorization")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method);
    
    // Crear handler de WebSocket
    ws_handler = std::make_unique<Handlers::WebSocketHandler>();
    
    // ==================== ENDPOINTS HTTP ====================
    
    // Endpoint de salud
    CROW_ROUTE(app, "/health")
    ([]() {
        return Handlers::HTTPHandler::handleHealth();
    });
    
    // Endpoint para mover mouse
    CROW_ROUTE(app, "/api/mouse/move")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleMouseMove(req);
    });
    
    // Endpoint para hacer click
    CROW_ROUTE(app, "/api/mouse/click")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleMouseClick(req);
    });
    
    // Endpoint para presionar una tecla
    CROW_ROUTE(app, "/api/keyboard/press")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleKeyPress(req);
    });
    
    // Endpoint para escribir texto
    CROW_ROUTE(app, "/api/keyboard/type")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleTypeText(req);
    });
    
    // ==================== WEBSOCKET ====================
    
CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([](crow::websocket::connection& conn) {
        std::cout << " Cliente WebSocket conectado" << std::endl;
        ws_handler->addConnection(conn);
    })
    .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        std::cout << " Cliente WebSocket desconectado (código " << code << "): " << reason << std::endl;
        ws_handler->removeConnection(conn);
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& message, bool /*is_binary*/) {
        ws_handler->handleMessage(conn, message);
    });

    
    // Iniciar el handler de WebSocket
    ws_handler->start();
    
    std::cout << "\n Endpoints HTTP disponibles:" << std::endl;
    std::cout << "   GET  /health               - Estado del servidor" << std::endl;
    std::cout << "   POST /api/mouse/move       - Mover mouse" << std::endl;
    std::cout << "   POST /api/mouse/click      - Click del mouse" << std::endl;
    std::cout << "   POST /api/keyboard/press   - Presionar tecla" << std::endl;
    std::cout << "   POST /api/keyboard/type    - Escribir texto" << std::endl;
    std::cout << "\n WebSocket:" << std::endl;
    std::cout << "   ws://localhost:" << Config::WEBSOCKET_PORT << "/ws" << std::endl;
    std::cout << "\n Servidor iniciado en puerto " << Config::WEBSOCKET_PORT << std::endl;
    std::cout << "   Presiona Ctrl+C para detener\n" << std::endl;
    
    // Iniciar servidor
    app.port(Config::WEBSOCKET_PORT)
       .multithreaded()
       .run();
    
    return 0;
}