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

    // ==================== ENDPOINTS DE AUTENTICACIÓN ====================
    
    // Login
    CROW_ROUTE(app, "/api/auth/login")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::AuthHandler::handleLogin(req);
    });
    
    // Logout
    CROW_ROUTE(app, "/api/auth/logout")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::AuthHandler::handleLogout(req);
    });
    
    // ==================== ENDPOINTS HTTP (CONTROL) ====================
    
    // Endpoint de salud (NO requiere auth)
    CROW_ROUTE(app, "/health")
    ([]() {
        return Handlers::HTTPHandler::handleHealth();
    });
    
    // Click del mouse (REQUIERE auth + FULL_CONTROL)
    CROW_ROUTE(app, "/api/mouse/click")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleMouseClick(req);
    });
    
    // Presionar tecla (REQUIERE auth + FULL_CONTROL)
    CROW_ROUTE(app, "/api/keyboard/press")
    .methods("POST"_method)
    ([](const crow::request& req) {
        return Handlers::HTTPHandler::handleKeyPress(req);
    });
    
    // ==================== WEBSOCKET (STREAMING) ====================
    
    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([](crow::websocket::connection& conn) {
            std::cout << " Cliente WebSocket conectado" << std::endl;
            // TODO: Verificar autenticación en WebSocket
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
    
    std::cout << "\n Endpoints disponibles:" << std::endl;
    
    std::cout << "\n Autenticación:" << std::endl;
    std::cout << "   POST /api/auth/login       - Iniciar sesión" << std::endl;
    std::cout << "   POST /api/auth/logout      - Cerrar sesión" << std::endl;
    
    std::cout << "\n Control (requiere autenticación):" << std::endl;
    std::cout << "   GET  /health               - Estado del servidor" << std::endl;
    std::cout << "   POST /api/mouse/click      - Click del mouse" << std::endl;
    std::cout << "   POST /api/keyboard/press   - Presionar tecla" << std::endl;
    
    std::cout << "\n Streaming (WebSocket):" << std::endl;
    std::cout << "   ws://0.0.0.0:" << Config::WEBSOCKET_PORT << "/ws" << std::endl;
    std::cout << "\n Servidor iniciado en puerto " << Config::WEBSOCKET_PORT << std::endl;
    
    // Iniciar servidor
    app.port(Config::WEBSOCKET_PORT)
       .multithreaded()
       .run();
    
    return 0;
}